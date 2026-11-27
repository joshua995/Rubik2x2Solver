// gcc -O3 -march=native -flto -DNDEBUG cube.c -o cube

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_SIZE 3674160
#define HASH_SIZE (1ULL << 23)
#define HASH_MASK (HASH_SIZE - 1)

#define PERM_OFFSET 16 // >> 16 to get list of perms MSB -> LSB 0, 1, 2, ... corners
#define ULB_OFFSET 21  // * 3
#define UBR_OFFSET 18
#define URF_OFFSET 15
#define UFL_OFFSET 12
#define DLF_OFFSET 9
#define DFR_OFFSET 6
#define DRB_OFFSET 3
#define DBL_OFFSET 0

#define U_MOVE 0
#define Ui_MOVE 1
#define U2_MOVE 2
#define D_MOVE 3
#define Di_MOVE 4
#define D2_MOVE 5
#define R_MOVE 6
#define Ri_MOVE 7
#define R2_MOVE 8
#define L_MOVE 9
#define Li_MOVE 10
#define L2_MOVE 11
#define F_MOVE 12
#define Fi_MOVE 13
#define F2_MOVE 14
#define B_MOVE 15
#define Bi_MOVE 16
#define B2_MOVE 17
#define X_MOVE 18
#define Xi_MOVE 19
#define X2_MOVE 20
#define Z_MOVE 21
#define Zi_MOVE 22
#define Z2_MOVE 23
#define Y_MOVE 24
#define Yi_MOVE 25
#define Y2_MOVE 26

#define ULB 0b000
#define UBR 0b001
#define URF 0b010
#define UFL 0b011
#define DLF 0b100
#define DFR 0b101
#define DRB 0b110
#define DBL 0b111

#define ORIENT0 0b00
#define ORIENT1 0b01 // i.e. ULB -> BUL
#define ORIENT2 0b10 // i.e. ULB -> LBU

#define ULB_OFFSET_O 14 // * 2
#define UBR_OFFSET_O 12
#define URF_OFFSET_O 10
#define UFL_OFFSET_O 8
#define DLF_OFFSET_O 6
#define DFR_OFFSET_O 4
#define DRB_OFFSET_O 2
#define DBL_OFFSET_O 0

#define WHITE 0b000
#define YELLOW 0b001
#define BLUE 0b010
#define GREEN 0b011
#define RED 0b100
#define ORANGE 0b101

#define R_CORNER_MASK 0b000111111000000111111000
#define L_CORNER_MASK 0b111000000111111000000111
#define F_CORNER_MASK 0b000000111111111111000000
#define B_CORNER_MASK 0b111111000000000000111111

#define R_CORNER_O_MASK 0b0011110000111100
#define L_CORNER_O_MASK 0b1100001111000011
#define F_CORNER_O_MASK 0b0000111111110000
#define B_CORNER_O_MASK 0b1111000000001111

#define CLEAR_PERM 0b0000000000000000000000001111111111111111

#define CLEAR_U_CORNERS 0b0000000000001111111111111111111111111111
#define CLEAR_D_CORNERS 0b1111111111110000000000001111111111111111
#define CLEAR_R_CORNERS 0b1110000001111110000001111111111111111111
#define CLEAR_L_CORNERS 0b0001111110000001111110001111111111111111
#define CLEAR_F_CORNERS 0b1111110000000000001111111111111111111111
#define CLEAR_B_CORNERS 0b0000001111111111110000001111111111111111

#define CLEAR_ORIENT 0b1111111111111111111111110000000000000000

#define CLEAR_U_CORNERS_O 0b1111111111111111111111110000000011111111
#define CLEAR_D_CORNERS_O 0b1111111111111111111111111111111100000000
#define CLEAR_R_CORNERS_O 0b1111111111111111111111111100001111000011
#define CLEAR_L_CORNERS_O 0b1111111111111111111111110011110000111100
#define CLEAR_F_CORNERS_O 0b1111111111111111111111111111000000001111
#define CLEAR_B_CORNERS_O 0b1111111111111111111111110000111111110000

#define CLEAR_ULB_O 0b1111111111111111111111110011111111111111
#define CLEAR_UBR_O 0b1111111111111111111111111100111111111111
#define CLEAR_URF_O 0b1111111111111111111111111111001111111111
#define CLEAR_UFL_O 0b1111111111111111111111111111110011111111
#define CLEAR_DLF_O 0b1111111111111111111111111111111100111111
#define CLEAR_DFR_O 0b1111111111111111111111111111111111001111
#define CLEAR_DRB_O 0b1111111111111111111111111111111111110011
#define CLEAR_DBL_O 0b1111111111111111111111111111111111111100

