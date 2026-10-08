#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static const uint8_t inverse_move[MOVES] = {2, 1, 0, 5, 4, 3, 8, 7, 6};
static const uint8_t col_face[MOVES] = {0, 0, 0, 1, 1, 1, 2, 2, 2};
static const uint8_t col_turn[MOVES] = {1, 2, 3, 1, 2, 3, 1, 2, 3};
static const uint8_t source[3][CUBIES] = {
    {1, 4, 2, 0, 3, 5, 6},
    {0, 1, 2, 4, 5, 6, 3},
    {0, 2, 5, 3, 1, 4, 6},
};
static const uint8_t twist[3][CUBIES] = {
    {1, 2, 0, 2, 1, 0, 0},
    {0, 0, 0, 1, 2, 1, 2},
    {0, 0, 0, 0, 0, 0, 0},
};

static state_t quarter_turn(state_t state, uint8_t face)
{
    state_t result;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t from = source[face][i];
        result.p[i] = state.p[from];
        result.o[i] = (uint8_t) ((state.o[from] + twist[face][i]) % 3U);
    }
    return result;
}

static uint32_t rank_state(const state_t *state)
{
    uint32_t p = 0, o = 0;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t smaller = 0;
        for (uint8_t j = (uint8_t) (i + 1U); j < CUBIES; ++j)
            if (state->p[j] < state->p[i])
                ++smaller;
        p = p * (CUBIES - i) + smaller;
    }
    for (uint8_t i = 0; i < 6; ++i)
        o = o * 3U + state->o[i];
    return p * ORIENTATIONS + o;
}

static void unrank_state(uint32_t rank, state_t *state)
{
    uint8_t available[CUBIES] = {0, 1, 2, 3, 4, 5, 6};
    uint32_t p = rank / ORIENTATIONS, o = rank % ORIENTATIONS, f = 720;
    uint8_t sum = 0;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t q = (uint8_t) (p / f);
        p %= f;
        state->p[i] = available[q];
        for (uint8_t j = q; j + 1U < CUBIES - i; ++j)
            available[j] = available[j + 1U];
        if (i < 5)
            f /= 6U - i;
    }
    for (uint8_t i = 6; i-- > 0;) {
        state->o[i] = (uint8_t) (o % 3U);
        sum = (uint8_t) (sum + state->o[i]);
        o /= 3U;
    }
    state->o[6] = (uint8_t) ((3U - sum % 3U) % 3U);
}
//declare tables for permutation and orientation transitions
static uint16_t permutation[3][PERMUTATIONS];
static uint16_t orientation[3][ORIENTATIONS];
static uint8_t h_P[PERMUTATIONS]; //table for storing the number of moves to reach the solved state from each permutation
static uint8_t h_O[ORIENTATIONS]; //table for storing the number of moves to reach the solved state from each orientation

static void build_tables(void) //declare my own buil_tables
{
    state_t state;
    for (uint16_t rank = 0; rank < PERMUTATIONS; ++rank) {
        unrank_state((uint32_t) rank * ORIENTATIONS, &state);
        for (uint8_t face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, face);
            permutation[face][rank] =
                (uint16_t) (rank_state(&next) / ORIENTATIONS);
        }
    }
    for (uint16_t rank = 0; rank < ORIENTATIONS; ++rank) {
        unrank_state(rank, &state);
        for (uint8_t face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, face);
            orientation[face][rank] =
                (uint16_t) (rank_state(&next) % ORIENTATIONS);
        }
    }

    memset(h_P, UINT8_MAX, PERMUTATIONS);
    h_P[0] = 0;
    uint16_t p_queue[PERMUTATIONS];
    p_queue[0] = 0;
    uint32_t p_head = 0, p_tail = 1;
    while (p_head < p_tail) {
        uint16_t here = p_queue[p_head++];
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t next_p = here;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                next_p = permutation[face][next_p];
                if(h_P[next_p]== UINT8_MAX){
                    h_P[next_p] = (uint8_t)(h_P[here]+1);
                    p_queue[p_tail++] = next_p;
                }
            }
        }
    }
    printf("Permutation table built. Total permutations: %u\n", (unsigned int) p_tail);
    int max_dist = 0;
    for(int i = 0; i < PERMUTATIONS; ++i) {
        if(h_P[i] > max_dist) {
            max_dist = h_P[i];
        }
    }
    printf("Maximum distance found: %d\n", max_dist);
    printf("Distance to solved state: %d\n", h_P[0]);


    memset(h_O, UINT8_MAX, ORIENTATIONS);
    h_O[0] = 0;
    uint16_t o_queue[ORIENTATIONS];
    o_queue[0] = 0;
    uint32_t o_head = 0, o_tail = 1;
    while (o_head < o_tail) {
        uint16_t here = o_queue[o_head++];
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t next_o = here;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                next_o = orientation[face][next_o];
                if(h_O[next_o]== UINT8_MAX){
                    h_O[next_o] = (uint8_t)(h_O[here]+1);
                    o_queue[o_tail++] = next_o;
                }
            }
        }
    }
    printf("Orientation table built. Total orientations: %u\n", (unsigned int) o_tail);
    int max_dist_o = 0;
    for(int i = 0; i < ORIENTATIONS; ++i) {
        if(h_O[i] > max_dist_o) {
            max_dist_o = h_O[i];
        }
    }
    printf("Maximum orientation distance found: %d\n", max_dist_o);
    printf("Orientation distance to solved state: %d\n", h_O[0]);
}
static uint8_t move_dis[11];
static uint8_t next_move[12];
static uint16_t page_p[12];
static uint16_t page_o[12];
static uint32_t check;

