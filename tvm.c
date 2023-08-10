#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct STACK
{
    uint64_t * data;
    uint16_t stacksize;
    int16_t top;
};

// Opcodes
enum opcode{

NOOP = 0, ENDP = 1,

SETX = 5, SETY = 6, SETA = 7, SETB = 8,

JUMP = 9,

HEWO = 10,

LODX = 11, LODY = 12, LODA = 13, LODB = 14,

STBI = 15, LDBI = 16, CLBI = 17,

OUTX = 18, OUTY = 19, OUTA = 20, OUTB = 21,

INCX = 22, INCY = 23, INCA = 24, INCB = 25,

DECX = 26, DECY = 27, DECA = 28, DECB = 29,

INBI = 30, DEBI = 31,

STRX = 32, STRY = 33, STRA = 34, STRB = 35,

SWXY = 36, SWAB = 37,

GETS = 38,

INPX = 39, INPY = 40, INPA = 41, INPB = 42,

GEXY = 43, GEAB = 44, LEXY = 45, LEAB = 46,

EQXY = 47, EQAB = 48,

GTXY = 49, GTAB = 50, LTXY = 51, LTAB = 52,

JPXT = 53, JPXF = 54,

FLGA = 55, FLGO = 56, FLGN = 57,

TRXY = 58, TRXA = 59, TRXB = 60,

TRYX = 61, TRYA = 62, TRYB = 63,

TRAX = 64, TRAY = 65, TRAB = 66,

TRBX= 67, TRBY = 68, TRBA = 69,

TBIX = 70, TBIY = 71, TBIA = 72, TBIB = 73,

TXBI = 74, TYBI = 75, TABI = 76, TBBI = 77,

SNPN = 78, SNPP = 79, NPFL = 80, FLNP = 81,

ASXY = 82, ASAB = 83,

MUXY = 84, MUAB = 85,

DVXY = 86, DVAB = 87,

DRXY = 88, DRAB = 89,

BSLX = 90, BSLY = 91, BSLA = 92, BSLB = 93,

BSRX = 94, BSRY = 95, BSRA = 96, BSRB = 97,

BAXY = 98, BAAB = 99,

BOXY = 100, BOAB = 101,

BTNX = 102, BTNY = 103, BTNA = 104, BTNB = 105,

OUTC = 106,

JSUB = 107, RTRN = 108, SUBR = 109, RTNP = 110,

};



enum flag{NP = 0};

// Program File being loaded
FILE *ProgramFile;


// Program limit variable
uint64_t ProgramLimit = 0;

// Program
uint64_t *Program;

// RAM limit variable
uint64_t RAMLimit = 0;

// RAM (Will add more if necessary)
uint64_t *RAM;

// Determines whether or not Program is being executed directly from RAM
bool InRAM = false;

struct STACK SubroutineStack;


// Input Buffer
char Input[1000];

// Input pointer
char *inptr = Input;

// Error
bool Error = false;

// Early End
bool EarlyEnd = false;


// System Flags
bool SystemFlags[8] = {true, false, false, false, false, false, false, false};

// Program Flags
bool ProgramFlags[64];



uint64_t swap;

// X register, Y register, A register, and B register (used for quick operations)
uint64_t XREG = 0, YREG = 0, AREG = 0, BREG = 0;

// Special Registers
uint64_t BIAS = 0;



int ProgramCounter = 0; // Points to current instruction


int bfgets (char *string, unsigned int limit) {
	char * nptr;
	char function_string[(limit + 1)];
	/*fgets can only write to the same string once, so this creates a temporary string
	 * that will be destroyed. From there, I use sprintf to write to the real string,
	 * using the temporary string.
	 */
	fgets(function_string, limit, stdin);
	//makes a pointer to find the location of \n in the string.
	nptr = strchr(function_string, '\n');
	//If the there is no \n then it will flush out the next line in the input buffer
	if (nptr == NULL) {
		while ((getchar()) != '\n');
	}
	//if there is then it is removed.
	else {
		function_string[strcspn(function_string, "\n")] = 0;
	}
	sprintf(string, "%s", function_string);
    return 0;
}


