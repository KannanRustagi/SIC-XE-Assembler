## ASSIGNMENT-2
Name- Kannan Rustagi
Roll no.-210101054

===============================================================================================================================

## INSTRUCTIONS FOR RUNNING THE CODE & INPUT FORMAT
1. The environment required for running the code is Linux system with g++ compiler installed.
 
2. Open the terminal and navigate to the directory where the contents of this zip file have been extracted.

3. The input to the assembler should be put in the input.txt file.

4. input.txt format- Label field: 10chars long, Opcode field: 10chars long, operand field: >= 30 chars long
                    input.txt file is already in this format

5. To compile and run run the SIC/XE assmbler, run the following commands-
    g++ assembler.cpp -o assembler
    ./assembler input.txt

6. To compile and run the loader, run the following commands-
    g++ loader.cpp -o loader
    ./loader

===============================================================================================================================

## OUTPUT
1. On running the assembler, 2 files will be generated- intermediate.txt(generated after pass1) and assemblerOutput.txt(containing the final assembled opcode generated after the completion of both the passes)

2. The loader takes as input assemblerOutput.txt(ouput of the assembler on input.txt). After running, i.e, the completion of 2 passes of the loader, 2 files will be generated- loaderOutput.txt, memory.txt. The memory visual for the loaded program can be found in memory.txt while the line wise output(address and the instruction loaded at that address in left and right columns respectively) can be find in loaderOutput.txt.

===============================================================================================================================

## CODE EXPLANATION

# Assembler
Pass 1 of the assembler will generate an intermediate file and it will update the symbol tables for each sxection and literal tables as well storing the required addresses corresponding to the literals and labels. 

Then in pass2, it will parse the intermediate file generated in pass1. the assembler will operate on the EXTDEF and EXTREF directives to generate the define and refer records foe each control section separately. Modification records for each control section will also be generated for whichever instructions they are required.

Proper comments have been added in the code which explains how each function is working.

# Loader
For the loader, I have assumed that the program will be loaded at address 0 and accordingly it is visible in the output files. The address where the program is to be loaded can be changed in the code, however, since we only need to show the function of a loader, it will bw visible in this case as well.

The character array mmap is used to store the simulated memory of the program.
unordered_map<string, int> estab is the symbol table for the external references and csect names,

The PASS1(being done in main function only) reads the input record file and stores all the external definitions and csect names. The PASS2(being done in main function only) reads the input record file and loads the text records in the simulated memory and then loads the modifications.

Proper comments have been added in the code which explains how each function is working.

===============================================================================================================================

Sample output files for both the assembler and loader, for the give input file has also been included the submission zip for reference