static const uint8_t ORIENT_PLUS2[4] = {2, 0, 1, 0};
static const uint8_t ORIENT_PLUS1[4] = {1, 2, 0, 0};

uint64_t cube = 0b0000010100111001011101110000000000000000;

typedef struct StateStruct {
  uint64_t cube;
  uint64_t movesNdepth;
} State;

// State stored in primitive arrays
State stateQueue[MAX_SIZE];

uint32_t stateQueueHead = 0;
uint32_t stateQueueTail = 0;

uint64_t cubeHashTable[HASH_SIZE];

uint64_t hash64(uint64_t x) {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}

uint8_t hashsetInsert(uint64_t cube) {
  uint64_t i = hash64(cube) & HASH_MASK;

  while (cubeHashTable[i] != 0) {
    if (cubeHashTable[i] == cube) {
      return 0;
    }
    i = (i + 1) & HASH_MASK;
  }

  cubeHashTable[i] = cube;
  return 1;
}

void U() {
  cube = (((cube & CLEAR_U_CORNERS) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 3) & 0b000111111111) |
                                        ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 9) & 0b111000000000))
                                       << (PERM_OFFSET + 12))) &
          CLEAR_U_CORNERS_O) |
         ((((((cube >> 8) & 0b11111111) >> 2) & 0b00111111) | ((((cube >> 8) & 0b11111111) << 6) & 0b11000000)) << 8);
}

void Ui() {
  cube = (((cube & CLEAR_U_CORNERS) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 3) & 0b111111111000) |
                                        ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 9) & 0b000000000111))
                                       << (PERM_OFFSET + 12))) &
          CLEAR_U_CORNERS_O) |
         ((((((cube >> 8) & 0b11111111) << 2) & 0b11111100) | ((((cube >> 8) & 0b11111111) >> 6) & 0b00000011)) << 8);
}

void U2() {
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O) |
         ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 6) & 0b111111000000) |
           ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 6) & 0b000000111111))
          << (PERM_OFFSET + 12)) |
         ((((((cube >> 8) & 0b11111111) << 4) & 0b11110000) | ((((cube >> 8) & 0b11111111) >> 4) & 0b00001111)) << 8);
}

void D() {
  cube = (((cube & CLEAR_D_CORNERS) | ((((((cube >> PERM_OFFSET) & 0b111111111111) >> 3) & 0b000111111111) |
                                        ((((cube >> PERM_OFFSET) & 0b111111111111) << 9) & 0b111000000000))
                                       << PERM_OFFSET)) &
          CLEAR_D_CORNERS_O) |
         ((((cube & 0b11111111) >> 2) & 0b00111111) | (((cube & 0b11111111) << 6) & 0b11000000));
}

void Di() {
  cube = (((cube & CLEAR_D_CORNERS) | ((((((cube >> PERM_OFFSET) & 0b111111111111) << 3) & 0b111111111000) |
                                        ((((cube >> PERM_OFFSET) & 0b111111111111) >> 9) & 0b000000000111))
                                       << PERM_OFFSET)) &
          CLEAR_D_CORNERS_O) |
         ((((cube & 0b11111111) << 2) & 0b11111100) | (((cube & 0b11111111) >> 6) & 0b00000011));
}

void D2() {
  cube = (cube & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & 0b111111111111) << 6) & 0b111111000000) |
           ((((cube >> PERM_OFFSET) & 0b111111111111) >> 6) & 0b000000111111))
          << PERM_OFFSET) |
         ((((cube & 0b11111111) << 4) & 0b11110000) | (((cube & 0b11111111) >> 4) & 0b00001111));
}

void R() {
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 15) & 0b000000000000000000111000) |
           ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 3) & 0b000111000000000111000000) |
           ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 9) & 0b000000111000000000000000))
          << PERM_OFFSET) |
         ((((cube & R_CORNER_O_MASK) << 2) & 0b0011000000110000) |
          (((cube & R_CORNER_O_MASK) >> 10) & 0b0000000000001100) |
          (((cube & R_CORNER_O_MASK) << 6) & 0b0000110000000000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O);
}