void ClearRam() // Clears/Initializes RAM
{
    for (int i = 0; i < RAMLimit; ++i)
    {
        RAM[i] = 0;
    }
}
void ClearProgram() // Clears/Initializes Program Memory
{
    for (int i = 0; i < ProgramLimit; ++i)
    {
        Program[i] = 0;
    }

    Program[ProgramLimit - 1] = ENDP;

}
void ClearInput() // Clears/Initializes Input Buffer
{
    for (int i = 0; i < 1000; ++i)
    {
        Input[i] = 0;
    }
}

void ClearProgramFlags() // Clears/Initializes Program Flags
{
    for (int i = 0; i < 64; ++i)
    {
        ProgramFlags[i] = false;
    }
}

void InitStack() // Initialize Subroutine Stack
{
    SubroutineStack.stacksize = 512;
    SubroutineStack.top = -1;
    SubroutineStack.data = (uint64_t*) malloc(SubroutineStack.stacksize*sizeof(uint64_t));
}

void Push(struct STACK *Stack, uint64_t value)
{
    if (Stack->top >= Stack->stacksize - 1)
    {
        Error = true;
    }
    else
    {
        Stack->top += 1;
        Stack->data[Stack->top] = value;
    }
}

uint64_t Pop(struct STACK *Stack)
{
    uint64_t Value;
    if (Stack->top < 0)
    {
        Error = true;
        return 0;
    }
    Value = Stack->data[Stack->top];
    --Stack->top;
    return Value;
}

void LoadProgram ()
{
    int i = 0;
    uint64_t CurrentWrite[1] = {0};
    uint8_t CurrentByte[1];
    while (!feof(ProgramFile) && i < ProgramLimit)
    {
        for (int j = 0; j < 8; ++j)
        {
            CurrentWrite[0] = CurrentWrite[0] << 8; // Bit Shift to left to make way for next-most significant byte in every iteration (For little-endian cpus)
            fread(CurrentByte, sizeof(uint8_t), 1, ProgramFile); // Read a byte from program file
            CurrentWrite[0] += CurrentByte[0];
            printf("\n%d", j);
        }
        printf("\nRead: %llu", CurrentWrite[0]);

        Program[i] = CurrentWrite[0];
        CurrentWrite[0] = 0;
        ++i;
    }
}

char NextChar()
{
    char a;
    if (*inptr == 0)
        return 0;
    a = *inptr;
    ++inptr;
    return a;
}

