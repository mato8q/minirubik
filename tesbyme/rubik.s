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

#C:encode intput t1=p, t2 =o, t3=a, t4=b
la a0, input

  # p: 6 rounds (Horner, x6 x5 x4 x3 x2 with shift/add)
  li a1, 0
  jal ra, cnt
  mv t1, a3              # round 0: p = s0

  li a1, 1
  jal ra, cnt
  slli t5, t1, 2
  slli t6, t1, 1
  add  t1, t5, t6        # p*6
  add  t1, t1, a3

  li a1, 2
  jal ra, cnt
  slli t5, t1, 2
  add  t1, t5, t1        # p*5
  add  t1, t1, a3

  li a1, 3
  jal ra, cnt
  slli t1, t1, 2         # p*4
  add  t1, t1, a3

  li a1, 4
  jal ra, cnt
  slli t5, t1, 1
  add  t1, t5, t1        # p*3
  add  t1, t1, a3

  li a1, 5
  jal ra, cnt
  slli t1, t1, 1         # p*2
  add  t1, t1, a3

  # o: 6 rounds
  li t2, 0
  li a1, 0
o_loop:
  slli t5, t2, 1
  add  t2, t5, t2        # o*3
  add  t0, a0, a1
  lbu  t0, 7(t0)         # orientation char of slot i
  addi t0, t0, -49
  add  t2, t2, t0
  addi a1, a1, 1
  li   t0, 6
  bltu a1, t0, o_loop    # i < 6 -> loop

  # a, b
  li a3, 49              # '1' = cubie 0
  jal ra, coord
  mv t3, a4
  li a3, 50              # '2' = cubie 1
  jal ra, coord
  mv t4, a4

# root stores on page[0]
  sh t1, 0(s0)           # page_p[0] = p
  sh t2, 24(s0)          # page_o[0] = o
  sb t3, 48(s0)          # page_a[0] = a
  sb t4, 60(s0)          # page_b[0] = b
  li s11, 0              # check = 0

# h_root
  jal ra, heur # t5 = h(root)
  li a0, 0  # answer=0
  li a1, 1 #verified (0 moves solves a solved cube)
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
  jal ra, rot
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
  addi s9, s10, 1        # n = depth + 1 (s9 no longer needed as bound)
  #e: replay move_dis from root, must reach solved
  lhu t1, 0(s0)          # start from page[0] = root
  lhu t2, 24(s0)
  lbu t3, 48(s0)
  lbu t4, 60(s0)
  li  s10, 0             # k = 0
e_loop:
  bgeu s10, s9, e_check  # k >= n -> all moves replayed
  add  t6, s0, s10
  lbu  t0, 84(t6)        # m = move_dis[k]
  add  t5, a5, t0
  lbu  a0, 0(t5)         # face
  add  t5, a6, t0
  lbu  a1, 0(t5)         # turn
  jal  ra, rot
  addi s10, s10, 1       # k++
  j e_loop

e_check:
  or   t0, t1, t2        # t0 == 0 only if p == 0 and o == 0
  li   a1, 1             # assume OK
  beqz t0, e_ok
  li   a1, 0             # failed
e_ok:
  mv   a0, s9            # a0 = answer
  j finish

not_found:
  li a0, -1
  li a1, 0               #no solution to verify

finish:
  mv   s9, a0            # keep answer (a0 is needed by ecall)
  li   a7, 1             # print int
  ecall                  # prints a0 = answer
  li   a0, 10            # '\n'
  li   a7, 11            # print char
  ecall
  la   t3, move_names
  li   s10, 0            # k = 0
p_loop:
  bge  s10, s9, p_done   # k >= n -> done (also handles n = -1, 0)
  add  t6, s0, s10
  lbu  t0, 84(t6)        # m = move_dis[k]
  slli t0, t0, 2         # m * 4 (each name is 4 bytes)
  add  a0, t3, t0        # a0 = &move_names[m]
  li   a7, 4             # print string
  ecall
  li   a0, 32            # ' '
  li   a7, 11
  ecall
  addi s10, s10, 1
  j p_loop
p_done:
  li   a0, 10            # '\n'
  li   a7, 11
  ecall
  xori a0, a1, 1            # answer back in a0
  li   a7, 93
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

rot:
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
rot_loop:

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
  bnez a1, rot_loop    # still left -> loop
  ret

# count_smallwer: a1=i -> a3 = count of p[j], j>i  
cnt:
  add  t0, a0, a1
  lbu  a4, 0(t0)         # a4 = p[i]
  li   a3, 0             # n = 0
  addi a2, a1, 1         # j = i + 1
cnt_loop:
  li   t0, 7
  bgeu a2, t0, cnt_done  # j >= 7 -> done
  add  t0, a0, a2
  lbu  t0, 0(t0)         # p[j]
  bgeu t0, a4, cnt_next  # p[j] >= p[i] -> skip
  addi a3, a3, 1         # n++
cnt_next:
  addi a2, a2, 1         # j++
  j cnt_loop
cnt_done:
  ret

#cub_coord: a3 = cubie char -> a4 = (pos<<2)|ori 
coord:
  li   a1, 0             # i = 0
coord_loop:
  add  t0, a0, a1
  lbu  a2, 0(t0)         # p[i]
  beq  a2, a3, coord_found
  addi a1, a1, 1
  j coord_loop
coord_found:
  lbu  a2, 7(t0)         # o[i]
  addi a2, a2, -49       # char -> number
  slli a4, a1, 2
  or   a4, a4, a2        # (i<<2) | o[i]
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
  
input:
  .string "21345671111111"   
  .byte 0 

move_names:
  .string "R"
  .byte 0, 0
  .string "R2"
  .byte 0
  .string "R'"
  .byte 0
  .string "B"
  .byte 0, 0
  .string "B2"
  .byte 0
  .string "B'"
  .byte 0
  .string "D"
  .byte 0, 0
  .string "D2"
  .byte 0
  .string "D'"
  .byte 0                
