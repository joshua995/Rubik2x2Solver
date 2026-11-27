#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

typedef struct State
{
    uint64_t cube;
} State;
int main()
{
    State s = {54};
    State sCopy = s;
    printf("OG %" PRId64 "\n", s.cube);
    printf("Copy %" PRId64 "\n", sCopy.cube);
    sCopy.cube = 2000;
    printf("OG %" PRId64 "\n", s.cube);
    printf("Copy %" PRId64 "\n", sCopy.cube);
    return 0;
}