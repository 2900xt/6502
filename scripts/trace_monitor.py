#!/usr/bin/env python3
"""Symbol-aware serial trace viewer for the Arduino W65C02S monitor."""

from __future__ import annotations

import argparse
import bisect
import re
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, TextIO

try:
    import serial
except ImportError:  # Allow parsing captured traces without pyserial installed.
    serial = None


BUS_LINE = re.compile(
    r"^\s*(?:0x)?([0-9a-f]{4})\s+(?:0x)?([0-9a-f]{2})"
    r"\s+([rRwW])(?:\s+([A-Za-z]*))?\s*$",
    re.IGNORECASE,
)
LABEL_LINE = re.compile(r"^\s*al\s+([0-9a-fA-F]{6,8})\s+\.?(\S+)\s*$")


# W65C02 opcode names. Undefined/reserved opcodes are deliberately shown as NOP.
OPCODE_NAMES = tuple(
    """BRK ORA NOP NOP TSB ORA ASL RMB0 PHP ORA ASL NOP TSB ORA ASL BBR0
    BPL ORA ORA NOP TRB ORA ASL RMB1 CLC ORA INC NOP TRB ORA ASL BBR1
    JSR AND NOP NOP BIT AND ROL RMB2 PLP AND ROL NOP BIT AND ROL BBR2
    BMI AND AND NOP BIT AND ROL RMB3 SEC AND DEC NOP BIT AND ROL BBR3
    RTI EOR NOP NOP NOP EOR LSR RMB4 PHA EOR LSR NOP JMP EOR LSR BBR4
    BVC EOR EOR NOP NOP EOR LSR RMB5 CLI EOR PHY NOP NOP EOR LSR BBR5
    RTS ADC NOP NOP STZ ADC ROR RMB6 PLA ADC ROR NOP JMP ADC ROR BBR6
    BVS ADC ADC NOP STZ ADC ROR RMB7 SEI ADC PLY NOP JMP ADC ROR BBR7
    BRA STA NOP NOP STY STA STX SMB0 DEY BIT TXA NOP STY STA STX BBS0
    BCC STA STA NOP STY STA STX SMB1 TYA STA TXS NOP STZ STA STZ BBS1
    LDY LDA LDX NOP LDY LDA LDX SMB2 TAY LDA TAX NOP LDY LDA LDX BBS2
    BCS LDA LDA NOP LDY LDA LDX SMB3 CLV LDA TSX NOP LDY LDA LDX BBS3
    CPY CMP NOP NOP CPY CMP DEC SMB4 INY CMP DEX WAI CPY CMP DEC BBS4
    BNE CMP CMP NOP NOP CMP DEC SMB5 CLD CMP PHX STP NOP CMP DEC BBS5
    CPX SBC NOP NOP CPX SBC INC SMB6 INX SBC NOP NOP CPX SBC INC BBS6
    BEQ SBC SBC NOP NOP SBC INC SMB7 SED SBC PLX NOP NOP SBC INC BBS7""".split()
)

THREE_BYTE = {
    0x0C, 0x0D, 0x0E, 0x0F, 0x19, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x2C, 0x2D, 0x2E, 0x2F, 0x39, 0x3C, 0x3D, 0x3E, 0x3F,
    0x4C, 0x4D, 0x4E, 0x4F, 0x59, 0x5D, 0x5E, 0x5F,
    0x6C, 0x6D, 0x6E, 0x6F, 0x79, 0x7C, 0x7D, 0x7E, 0x7F,
    0x8C, 0x8D, 0x8E, 0x8F, 0x99, 0x9C, 0x9D, 0x9E, 0x9F,
    0xAC, 0xAD, 0xAE, 0xAF, 0xB9, 0xBC, 0xBD, 0xBE, 0xBF,
    0xCC, 0xCD, 0xCE, 0xCF, 0xD9, 0xDD, 0xDE, 0xDF,
    0xEC, 0xED, 0xEE, 0xEF, 0xF9, 0xFD, 0xFE, 0xFF,
    0x5C, 0xDC, 0xFC,  # Three-byte WDC NOP encodings.
}
ONE_BYTE = {
    0x00, 0x08, 0x0A, 0x18, 0x1A, 0x28, 0x2A, 0x38, 0x3A, 0x40, 0x48,
    0x4A, 0x58, 0x5A, 0x60, 0x68, 0x6A, 0x78, 0x7A, 0x88, 0x8A, 0x98,
    0x9A, 0xA8, 0xAA, 0xB8, 0xBA, 0xC8, 0xCA, 0xCB, 0xD8, 0xDA, 0xDB,
    0xE8, 0xEA, 0xF8, 0xFA,
    # Single-byte reserved NOP encodings.
    0x03, 0x0B, 0x13, 0x1B, 0x23, 0x2B, 0x33, 0x3B,
    0x43, 0x4B, 0x53, 0x5B, 0x63, 0x6B, 0x73, 0x7B,
    0x83, 0x8B, 0x93, 0x9B, 0xA3, 0xAB, 0xB3, 0xBB,
    0xC3, 0xD3, 0xE3, 0xEB, 0xF3, 0xFB,
}

