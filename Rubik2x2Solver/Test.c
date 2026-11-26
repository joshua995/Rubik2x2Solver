#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <windows.h>

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
uint8_t ULB_OFFSET = 21; // * 3
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

uint32_t R_CORNER_MASK = 0b000'111'111'000'000'111'111'000;
uint32_t L_CORNER_MASK = 0b111'000'000'111'111'000'000'111;
uint32_t F_CORNER_MASK = 0b000'000'111'111'111'111'000'000;
uint32_t B_CORNER_MASK = 0b111'111'000'000'000'000'111'111;

uint32_t R_CORNER_O_MASK = 0b00'11'11'00'00'11'11'00;
uint32_t L_CORNER_O_MASK = 0b11'00'00'11'11'00'00'11;
uint32_t F_CORNER_O_MASK = 0b00'00'11'11'11'11'00'00;
uint32_t B_CORNER_O_MASK = 0b11'11'00'00'00'00'11'11;

uint64_t CLEAR_PERM = 0b000'000'000'000'000'000'000'000'11'11'11'11'11'11'11'11l;

uint64_t CLEAR_U_CORNERS = 0b000'000'000'000'111'111'111'111'11'11'11'11'11'11'11'11l;
uint64_t CLEAR_D_CORNERS = 0b111'111'111'111'000'000'000'000'11'11'11'11'11'11'11'11l;
uint64_t CLEAR_R_CORNERS = 0b111'000'000'111'111'000'000'111'11'11'11'11'11'11'11'11l;
uint64_t CLEAR_L_CORNERS = 0b000'111'111'000'000'111'111'000'11'11'11'11'11'11'11'11l;
uint64_t CLEAR_F_CORNERS = 0b111'111'000'000'000'000'111'111'11'11'11'11'11'11'11'11l;
uint64_t CLEAR_B_CORNERS = 0b000'000'111'111'111'111'000'000'11'11'11'11'11'11'11'11l;

uint64_t CLEAR_ORIENT = 0b111'111'111'111'111'111'111'111'00'00'00'00'00'00'00'00l;

uint64_t CLEAR_U_CORNERS_O = 0b111'111'111'111'111'111'111'111'00'00'00'00'11'11'11'11l;
uint64_t CLEAR_D_CORNERS_O = 0b111'111'111'111'111'111'111'111'11'11'11'11'00'00'00'00l;
uint64_t CLEAR_R_CORNERS_O = 0b111'111'111'111'111'111'111'111'11'00'00'11'11'00'00'11l;
uint64_t CLEAR_L_CORNERS_O = 0b111'111'111'111'111'111'111'111'00'11'11'00'00'11'11'00l;
uint64_t CLEAR_F_CORNERS_O = 0b111'111'111'111'111'111'111'111'11'11'00'00'00'00'11'11l;
uint64_t CLEAR_B_CORNERS_O = 0b111'111'111'111'111'111'111'111'00'00'11'11'11'11'00'00l;

uint64_t CLEAR_ULB_O = 0b111'111'111'111'111'111'111'111'00'11'11'11'11'11'11'11l;
uint64_t CLEAR_UBR_O = 0b111'111'111'111'111'111'111'111'11'00'11'11'11'11'11'11l;
uint64_t CLEAR_URF_O = 0b111'111'111'111'111'111'111'111'11'11'00'11'11'11'11'11l;
uint64_t CLEAR_UFL_O = 0b111'111'111'111'111'111'111'111'11'11'11'00'11'11'11'11l;
uint64_t CLEAR_DLF_O = 0b111'111'111'111'111'111'111'111'11'11'11'11'00'11'11'11l;
uint64_t CLEAR_DFR_O = 0b111'111'111'111'111'111'111'111'11'11'11'11'11'00'11'11l;
uint64_t CLEAR_DRB_O = 0b111'111'111'111'111'111'111'111'11'11'11'11'11'11'00'11l;
uint64_t CLEAR_DBL_O = 0b111'111'111'111'111'111'111'111'11'11'11'11'11'11'11'00l;

uint64_t state = 0b000'001'010'011'100'101'110'111'00'00'00'00'00'00'00'00;

void U();

int main() {
	//printf("Hello, World! %"PRId64"\n", state);
	printf("Hello, World! %x\n", state);
    LARGE_INTEGER startT, endT, frequency;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startT);

    // Your code here
    for (int i = 0;i < 33'000'000;i++) {
        U();
    }

    QueryPerformanceCounter(&endT);

    int64_t elapsed = (int64_t)((endT.QuadPart - startT.QuadPart)
        * 1000000000LL / frequency.QuadPart);

    printf("%lld ns\n", elapsed);
	return 0;
}

void U() {
    state = (((state & CLEAR_U_CORNERS)
        | ((((((state >> (PERM_OFFSET + 12)) & 0b111'111'111'111) >> 3) & 0b000'111'111'111)
            | ((((state >> (PERM_OFFSET + 12)) & 0b111'111'111'111) << 9)
                & 0b111'000'000'000)) << (PERM_OFFSET + 12)))
        & CLEAR_U_CORNERS_O)
        | ((((((state >> 8) & 0b11'11'11'11) >> 2) & 0b00'11'11'11)
            | ((((state >> 8) & 0b11'11'11'11) << 6) & 0b11'00'00'00)) << 8);
}