void Ri() {
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 9) & 0b000000000000000111000000) |
           ((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 3) & 0b000000111000000000111000) |
           ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 15) & 0b000111000000000000000000))
          << PERM_OFFSET) |
         ((((cube & R_CORNER_O_MASK) >> 2) & 0b0000110000001100) |
          (((cube & R_CORNER_O_MASK) << 10) & 0b0011000000000000) |
          (((cube & R_CORNER_O_MASK) >> 6) & 0b0000000000110000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O);
}

void R2() {
  cube =
      (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 12) & 0b000000000000000111111000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 12) & 0b000111111000000000000000))
       << PERM_OFFSET) |
      ((((cube & R_CORNER_O_MASK) << 8) & 0b0011110000000000) | (((cube & R_CORNER_O_MASK) >> 8) & 0b0000000000111100));
}

void L() {
  cube = ((cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
          ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 9) & 0b000000000111000000000111) |
            ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 21) & 0b111000000000000000000000) |
            ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 3) & 0b000000000000111000000000))
           << PERM_OFFSET) |
          ((((cube & L_CORNER_O_MASK) >> 6) & 0b0000001100000011) |
           (((cube & L_CORNER_O_MASK) >> 2) & 0b0000000011000000) |
           (((cube & L_CORNER_O_MASK) << 14) & 0b1100000000000000)));
  cube = (cube & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O);
}

void Li() {
  cube =
      (cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 9) & 0b111000000000111000000000) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 21) & 0b000000000000000000000111) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 3) & 0b000000000111000000000000))
       << PERM_OFFSET) |
      ((((cube & L_CORNER_O_MASK) << 6) & 0b1100000011000000) | (((cube & L_CORNER_O_MASK) << 2) & 0b0000001100000000) |
       (((cube & L_CORNER_O_MASK) >> 14) & 0b0000000000000011));
  cube = (cube & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O);
}

void L2() {
  cube =
      (cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 12) & 0b000000000000111000000111) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 12) & 0b111000000111000000000000))
       << PERM_OFFSET) |
      ((((cube & L_CORNER_O_MASK) >> 8) & 0b0000000011000011) | (((cube & L_CORNER_O_MASK) << 8) & 0b1100001100000000));
}

void F() {
  cube = ((cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) |
          ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 3) & 0b000000111111111000000000) |
            ((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 9) & 0b000000000000000111000000))
           << PERM_OFFSET) |
          ((((cube & F_CORNER_O_MASK) << 2) & 0b0000111111000000) |
           (((cube & F_CORNER_O_MASK) >> 6) & 0b0000000000110000)));

  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O);
}

void Fi() {
  cube = ((cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) |
          ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 3) & 0b000000000111111111000000) |
            ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 9) & 0b000000111000000000000000))
           << PERM_OFFSET) |
          ((((cube & F_CORNER_O_MASK) >> 2) & 0b0000001111110000) |
           (((cube & F_CORNER_O_MASK) << 6) & 0b0000110000000000)));

  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O);
}

void F2() {
  cube =
      (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 6) & 0b000000000000111111000000) |
        ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 6) & 0b000000111111000000000000))
       << PERM_OFFSET) |
      ((((cube & F_CORNER_O_MASK) >> 4) & 0b0000000011110000) | (((cube & F_CORNER_O_MASK) << 4) & 0b00111100000000));
}

void B() {
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 3) & 0b111000000000000000111000) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 21) & 0b000000000000000000000111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 15) & 0b000111000000000000000000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) << 2) & 0b1100000000001100) |
          (((cube & B_CORNER_O_MASK) >> 14) & 0b0000000000000011) |
          (((cube & B_CORNER_O_MASK) << 10) & 0b0011000000000000));
  cube = (cube & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O);
}

void Bi() {
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 3) & 0b000111000000000000000111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 21) & 0b111000000000000000000000) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 15) & 0b000000000000000000111000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) >> 2) & 0b0011000000000011) |
          (((cube & B_CORNER_O_MASK) << 14) & 0b1100000000000000) |
          (((cube & B_CORNER_O_MASK) >> 10) & 0b0000000000001100));
  cube = (cube & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O);
}

void B2() {
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 18) & 0b000000000000000000111111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 18) & 0b111111000000000000000000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) >> 12) & 0b0000000000001111) |
          (((cube & B_CORNER_O_MASK) << 12) & 0b1111000000000000));
}

