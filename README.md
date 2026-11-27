**cube.c**<br/>
Generates a look up file for a 2x2 Rubik's cube using permutation and orientation to store the cube state.<br/>
Cube stored in a uint64_t with the corners: ULB UBR URF UFL DLF DFR DRB DBL for the permutation (3 bits each) followed by orientation (2 bits each).<br/>

