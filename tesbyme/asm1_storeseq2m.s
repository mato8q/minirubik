.text
main:
    lui t0, 0x1E8     #t0 = 0x001E8000
    addi t0, t0, 0x480 # t0 = 0x001E8480
    lui t1, 0x10000 # initialize t1

loop:
    addi t0, t0, -1 # t0 -=1
    sw t0, 0(t1) #store address
    addi t1, t1, 4 # t1 = t1+4
    bne t0, x0, loop # t0 != 0 j -> loop
    addi a7, x0, 10
    ecall
# k = 4 (addi, sw,addi, bne)
# c = 5 (lui, addi, lui, addi a7, ecall)
#expected--iret =(4*2m)+5= 8,000,005