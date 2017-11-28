
/*

The project is developed as part of Computer Organisation class
Project Name: Functional Simulator for subset of ARM Processor

*/


/* coolSim.cpp
Purpose of this file: implementation file for coolSim core functions
*/

#include "coolSim.h"
#include <bits/stdc++.h>

using namespace std;

function<void(void)> exec_func;
function<void(void)> wb_func;
function<void(void)> mem_func;
function<void(void)> waitF;

//Register file
static unsigned int R[16];
//flags
static int N,C,V,Z;
//memory
static unsigned char MEM[4000];

//intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int operand1;
static unsigned int operand2;
static unsigned int Rd;
static unsigned int opcode;
static unsigned int imm;
static unsigned int res;

void runOnce(){
    exec_func = [&] () -> void{
		cout<<"No Execute Operation.";
	};

	mem_func = [&] () -> void{
        cout<<"No Memory Operation.";
	};

	wb_func = [&] () -> void{
		R[Rd] = res;
		cout<<"Writing "<<res<<" to R"<<Rd<<"\n";
	};

    fetch();
    decode();
    execute();
    mem();
    write_back();
    cout<<"\n";
    printRegisters();
    cout<<"\n";

    waitF();
}

void setWait(function<void(void)> wtF){
    waitF = wtF;
}

void printRegisters(){
    for(int i=0; i<=15; i++){
        cout<<"R"<<i<<" : "<<R[i]<<"\t";
    }
    cout<<"\n";
    cout<<"N : "<<N<<"\t"<<"C : "<<C<<"\t"<<"V : "<<V<<"\t"<<"Z : "<<Z<<"\n";
}

void run_armsim() {
	while(1) {
        runOnce();
	}
}

// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
	memset(R,0,sizeof(R));
	N=C=V=Z=0;
	memset(MEM,0,sizeof(MEM));
	instruction_word = 0, operand1 = 0, operand2 = 0;
}

//load_program_memory reads the input memory, and populates the instruction
// memory
void load_program_memory(char *file_name) {
	FILE *fp;
	unsigned int address, instruction;
	fp = fopen(file_name, "r");
	if(fp == NULL) {
		printf("Error opening input mem file\n");
		exit(1);
	}
	while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
		write_word(MEM, address, instruction);
	}
	fclose(fp);
}

//writes the data memory in "data_out.mem" file
void write_data_memory() {
	FILE *fp;
	unsigned int i;
	fp = fopen("data_out.mem", "w");
	if(fp == NULL) {
		printf("Error opening dataout.mem file for writing\n");
		return;
	}

	for(i=0; i < 4000; i = i+4){
		fprintf(fp, "%x %x\n", i, read_word(MEM, i));
	}
	fclose(fp);
}

//should be called when instruction is swi_exit
void swi_exit() {
	write_data_memory();
	exit(0);
}


