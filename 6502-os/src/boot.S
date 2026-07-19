; Minimal cc65 startup for the breadboard W65C02S computer.

        .setcpu "65C02"

        .export __STARTUP__ : absolute = 1
        .export _exit

        .import __start
        .import __STACKSTART__
        .import __DATA_SIZE__

; Writable initialized globals/statics are forbidden.
        .assert __DATA_SIZE__ = 0, lderror, "writable initialized globals/statics are forbidden"

; cc65's ABI expects these zero-page symbols for it's calling covention.
        .exportzp sp, sreg, regsave
        .exportzp ptr1, ptr2, ptr3, ptr4
        .exportzp tmp1, tmp2, tmp3, tmp4, regbank

        .segment "ZEROPAGE"

sp:         .res 2
sreg:       .res 2
regsave:    .res 4
ptr1:       .res 2
ptr2:       .res 2
ptr3:       .res 2
ptr4:       .res 2
tmp1:       .res 1
tmp2:       .res 1
tmp3:       .res 1
tmp4:       .res 1
regbank:    .res 6

        .segment "STARTUP"

reset:
        sei                     ; No interrupts during C runtime setup.
        cld                     ; C code assumes binary arithmetic.
        ldx #$ff
        txs                     ; Hardware stack occupies $0100-$01ff.

        ; cc65 uses a separate software stack for arguments and locals.
        lda #<__STACKSTART__
        sta sp
        lda #>__STACKSTART__
        sta sp+1

        jsr __start

_exit:
halt:
        bra halt                ; main() is not expected to return.

        .segment "CODE"

nmi:
        rti

irq:
        rti

        .segment "VECTORS"
        .word nmi
        .word reset
        .word irq
