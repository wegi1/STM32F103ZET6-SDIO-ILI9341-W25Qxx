
    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

    .thumb_func
    .global my_htoa32
    .type   my_htoa32, %function

    .text
//----------------

//======================================================
// HEX 32BIT value convert to string
//====================================
       // INPUT:
       //    R0, =  ; POINTER TO OUTPUT char DATA BUFFER
       //    R1, =  ; VALUE TO HEXADECIMAL CONVERT

       // output: no output, data in buffer are output
//=======================================================
//=========================
my_htoa32:

        MOV    R3, 0x7830     // "0x"
        STRH   R3, [R0]       // STORE "0x" TO BUFFER
        MOVS   R3, 2          // output data 2 bytes after start buffer
//--
LP01:
        LSRS   R2, R1, #28    // R2 = OUR DATA TO STRING CONVERT 1 BYTE = NYBBLE 0..F
        LSLS   R1, R1, #4     // R1 * 16 = R1 << 4
        ADDS   R2, R2, #0x30  // CONVERT TO NUMBER
        CMP    R2, #0x3A
        BCC    LP01A          // if data < than 0x3a don't  add #7
        ADDS   R2,R2, 0x07    // CONVER TO LETTER
LP01A:
        STRH   R2, [R0, R3]   // STRH store byte and zero terminated
        ADDS   R3, R3, #1
        CMP    R3, #10
        BNE    LP01
//---
        BX     LR
//=============================================================
