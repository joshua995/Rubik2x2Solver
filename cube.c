// gcc -O3 -march=native test.c -o test

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#define MAX_SIZE 3674160

// For file
uint8_t U_MOVE = 0;
uint8_t Ui_MOVE = 1;
uint8_t U2_MOVE = 2;
uint8_t D_MOVE = 3;
uint8_t Di_MOVE = 4;
uint8_t D2_MOVE = 5;
uint8_t R_MOVE = 6;
uint8_t Ri_MOVE = 7;
uint8_t R2_MOVE = 8;
uint8_t L_MOVE = 9;
uint8_t Li_MOVE = 10;
uint8_t L2_MOVE = 11;
uint8_t F_MOVE = 12;
uint8_t Fi_MOVE = 13;
uint8_t F2_MOVE = 14;
uint8_t B_MOVE = 15;
uint8_t Bi_MOVE = 16;
uint8_t B2_MOVE = 17;
uint8_t X_MOVE = 18;
uint8_t Xi_MOVE = 19;
uint8_t X2_MOVE = 20;
uint8_t Z_MOVE = 21;
uint8_t Zi_MOVE = 22;
uint8_t Z2_MOVE = 23;
uint8_t Y_MOVE = 24;
uint8_t Yi_MOVE = 25;
uint8_t Y2_MOVE = 26;

uint8_t ULB = 0b000;
uint8_t UBR = 0b001;
uint8_t URF = 0b010;
uint8_t UFL = 0b011;
uint8_t DLF = 0b100;
uint8_t DFR = 0b101;
uint8_t DRB = 0b110;
uint8_t DBL = 0b111;

uint8_t ORIENT0 = 0b00;
uint8_t ORIENT1 = 0b01; // i.e. ULB -> BUL
uint8_t ORIENT2 = 0b10; // i.e. ULB -> LBU

uint8_t PERM_OFFSET = 16; // >> 14 to get list of perms MSB -> LSB 0, 1, 2, ... corners
uint8_t ULB_OFFSET = 21;  // * 3
uint8_t UBR_OFFSET = 18;
uint8_t URF_OFFSET = 15;
uint8_t UFL_OFFSET = 12;
uint8_t DLF_OFFSET = 9;
uint8_t DFR_OFFSET = 6;
uint8_t DRB_OFFSET = 3;
uint8_t DBL_OFFSET = 0;

uint8_t ULB_OFFSET_O = 14; // * 2
uint8_t UBR_OFFSET_O = 12;
uint8_t URF_OFFSET_O = 10;
uint8_t UFL_OFFSET_O = 8;
uint8_t DLF_OFFSET_O = 6;
uint8_t DFR_OFFSET_O = 4;
uint8_t DRB_OFFSET_O = 2;
uint8_t DBL_OFFSET_O = 0;

uint8_t WHITE = 0b000;
uint8_t YELLOW = 0b001;
uint8_t BLUE = 0b010;
uint8_t GREEN = 0b011;
uint8_t RED = 0b100;
uint8_t ORANGE = 0b101;

uint32_t R_CORNER_MASK = 0b000111111000000111111000;
uint32_t L_CORNER_MASK = 0b111000000111111000000111;
uint32_t F_CORNER_MASK = 0b000000111111111111000000;
uint32_t B_CORNER_MASK = 0b111111000000000000111111;

uint32_t R_CORNER_O_MASK = 0b0011110000111100;
uint32_t L_CORNER_O_MASK = 0b1100001111000011;
uint32_t F_CORNER_O_MASK = 0b0000111111110000;
uint32_t B_CORNER_O_MASK = 0b1111000000001111;

uint64_t CLEAR_PERM = 0b0000000000000000000000001111111111111111;

uint64_t CLEAR_U_CORNERS = 0b0000000000001111111111111111111111111111;
uint64_t CLEAR_D_CORNERS = 0b1111111111110000000000001111111111111111;
uint64_t CLEAR_R_CORNERS = 0b1110000001111110000001111111111111111111;
uint64_t CLEAR_L_CORNERS = 0b0001111110000001111110001111111111111111;
uint64_t CLEAR_F_CORNERS = 0b1111110000000000001111111111111111111111;
uint64_t CLEAR_B_CORNERS = 0b0000001111111111110000001111111111111111;