static int ida_star(uint16_t p, uint16_t o){
  check = 0;
  uint8_t h_root = (h_P[p]> h_O[o]) ? h_P[p] : h_O[o];
  if(h_root == 0) {
    return 0; // Already solved
  }
  int bound = h_root;
  while(bound <=11){
    int depth = 0;
    page_p[0] = p, page_o[0] = o;
    next_move[0] = 0;
    while(depth >= 0){
      // Implementation for IDA* search
      uint16_t h_current = (h_P[page_p[depth]] > h_O[page_o[depth]]) ? h_P[page_p[depth]] : h_O[page_o[depth]]; 
      if(h_current == 0) { // 1st condition
        return depth; // Found solution
      }
      if(next_move[depth] == 9) { // 2nd condition
          depth--;
          continue;
        }
      uint8_t move = next_move[depth]++;
      if(depth > 0 && col_face[move] == col_face[move_dis[depth-1]]) continue; // Avoid consecutive moves on the same face
      int face = col_face[move];
      int turn = col_turn[move];
      
      uint16_t c_P = page_p[depth], c_O = page_o[depth];
      for(int i=0; i < turn; i++){
        c_P = permutation[face][c_P];
        c_O = orientation[face][c_O];
      }
      check++;
      uint16_t h_child = (h_P[c_P] > h_O[c_O]) ? h_P[c_P] : h_O[c_O];
      if((depth+1) + h_child <= bound){
        move_dis[depth] = move;
        depth++;
        page_p[depth] = c_P;
        page_o[depth] = c_O;
        next_move[depth] = 0;
      }
    }

    bound++;
  }

  return -1;
}
static uint8_t *fullBFS_h_P(void){
  uint8_t *dist = malloc(STATES);
  uint32_t *queue = malloc(STATES * sizeof(uint32_t));
  memset(dist, UINT8_MAX, STATES);
  dist[0] = 0;
  queue[0] = 0;
  uint32_t head = 0, tail = 1;
  while(head < tail){
    uint32_t here = queue[head++];
    uint16_t p = here / ORIENTATIONS;
    uint16_t o = here % ORIENTATIONS;
    for(uint8_t turn = 0; turn < 3; turn++){
      for(uint8_t face = 0; face < 3; face++){
        uint16_t next_p = p, next_o = o;
        for(uint8_t i=0; i < turn+1; i++){
          next_p = permutation[face][next_p];
          next_o = orientation[face][next_o];
        }
        uint32_t next_state = next_p * ORIENTATIONS + next_o;
        if(dist[next_state] == UINT8_MAX){
          dist[next_state] = (uint8_t)(dist[here]+1);
          queue[tail++] = next_state;
        }
      }
    }
  }
  free(queue);
  printf("BFS table built. Total states: %u\n", (unsigned int) tail);
  int max_dist = 0;
    for(uint32_t i = 0; i < STATES; ++i) {
        if(dist[i] > max_dist) {
            max_dist = dist[i];
        }
    }
    printf("Maximum distance in BFS table: %d\n", max_dist);
  return dist;
}

int main(void)
{
    build_tables();
    uint8_t *bfs_table = fullBFS_h_P();
    uint32_t ct = 0; // count h that exceeds d
    for(uint32_t rank = 0; rank < STATES; ++rank) {
        uint16_t p = rank / ORIENTATIONS;
        uint16_t o = rank % ORIENTATIONS;
        uint8_t h = (h_P[p] > h_O[o]) ? h_P[p] : h_O[o];
        if(h > bfs_table[rank]) {
            printf("Discrepancy found at rank %u: h=%u, bfs=%u\n", (unsigned int)rank, (unsigned int)h, (unsigned int)bfs_table[rank]);
            ct++;
        }
    }
  
    printf("Number of discrepancies found: %u\n", (unsigned int)ct);
    #if 0
    uint32_t not_same =0;
    uint32_t worst_node = 0;
    uint32_t highest_rank = 0;
    for(uint32_t rank = 0; rank < STATES; ++rank) {
      if(rank % 500000 == 0) printf("H3 progress: %u/%u\n", (unsigned int)rank, (unsigned int)STATES);
        uint16_t p = rank / ORIENTATIONS;
        uint16_t o = rank % ORIENTATIONS;
        uint8_t len = ida_star(p, o);
        if(len != bfs_table[rank]) {
          not_same++; }
        if(bfs_table[rank] == 11 && check > worst_node) {
          worst_node = check;
          highest_rank = rank;
        }
    }
    printf("H3 mismatches: %u\n", (unsigned int) not_same);
    printf("H3 worst nodes at distance 11: %u (rank %u)\n",
           (unsigned int) worst_node, (unsigned int) highest_rank);
    #endif
    int n = ida_star(0, 0);
    printf("Minimum number of moves to solve the cube: %d\n", n);
    

    state_t s;
    const char *input = "21345671111111"; // example input string representing a cube state
    for(int i = 0; i < 7; i++){
      s.p[i]= input[i]-'1';
      s.o[i]= input[i+7]- '1';
    }
    uint32_t rank = rank_state(&s);
    uint16_t p = rank / ORIENTATIONS;
    uint16_t o = rank % ORIENTATIONS;
    n = ida_star(p, o);
    printf("Minimum number of moves to solve the cube from the given state: %d\n", n);
    printf("Number of states checked during search: %u\n", (unsigned int) check);
    for(int i = 0; i < n; i++){
      printf("%s ", move_names[move_dis[i]]);
    }
    printf("\n");
    free(bfs_table);
    return 0;
}
