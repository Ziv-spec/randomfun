#include <stdlib.h>
#include <stdio.h>

static unsigned short program1[] = {
	//
	// Instructions
	//

	0x7000, // format 
	0x7CF0, // colorline
	0x3822, // rectangle
	0x7C70, // colorline
	0x9002, 0x0024, // vector
	0x9002, 0x0026, // vector
	0x9002, 0x0028, // vector
	0x9002, 0x002A, // vector
	0x9002, 0x002C, // vector
	0x9002, 0x002E, // vector
	0x9002, 0x0030, // vector
	0x9002, 0x0032, // vector
	0x7CD0, // colorline
	0x9808, 0x0033, // circle
	0x7CE0, // colorline
	0x9808, 0x0033, // circle
	0x7CC0, // colorline  
	0x8904, 0x0033, // alpha
	0x7CA0, // colorline
	0x8914, 0x0035, // alpha
	0x7C90, // colorline
	0x8914, 0x003F, // alpha
	0x0803, // halt/end

	//
	// Data
	// 

	0x0000, 
	0x0000, 
	0x0C80, 
	0x0C80, 
	0x7CA5, 
	0x0640, 
	0xFCA5, 
	0x79C1, 
	0x035C, 
	0x0640, 
	0x835C, 
	0x79C1, 
	0x79C1, 
	0x035C, 
	0x8640, 
	0x035C, 
	0x79C1, 
	0x0000, 
	0xFE53, 
	0x0000,
	0x01AE, 
	0x0000,
	0x8640, 
	0x0000,
	0x79C1, 
	0x7CA5,
	0x8640,
	0x7CA5,
	0x0000,
	0x0640,
	0x8000,
	0x01AE,
	0x0000,
	0x7E53,
	0x8000,
	0x79C1,
	0x0000,
	0x0000,
	0x0640,
	0x0000,
	0x0000,
	0x0350,
	0x1F70,
	0x0000,
	0x0000,
	0x1405,
	0x1314,
	0x1D78,
	0x06A4,
	0x0000,
	0x5354,
	0x4943,
	0x4B00,
	0x464F,
	0x5243,
	0x4500,
	0x2D00,
	0x7271,
	0x2E74,
	0x7000,
	0x1D78,
	0x195C,
	0x0000,
	0x4655,
	0x454C,
	0x0051,
	0x5459,
	0x002D,
	0x0076,
	0x7070,
	0x7000,
	0x4C42,
	0x5300
};

// The stroke program
static unsigned short program[] = {
	// Instructions
	0x7000, 
	0x7c90,
	0x9026, 0x0009,
	0x902c, 0x0053, 
	0x902e, 0x00a9,
	0x902c, 0x0103,
	0x0803,
	// Data 
	0x7eab,
	0x0150,
	0x8155,
	0x0150,
	0x0155,
	0x0148,
	0xFeab,
	0x0148,
	0x7eab, 
	0x0140,
	0x8155,
	0x0140,
	0x0155,
	0x0138
};

static void 
st_rect(short *code) {

	// offset to data
	short offset = *code & 0x7ff; 
	code = code + offset; 

	short xcenter = *code; 
	code = code + 1;
	short ycenter = *code; 
	code = code + 1;
	short xlen = *code;
	code = code + 1;
	short ylen = *code;

	printf("rectangle %d %d %d %d\n", xcenter, ycenter, xlen, ylen);
}

static char *colors[16] = {
	[0] = "color-off",
	[1] = "DIM BLUE",
	[2] = "DIM GREEN",
	[3] = "DIM CYAN",
	[4] = "DIM RED",
	[5] = "DIM MAGENTA",
	[6] = "DIM AMBER",
	[7] = "DIM WHITE",
	[8] = "ORANGE",
	[9] = "BLUE",
	[10] = "GREEN",
	[11] = "CYAN",
	[12] = "RED",
	[13] = "MAGENTA",
	[14] = "AMBER",
	[15] = "WHITE",
};

static void 
st_linecolor(short *code, int format) {

	short offset = *code & 0x03ff;
	code = code + offset;

	if (*code == 0x0) return;

	short rgb = 0;
	if (format == 1) {

		if ((*code & 0x400) == 0) { 
			rgb = (*code >> 12) & 0xf;
		}
	}
	else { 

		if ((*code & 0x004) == 0) {
			rgb = (*code >> 4) & 0xf; 
		}
	}

	printf("colorline: %s\n", colors[rgb]);
}

static void 
st_vector(short *code) { 
	printf("vector: ");

	short xy_data_words = *code & 0x01ff;

	code = code + 1;
	if ((*code & 0x800) == 0x8000) return; // flag should not beset
	code = code + (*code & 0x07ff) -1; // data offset
	
	printf("data words: %d ", xy_data_words);
	for (int i = 0; i < xy_data_words; i++) {
		short xvec = *code; code = code + 1;
		short yvec = *code; code = code + 1;
		
		// for some reason it seems first point is normal
		// second point has something called "bright up" essentially 
		// an or with 0x8000 for a "bright up" on the xvec

		/*
		xvec = ((xvec & 0x8fff) == xvec) ? xvec & 0x0fff: 0x8000 - xvec;
		yvec = ((yvec & 0x8fff) == yvec) ? yvec & 0x0fff: 0x8000 - yvec;
		printf("(%d %d) ", xvec, yvec);
		*/
		printf("(%x %x) ", xvec, yvec);
		

	}
	printf("\n");

}


int main() { 


	if (program1[0] != 0x7000) {
		printf("not format\n");
		return -1;
	}

	unsigned short *opcode = program1 + 1;
	unsigned short code = *opcode & 0xf800;

	int format = 0;

	while (code != 0x0800) {

		switch (code) {
			case 0x0000: { // NOP
				opcode = opcode + 1;
			} break;

			case 0x0800: { //terminator
				printf("halt\n");
		  } return 0;

			case 0x3800: {
				st_rect(opcode);
				opcode = opcode + 1;
		  } break;

			case 0x7000: {
				printf("second format is illegal\n");
			} return -1;

			case 0x7800: {
				format = ((*opcode & 0x0400) == 0) ? 1 : 2;

				st_linecolor(opcode, format);

				if (format == 1) {
					opcode = opcode + 2;
				}
				else { // format 2
					opcode = opcode + 1;
				}
		  }	break;

			case 0x8000: {
				printf("line structure\n");
				opcode = opcode + 2;
		  } break;

			case 0x8800: {
				printf("alpha\n"); 
				opcode = opcode + 2;
		  } break;
			 
			case 0x9000: { 
			  st_vector(opcode);
			  opcode = opcode + 2;
		  } break;

			case 0x9800: { 
				printf("conic\n"); 
				opcode = opcode + 2;
	 	  } break;
			default: { printf("what is going on here?\n"); return 0; }
		}

		code = *opcode & 0xf800;
	}

	return 0;
}