uint64_t CLEAR_ORIENT = 0b1111111111111111111111110000000000000000;

uint64_t CLEAR_U_CORNERS_O = 0b1111111111111111111111110000000011111111;
uint64_t CLEAR_D_CORNERS_O = 0b1111111111111111111111111111111100000000;
uint64_t CLEAR_R_CORNERS_O = 0b1111111111111111111111111100001111000011;
uint64_t CLEAR_L_CORNERS_O = 0b1111111111111111111111110011110000111100;
uint64_t CLEAR_F_CORNERS_O = 0b1111111111111111111111111111000000001111;
uint64_t CLEAR_B_CORNERS_O = 0b1111111111111111111111110000111111110000;

uint64_t CLEAR_ULB_O = 0b1111111111111111111111110011111111111111;
uint64_t CLEAR_UBR_O = 0b1111111111111111111111111100111111111111;
uint64_t CLEAR_URF_O = 0b1111111111111111111111111111001111111111;
uint64_t CLEAR_UFL_O = 0b1111111111111111111111111111110011111111;
uint64_t CLEAR_DLF_O = 0b1111111111111111111111111111111100111111;
uint64_t CLEAR_DFR_O = 0b1111111111111111111111111111111111001111;
uint64_t CLEAR_DRB_O = 0b1111111111111111111111111111111111110011;
uint64_t CLEAR_DBL_O = 0b1111111111111111111111111111111111111100;

uint64_t cube = 0b0000010100111001011101110000000000000000;

// TODO HashSet
uint64_t canonStates[MAX_SIZE];
uint32_t canonStatesI = 0;

typedef struct StateStruct
{
  uint64_t cube;
  uint64_t moves;
  uint8_t depth;
  uint32_t canonMovesToWrite;
} State;

// State stored in primitive arrays
State stateQueue[MAX_SIZE];

uint32_t stateQueueHead = 0;
uint32_t stateQueueTail = 0;

void enqueueState(State state)
{
  stateQueue[stateQueueTail++] = state;
}

void U()
{
  cube = (((cube & CLEAR_U_CORNERS) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 3) & 0b000111111111) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 9) & 0b111000000000)) << (PERM_OFFSET + 12))) & CLEAR_U_CORNERS_O) | ((((((cube >> 8) & 0b11111111) >> 2) & 0b00111111) | ((((cube >> 8) & 0b11111111) << 6) & 0b11000000)) << 8);
}

void Ui()
{
  cube = (((cube & CLEAR_U_CORNERS) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 3) & 0b111111111000) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 9) & 0b000000000111)) << (PERM_OFFSET + 12))) & CLEAR_U_CORNERS_O) | ((((((cube >> 8) & 0b11111111) << 2) & 0b11111100) | ((((cube >> 8) & 0b11111111) >> 6) & 0b00000011)) << 8);
}

void U2()
{
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 6) & 0b111111000000) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 6) & 0b000000111111)) << (PERM_OFFSET + 12)) | ((((((cube >> 8) & 0b11111111) << 4) & 0b11110000) | ((((cube >> 8) & 0b11111111) >> 4) & 0b00001111)) << 8);
}

void D()
{
  cube = (((cube & CLEAR_D_CORNERS) | ((((((cube >> PERM_OFFSET) & 0b111111111111) >> 3) & 0b000111111111) | ((((cube >> PERM_OFFSET) & 0b111111111111) << 9) & 0b111000000000)) << PERM_OFFSET)) & CLEAR_D_CORNERS_O) | ((((cube & 0b11111111) >> 2) & 0b00111111) | (((cube & 0b11111111) << 6) & 0b11000000));
}

void Di()
{
  cube = (((cube & CLEAR_D_CORNERS) | ((((((cube >> PERM_OFFSET) & 0b111111111111) << 3) & 0b111111111000) | ((((cube >> PERM_OFFSET) & 0b111111111111) >> 9) & 0b000000000111)) << PERM_OFFSET)) & CLEAR_D_CORNERS_O) | ((((cube & 0b11111111) << 2) & 0b11111100) | (((cube & 0b11111111) >> 6) & 0b00000011));
}

