## Architecture
TVM's "architecture" consists of RAM, program space, five registers, eight system flags (though only one is currently in use), 64 program flags (which can all be modified by the program), and an input buffer.

### Addressing
While many systems' addressing modes count by bytes, TVM's counts by 64-bit words. Though opcodes do not range beyond even the 8-bit integer limit, instructions are still 8 bytes. I am aware that this is inefficient.
### Registers
The five registers mentioned are the X register, the Y register, the A register, the B register, and the Bias register. The first four (X, Y, A, and B) work together in pairs in some instructions.
For example, there are no instructions to calculate X - A or B - Y. There are only instructions for X - Y and A - B. This is true for all instructions that perform operations on two registers at once. 
The bias register is added to or subtracted from the argument of an opcode (if there is one) based on the value of the NP system flag. (This will be covered later in the passage.)
### System and Program Flags
The one system flag in use is the NP or Negative/Positive flag and is used to determine whether the bias shall be added or subtracted from the argument of an opcode. 
It is also used to decide if addition or subtraction should be done, as they do not have seperate instructions. All of the 64 program flags can only be modified by the program and do not have any individual purposes (other than any the programmer decides to give them). 
Program flags can be modified by sets of instructions that will turn them on or off based on a specific condition (for example x == y). They can also be modified with AND, OR, and NOT gates.
This is meant to be a hacky way to have more complex conditionals. Jump instructions can then be performed if a specific flag is 1 or if a specific flag is 0.
### Input Buffer
The input buffer consists of 1000 characters (including a NULL terminator). The GETS instruction will prompt the user for input, and store it in the input buffer. 
Then, the buffer can be accessed by the program one character at a time through the INPX, INPY, INPA, and INPB instructions, which each store the next character in the input buffer to their respective registers.

------------------------------------------------------------------------------------------------------------------------------
## Other Features
- Opcodes are represented with four letter abbreviations rather than three letter abbreviations.
- You may have noticed an instruction that prints, "Hello, World!" and ends the program. This is for an esoteric programming language I am working on that uses this VM.
- The SUBR instruction jumps to a location in RAM rather than a location in the program as if it were a subroutine. This is so users can write their own programs for the VM using a program written in the the VM.
