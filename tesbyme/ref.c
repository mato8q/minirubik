#include <stdint.h>
#include "tables.h"

enum {
    CUBIES = 7,
    PERMUTATIONS = 5040,
    ORIENTATIONS = 729,
    STATES = PERMUTATIONS * ORIENTATIONS,
    MOVES = 9
};

typedef struct {
    uint8_t p[CUBIES], o[CUBIES];
} state_t;
static const char *const move_names[MOVES] = {"R",  "R2", "R'", "B", "B2",
                                              "B'", "D",  "D2", "D'"};
static const uint8_t col_face[MOVES] = {0, 0, 0, 1, 1, 1, 2, 2, 2};
static const uint8_t col_turn[MOVES] = {1, 2, 3, 1, 2, 3, 1, 2, 3};

static uint8_t heuristic(uint16_t p, uint16_t o, uint8_t a, uint8_t b0){
    uint8_t h = h_O[o];
    uint8_t oa = a&3, ob=b0&3;
    uint8_t hn = h_N[(p<<3)+p +(oa<<1)+ oa+ob];
    return (hn > h)? hn : h;
}
static uint8_t cub_coord(const state_t *state, uint8_t cubie){
    for(uint8_t i = 0; i < CUBIES; i++){
        if(state->p[i] == cubie){
            return (uint8_t)(i<<2 | state->o[i]);
        }
    }
    return 0; // Should never happen
}
static uint8_t count_smaller(const state_t *s, uint8_t i){
    uint8_t n=0;
    for (uint8_t j = i+1; j < CUBIES; ++j) {
        if (s->p[j] < s-> p[i]) {
            ++n;
        }
        
    }
    return n;
}
static void encode_state(const state_t *s, uint16_t *p_out, uint16_t *o_out, uint8_t *a_out, uint8_t *b_out){
    uint16_t p = count_smaller(s,0);
    p = (p<<2)+(p<<1)+ count_smaller(s,1); // x6
    p = (p<<2)+p+ count_smaller(s,2);//x5
    p = (p<<2)+count_smaller(s,3); //x4
    p = (p<<1)+p+ count_smaller(s,4); //x3
    p = (p<<1)+count_smaller(s,5); //x2
    
    uint16_t o=0;
    for(uint8_t i =0; i<6; ++i) o = (o<<1)+o+ s->o[i];
    *p_out=p;
    *o_out=o;
    *a_out=cub_coord(s,0);
    *b_out=cub_coord(s,1);
}


static uint8_t page_a[12];
static uint8_t page_b[12];
static uint8_t move_dis[11];
static uint8_t next_move[12];
static uint16_t page_p[12];
static uint16_t page_o[12];
static uint32_t check;

static int ida_star(uint16_t p, uint16_t o, uint8_t a, uint8_t b0){
  check = 0;
  uint8_t h_root = heuristic(p, o, a, b0);
  if(h_root == 0) {
    return 0; // Already solved
  }
  int bound = h_root;
  while(bound <=11){
    int depth = 0;
    page_p[0] = p, page_o[0] = o, page_a[0] =a, page_b[0]= b0;
    next_move[0] = 0;
    while(depth >= 0){
      // Implementation for IDA* search

      if(next_move[depth] == 9) { // 2nd condition
          depth--;
          continue;
        }
      uint8_t move = next_move[depth]++;
      if(depth > 0 && col_face[move] == col_face[move_dis[depth-1]]) continue; // Avoid consecutive moves on the same face
      int face = col_face[move];
      int turn = col_turn[move];
      
      uint16_t c_P = page_p[depth], c_O = page_o[depth];
      uint8_t c_A = page_a[depth], c_B = page_b[depth];
      for(int i=0; i < turn; i++){
        c_P = permutation[face][c_P];
        c_O = orientation[face][c_O];
        c_A = cub_move[face][c_A];
        c_B = cub_move[face][c_B];
      }
      check++;
      uint16_t h_child = heuristic(c_P, c_O, c_A, c_B);
      if((depth+1) + h_child <= bound){
        move_dis[depth] = move;
        if(h_child==0) return depth+1;
        depth++;
        page_p[depth] = c_P;
        page_o[depth] = c_O;
        page_a[depth] = c_A;
        page_b[depth] = c_B;
        next_move[depth] = 0;
      }
    }

    bound++;
  }

  return -1;
}

static void print_int(int x){
    register int a0 asm("a0") = x;
    register int a7 asm("a7") = 1;
    asm volatile("ecall" : : "r"(a0), "r"(a7));
}

static void print_char(char c){
    register int a0 asm("a0") = c;
    register int a7 asm("a7") = 11;
    asm volatile("ecall" : : "r"(a0), "r"(a7));
}

static void print_str(const char *s){
    register int a0 asm("a0") = (int)s;
    register int a7 asm("a7") = 4;
    asm volatile("ecall" : : "r"(a0), "r"(a7));
}

static void sys_exit(int code){
    register int a0 asm("a0") = code;
    register int a7 asm("a7") = 93;
    asm volatile("ecall" : : "r"(a0), "r"(a7));
    for(;;);
}

void _start(void){
    const char *input = "21345671111111";
    //const char *input = "54721631111111"; worst case
    state_t s;
    for(int i=0; i<7;i++){
        s.p[i]=input[i]-'1'; // 0~6 idx
        s.o[i]=input[i+7]-'1'; //7~13 twist
    }
    uint16_t p,o;
    uint8_t a,b;
    encode_state(&s,&p,&o,&a,&b);
    
    int n =ida_star(p,o,a,b);
    for(int k=0; k<n; k++){
        uint8_t m=move_dis[k];
        int face = col_face[m];
        int turn=col_turn[m];

        for(int i=0; i<turn; i++){
            p=permutation[face][p];
            o=orientation[face][o];
            a=cub_move[face][a];
            b=cub_move[face][b];
        }
    }
    print_int(n);
    print_char('\n');

    for(int k=0; k<n; k++){
        print_str(move_names[move_dis[k]]);
        print_char(' ');
    }
    print_char('\n');

    if(p==0 && o==0) sys_exit(0);
    else sys_exit(1);
}