void D2()
{
  cube = (cube & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) | ((((((cube >> PERM_OFFSET) & 0b111111111111) << 6) & 0b111111000000) | ((((cube >> PERM_OFFSET) & 0b111111111111) >> 6) & 0b000000111111)) << PERM_OFFSET) | ((((cube & 0b11111111) << 4) & 0b11110000) | (((cube & 0b11111111) >> 4) & 0b00001111));
}

void R()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 15) & 0b000000000000000000111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 3) & 0b000111000000000111000000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) << 2) & 0b0011000000110000) | (((cube & R_CORNER_O_MASK) >> 10) & 0b0000000000001100) | (((cube & R_CORNER_O_MASK) << 6) & 0b0000110000000000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) | ((((cube >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << URF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UBR_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DRB_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DFR_OFFSET_O);
}

void Ri()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 9) & 0b000000000000000111000000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 3) & 0b000000111000000000111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) >> 2) & 0b0000110000001100) | (((cube & R_CORNER_O_MASK) << 10) & 0b0011000000000000) | (((cube & R_CORNER_O_MASK) >> 6) & 0b0000000000110000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) | ((((cube >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << URF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UBR_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DRB_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DFR_OFFSET_O);
}

void R2()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 12) & 0b000000000000000111111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 12) & 0b000111111000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) << 8) & 0b0011110000000000) | (((cube & R_CORNER_O_MASK) >> 8) & 0b0000000000111100));
}

void L()
{
  cube = ((cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 9) & 0b000000000111000000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 3) & 0b000000000000111000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) >> 6) & 0b0000001100000011) | (((cube & L_CORNER_O_MASK) >> 2) & 0b0000000011000000) | (((cube & L_CORNER_O_MASK) << 14) & 0b1100000000000000)));
  cube = (cube & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((cube >> ULB_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << ULB_OFFSET_O) |
         ((((cube >> UFL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UFL_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DLF_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DBL_OFFSET_O);
}

void Li()
{
  cube = (cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 9) & 0b111000000000111000000000) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 3) & 0b000000000111000000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) << 6) & 0b1100000011000000) | (((cube & L_CORNER_O_MASK) << 2) & 0b0000001100000000) | (((cube & L_CORNER_O_MASK) >> 14) & 0b0000000000000011));
  cube = (cube & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((cube >> ULB_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << ULB_OFFSET_O) |
         ((((cube >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << UFL_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                 : ORIENT1)
          << DLF_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << DBL_OFFSET_O);
}

void L2()
{
  cube = (cube & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 12) & 0b000000000000111000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 12) & 0b111000000111000000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) >> 8) & 0b0000000011000011) | (((cube & L_CORNER_O_MASK) << 8) & 0b1100001100000000));
}

void F()
{
  cube = ((cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 3) & 0b000000111111111000000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 9) & 0b000000000000000111000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) << 2) & 0b0000111111000000) | (((cube & F_CORNER_O_MASK) >> 6) & 0b0000000000110000)));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) | ((((cube >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << UFL_OFFSET_O) |
         ((((cube >> URF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << URF_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DFR_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DLF_OFFSET_O);
}

void Fi()
{
  cube = ((cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 3) & 0b000000000111111111000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) >> 2) & 0b0000001111110000) | (((cube & F_CORNER_O_MASK) << 6) & 0b0000110000000000)));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) | ((((cube >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << UFL_OFFSET_O) |
         ((((cube >> URF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << URF_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DFR_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DLF_OFFSET_O);
}

void F2()
{
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 6) & 0b000000000000111111000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 6) & 0b000000111111000000000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) >> 4) & 0b0000000011110000) | (((cube & F_CORNER_O_MASK) << 4) & 0b00111100000000));
}

