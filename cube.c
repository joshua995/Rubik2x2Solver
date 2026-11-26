// gcc -O3 -march=native test.c -o test

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

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

uint64_t state = 0b0000010100111001011101110000000000000000;

uint64_t canonStates[3674160];
uint32_t canonStatesI = 0;

typedef struct StateToWrite
{
  uint64_t stateToWrite;
  uint64_t movesToWrite;
} StateToWrite;

void U()
{
  state = (((state & CLEAR_U_CORNERS) | ((((((state >> (PERM_OFFSET + 12)) & 0b111111111111) >> 3) & 0b000111111111) | ((((state >> (PERM_OFFSET + 12)) & 0b111111111111) << 9) & 0b111000000000)) << (PERM_OFFSET + 12))) & CLEAR_U_CORNERS_O) | ((((((state >> 8) & 0b11111111) >> 2) & 0b00111111) | ((((state >> 8) & 0b11111111) << 6) & 0b11000000)) << 8);
}

void Ui()
{
  state = (((state & CLEAR_U_CORNERS) | ((((((state >> (PERM_OFFSET + 12)) & 0b111111111111) << 3) & 0b111111111000) | ((((state >> (PERM_OFFSET + 12)) & 0b111111111111) >> 9) & 0b000000000111)) << (PERM_OFFSET + 12))) & CLEAR_U_CORNERS_O) | ((((((state >> 8) & 0b11111111) << 2) & 0b11111100) | ((((state >> 8) & 0b11111111) >> 6) & 0b00000011)) << 8);
}

void U2()
{
  state = (state & CLEAR_U_CORNERS & CLEAR_U_CORNERS_O) | ((((((state >> (PERM_OFFSET + 12)) & 0b111111111111) << 6) & 0b111111000000) | ((((state >> (PERM_OFFSET + 12)) & 0b111111111111) >> 6) & 0b000000111111)) << (PERM_OFFSET + 12)) | ((((((state >> 8) & 0b11111111) << 4) & 0b11110000) | ((((state >> 8) & 0b11111111) >> 4) & 0b00001111)) << 8);
}

void D()
{
  state = (((state & CLEAR_D_CORNERS) | ((((((state >> PERM_OFFSET) & 0b111111111111) >> 3) & 0b000111111111) | ((((state >> PERM_OFFSET) & 0b111111111111) << 9) & 0b111000000000)) << PERM_OFFSET)) & CLEAR_D_CORNERS_O) | ((((state & 0b11111111) >> 2) & 0b00111111) | (((state & 0b11111111) << 6) & 0b11000000));
}

void Di()
{
  state = (((state & CLEAR_D_CORNERS) | ((((((state >> PERM_OFFSET) & 0b111111111111) << 3) & 0b111111111000) | ((((state >> PERM_OFFSET) & 0b111111111111) >> 9) & 0b000000000111)) << PERM_OFFSET)) & CLEAR_D_CORNERS_O) | ((((state & 0b11111111) << 2) & 0b11111100) | (((state & 0b11111111) >> 6) & 0b00000011));
}

void D2()
{
  state = (state & CLEAR_D_CORNERS & CLEAR_D_CORNERS_O) | ((((((state >> PERM_OFFSET) & 0b111111111111) << 6) & 0b111111000000) | ((((state >> PERM_OFFSET) & 0b111111111111) >> 6) & 0b000000111111)) << PERM_OFFSET) | ((((state & 0b11111111) << 4) & 0b11110000) | (((state & 0b11111111) >> 4) & 0b00001111));
}