IMMEDIATE = {
    0x09, 0x29, 0x49, 0x69, 0x89, 0xA0, 0xA2, 0xA9, 0xC0, 0xC9, 0xE0, 0xE9,
}
RELATIVE = {0x10, 0x30, 0x50, 0x70, 0x80, 0x90, 0xB0, 0xD0, 0xF0}
BIT_BRANCH = {opcode for opcode in range(0x0F, 0x100, 0x10)}
ABSOLUTE_TARGET = {0x20, 0x4C}

BUILTIN_SYMBOLS = {
    0x6000: "VIA_ORB",
    0x6001: "VIA_ORA",
    0x6002: "VIA_DDRB",
    0x6003: "VIA_DDRA",
    0x6004: "VIA_T1CL",
    0x6005: "VIA_T1CH",
    0x6006: "VIA_T1LL",
    0x6007: "VIA_T1LH",
    0x6008: "VIA_T2CL",
    0x6009: "VIA_T2CH",
    0x600A: "VIA_SR",
    0x600B: "VIA_ACR",
    0x600C: "VIA_PCR",
    0x600D: "VIA_IFR",
    0x600E: "VIA_IER",
    0x600F: "VIA_ORA_NO_HANDSHAKE",
}


def instruction_length(opcode: int) -> int:
    if opcode in ONE_BYTE:
        return 1
    if opcode in THREE_BYTE:
        return 3
    return 2


@dataclass(frozen=True)
class BusCycle:
    address: int
    data: int
    read: bool
    flags: str
    raw: str

    @property
    def sync(self) -> bool:
        return "S" in self.flags.upper()


@dataclass
class InstructionTrace:
    pc: int
    opcode: int
    cycles: list[BusCycle] = field(default_factory=list)

    def operands(self) -> list[int | None]:
        result: list[int | None] = []
        for offset in range(1, instruction_length(self.opcode)):
            address = (self.pc + offset) & 0xFFFF
            cycle = next(
                (c for c in self.cycles if c.read and c.address == address), None
            )
            result.append(cycle.data if cycle else None)
        return result


class SymbolTable:
    def __init__(self) -> None:
        self.by_address: dict[int, list[str]] = {}
        for address, name in BUILTIN_SYMBOLS.items():
            self.add(address, name)
        self._addresses: list[int] = []

    def add(self, address: int, name: str) -> None:
        names = self.by_address.setdefault(address & 0xFFFF, [])
        if name not in names:
            names.append(name)

    def load(self, filename: Path) -> None:
        with filename.open(encoding="utf-8", errors="replace") as labels:
            for line in labels:
                match = LABEL_LINE.match(line)
                if match:
                    self.add(int(match.group(1), 16), match.group(2).lstrip("."))

    def finish(self) -> None:
        self._addresses = sorted(self.by_address)

    def exact(self, address: int) -> str | None:
        names = self.by_address.get(address & 0xFFFF)
        if not names:
            return None
        # Prefer human source labels over linker-generated metadata and locals.
        return min(names, key=lambda n: (n.startswith("__"), n.startswith("L"), len(n)))

    def describe(self, address: int) -> str:
        address &= 0xFFFF
        exact = self.exact(address)
        if exact:
            return exact

        index = bisect.bisect_right(self._addresses, address) - 1
        if index >= 0:
            base = self._addresses[index]
            # A small limit avoids calling unrelated regions huge symbol offsets.
            if address - base <= 0x40:
                return f"{self.exact(base)}+${address - base:X}"

        if 0x0100 <= address <= 0x01FF:
            return f"hardware_stack+${address - 0x0100:02X}"
        if 0x0000 <= address <= 0x00FF:
            return f"zero_page+${address:02X}"
        if 0x0200 <= address <= 0x5FFF:
            return f"ram+${address - 0x0200:04X}"
        if 0x8000 <= address <= 0xFFFF:
            return f"eeprom+${address - 0x8000:04X}"
        return f"${address:04X}"

    def containing_name(self, address: int) -> str | None:
        """Return a nearby source-level label suitable for a stack frame."""
        address &= 0xFFFF
        index = bisect.bisect_right(self._addresses, address) - 1
        while index >= 0 and address - self._addresses[index] <= 0x100:
            names = self.by_address[self._addresses[index]]
            candidates = [
                name for name in names
                if not name.startswith("__") and not name.startswith("L")
            ]
            if candidates:
                return min(candidates, key=len)
            index -= 1
        return None