void ExecOpcode(enum opcode opcode, uint64_t arg)
{
    switch (opcode)
    {
    case SETX: // Increment the program counter to skip over following data, and load the constant into the x register
        ProgramCounter++;
        XREG = arg;
        break;
    case SETY: // Same as X but with different register
        ProgramCounter++;
        YREG = arg;
        break;
    case SETA: // Same as X but with different register
        ProgramCounter++;
        AREG = arg;
        break;
    case SETB: // Same as X but with different register
        ProgramCounter++;
        BREG = arg;
        break;

    case JUMP: // Jump to specified place in program memory
        ProgramCounter = arg - 1;
        break;
    case HEWO: // Hello World code golfing
        printf("Hello, World!");
        EarlyEnd = true;
        break;
    case LODX: // Load memory value to X
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            XREG = RAM[arg];
        }
        break;
    case LODY: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            YREG = RAM[arg];
        }
        break;
    case LODA: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            AREG = RAM[arg];
        }
        break;
    case LODB: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            BREG = RAM[arg];
        }
        break;
    case STBI: // Set Bias
        ProgramCounter++;
        BIAS = arg;
        break;
    case LDBI: // Load memory value to Bias
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            BIAS = RAM[arg];
        }
        break;
    case CLBI: // Set bias to 0
        BIAS = 0;
        break;
    case OUTX: // Outputs X register as a character
        putchar(XREG);
        break;
    case OUTY: // Same as X but with different register
        putchar(YREG);
        break;
    case OUTA: // Same as X but with different register
        putchar(AREG);
        break;
    case OUTB: // Same as X but with different register
        putchar(BREG);
        break;
    case INCX: // Increment X by 1
        ++XREG;
        break;
    case INCY: // Same as X but with different register
        ++YREG;
        break;
    case INCA: // Same as X but with different register
        ++AREG;
        break;
    case INCB: // Same as X but with different register
        ++BREG;
        break;
    case DECX: // Decrement X by 1
        --XREG;
        break;
    case DECY: // Same as X but with different register
        --YREG;
        break;
    case DECA: // Same as X but with different register
        --AREG;
        break;
    case DECB: // Same as X but with different register
        --BREG;
        break;
    case INBI: // Increment bias by 1
        ++BIAS;
        break;
    case DEBI: // Decrement bias by 1
        --BIAS;
        break;
    case STRX: // Store X to memory
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            RAM[arg] = XREG;
        }
        break;
    case STRY: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            RAM[arg] = YREG;
        }
        break;
    case STRA: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;

        else
        {
            ProgramCounter++;
            RAM[arg] = AREG;
        }
        break;
    case STRB: // Same as X but with different register
        if (arg >= RAMLimit)
            Error = true;
        else
        {
            ProgramCounter++;
            RAM[arg] = BREG;
        }
        break;
    case SWXY: // Swap X and Y
        swap = XREG;
        XREG = YREG;
        YREG = swap;
        break;
    case SWAB: // Swap A and B
        swap = AREG;
        AREG = BREG;
        BREG = swap;
        break;
    case GETS: // Get string and load into input buffer
        bfgets(Input, 1000);
        break;
    case INPX: // Load next char in input buffer to X
        XREG = NextChar();
        break;
    case INPY: // Load next char in input buffer to X
        YREG = NextChar();
        break;
    case INPA: // Load next char in input buffer to X
        AREG = NextChar();
        break;
    case INPB: // Load next char in input buffer to X
        BREG = NextChar();
        break;
    case GEXY: // Set specified program flag to true if X>=Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = XREG >= YREG;
        break;
    case GEAB: // Same as XY but with different registers
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = AREG >= BREG;
        break;
    case LEXY: // Set specified program flag to true if X<=Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = XREG <= YREG;
        break;
    case LEAB: // Same as XY but with different registers
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = AREG <= BREG;
        break;
    case EQXY: // Set specified program flag to true if X==Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = XREG == YREG;
        break;
    case EQAB: // Same as XY but with different registers
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = AREG == BREG;
        break;
    case GTXY: // Set specified program flag to true if X>Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = XREG > YREG;
        break;
    case GTAB: // Same as XY but with different registers
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = AREG > BREG;
        break;
    case LTXY: // Set specified program flag to true if X<Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = XREG < YREG;
        break;
    case LTAB: // Set specified program flag to true if X<Y
        ProgramCounter++;
        if (arg >= 64) Error = true;
        else ProgramFlags[arg] = AREG < BREG;
        break;
    case JPXT: // Jump if flag specified by X register is true
        if (XREG >= 64) Error = true;
        else if (ProgramFlags[XREG])
            ProgramCounter = arg - 1;
        break;
    case JPXF: // Jump if flag specified by X register is false
        if (XREG >= 64) Error = true;
        else if (!ProgramFlags[XREG])
            ProgramCounter = arg - 1;
        break;
    case FLGA: // Performs an AND operation on ProgramFlags[x] and ProgramFlags[y] and stores them in ProgramFlags[arg]
        if (XREG >= 64 || YREG >= 64) Error = true;
        else
        {
            ProgramCounter++;
            ProgramFlags[arg] = ProgramFlags[XREG] && ProgramFlags[YREG];
        }
        break;
    case FLGO: // Performs an OR operation on ProgramFlags[x] and ProgramFlags[y] and stores them in ProgramFlags[arg]
        if (XREG >= 64 || YREG >= 64) Error = true;
        else
        {
            ProgramCounter++;
            ProgramFlags[arg] = ProgramFlags[XREG] || ProgramFlags[YREG];
        }
        break;
    case FLGN: // Performs a NOT operation on specified program flag
        if (arg >= 64) Error = true;
        else
        {
            ProgramCounter++;
            ProgramFlags[arg] = !ProgramFlags[arg];
        }
        break;
    case TRXY: // Transfer X to Y
        YREG = XREG;
        break;
    case TRXA: // Transfer X to A
        AREG = XREG;
        break;
    case TRXB: // Transfer X to B
        BREG = XREG;
        break;
    case TRYX: // Transfer Y to X
        XREG = YREG;
        break;
    case TRYA: // Transfer Y to A
        AREG = YREG;
        break;
    case TRYB: // Transfer X to B
        BREG = YREG;
        break;
    case TRAX: // Transfer A to X
        XREG = AREG;
        break;
    case TRAY: // Transfer A to Y
        YREG = AREG;
        break;
    case TRAB: // Transfer A to B
        BREG = AREG;
        break;
    case TRBX: // Transfer B to X
        XREG = BREG;
        break;
    case TRBY: // Transfer B to Y
        YREG = BREG;
        break;
    case TRBA: // Transfer B to A
        AREG = BREG;
        break;
    case TBIX: // Transfer bias to X
        XREG = BIAS;
        break;
    case TBIY: // Same as X but with different register
        YREG = BIAS;
        break;
    case TBIA: // Same as X but with different register
        AREG = BIAS;
        break;
    case TBIB: // Same as X but with different register
        BREG = BIAS;
        break;
    case TXBI: // Transfer X to bias
        BIAS = XREG;
        break;
    case TYBI: // Same as X but with different register
        BIAS = YREG;
        break;
    case TABI: // Same as X but with different register
        BIAS = AREG;
        break;
    case TBBI: // Same as X but with different register
        BIAS = BREG;
        break;
    case SNPN: // Set Negative Positive flag to negative
        SystemFlags[NP] = 0;
        break;
    case SNPP: // Set Negative Positive flag to positive
        SystemFlags[NP] = 1;
        break;
    case NPFL: // Set Negative Positive flag to specified program flag
        if (arg >= 64) Error = true;
        else
        {
            SystemFlags[NP] = ProgramFlags[arg];
        }
        break;
    case FLNP: // Set specified flag to NP
        if (arg >= 64) Error = true;
        else
        {
            ProgramFlags[arg] = SystemFlags[NP];
        }
        break;
    case ASXY: // Add or subtract Y from X based on NP flag and store result in A
        if (SystemFlags[NP]) AREG = XREG + YREG;
        else AREG = XREG - YREG;
        break;
    case ASAB: // Same as XY but with different registers
        if (SystemFlags[NP]) XREG = AREG + BREG;
        else XREG = AREG - BREG;
        break;
    case MUXY: // Multiply X and Y and store result in A
        AREG = XREG * YREG;
        break;
    case MUAB: // Same as XY but with different registers
        XREG = AREG * BREG;
        break;
    case DVXY: // Divide X by Y, store the result in A, and toss the remainder
        if (YREG == 0) Error = true;
        else AREG = XREG / YREG;
        break;
    case DVAB: // Same as XY but with different registers
        if (BREG == 0) Error = true;
        else XREG = AREG / BREG;
        break;
    case DRXY: // Divide X by Y, store the result in A, and store the remainder in B
        if (YREG == 0) Error = true;
        else
        {
            AREG = XREG / YREG;
            BREG = XREG%YREG;
        }
        break;
    case DRAB: // Same as XY but with different registers
        if (BREG == 0) Error = true;
        else
        {
            XREG = AREG / BREG;
            YREG = AREG%BREG;
        }
        break;
    case BSLX: // Bit shift X to the left a specified number of times
        ProgramCounter++;
        XREG = XREG << arg;
        break;
    case BSLY: // Same as X but with different register
        ProgramCounter++;
        YREG = YREG << arg;
        break;
    case BSLA: // Same as X but with different register
        ProgramCounter++;
        AREG = AREG << arg;
        break;
    case BSLB: // Same as X but with different register
        ProgramCounter++;
        BREG = BREG << arg;
        break;
    case BSRX: // Bit shift X to the right a specified number of times
        ProgramCounter++;
        XREG = XREG >> arg;
        break;
    case BSRY: // Same as X but with different register
        ProgramCounter++;
        YREG = YREG >> arg;
        break;
    case BSRA: // Same as X but with different register
        ProgramCounter++;
        AREG = AREG >> arg;
        break;
    case BSRB: // Same as X but with different register
        ProgramCounter++;
        BREG = BREG >> arg;
        break;
    case BAXY: // Bitwise AND on X and Y, store result in A
        AREG = XREG & YREG;
        break;
    case BAAB: // Same as XY but with different registers
        XREG = AREG & BREG;
        break;
    case BOXY: // Bitwise OR on X and Y, store result in A
        AREG = XREG | YREG;
        break;
    case BOAB: // Same as XY but with different registers
        XREG = AREG | BREG;
        break;
    case BTNX: // Bitwise NOT on X register
        XREG = ~XREG;
        break;
    case BTNY: // Same as X but with different register
        YREG = ~YREG;
        break;
    case BTNA: // Same as X but with different register
        AREG = ~AREG;
        break;
    case BTNB: // Same as X but with different register
        BREG = ~BREG;
        break;
    case OUTC: // Output a char
        ProgramCounter++;
        putchar(arg);
        break;
    case JSUB: // Jump to subroutine
        ProgramCounter++;
        Push(&SubroutineStack, ProgramCounter);
        ProgramCounter = arg - 1;
        break;
    case RTRN: // Return from subroutine
        ProgramCounter = Pop(&SubroutineStack);
        break;
    case SUBR: // Subroutine to RAM location
        ProgramCounter++;
        Push(&SubroutineStack, ProgramCounter);
        ProgramCounter = arg - 1;
        InRAM = true;
        break;
    case RTNP: // Return from RAM to program
        ProgramCounter = Pop(&SubroutineStack);
        InRAM = false;
        break;


    }
}