void R()
{
  state = (state & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((state >> PERM_OFFSET) & R_CORNER_MASK) >> 15) & 0b000000000000000000111000) | ((((state >> PERM_OFFSET) & R_CORNER_MASK) << 3) & 0b000111000000000111000000) | ((((state >> PERM_OFFSET) & R_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((state & R_CORNER_O_MASK) << 2) & 0b0011000000110000) | (((state & R_CORNER_O_MASK) >> 10) & 0b0000000000001100) | (((state & R_CORNER_O_MASK) << 6) & 0b0000110000000000));

  state = (state & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) | ((((state >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << URF_OFFSET_O) |
          ((((state >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << UBR_OFFSET_O) |
          ((((state >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DRB_OFFSET_O) |
          ((((state >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DFR_OFFSET_O);
}

void Ri()
{
  state = (state & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((state >> PERM_OFFSET) & R_CORNER_MASK) >> 9) & 0b000000000000000111000000) | ((((state >> PERM_OFFSET) & R_CORNER_MASK) >> 3) & 0b000000111000000000111000) | ((((state >> PERM_OFFSET) & R_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((state & R_CORNER_O_MASK) >> 2) & 0b0000110000001100) | (((state & R_CORNER_O_MASK) << 10) & 0b0011000000000000) | (((state & R_CORNER_O_MASK) >> 6) & 0b0000000000110000));

  state = (state & CLEAR_URF_O & CLEAR_UBR_O & CLEAR_DRB_O & CLEAR_DFR_O) | ((((state >> URF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << URF_OFFSET_O) |
          ((((state >> UBR_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << UBR_OFFSET_O) |
          ((((state >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DRB_OFFSET_O) |
          ((((state >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DFR_OFFSET_O);
}

void R2()
{
  state = (state & CLEAR_R_CORNERS & CLEAR_R_CORNERS_O) | ((((((state >> PERM_OFFSET) & R_CORNER_MASK) >> 12) & 0b000000000000000111111000) | ((((state >> PERM_OFFSET) & R_CORNER_MASK) << 12) & 0b000111111000000000000000)) << PERM_OFFSET) | ((((state & R_CORNER_O_MASK) << 8) & 0b0011110000000000) | (((state & R_CORNER_O_MASK) >> 8) & 0b0000000000111100));
}

void L()
{
  state = ((state & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((state >> PERM_OFFSET) & L_CORNER_MASK) >> 9) & 0b000000000111000000000111) | ((((state >> PERM_OFFSET) & L_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((state >> PERM_OFFSET) & L_CORNER_MASK) >> 3) & 0b000000000000111000000000)) << PERM_OFFSET) | ((((state & L_CORNER_O_MASK) >> 6) & 0b0000001100000011) | (((state & L_CORNER_O_MASK) >> 2) & 0b0000000011000000) | (((state & L_CORNER_O_MASK) << 14) & 0b1100000000000000)));
  state = (state & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((state >> ULB_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << ULB_OFFSET_O) |
          ((((state >> UFL_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << UFL_OFFSET_O) |
          ((((state >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DLF_OFFSET_O) |
          ((((state >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DBL_OFFSET_O);
}

void Li()
{
  state = (state & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((state >> PERM_OFFSET) & L_CORNER_MASK) << 9) & 0b111000000000111000000000) | ((((state >> PERM_OFFSET) & L_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((state >> PERM_OFFSET) & L_CORNER_MASK) << 3) & 0b000000000111000000000000)) << PERM_OFFSET) | ((((state & L_CORNER_O_MASK) << 6) & 0b1100000011000000) | (((state & L_CORNER_O_MASK) << 2) & 0b0000001100000000) | (((state & L_CORNER_O_MASK) >> 14) & 0b0000000000000011));
  state = (state & CLEAR_ULB_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DBL_O) | ((((state >> ULB_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << ULB_OFFSET_O) |
          ((((state >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((state >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                    : ORIENT0)
           << UFL_OFFSET_O) |
          ((((state >> DLF_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                    : ORIENT1)
           << DLF_OFFSET_O) |
          ((((state >> DBL_OFFSET_O) & 0b11) == 0 ? ORIENT1 : ((state >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                                                                    : ORIENT0)
           << DBL_OFFSET_O);
}

void L2()
{
  state = (state & CLEAR_L_CORNERS & CLEAR_L_CORNERS_O) | ((((((state >> PERM_OFFSET) & L_CORNER_MASK) >> 12) & 0b000000000000111000000111) | ((((state >> PERM_OFFSET) & L_CORNER_MASK) << 12) & 0b111000000111000000000000)) << PERM_OFFSET) | ((((state & L_CORNER_O_MASK) >> 8) & 0b0000000011000011) | (((state & L_CORNER_O_MASK) << 8) & 0b1100001100000000));
}

void F()
{
  state = ((state & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((state >> PERM_OFFSET) & F_CORNER_MASK) << 3) & 0b000000111111111000000000) | ((((state >> PERM_OFFSET) & F_CORNER_MASK) >> 9) & 0b000000000000000111000000)) << PERM_OFFSET) | ((((state & F_CORNER_O_MASK) << 2) & 0b0000111111000000) | (((state & F_CORNER_O_MASK) >> 6) & 0b0000000000110000)));
  state = (state & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) | ((((state >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << UFL_OFFSET_O) |
          ((((state >> URF_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << URF_OFFSET_O) |
          ((((state >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DFR_OFFSET_O) |
          ((((state >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DLF_OFFSET_O);
}

void Fi()
{
  state = ((state & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((state >> PERM_OFFSET) & F_CORNER_MASK) >> 3) & 0b000000000111111111000000) | ((((state >> PERM_OFFSET) & F_CORNER_MASK) << 9) & 0b000000111000000000000000)) << PERM_OFFSET) | ((((state & F_CORNER_O_MASK) >> 2) & 0b0000001111110000) | (((state & F_CORNER_O_MASK) << 6) & 0b0000110000000000)));
  state = (state & CLEAR_URF_O & CLEAR_UFL_O & CLEAR_DLF_O & CLEAR_DFR_O) | ((((state >> UFL_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> UFL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << UFL_OFFSET_O) |
          ((((state >> URF_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> URF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << URF_OFFSET_O) |
          ((((state >> DFR_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DFR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DFR_OFFSET_O) |
          ((((state >> DLF_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DLF_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DLF_OFFSET_O);
}

void F2()
{
  state = (state & CLEAR_F_CORNERS & CLEAR_F_CORNERS_O) | ((((((state >> PERM_OFFSET) & F_CORNER_MASK) >> 6) & 0b000000000000111111000000) | ((((state >> PERM_OFFSET) & F_CORNER_MASK) << 6) & 0b000000111111000000000000)) << PERM_OFFSET) | ((((state & F_CORNER_O_MASK) >> 4) & 0b0000000011110000) | (((state & F_CORNER_O_MASK) << 4) & 0b00111100000000));
}

void B()
{
  state = (state & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((state >> PERM_OFFSET) & B_CORNER_MASK) << 3) & 0b111000000000000000111000) | ((((state >> PERM_OFFSET) & B_CORNER_MASK) >> 21) & 0b000000000000000000000111) | ((((state >> PERM_OFFSET) & B_CORNER_MASK) << 15) & 0b000111000000000000000000)) << PERM_OFFSET) | ((((state & B_CORNER_O_MASK) << 2) & 0b1100000000001100) | (((state & B_CORNER_O_MASK) >> 14) & 0b0000000000000011) | (((state & B_CORNER_O_MASK) << 10) & 0b0011000000000000));
  state = (state & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((state >> UBR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << UBR_OFFSET_O) |
          ((((state >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << ULB_OFFSET_O) |
          ((((state >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DBL_OFFSET_O) |
          ((((state >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DRB_OFFSET_O);
}

void Bi()
{
  state = (state & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((state >> PERM_OFFSET) & B_CORNER_MASK) >> 3) & 0b000111000000000000000111) | ((((state >> PERM_OFFSET) & B_CORNER_MASK) << 21) & 0b111000000000000000000000) | ((((state >> PERM_OFFSET) & B_CORNER_MASK) >> 15) & 0b000000000000000000111000)) << PERM_OFFSET) | ((((state & B_CORNER_O_MASK) >> 2) & 0b0011000000000011) | (((state & B_CORNER_O_MASK) << 14) & 0b1100000000000000) | (((state & B_CORNER_O_MASK) >> 10) & 0b0000000000001100));
  state = (state & CLEAR_ULB_O & CLEAR_UBR_O & CLEAR_DBL_O & CLEAR_DRB_O) | ((((state >> UBR_OFFSET_O) & 0b11) == 0 ? ORIENT2 : ((state >> UBR_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                                                                                                                                      : ORIENT1)
                                                                             << UBR_OFFSET_O) |
          ((((state >> ULB_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> ULB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << ULB_OFFSET_O) |
          ((((state >> DBL_OFFSET_O) & 0b11) == 0   ? ORIENT2
            : ((state >> DBL_OFFSET_O) & 0b11) == 1 ? ORIENT0
                                                    : ORIENT1)
           << DBL_OFFSET_O) |
          ((((state >> DRB_OFFSET_O) & 0b11) == 0   ? ORIENT1
            : ((state >> DRB_OFFSET_O) & 0b11) == 1 ? ORIENT2
                                                    : ORIENT0)
           << DRB_OFFSET_O);
}

void B2()
{
  state = (state & CLEAR_B_CORNERS & CLEAR_B_CORNERS_O) | ((((((state >> PERM_OFFSET) & B_CORNER_MASK) >> 18) & 0b000000000000000000111111) | ((((state >> PERM_OFFSET) & B_CORNER_MASK) << 18) & 0b111111000000000000000000)) << PERM_OFFSET) | ((((state & B_CORNER_O_MASK) >> 12) & 0b0000000000001111) | (((state & B_CORNER_O_MASK) << 12) & 0b11110000000000));
}

void writeToFile(StateToWrite stateToWrite, FILE *file)
{
  fprintf(file, "%" PRId64, stateToWrite.stateToWrite);
  fprintf(file, " %" PRId64 "\n", stateToWrite.movesToWrite);
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

void generateLookupFile(uint64_t state, FILE *file)
{
}
int main()
{
  // printf("Hello, World! %"PRId64"\n", state);
  // struct timespec startT;
  // clock_gettime(CLOCK_MONOTONIC, &startT);

  // // Your code here
  // for (int i = 0; i < 33000000; i++)
  // {
  //   U();
  //   Ui();
  //   U2();
  //   D();
  //   Di();
  //   D2();
  //   R();
  //   Ri();
  //   R2();
  //   L();
  //   Li();
  //   L2();
  //   F();
  //   Fi();
  //   F2();
  //   B();
  //   Bi();
  //   B2();
  //   // 67443633 linux ns
  // }

  // struct timespec endT;
  // clock_gettime(CLOCK_MONOTONIC, &endT);

  // int64_t elapsed = (int64_t)(endT.tv_sec - startT.tv_sec) * 1000000000LL + (endT.tv_nsec - startT.tv_nsec);
  // printf("%ld ns\n", elapsed);
  FILE *file = fopen("lookupFile.txt", "w"); // Open for writing

  if (file == NULL)
  {
    printf("Couldn't create file!\n");
    return 1;
  }
  // Store solved state
  StateToWrite stateToWrite = {state, 0b00};
  writeToFile(stateToWrite, file);
  generateLookupFile(state, file);
  fclose(file);

  return 0;
}