class TraceParser:
    def __init__(self) -> None:
        self.current: InstructionTrace | None = None

    @staticmethod
    def parse_cycle(line: str) -> BusCycle | None:
        match = BUS_LINE.match(line)
        if not match:
            return None
        return BusCycle(
            address=int(match.group(1), 16),
            data=int(match.group(2), 16),
            read=match.group(3).lower() == "r",
            flags=match.group(4) or "",
            raw=line.rstrip("\r\n"),
        )

    def feed(self, line: str) -> tuple[InstructionTrace | None, BusCycle | None]:
        cycle = self.parse_cycle(line)
        if cycle is None:
            return None, None

        completed = None
        if cycle.sync and cycle.read:
            completed = self.current
            self.current = InstructionTrace(cycle.address, cycle.data, [cycle])
        elif self.current is not None:
            self.current.cycles.append(cycle)
        return completed, cycle

    def flush(self) -> InstructionTrace | None:
        completed, self.current = self.current, None
        return completed


class TraceAnalyzer:
    def __init__(self, symbols: SymbolTable, output: TextIO = sys.stdout) -> None:
        self.symbols = symbols
        self.output = output
        self.stack: list[str] = []
        self.interrupt_frames: list[int] = []

    def reset(self) -> None:
        self.stack.clear()
        self.interrupt_frames.clear()

    @staticmethod
    def _interrupt_target(trace: InstructionTrace) -> int | None:
        vector_bytes = {
            cycle.address: cycle.data
            for cycle in trace.cycles
            if cycle.read and "V" in cycle.flags.upper()
        }
        for vector in (0xFFFA, 0xFFFE):
            if vector in vector_bytes and vector + 1 in vector_bytes:
                return vector_bytes[vector] | (vector_bytes[vector + 1] << 8)
        return None

    def _target(self, trace: InstructionTrace) -> int | None:
        operands = trace.operands()
        if trace.opcode in ABSOLUTE_TARGET and len(operands) == 2 and None not in operands:
            return int(operands[0]) | (int(operands[1]) << 8)
        if trace.opcode in RELATIVE and operands and operands[0] is not None:
            displacement = int(operands[0])
            if displacement >= 0x80:
                displacement -= 0x100
            return (trace.pc + 2 + displacement) & 0xFFFF
        if trace.opcode in BIT_BRANCH and len(operands) == 2 and operands[1] is not None:
            displacement = int(operands[1])
            if displacement >= 0x80:
                displacement -= 0x100
            return (trace.pc + 3 + displacement) & 0xFFFF
        return None

    def _disassemble(self, trace: InstructionTrace) -> str:
        mnemonic = OPCODE_NAMES[trace.opcode]
        operands = trace.operands()
        missing = "??"
        if not operands:
            return mnemonic
        if trace.opcode in IMMEDIATE:
            value = missing if operands[0] is None else f"{operands[0]:02X}"
            return f"{mnemonic} #${value}"
        if trace.opcode in RELATIVE:
            target = self._target(trace)
            return f"{mnemonic} {self.symbols.describe(target) if target is not None else '$????'}"
        if trace.opcode in BIT_BRANCH:
            zero_page = (
                "$??" if operands[0] is None else self.symbols.describe(int(operands[0]))
            )
            target = self._target(trace)
            destination = self.symbols.describe(target) if target is not None else "$????"
            return f"{mnemonic} {zero_page}, {destination}"
        if len(operands) == 2:
            if None in operands:
                return f"{mnemonic} $????"
            address = int(operands[0]) | (int(operands[1]) << 8)
            return f"{mnemonic} {self.symbols.describe(address)} (${address:04X})"
        value = missing if operands[0] is None else f"{operands[0]:02X}"
        return f"{mnemonic} ${value}"

    def render(self, trace: InstructionTrace) -> None:
        if not self.stack:
            self.stack.append(
                self.symbols.containing_name(trace.pc) or self.symbols.describe(trace.pc)
            )

        stack_text = " > ".join(self.stack)
        disassembly = self._disassemble(trace)
        for index, cycle in enumerate(trace.cycles):
            operation = "read" if cycle.read else "WRITE"
            annotation = f"{operation} {self.symbols.describe(cycle.address)}"
            if not cycle.read:
                annotation += f" <= ${cycle.data:02X}"
            columns = [f"{cycle.raw:<25}", annotation]
            if index == 0:
                columns.extend((f"insn: {disassembly}", f"stack: {stack_text}"))
            print(" | ".join(columns), file=self.output)

        # Apply control-flow effects after printing, ready for the next opcode fetch.
        if trace.opcode == 0x20:
            target = self._target(trace)
            self.stack.append(
                self.symbols.exact(target) or self.symbols.describe(target)
                if target is not None
                else "<?>"
            )
        elif trace.opcode == 0x60 and len(self.stack) > 1:  # RTS
            self.stack.pop()

        if trace.opcode == 0x40 and self.interrupt_frames:  # RTI
            if self.interrupt_frames[-1] == len(self.stack):
                self.stack.pop()
            self.interrupt_frames.pop()

        interrupt_target = self._interrupt_target(trace)
        if interrupt_target is not None:
            self.stack.append(
                self.symbols.exact(interrupt_target)
                or self.symbols.describe(interrupt_target)
            )
            self.interrupt_frames.append(len(self.stack))


