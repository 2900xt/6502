    .org $8000

reset:
    lda #$FF
    sta $6002
    jmp reset

    .org $FFFC
    .word reset
    .word $0000