//reads from the instruction memory and updates the instruction register
void fetch() {
	instruction_word = read_word(MEM,R[15]); // Load instruction from the address pointed by program counter.
	cout<<"FETCH :\t\tInstruction "<<hex<<instruction_word<<" from address 0x"<<(int)(R[15])<<dec<<"\n";
	R[15]+=4;

	if(R[15] >=4000){
		cout<<"Out of Memory Error...\n";
		swi_exit();
	}
}
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode() {
	unsigned int conditions = instruction_word >>28;
	unsigned int instruction_type = (instruction_word >>26) & 0x3;
	unsigned int Rn=-1,Rm=-1,Rd=-1;
	// Rn = first operand register
	// Rd = destination register
	// Rm = second operand register
	cout<<"DECODE :\tOperation is ";

	if(instruction_type == 0){
        // data processing type instruction
		unsigned int immediateBit = (instruction_word >> 25) & 0x1;
		unsigned int Rt = -1;
		opcode = (instruction_word & 0x1E00000)>> 21;
		Rn = (instruction_word >> 16) & 0xF ;
		Rd = (instruction_word >> 12) & 0xF ;
		::Rd=Rd;
		operand1 = R[Rn];

		if(immediateBit == 0){
            // it is not an immediate command, calculate the respective operand2
			unsigned int shiftAmt = 0;
			Rt = (instruction_word) & 0xF;

			if(instruction_word & 0x10 != 0){
                // this is a shift where shiftamount is inside register
				shiftAmt = R[(instruction_word >> 8) & 0xF];
			}
			else{
                // shift provided as a constant
				shiftAmt = (instruction_word>>7) & 0x1F;
			}

			unsigned int codeShift = (instruction_word >> 5 ) & 0x3;
			if(codeShift == 0){
                // Logical left
				operand2 = ((unsigned)R[Rt] << shiftAmt);
			}
			else if(codeShift == 1){
                // Logical right
				operand2 = ((unsigned)R[Rt] >> shiftAmt);
			}
			else if(codeShift == 2){
                // Arithmetic right
				operand2 = ((int)R[Rt] >> shiftAmt);
			}
			else if(codeShift == 3){
                // Rotate right
				operand2 = ((unsigned int)R[Rt] >> shiftAmt) | ((unsigned int)R[Rt] << (32-shiftAmt));
			}

		}
		else{
            // operand is immediate value as specified in the instruction
			unsigned int shiftAmt = (instruction_word >> 8 )& 0xF;
			unsigned int immVal = (instruction_word & 0xFF);
			operand2 = ((unsigned int)immVal >> shiftAmt) | ((unsigned int)immVal << (32-shiftAmt));
		}

		if(opcode == 0){
            // and instruction
			cout<<"AND ";
			exec_func = [] () -> void{
				res=operand1&operand2;
				cout<<operand1<<" AND "<<operand2<<" = "<<res;
			};
		}
		if(opcode == 1){
            // xor instruction
			cout<<"XOR ";
			exec_func = [] () -> void{
				res=operand1^operand2;
				cout<<operand1<<" XOR "<<operand2<<" = "<<res;
			};
		}
		if(opcode == 2){
            // sub instruction
			cout<<"SUB ";
			exec_func = [] () -> void{
				res=operand1-operand2;
				cout<<operand1<<" - "<<operand2<<" = "<<res;
			};
		}
		if(opcode == 3){
            // reverse subtract instruction
			cout<<"RSB ";
			exec_func = [] () -> void{
				res=operand2-operand1;
				cout<<operand2<<" - "<<operand1<<" = "<<res;
			};
		}
		if(opcode == 4){
            // add instruction
			cout<<"ADD ";
			exec_func = [] () -> void{
				res=operand1+operand2;
				cout<<operand1<<" + "<<operand2<<" = "<<res;
			};
		}
		if(opcode == 5){
            // add with constant instruction
			cout<<"ADC ";
			exec_func = [] () -> void{
				res=operand1+operand2+C;
				cout<<operand1<<" + "<<operand2<<" = "<<res<<" (with Carry)";
			};
		}
		if(opcode == 6){
            // subtract with constant instruction
			cout<<"SBC ";
			exec_func = [] () -> void{
				res=operand1-operand2+C-1;
				cout<<operand1<<" - "<<operand2<<" = "<<res<<" (with Carry)";
			};
		}
		if(opcode == 7){
            // reverse subtract with constant instruction
			cout<<"RSC ";
			exec_func = [] () -> void{
				res=operand2-operand1+C-1;
				cout<<operand2<<" - "<<operand1<<" = "<<res<<" (with Carry)";
			};
		}
		if(opcode == 8){
            // test Op1 AND Op2 instruction
			cout<<"TST ";
			exec_func = [] () -> void{
				Z = !(operand1&operand2);
				cout<<" NOT ( "<<operand1<<" AND "<<operand2<<") = "<<Z<<" = Z";
			};
		}
		if(opcode == 9){
            // test Op1 XOR Op2 instruction
			cout<<"TEQ ";
			exec_func = [] () -> void{
				Z = !(operand1^operand2);
				cout<<" NOT ( "<<operand1<<" XOR "<<operand2<<") = "<<Z<<" = Z";
			};
		}
		if(opcode == 10){
            // test Op1 - Op2 instruction
			cout<<"CMP ";
			exec_func = [] () -> void{
				N = (operand1-operand2<0);
				cout<<" ( ("<<operand1<<" - "<<operand2<<") < 0) = "<<N<<" = N";
				Z = (operand1-operand2==0);
				cout<<" ( ( "<<operand1<<" AND "<<operand2<<") == 0 ) = "<<Z<<" = Z";
			};
		}
		if(opcode == 11){
            // test Op1 + Op2 instruction
			cout<<"CMN ";
			exec_func = [] () -> void{
				N = (operand1+operand2<0);
				cout<<" ( ("<<operand1<<" + "<<operand2<<") < 0) = "<<N<<" = N";
				Z = !(operand1+operand2);                                                   // Check this again
				cout<<" NOT ( "<<operand1<<" + "<<operand2<<") = "<<Z<<" = Z";
			};
		}
		if(opcode == 12){
            // OR instruction
			cout<<"ORR ";
			exec_func = [] () -> void{
				res=operand1|operand2;
				cout<<operand1<<" OR "<<operand2<<" = "<<res;
			};
		}
		if(opcode == 13){
            // Move instruction
			cout<<"MOV ";
			exec_func = [] () -> void{
				res=operand2;
				cout<<"Moving "<<operand2<<" to R"<<::Rd;
			};
		}
		if(opcode == 14){
            // Bit Clear: Op1 AND ~Op2 instruction
			cout<<"BIC ";
			exec_func = [] () -> void{
				res=operand1&(!operand2);
				cout<<operand1<<" AND  !("<<operand2<<") = "<<res;
			};
		}
		if(opcode == 15){
            // Negation instruction
			cout<<"MVN ";
			exec_func = [] () -> void{
				res=~operand2;
				cout<<"Moving !"<<operand2<<" = "<<!operand2<<" to R"<<::Rd;
			};
		}
		cout<<"\n\t\tFirst Operand is : R"<<Rn<<" = "<<operand1;
		if(immediateBit == 1){
            cout<<",\n\t\tSecond immediate Operand is : "<< operand2<<" (after applying rotations)";
		}
		else{
            cout<<",\n\t\tSecond Operand is : R"<<Rt <<" = "<<operand2<<" (after applying shifts)";
		}
		cout<<",\n\t\tTarget Register is R"<<Rd;

	}
	else if(instruction_type == 1){
        // data transfer type instruction
	}
	else if(instruction_type == 2){
        // branching type instruction
        unsigned int offset= (instruction_word&0xFFFFFF);
        unsigned int link = (instruction_word>>24)&0x1;
        offset=offset<<2;
        if(link==1)	//Checking for branch with link
        {
        	R[14]=R[15];	//Saving the current value of the PC in the Link register
        }
        R[15]+=offset;	//Adding the Offset to the program counter
	}
	else if(instruction_type == 3){
        // software interrupts
        cout<<"Software interrupt";

        if(instruction_word == 0xEF000011){
            exec_func = []() -> void{
                cout<<"Exiting\n";
                swi_exit();
            };
        }
	}
	cout<<"\n";
}

//executes the ALU operation based on ALUop
void execute() {
    cout<<"EXECUTE :\t";
	exec_func();
	cout<<"\n";
}
//perform the memory operation
void mem(){
    cout<<"MEMORY :\t";
    mem_func();
    cout<<"\n";
}
//writes the results back to register file
void write_back() {
    cout<<"WRITEBACK :\t";
	wb_func();
	cout<<"\n";
}


int read_word(unsigned char *mem, unsigned int address) {
	int *data;
	data =  (int*) (mem + address);
	return *data;
}

void write_word(unsigned char *mem, unsigned int address, unsigned int data) {
	unsigned int *data_p;
	data_p = (unsigned int*) (mem + address);
	*data_p = data;
}