void y() {
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) |
         ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 3) & 0b000111111111) |
           ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 9) & 0b111000000000))
          << (PERM_OFFSET + 12)) |
         ((((((cube >> 8) & 0b11111111) >> 2) & 0b00111111) | ((((cube >> 8) & 0b11111111) << 6) & 0b11000000)) << 8) |
         ((((((cube >> PERM_OFFSET) & 0b111111111111) << 3) & 0b111111111000) |
           ((((cube >> PERM_OFFSET) & 0b111111111111) >> 9) & 0b000000000111))
          << PERM_OFFSET) |
         ((((cube & 0b11111111) << 2) & 0b11111100) | (((cube & 0b11111111) >> 6) & 0b00000011));
}

void yi() {
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) |
         ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 3) & 0b111111111000) |
           ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 9) & 0b000000000111))
          << (PERM_OFFSET + 12)) |
         ((((((cube >> 8) & 0b11111111) << 2) & 0b11111100) | ((((cube >> 8) & 0b11111111) >> 6) & 0b00000011)) << 8) |
         ((((((cube >> PERM_OFFSET) & 0b111111111111) >> 3) & 0b000111111111) |
           ((((cube >> PERM_OFFSET) & 0b111111111111) << 9) & 0b111000000000))
          << PERM_OFFSET) |
         ((((cube & 0b11111111) >> 2) & 0b00111111) | (((cube & 0b11111111) << 6) & 0b11000000));
}

void y2() {
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) |
         ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 6) & 0b111111000000) |
           ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 6) & 0b000000111111))
          << (PERM_OFFSET + 12)) |
         ((((((cube >> 8) & 0b11111111) << 4) & 0b11110000) | ((((cube >> 8) & 0b11111111) >> 4) & 0b00001111)) << 8) |
         ((((((cube >> PERM_OFFSET) & 0b111111111111) << 6) & 0b111111000000) |
           ((((cube >> PERM_OFFSET) & 0b111111111111) >> 6) & 0b000000111111))
          << PERM_OFFSET) |
         ((((cube & 0b11111111) << 4) & 0b11110000) | (((cube & 0b11111111) >> 4) & 0b00001111));
}

void z() {
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 3) & 0b000000111111111000000000) |
           ((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 9) & 0b000000000000000111000000))
          << PERM_OFFSET) |
         ((((cube & F_CORNER_O_MASK) << 2) & 0b0000111111000000) |
          (((cube & F_CORNER_O_MASK) >> 6) & 0b0000000000110000)) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 3) & 0b000111000000000000000111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 21) & 0b111000000000000000000000) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 15) & 0b000000000000000000111000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) >> 2) & 0b0011000000000011) |
          (((cube & B_CORNER_O_MASK) << 14) & 0b1100000000000000) |
          (((cube & B_CORNER_O_MASK) >> 10) & 0b0000000000001100));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O &
          CLEAR_DRB_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O);
}

void zi() {
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 3) & 0b000000000111111111000000) |
           ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 9) & 0b000000111000000000000000))
          << PERM_OFFSET) |
         ((((cube & F_CORNER_O_MASK) >> 2) & 0b0000001111110000) |
          (((cube & F_CORNER_O_MASK) << 6) & 0b0000110000000000)) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 3) & 0b111000000000000000111000) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 21) & 0b000000000000000000000111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 15) & 0b000111000000000000000000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) << 2) & 0b1100000000001100) |
          (((cube & B_CORNER_O_MASK) >> 14) & 0b0000000000000011) |
          (((cube & B_CORNER_O_MASK) << 10) & 0b0011000000000000));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O &
          CLEAR_DRB_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O);
}

void z2() {
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) |
         ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 6) & 0b000000000000111111000000) |
           ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 6) & 0b000000111111000000000000))
          << PERM_OFFSET) |
         ((((cube & F_CORNER_O_MASK) >> 4) & 0b0000000011110000) |
          (((cube & F_CORNER_O_MASK) << 4) & 0b0000111100000000)) |
         ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 18) & 0b000000000000000000111111) |
           ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 18) & 0b111111000000000000000000))
          << PERM_OFFSET) |
         ((((cube & B_CORNER_O_MASK) >> 12) & 0b0000000000001111) |
          (((cube & B_CORNER_O_MASK) << 12) & 0b1111000000000000));
}

