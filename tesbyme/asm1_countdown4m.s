.text
main:
    lui t0, 0x3D1    #t0 = 0x003D1000  (not 0x3D0)
    addi t0, t0, -1792 # t0 = 0x003D0900 = 4,000,000

loop:
    addi t0, t0, -1 # t0 -=1
    bne t0, x0, loop # t0 != 0 j -> loop
    addi a7, x0, 10
    ecall
# wrap up countdown4m.s (N = 4m)
#loop body: 2 instruction/iteration (k = 2)
#loop ooutside: 4 istructions (c=4)
#expected retired instruction = 8,000,004