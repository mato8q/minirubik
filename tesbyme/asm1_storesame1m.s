.text
main:
    lui t0, 0xF4    #t0 = 0x000F4000
    addi t0, t0, 0x240 # t0 = 0x000F4240
    lui t1, 0x10000 # initialize t1

loop:
    addi t0, t0, -1 # t0 -=1
    sw t0, 0(t1)    #store addr
    bne t0, x0, loop # t0 != 0 j -> loop
    addi a7, x0, 10
    ecall
    # k = 4 (addi, sw,addi, bne)
    # c = 5 (lui, addi, lui, addi a7, ecall)
    #expected--iret =(4*1m)+5= 4,000,005