void x() {
  cube =
      (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 15) & 0b000000000000000000111000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 3) & 0b000111000000000111000000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 9) & 0b000000111000000000000000))
       << PERM_OFFSET) |
      ((((cube & R_CORNER_O_MASK) << 2) & 0b0011000000110000) |
       (((cube & R_CORNER_O_MASK) >> 10) & 0b0000000000001100) |
       (((cube & R_CORNER_O_MASK) << 6) & 0b0000110000000000)) |
      ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 9) & 0b111000000000111000000000) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 21) & 0b000000000000000000000111) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 3) & 0b000000000111000000000000))
       << PERM_OFFSET) |
      ((((cube & L_CORNER_O_MASK) << 6) & 0b1100000011000000) | (((cube & L_CORNER_O_MASK) << 2) & 0b0000001100000000) |
       (((cube & L_CORNER_O_MASK) >> 14) & 0b0000000000000011));
  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O &
          CLEAR_DBL_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O);
}

void xi() {
  cube =
      (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 9) & 0b000000000000000111000000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 3) & 0b000000111000000000111000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 15) & 0b000111000000000000000000))
       << PERM_OFFSET) |
      ((((cube & R_CORNER_O_MASK) >> 2) & 0b0000110000001100) |
       (((cube & R_CORNER_O_MASK) << 10) & 0b0011000000000000) |
       (((cube & R_CORNER_O_MASK) >> 6) & 0b0000000000110000)) |
      ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 9) & 0b000000000111000000000111) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 21) & 0b111000000000000000000000) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 3) & 0b000000000000111000000000))
       << PERM_OFFSET) |
      ((((cube & L_CORNER_O_MASK) >> 6) & 0b0000001100000011) | (((cube & L_CORNER_O_MASK) >> 2) & 0b0000000011000000) |
       (((cube & L_CORNER_O_MASK) << 14) & 0b1100000000000000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O &
          CLEAR_DBL_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> URF_OFFSET_O) & 3] << URF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UBR_OFFSET_O) & 3] << UBR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DRB_OFFSET_O) & 3] << DRB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DFR_OFFSET_O) & 3] << DFR_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> ULB_OFFSET_O) & 3] << ULB_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> UFL_OFFSET_O) & 3] << UFL_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS2[(cube >> DLF_OFFSET_O) & 3] << DLF_OFFSET_O) |
         ((uint64_t)ORIENT_PLUS1[(cube >> DBL_OFFSET_O) & 3] << DBL_OFFSET_O);
}

void x2() {
  cube =
      (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) |
      ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 12) & 0b000000000000000111111000) |
        ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 12) & 0b000111111000000000000000))
       << PERM_OFFSET) |
      ((((cube & R_CORNER_O_MASK) << 8) & 0b0011110000000000) |
       (((cube & R_CORNER_O_MASK) >> 8) & 0b0000000000111100)) |
      ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 12) & 0b000000000000111000000111) |
        ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 12) & 0b111000000111000000000000))
       << PERM_OFFSET) |
      ((((cube & L_CORNER_O_MASK) >> 8) & 0b0000000011000011) | (((cube & L_CORNER_O_MASK) << 8) & 0b1100001100000000));
}

uint32_t setCanonicalState() {
  if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL && ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT0) {
    return 0;
  } else if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL && ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT1) {
    x();
    yi();
    return (X_MOVE << 5) | Yi_MOVE;
  } else if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL && ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT2) {
    y();
    xi();
    return (Y_MOVE << 5) | Xi_MOVE;
  } else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL && ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT0) {
    yi();
    return Yi_MOVE;
  } else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL && ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT1) {
    z();
    y2();
    return (Z_MOVE << 5) | Y2_MOVE;
  } else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL && ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT2) {
    xi();
    return Xi_MOVE;
  } else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL && ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT0) {
    y2();
    return Y2_MOVE;
  } else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL && ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT1) {
    xi();
    y();
    return (Xi_MOVE << 5) | Y_MOVE;
  } else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL && ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT2) {
    xi();
    zi();
    return (Xi_MOVE << 5) | Zi_MOVE;
  } else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL && ((cube >> URF_OFFSET_O) & 0b11) == ORIENT0) {
    y();
    return Y_MOVE;
  } else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL && ((cube >> URF_OFFSET_O) & 0b11) == ORIENT1) {
    zi();
    return Zi_MOVE;
  } else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL && ((cube >> URF_OFFSET_O) & 0b11) == ORIENT2) {
    z2();
    x();
    return (Z2_MOVE << 5) | X_MOVE;
  } else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL && ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT0) {
    z2();
    y();
    return (Z2_MOVE << 5) | Y_MOVE;
  } else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL && ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT1) {
    z();
    return Z_MOVE;
  } else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL && ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT2) {
    x();
    return X_MOVE;
  } else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL && ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT0) {
    z2();
    return Z2_MOVE;
  } else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL && ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT1) {
    x();
    y();
    return (X_MOVE << 5) | Y_MOVE;
  } else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL && ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT2) {
    x();
    zi();
    return (X_MOVE << 5) | Zi_MOVE;
  } else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL && ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT0) {
    x2();
    return X2_MOVE;
  } else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL && ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT1) {
    z();
    xi();
    return (Z_MOVE << 5) | Xi_MOVE;
  } else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL && ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT2) {
    x();
    z();
    return (X_MOVE << 5) | Z_MOVE;
  } else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL && ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT0) {
    x2();
    y();
    return (X2_MOVE << 5) | Y_MOVE;
  } else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL && ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT1) {
    x2();
    zi();
    return (X2_MOVE << 5) | Zi_MOVE;
  } else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL && ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT2) {
    z2();
    xi();
    return (Z2_MOVE << 5) | Xi_MOVE;
  }
  return 0;
}

