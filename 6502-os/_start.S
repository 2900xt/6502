    .org $8000

reset:
    lda #$FF
    sta $6002
    lda #$55
    sta $6000
.loop:
    jmp .loop

memtest:
    lda #$55
    sta $3FFF
    lda #$AA
    sta $5FFF
    lda $3FFF
    sta $0001
    lda $5FFF 
    sta $0000
    jmp reset

    .org $FFFC
    .word reset
    .word $0000