void B()
{
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 3) & 0b111000000000000000111000) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) << 2) & 0b1100000000001100) | (((cube & B_CORNER_O_MASK) >> 14) & 0b0000000000000011) | (((cube & B_CORNER_O_MASK) << 10) & 0b0011000000000000));
  cube = (cube & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((cube >> UBR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << UBR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << ULB_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DBL_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DRB_OFFSET_O);
}

void Bi()
{
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 3) & 0b000111000000000000000111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 15) & 0b000000000000000000111000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) >> 2) & 0b0011000000000011) | (((cube & B_CORNER_O_MASK) << 14) & 0b1100000000000000) | (((cube & B_CORNER_O_MASK) >> 10) & 0b0000000000001100));
  cube = (cube & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((cube >> UBR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                  : ORIENT1)
                                                                           << UBR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << ULB_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DBL_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DRB_OFFSET_O);
}

void B2()
{
  cube = (cube & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 18) & 0b000000000000000000111111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 18) & 0b111111000000000000000000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) >> 12) & 0b0000000000001111) | (((cube & B_CORNER_O_MASK) << 12) & 0b11110000000000));
}

void y()
{
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 3) & 0b000111111111) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 9) & 0b111000000000)) << (PERM_OFFSET + 12)) | ((((((cube >> 8) & 0b11111111) >> 2) & 0b00111111) | ((((cube >> 8) & 0b11111111) << 6) & 0b11000000)) << 8) | ((((((cube >> PERM_OFFSET) & 0b111111111111) << 3) & 0b111111111000) | ((((cube >> PERM_OFFSET) & 0b111111111111) >> 9) & 0b000000000111)) << PERM_OFFSET) | ((((cube & 0b11111111) << 2) & 0b11111100) | (((cube & 0b11111111) >> 6) & 0b00000011));
}

void yi()
{
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 3) & 0b111111111000) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 9) & 0b000000000111)) << (PERM_OFFSET + 12)) | ((((((cube >> 8) & 0b11111111) << 2) & 0b11111100) | ((((cube >> 8) & 0b11111111) >> 6) & 0b00000011)) << 8) | ((((((cube >> PERM_OFFSET) & 0b111111111111) >> 3) & 0b000111111111) | ((((cube >> PERM_OFFSET) & 0b111111111111) << 9) & 0b111000000000)) << PERM_OFFSET) | ((((cube & 0b11111111) >> 2) & 0b00111111) | (((cube & 0b11111111) << 6) & 0b11000000));
}

void y2()
{
  cube = (cube & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) | ((((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) << 6) & 0b111111000000) | ((((cube >> (PERM_OFFSET + 12)) & 0b111111111111) >> 6) & 0b000000111111)) << (PERM_OFFSET + 12)) | ((((((cube >> 8) & 0b11111111) << 4) & 0b11110000) | ((((cube >> 8) & 0b11111111) >> 4) & 0b00001111)) << 8) | ((((((cube >> PERM_OFFSET) & 0b111111111111) << 6) & 0b111111000000) | ((((cube >> PERM_OFFSET) & 0b111111111111) >> 6) & 0b000000111111)) << PERM_OFFSET) | ((((cube & 0b11111111) << 4) & 0b11110000) | (((cube & 0b11111111) >> 4) & 0b00001111));
}

void z()
{
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 3) & 0b000000111111111000000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 9) & 0b000000000000000111000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) << 2) & 0b0000111111000000) | (((cube & F_CORNER_O_MASK) >> 6) & 0b0000000000110000)) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 3) & 0b000111000000000000000111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 15) & 0b000000000000000000111000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) >> 2) & 0b0011000000000011) | (((cube & B_CORNER_O_MASK) << 14) & 0b1100000000000000) | (((cube & B_CORNER_O_MASK) >> 10) & 0b0000000000001100));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((cube >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                                                                          : ORIENT1)
                                                                                                                                   << UFL_OFFSET_O) |
         ((((cube >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << URF_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                 : ORIENT1)
          << DFR_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << DLF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                 : ORIENT1)
          << UBR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << ULB_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                 : ORIENT1)
          << DBL_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                 : ORIENT0)
          << DRB_OFFSET_O);
}