void updateNewState(uint64_t currentStateMoves, uint8_t currentDepth, uint32_t nextMove, FILE *file) {
  // Store current state before canonicalization
  uint64_t tempCube = cube;
  uint32_t canonMoves = setCanonicalState();
  if (hashsetInsert(cube)) {
    uint64_t currentMoves = ((currentStateMoves << 5) | nextMove);
    // Also encode the depth for decoding later
    State s = {cube, (currentMoves << 4) | ((currentDepth + 1) & 0b1111)};
    fprintf(file, "%" PRId64 " %" PRId64 " %d\n", s.cube, s.movesNdepth, canonMoves);
    s.cube = tempCube;

    stateQueue[stateQueueTail++] = s;
  }
}

void generateLookupFile(uint64_t cubei, FILE *file) {
  hashsetInsert(cubei);
  State state = {cubei, 0};
  stateQueue[stateQueueTail++] = state;
  while (stateQueueHead < stateQueueTail) {
    // Breadth first search method
    state = stateQueue[stateQueueHead];
    if ((state.movesNdepth & 0b1111) >= 11) {
      stateQueueHead++;
      continue;
    }
    cube = state.cube;
    uint64_t currentCube = cube;

    U();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), U_MOVE, file);
    cube = currentCube;
    Ui();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Ui_MOVE, file);
    cube = currentCube;
    U2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), U2_MOVE, file);
    cube = currentCube;
    D();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), D_MOVE, file);
    cube = currentCube;
    Di();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Di_MOVE, file);
    cube = currentCube;
    D2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), D2_MOVE, file);
    cube = currentCube;
    R();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), R_MOVE, file);
    cube = currentCube;
    Ri();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Ri_MOVE, file);
    cube = currentCube;
    R2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), R2_MOVE, file);
    cube = currentCube;
    L();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), L_MOVE, file);
    cube = currentCube;
    Li();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Li_MOVE, file);
    cube = currentCube;
    L2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), L2_MOVE, file);
    cube = currentCube;
    F();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), F_MOVE, file);
    cube = currentCube;
    Fi();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Fi_MOVE, file);
    cube = currentCube;
    F2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), F2_MOVE, file);
    cube = currentCube;
    B();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), B_MOVE, file);
    cube = currentCube;
    Bi();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), Bi_MOVE, file);
    cube = currentCube;
    B2();
    updateNewState(state.movesNdepth, (state.movesNdepth & 0b1111), B2_MOVE, file);
    stateQueueHead++;
  }
}

int main() {
  FILE *file = fopen("lookupFile.txt", "w"); // Open for writing

  if (file == NULL) {
    printf("Couldn't create file!\n");
    return 1;
  }
  setvbuf(file, NULL, _IOFBF, 1 << 20);
  struct timespec startT;
  clock_gettime(CLOCK_MONOTONIC, &startT);
  // Store solved cube
  State state = {cube, 0};
  fprintf(file, "%" PRId64 " %d %d\n", state.cube, 0, 0);
  generateLookupFile(cube, file);
  fclose(file);
  struct timespec endT;
  clock_gettime(CLOCK_MONOTONIC, &endT);

  int64_t elapsed = (int64_t)(endT.tv_sec - startT.tv_sec) * 1000000000LL + (endT.tv_nsec - startT.tv_nsec);
  printf("%ld ns\n", elapsed / 1000000);

  return 0;
}