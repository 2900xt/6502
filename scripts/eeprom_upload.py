#!/usr/bin/env python

import argparse
import time

import serial
from tqdm import tqdm


def main():
    parser = argparse.ArgumentParser(
        description="at28c256 eeprom programmer using ATMEGA 6502 console"
    )
    parser.add_argument(
        "--filename", type=str, help="path to .BIN file to upload", default=""
    )
    parser.add_argument(
        "--port",
        type=str,
        default="/dev/ttyUSB0",
        help="serial port running ATMEGA 6502 console",
    )
    parser.add_argument("--baud", type=int, default=115200, help="baud rate to use")
    parser.add_argument(
        "--byte", type=str, default="", help="write a single byte to the file"
    )

    args = parser.parse_args()

    print("--- UPLOAD to AT28C256 ---")

    try:
        ser = serial.Serial(port=args.port, baudrate=args.baud, timeout=1)
    except:
        print(f"ERROR: cannot connect to '{args.port}'")
        exit(0)

    print(f"Connected to '{args.port}' @ {args.baud}")

    # initialize the reader
    try:
        resp = ser.read_until(b">")
        assert resp.endswith(b">")

        ser.write(b"1")

        time.sleep(1)
        resp = ser.read_until(b">")
        assert resp.endswith(b">") and (b"OK" in resp)
    except:
        print("ERROR: chip init failed")
        exit(0)

    if args.filename != "":
        # read raw binary file
        try:
            with open(args.filename, "rb") as file:
                bin = file.read()
        except:
            print(f"ERROR: file '{args.filename}' read error")
            exit(0)

        # assert file length = 32k
        if len(bin) != 0x8000:
            print(f"ERROR: file '{args.filename}' is not 32K bytes")
            exit(0)
    elif args.byte != "":
        bin = bytes.fromhex(args.byte) * 0x8000
    else:
        print("ERROR: NO INPUT FILE SPECIFIED")
        exit(0)

    try:
        with tqdm(total=0x8000, unit="B", unit_scale=True, desc="Uploading") as pbar:
            for page_start in range(0, len(bin), 64):
                ser.write(f"P {page_start:04X}\n".encode())
                ser.write(bin[page_start : page_start + 64])

                ser.write(b"OK")
                response = ser.read_until(b">")
                assert response.endswith(b">") and (b"OK" in response)
                pbar.update(64)
    except:
        print("ERROR: bad response: ", response)
        exit(0)

    print("------------------ UPLOAD COMPLETE --------------------")


if __name__ == "__main__":
    main()
