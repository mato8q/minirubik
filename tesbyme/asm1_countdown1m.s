.text
main:
    lui t0, 0xF4    #t0 = 0x000F4000
    addi t0, t0, 0x240 # t0 = 0x000F4240

loop:
    addi t0, t0, -1 # t0 -=1
    bne t0, x0, loop # t0 != 0 j -> loop
    addi a7, x0, 10
    ecall
# wrap up countdown1m.s (N = 1m)
#loop body: x instruction/iteration (k = 2)
#loop ooutside: 4 istructions (c=4)
#expected retired instruction = 2,000,004