void RunProgram()
{


    uint64_t op, arg, Boundary;
    while (1)
    {
        if (!InRAM)
        {
            op = Program[ProgramCounter];
            arg = Program[ProgramCounter+1];
            Boundary = ProgramLimit;
        }
        else
        {
            op = RAM[ProgramCounter];
            arg = RAM[ProgramCounter+1];
            Boundary = RAMLimit;
        }
        if (op == ENDP) {break;}
        if (ProgramCounter >= Boundary) {printf("Program jumped out of bounds.\n");}

        // Apply bias based on Negative/Positive flag
        if (SystemFlags[NP]) arg += BIAS;
        else arg -= BIAS;


        // Execute opcode
        ExecOpcode(op, arg);

        if (Error) {printf("Error at location %llu. Opcode: %llu Argument: %llu\n", ProgramCounter, op, arg); break;}
        if (EarlyEnd) break;
        ProgramCounter++;

    }
}




void debug() // Show debug info
{
    printf("\nLocation: %d.\nXREG: %llu\nYREG: %llu\nAREG: %llu\nBREG: %llu\nBIAS: %llu\n\n", ProgramCounter, XREG, YREG, AREG, BREG, BIAS);
    puts("System Flags:");
    for (int i = 0; i < 8; ++i)
    {
        printf("%d", SystemFlags[i]);
    }
    puts("\n\nProgram Flags:");
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            printf("%d", ProgramFlags[i*8+j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    // Memory Allocation and failure messages handling allocation errors and argument errors
    if (argc != 4) {printf("Please run program like this: tvm.exe [file] [Program ROM Length in bytes] [RAM Length in bytes]\n"); return 0;} // Argument error


    sscanf(argv[2], "%llu", &ProgramLimit); // Set ProgramLimit
    sscanf(argv[3], "%llu", &RAMLimit); // Set RAMLimit
    if (ProgramLimit == 0 || RAMLimit == 0) {printf("Invalid input for Program or RAM length."); return 0;} // Input Error
    Program = (uint64_t*)malloc(ProgramLimit * sizeof(uint64_t)); // Allocate Program
    RAM = (uint64_t*)malloc(RAMLimit * sizeof(uint64_t)); // Allocate RAM
    if (Program == NULL || RAM == NULL) {printf("Memory Allocation Error."); return 0;} // Allocation Error


    ProgramFile = fopen(argv[1], "rb");
    if (ProgramFile == NULL) {printf("Failed to open file:\"%s\"", argv[1]); return 0;}


    printf("Welcome to Tiny Virtual Machine!\n"); // Welcome

    printf("Clearing RAM...\n");
    ClearRam();
    printf("Clearing Program Memory...\n");
    ClearProgram();
    printf("Clearing Virtual Input Buffer...\n");
    ClearInput();
    printf("Loading Program...\n");
    LoadProgram();
    printf("Initializing Stack...\n");
    InitStack();
    printf("Done!\n");

    printf("Executing Program...\n");
    RunProgram();
    printf("\nProgram Ended.\n");
    debug();

    free(Program);
    free(RAM);
    return 0;
}
