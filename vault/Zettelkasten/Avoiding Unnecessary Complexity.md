202308262052
Status: #idea
Tags: #story

# Avoiding Unnecessary Complexity

I am currently working on my first big "real" project which is a compiler. This will be a short story of some progress I made towards removing unnecessary complexity that was in the old system of code generation. 

Hopefully the lessons learned here will be valuable for people to avoid unnecessary complexity in cases where handling a too generic case would be worse in terms of complexity and the effort. 

For my case what I wanted to do is implement a encoder for the x64 instruction set. The encoder would need to take a instruction and it's operands, and output to a buffer the correct machine code for that given instruction. 

The first implementation was me trying to be as general as possible. I ended up with this implementation because I didn't have a real world use case beyond small snippets to show it works. I will illustrate this point later with the example itself. 

This is the general pattern that I came up with. 


```c
// new x64.c

struct  Instruction_Desc { 
	u8 op;  // opcode
	// for immediates
	u8 op_i;
	u8 reg_i;
}; 

enum Inst_Kind {
	ADD, OR, AND, SUB, XOR, CMP
}; 

static Instruction_Desc inst_table[] = {
	[ADD] = {0x00, 0x80, 0x0},
	[OR]  = {0x08, 0x80, 0x1},
	[AND] = {0x20, 0x80, 0x4},
	[SUB] = {0x28, 0x80, 0x5},
	[XOR] = {0x30, 0x80, 0x6},
	[CMP] = {0x38, 0x80, 0x7}
};

static void 
inst_unary(Builder *b, Inst_Kind op, Operand a) {
	//...
}

static void 
inst_binary(Builder *b, Inst_Kind op, Operand a, Operand b) {
	Instruction_Desc *inst = &inst_table[op];
	//...
	Data_Type dt = a.dt;	
	//...
}

static void 
inst_add(Builder *b, Inst_Kind op, Operand a, Operand b) {
	//...
	inst_binary(b, op, a, b); 
	//...
}

```




A table with certain properties you have to insert, along with a single function that takes that table and operands and encodes it into a single machine code instruction. 

```c
// old x64.c

struct Instruction {
	u8 kind;
	u8 opcode;
	u8 opcode_reg;      // special reg value usually for immediates
	u8 operand_kind[2];
}; 

// instruction tables
static Instruction add[] = {
	{ OPERAND, .opcode = 0x00, .operand_kind = { M |B8, R |B8 } },
	{ OPERAND, .opcode = 0x01, .operand_kind = { M |B16|B32|B64, R|B16|B32|B64 } }, 
	{ OPERAND, .opcode = 0x02, .operand_kind = { R |B8, M |B8 } },
	{ OPERAND, .opcode = 0x03, .operand_kind = { R |B16|B32|B64, M B16|B32|B64 } },
	//...
}; 

//... 

static void 
encode(Builder *b, Instruction *inst, Operand to, Operand from) {
	//...
}

```

Since it is so general you can mix and match all the different parameters in many ways. Adding instructions should be as "simple" as adding the correct rows into a instruction's table and done. Unfortunately this approached proved both too generic to be simple, took way more time to build than the simpler approach, made it easier to make mistakes when adding instructions, and ended up with more lines. 

The main culprit was the generality of the function and lack of understanding of the problem. This along the assumptions and effort that could be passed along to the programmer leading to lesser complexity. 

```c
// new x64.c

struct  Instruction_Desc { 
	const char *mnemonic; 
	u8 cat; // category
	u8 op;  // opcode
	// for immediates
	u8 op_i;
	u8 reg_i;
}; 

enum Instruction_Category {
	INST_UNARY,
	INST_UNARY_EXT,
	INST_BINARY, 
	INST_BINARY_EXT, 
	//...
};

#define INSTRUCTIONS_TABLE \
	X(ADD,   "add",   BINARY,  0x00,  0x80,  0x0) \
	X(OR,    "or",    BINARY,  0x08,  0x80,  0x1) \
	X(AND,   "and",   BINARY,  0x20,  0x80,  0x4) \
	X(SUB,   "sub",   BINARY,  0x28,  0x80,  0x5)

enum Inst_Kind {
#define X(a, ...) a,
INSTRUCTIONS_TABLE
#undef X 
}; 

static Instruction_Desc inst_table[] = {
	#define X(a, b, c, ...) [a] = { .mnemonic = b, .cat = INST_##c, __VA_ARGS__}, 
	INSTRUCTIONS_TABLE
#undef X
};

static void 
inst0(Builder *b, Inst_Kind op, Data_Type dt) {
 //...
}

static void 
inst1(Builder *b, Inst_Kind op, Operand a, Data_Type dt) {
	//...
}

static void 
inst2(Builder *b, Inst_Kind op, Operand a, Operand b, Data_Type dt) {
	Instruction_Desc *inst = &inst_table[op];
	//...
	if (inst->cat == INST_BINARY) {
		// do specific things for the BINARY category
	}
	//...
}
```

Although it is not immediately obvious why the second option is a better. There are key aspects I am hiding from you. 


NOTE(ziv): There was another version which I forgot about. It is the python version. I should include that also (in C form I guess) which should detail how it went from specific to... generic to... specific again. This of-course wasted a bunch of time but it made me lean a ton. Especially about the temptation of generality. The general implementation idea was given to me by another programmer video I watched (which is funny) since I guess he is unskilled. Where as my first implementation is way closer to the better programmers which is the last implementation. This is quite a curious thing. This temptation which he made me fall for wasted so much time. Where as I was significantly closer to the result I needed just solving the problem I had which was my first intuition about the problem. 



---

# Resources

[^1]: https://caseymuratori.com/blog_0015
[^2]: https://caseymuratori.com/blog_0016

