.text
main:
    lui t0, 0x1E8     #t0 = 0x001E8000
    addi t0, t0, 0x480 # t0 = 0x001E8480

loop:
    addi t0, t0, -1 # t0 -=1
    bne t0, x0, loop # t0 != 0 j -> loop
    addi a7, x0, 10
    ecall
# wrap up countdown2m.s (N = 2m)
#loop body: x instruction/iteration (k = 2)
#loop ooutside: 4 istructions (c=4)
#expected retired instruction = 4,000,004