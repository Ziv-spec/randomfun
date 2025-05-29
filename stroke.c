#include <stdio.h>

// The stroke program
static unsigned short program[] = {
	0x7000, 
	0x7c90,
	0x9026,
	0x0009,
	0x902c,
	0x0053, 
	0x902e,
	0x00a9,
	0x902c,
	0x0103,
	0x0803,
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

int main() { 


	if (program[0] != 0x7000) {
		printf("not format\n");
		return -1;
	}

	unsigned short *opcode = program + 1;
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
				printf("rectangle\n");
		  } break;

			case 0x7000: {
				printf("second format is illegal\n");
			} return -1;

			case 0x7800: {
				printf("LCI - Line color input\n"); 
				if ((*opcode & 0x0400) == 0)
					format = 1;
				else format = 2;
				// LineColor; 
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
			  printf("vector\n");

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




