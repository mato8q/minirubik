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
/* Each destination takes a cubie from source[face][destination]. */
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

int main(void)
{
    build_tables();
    return 0;
}