void zi()
{
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 3) & 0b000000000111111111000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) >> 2) & 0b0000001111110000) | (((cube & F_CORNER_O_MASK) << 6) & 0b0000110000000000)) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 3) & 0b111000000000000000111000) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) << 2) & 0b1100000000001100) | (((cube & B_CORNER_O_MASK) >> 14) & 0b0000000000000011) | (((cube & B_CORNER_O_MASK) << 10) & 0b0011000000000000));
  cube = (cube & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((cube >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                                                                          : ORIENT1)
                                                                                                                                   << UFL_OFFSET_O) |
         ((((cube >> URF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << URF_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DFR_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DLF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << UBR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << ULB_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DBL_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DRB_OFFSET_O);
}

void z2()
{
  cube = (cube & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((cube >> PERM_OFFSET) & F_CORNER_MASK) >> 6) & 0b000000000000111111000000) | ((((cube >> PERM_OFFSET) & F_CORNER_MASK) << 6) & 0b000000111111000000000000)) << PERM_OFFSET) | ((((cube & F_CORNER_O_MASK) >> 4) & 0b0000000011110000) | (((cube & F_CORNER_O_MASK) << 4) & 0b00111100000000)) | ((((((cube >> PERM_OFFSET) & B_CORNER_MASK) >> 18) & 0b000000000000000000111111) | ((((cube >> PERM_OFFSET) & B_CORNER_MASK) << 18) & 0b111111000000000000000000)) << PERM_OFFSET) | ((((cube & B_CORNER_O_MASK) >> 12) & 0b0000000000001111) | (((cube & B_CORNER_O_MASK) << 12) & 0b11110000000000));
}