def parse_symbol_argument(value: str) -> tuple[int, str]:
    try:
        name, address_text = value.split("=", 1)
        address = int(address_text.removeprefix("$").removeprefix("0x"), 16)
    except (ValueError, AttributeError) as error:
        raise argparse.ArgumentTypeError("symbols must have the form NAME=ADDRESS") from error
    if not name or not 0 <= address <= 0xFFFF:
        raise argparse.ArgumentTypeError("symbol address must be between 0000 and FFFF")
    return address, name


def analyze_lines(lines: Iterable[str], analyzer: TraceAnalyzer) -> None:
    parser = TraceParser()
    try:
        for line in lines:
            if "RESET" in line.upper():
                pending = parser.flush()
                if pending:
                    analyzer.render(pending)
                analyzer.reset()
                print(line.rstrip("\r\n"), file=analyzer.output)
                continue

            completed, cycle = parser.feed(line)
            if completed:
                analyzer.render(completed)
            if cycle is None and line.strip():
                print(line.rstrip("\r\n"), file=analyzer.output)
    finally:
        pending = parser.flush()
        if pending:
            analyzer.render(pending)


def serial_lines(port: str, baud: int, select_monitor: bool, startup_delay: float):
    if serial is None:
        raise RuntimeError("pyserial is required for live serial monitoring")
    with serial.Serial(port=port, baudrate=baud, timeout=0.25) as connection:
        print(f"Connected to {port} at {baud} baud", file=sys.stderr)
        if select_monitor:
            time.sleep(startup_delay)
            connection.write(b"0")
            connection.flush()
        while True:
            raw = connection.readline()
            if raw:
                yield raw.decode("utf-8", errors="replace")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Annotate W65C02S monitor bus traces with opcodes, symbols, and a call stack."
    )
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--input", type=Path, help="parse a captured trace instead of serial")
    source.add_argument("--stdin", action="store_true", help="parse a captured trace from stdin")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="monitor serial port")
    parser.add_argument("--baud", type=int, default=115200, help="serial baud rate")
    parser.add_argument(
        "--labels", type=Path, default=Path("6502-os/build/OS.lbl"), help="cc65 .lbl file"
    )
    parser.add_argument(
        "--symbol", action="append", default=[], type=parse_symbol_argument,
        metavar="NAME=ADDR", help="add a symbol (repeatable)",
    )
    parser.add_argument(
        "--no-select", action="store_true",
        help="do not send '0' to select the W65C02S monitor after connecting",
    )
    parser.add_argument(
        "--startup-delay", type=float, default=2.0,
        help="seconds to wait before selecting the monitor (default: 2)",
    )
    args = parser.parse_args()

    symbols = SymbolTable()
    if args.labels:
        try:
            symbols.load(args.labels)
        except OSError as error:
            parser.error(f"cannot read label file {args.labels}: {error}")
    for address, name in args.symbol:
        symbols.add(address, name)
    symbols.finish()
    analyzer = TraceAnalyzer(symbols)

    try:
        if args.input:
            with args.input.open(encoding="utf-8", errors="replace") as trace:
                analyze_lines(trace, analyzer)
        elif args.stdin:
            analyze_lines(sys.stdin, analyzer)
        else:
            analyze_lines(
                serial_lines(args.port, args.baud, not args.no_select, args.startup_delay),
                analyzer,
            )
    except KeyboardInterrupt:
        print("\nTrace stopped.", file=sys.stderr)
    except (OSError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
