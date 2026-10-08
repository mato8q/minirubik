.text
  la s1, h_O #p
  la s2, h_N  
  la s3, permutation
  la s4, orientation
  la s5, cub_move
  la s6, perm_off
  la s7, ori_off
  la s8, cub_off
  la s0, work
  la a5, col_face
  la a6, col_turn  

# root stores on page[0]
  li t1, 720 #p
  li t2, 0  #o
  li t3, 4  #a cubic 0 at 1, rotate 0-> 1<<2 = 4
  li t4, 0  #b cubic 1 at 0, rotate 0
  sh t1, 0(s0)           # page_p[0] = p
  sh t2, 24(s0)          # page_o[0] = o
  sb t3, 48(s0)          # page_a[0] = a
  sb t4, 60(s0)          # page_b[0] = b
  li s11, 0              # check = 0

# h_root
  jal ra, heur # t5 = h(root)
  li a0, 0  # answer=0
  beqz t5,finish  #h_root ==0 solved ->0
  mv  s9, t5 # bound = h_root

# bound
bound_loop:
  li t0, 11
  bgtu s9, t0, not_found #bound > 11 -> not seen
  li s10, 0  # dept = 0
  sb zero, 72(s0) # next_move[0] =0

# dfs  
dfs_loop:
  bltz s10, next_bound   # depth < 0 -> try next bound 
  add t6, s0, s10        # t6 = s0 + depth
  lbu t0, 72(t6)         # m = next_move[depth]
  li t5, 9
  beq t0, t5, backtrack  # try all 9 moves-> move back
  addi t5, t0, 1
  sb t5, 72(t6)          # next_move[depth]++

  add t5, a5, t0
  lbu a0, 0(t5)          # a0 = face = col_face[m]
  add t5, a6, t0
  lbu a1, 0(t5)          # a1 = turn = col_turn[m]

  beqz s10, gen          # depth == 0 ; dont need 2 check redundant page
  lbu t5, 83(t6)         # move_dis[depth-1]
  add t5, a5, t5
  lbu t5, 0(t5)          # col_face[previous page]
  beq a0, t5, dfs_loop   # same page -> skip

gen:
  slli t5, s10, 1        # depth*2 (.half)
  add t5, s0, t5
  lhu t1, 0(t5)          # p = page_p[depth]
  lhu t2, 24(t5)         # o = page_o[depth]
  lbu t3, 48(t6)         # a = page_a[depth]
  lbu t4, 60(t6)         # b = page_b[depth]
  
  slli t0, a0, 2  #t0 = face*4
  add t5, s6, t0 # t5 =&perm_off[face]
  lw t5, 0(t5)  # t5 =face*10080
  add a2, s3, t5 # a2 =&permutation[face][0]
  
  add t5, s7, t0 # t5 =&ori_off[face]
  lw t5, 0(t5)    # t5 = face*1458
  add a3, s4, t5  #a3 =&orientation[face][0]

  add t5, s8, t0 # t5 =s81 +0t2 = &cub_off[face]
  lw t5, 0(t5)    # t5 = face*28
  add a4, s5, t5  #a4 = &cub_move[face][0]

turn_loop:
  slli t0, t1, 1        # p*2 (.half)
  add  t0, a2, t0
  lhu  t1, 0(t0)        # p= permutation[face][p]

  slli t0, t2, 1        # o*2 (.half)
  add  t0, a3, t0
  lhu  t2, 0(t0)        # o = orientation[face][o]

  add  t0, a4, t3       # a × 1  (.byte no shift)
  lbu  t3, 0(t0)        # a = cub_move[face][a]

  add  t0, a4, t4
  lbu  t4, 0(t0)        # b = cub_move[face][b]

  addi a1, a1, -1       # turn--
  bnez a1, turn_loop    # still left -> loop
  addi s11, s11, 1   # check++
  jal ra, heur        #t5=h(child)

# depth+1+h <= bound 
  addi t0, s10, 1
  add  t0, t0, t5
  bgtu t0, s9, dfs_loop  # exceeded bound-> leave

  # get child
  add  t6, s0, s10       # if t6 got used by heur -> leave it
  lbu  t0, 72(t6)
  addi t0, t0, -1        # m = next_move[depth] - 1
  sb   t0, 84(t6)        # move_dis[depth] = m
  beqz t5, found         # h == 0 -> solved already

  addi s10, s10, 1       # depth++
  slli t0, s10, 1
  add  t0, s0, t0
  sh   t1, 0(t0)         # page_p[depth] = p
  sh   t2, 24(t0)        # page_o[depth] = o
  add  t6, s0, s10
  sb   t3, 48(t6)        # page_a[depth] = a
  sb   t4, 60(t6)        # page_b[depth] = b
  sb   zero, 72(t6)      # next_move[depth] = 0
  j dfs_loop

backtrack:
  addi s10, s10, -1      # depth--
  j dfs_loop

next_bound:
  addi s9, s9, 1         # bound++
  j bound_loop

found:
  addi a0, s10, 1        # result = depth + 1
  j finish

not_found:
  li a0, -1

finish:
  li a7, 10
  ecall


# heuristic
heur:
  add t0, s1, t2 # t0 = addr h_O[o]=s1+o : this heristic starts
  lbu t5, 0(t0)  # t5 = curr vallue = h_O[o]
  #t6 = p*9 + oa*3 +ob
  #px9 = (p<<3) +p
  slli t6, t1, 3  #t6 = p<<3  = px8
  add t6, t6, t1  # t6 = px8+p = px9

  # +oa*3 = +oa + (oa <<1)
  andi t0, t3, 3  #t0 = oa = a&3 (2 lower bits of a = rotate)
  add t6, t6, t0  #t6+=oa
  slli t0, t0, 1  #t0 = oa <<1 = oa*2
  add t6, t6, t0  # t6+=oa*2

  #+ob
  andi t0, t4, 3 # t0 = ob = b&3
  add t6, t6, t0 # t6+=ob
  
  #hn = h_N[idx]
  add t6, s2, t6 # t6 = add h_N[idx] = s2+idx
  lbu t6, 0(t6)   #t6 = h_N[idx]
  bgeu t5, t6, heur_done #if h>= hn ->h correct jumpt to done
  mv t5, t6       # not jumpt -> hn> h -> h =hn

heur_done:  
  ret

.data
perm_off: .word 0, 10080, 20160
ori_off:  .word 0, 1458, 2916
cub_off:  .word 0, 28, 56
 
work:                                          
  .half 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # page_p     +0
  .half 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # page_o     +24
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # page_a     +48
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # page_b     +60
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # next_move  +72
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0     # move_dis   +84  
  