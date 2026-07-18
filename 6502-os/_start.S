    .org $8000

reset:
    lda #$55
    sta $1002
    lda #$A9
    sta $0067
    lda $1002
    sta $0067
    jmp reset

    .org $FFFC
    .word reset
    .word $0000