void x()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 15) & 0b000000000000000000111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 3) & 0b000111000000000111000000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) << 2) & 0b0011000000110000) | (((cube & R_CORNER_O_MASK) >> 10) & 0b0000000000001100) | (((cube & R_CORNER_O_MASK) << 6) & 0b0000110000000000)) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 9) & 0b111000000000111000000000) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 3) & 0b000000000111000000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) << 6) & 0b1100000011000000) | (((cube & L_CORNER_O_MASK) << 2) & 0b0000001100000000) | (((cube & L_CORNER_O_MASK) >> 14) & 0b0000000000000011));
  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((cube >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                                                                          : ORIENT1)
                                                                                                                                   << URF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UBR_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DRB_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DFR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << ULB_OFFSET_O) |
         ((((cube >> UFL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UFL_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DLF_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DBL_OFFSET_O);
}

void xi()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 9) & 0b000000000000000111000000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 3) & 0b000000111000000000111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) >> 2) & 0b0000110000001100) | (((cube & R_CORNER_O_MASK) << 10) & 0b0011000000000000) | (((cube & R_CORNER_O_MASK) >> 6) & 0b0000000000110000)) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 9) & 0b000000000111000000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 3) & 0b000000000000111000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) >> 6) & 0b0000001100000011) | (((cube & L_CORNER_O_MASK) >> 2) & 0b0000000011000000) | (((cube & L_CORNER_O_MASK) << 14) & 0b1100000000000000));

  cube = (cube & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((cube >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((cube >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                                                                          : ORIENT1)
                                                                                                                                   << URF_OFFSET_O) |
         ((((cube >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UBR_OFFSET_O) |
         ((((cube >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DRB_OFFSET_O) |
         ((((cube >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DFR_OFFSET_O) |
         ((((cube >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << ULB_OFFSET_O) |
         ((((cube >> UFL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << UFL_OFFSET_O) |
         ((((cube >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT2
           : ((cube >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                  : ORIENT1)
          << DLF_OFFSET_O) |
         ((((cube >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT1
           : ((cube >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                  : ORIENT0)
          << DBL_OFFSET_O);
}

void x2()
{
  cube = (cube & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((cube >> PERM_OFFSET) & R_CORNER_MASK) >> 12) & 0b000000000000000111111000) | ((((cube >> PERM_OFFSET) & R_CORNER_MASK) << 12) & 0b000111111000000000000000)) << PERM_OFFSET) | ((((cube & R_CORNER_O_MASK) << 8) & 0b0011110000000000) | (((cube & R_CORNER_O_MASK) >> 8) & 0b0000000000111100)) | ((((((cube >> PERM_OFFSET) & L_CORNER_MASK) >> 12) & 0b000000000000111000000111) | ((((cube >> PERM_OFFSET) & L_CORNER_MASK) << 12) & 0b111000000111000000000000)) << PERM_OFFSET) | ((((cube & L_CORNER_O_MASK) >> 8) & 0b0000000011000011) | (((cube & L_CORNER_O_MASK) << 8) & 0b1100001100000000));
}

void writeToFile(State state, FILE *file)
{
  fprintf(file, "%" PRId64, state.cube);
  fprintf(file, " %" PRId64, state.moves);
  fprintf(file, " %d", state.depth);
  fprintf(file, " %d\n", state.canonMovesToWrite);
}

uint8_t addToCanonState(uint64_t stateToAdd)
{
  for (int i = 0; i <= canonStatesI; i++)
  {
    if (canonStates[i] == stateToAdd)
    {
      return 0;
    }
  }
  canonStates[canonStatesI++] = stateToAdd;
  return 1;
}

uint32_t setCanonicalState()
{
  if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL &&
      ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT0)
  {
    return 0;
  }
  else if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL &&
           ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT1)
  {
    x();
    yi();
    return (X_MOVE << 5) | Yi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> UFL_OFFSET) & 0b111) == UFL &&
           ((cube >> UFL_OFFSET_O) & 0b11) == ORIENT2)
  {
    y();
    xi();
    return (Y_MOVE << 5) | Xi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL &&
           ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT0)
  {
    yi();
    return Yi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL &&
           ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT1)
  {
    z();
    y2();
    return (Z_MOVE << 5) | Y2_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> ULB_OFFSET) & 0b111) == UFL &&
           ((cube >> ULB_OFFSET_O) & 0b11) == ORIENT2)
  {
    xi();
    return Xi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL &&
           ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT0)
  {
    y2();
    return Y2_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL &&
           ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT1)
  {
    yi();
    xi();
    return (Yi_MOVE << 5) | Xi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> UBR_OFFSET) & 0b111) == UFL &&
           ((cube >> UBR_OFFSET_O) & 0b11) == ORIENT2)
  {
    xi();
    zi();
    return (Xi_MOVE << 5) | Zi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL &&
           ((cube >> URF_OFFSET_O) & 0b11) == ORIENT0)
  {
    y();
    return Y_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL &&
           ((cube >> URF_OFFSET_O) & 0b11) == ORIENT1)
  {
    zi();
    return Zi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> URF_OFFSET) & 0b111) == UFL &&
           ((cube >> URF_OFFSET_O) & 0b11) == ORIENT2)
  {
    z2();
    x();
    return (Z2_MOVE << 5) | X_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL &&
           ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT0)
  {
    z2();
    y();
    return (Z2_MOVE << 5) | Y_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL &&
           ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT1)
  {
    z();
    return Z_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DLF_OFFSET) & 0b111) == UFL &&
           ((cube >> DLF_OFFSET_O) & 0b11) == ORIENT2)
  {
    x();
    return X_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL &&
           ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT0)
  {
    z2();
    return Z2_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL &&
           ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT1)
  {
    x();
    y();
    return (X_MOVE << 5) | Y_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DFR_OFFSET) & 0b111) == UFL &&
           ((cube >> DFR_OFFSET_O) & 0b11) == ORIENT2)
  {
    x();
    zi();
    return (X_MOVE << 5) | Zi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL &&
           ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT0)
  {
    x2();
    return X2_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL &&
           ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT1)
  {
    z();
    xi();
    return (Z_MOVE << 5) | Xi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DBL_OFFSET) & 0b111) == UFL &&
           ((cube >> DBL_OFFSET_O) & 0b11) == ORIENT2)
  {
    x();
    z();
    return (X_MOVE << 5) | Z_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL &&
           ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT0)
  {
    x2();
    y();
    return (X2_MOVE << 5) | Y_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL &&
           ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT1)
  {
    x2();
    zi();
    return (X2_MOVE << 5) | Zi_MOVE;
  }
  else if (((cube >> PERM_OFFSET >> DRB_OFFSET) & 0b111) == UFL &&
           ((cube >> DRB_OFFSET_O) & 0b11) == ORIENT2)
  {
    z2();
    xi();
    return (Z2_MOVE << 5) | Xi_MOVE;
  }
  return 0;
}

void updateNewState(uint64_t currentStateMoves, uint8_t currentDepth, uint32_t nextMove, FILE *file)
{
  // Store current state before canonicalization
  uint64_t tempCube = cube;
  uint32_t canonMoves = setCanonicalState();
  // printf("%" PRId64 " %" PRId64 "\n", cube, tempCube);
  if (addToCanonState(cube))
  {
    uint64_t currentMoves = ((currentStateMoves << 5) | nextMove);
    // Also encode the depth for decoding later
    State s = {cube, currentMoves, currentDepth + 1, canonMoves};
    writeToFile(s, file);
    s.cube = tempCube;
    enqueueState(s);
  }
}

void generateLookupFile(uint64_t cube, FILE *file)
{
  addToCanonState(cube);
  State state = {cube, 0, 0, 0};
  enqueueState(state);
  while (stateQueueHead < MAX_SIZE)
  {
    // Breadth first search method
    state =
        stateQueue[stateQueueHead];
    if (state.depth >= 11)
    {
      stateQueueHead++;
      continue;
    }

    cube = state.cube;
    uint64_t currentCube = cube;

    U();
    updateNewState(state.moves, state.depth, U_MOVE, file);

    cube = currentCube;
    Ui();
    updateNewState(state.moves, state.depth, Ui_MOVE, file);

    cube = currentCube;
    U2();
    updateNewState(state.moves, state.depth, U2_MOVE, file);

    cube = currentCube;
    D();
    updateNewState(state.moves, state.depth, D_MOVE, file);

    cube = currentCube;
    Di();
    updateNewState(state.moves, state.depth, Di_MOVE, file);

    cube = currentCube;
    D2();
    updateNewState(state.moves, state.depth, D2_MOVE, file);

    cube = currentCube;
    R();
    updateNewState(state.moves, state.depth, R_MOVE, file);

    cube = currentCube;
    Ri();
    updateNewState(state.moves, state.depth, Ri_MOVE, file);

    cube = currentCube;
    R2();
    updateNewState(state.moves, state.depth, R2_MOVE, file);

    cube = currentCube;
    L();
    updateNewState(state.moves, state.depth, L_MOVE, file);

    cube = currentCube;
    Li();
    updateNewState(state.moves, state.depth, Li_MOVE, file);

    cube = currentCube;
    L2();
    updateNewState(state.moves, state.depth, L2_MOVE, file);

    cube = currentCube;
    F();
    updateNewState(state.moves, state.depth, F_MOVE, file);

    cube = currentCube;
    Fi();
    updateNewState(state.moves, state.depth, Fi_MOVE, file);

    cube = currentCube;
    F2();
    updateNewState(state.moves, state.depth, F2_MOVE, file);

    cube = currentCube;
    B();
    updateNewState(state.moves, state.depth, B_MOVE, file);

    cube = currentCube;
    Bi();
    updateNewState(state.moves, state.depth, Bi_MOVE, file);

    cube = currentCube;
    B2();
    updateNewState(state.moves, state.depth, B2_MOVE, file);
    stateQueueHead++;
  }
}

int main()
{
  // printf("Hello, World! %"PRId64"\n", cube);
  // struct timespec startT;
  // clock_gettime(CLOCK_MONOTONIC, &startT);

  // struct timespec endT;
  // clock_gettime(CLOCK_MONOTONIC, &endT);

  // int64_t elapsed = (int64_t)(endT.tv_sec - startT.tv_sec) * 1000000000LL + (endT.tv_nsec - startT.tv_nsec);
  // printf("%ld ns\n", elapsed);

  U();
  Ui();
  U2();
  D();
  Di();
  D2();
  R();
  Ri();
  R2();
  printf("%" PRId64 "\n", cube);

  // FILE *file = fopen("lookupFile.txt", "w"); // Open for writing

  // if (file == NULL)
  // {
  //   printf("Couldn't create file!\n");
  //   return 1;
  // }
  // // Store solved cube
  // State state = {cube, 0, 0, 0};
  // writeToFile(state, file);
  // generateLookupFile(cube, file);
  // fclose(file);

  return 0;
}