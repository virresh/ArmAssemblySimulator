
/*

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of ARM Processor

Developer's Name:
Developer's Email id:
Date:

*/


/* myARMSim.cpp
Purpose of this file: implementation file for myARMSim
*/

#include "myARMSim.h"
#include <bits/stdc++.h>

using namespace std;

function<void(void)> exec_func;
function<void(void)> wb_func;
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

void getOPCode(){
	unsigned int op = instruction_word;
}

void test(){
    // tests memory loaded or not
    cout<<hex;
    for(int i=0; i<32; i++){
        cout<<(int)MEM[i]<<" ";
    }
    cout<<dec<<"\n";
}


void run_armsim() {
    //test();
    exec_func = [&] () -> void{
    	cout<<"unitialised\n";
    };
    wb_func = [&] () -> void{
    	R[Rd] = res;
    	cout<<"Writing "<<res<<" to R"<<Rd<<'\n';
    };
	while(1) {
		fetch();
		decode();
		execute();
		mem();
		write_back();
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
    cout<<"FETCH : Instruction "<<hex<<instruction_word<<" from address 0x"<<(int)(R[15])<<dec<<"\n";
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
	cout<<"DECODE : Operation is ";
	//<<hex<<conditions<<" "<<opcode<<dec;
	if(instruction_type == 0){
        // data processing type instruction
        unsigned int immediateBit = (instruction_word >> 25) & 0x1;
        opcode = (instruction_word & 0x1E00000)>> 21;
        Rn = (instruction_word >> 16) & 0xF ;
        Rd = (instruction_word >> 12) & 0xF ;
        if(immediateBit == 0){
            Rm = instruction_word & 0xF;
            imm = 0;

            operand1 = R[Rn];
            operand2 = R[(instruction_word) & 0xF];
            //cout<<"DEBUG: "<<operand1<<' '<<operand2<<'\n';
        }
        else{
            imm = 1;

            operand1 = R[Rn];
            operand2 = (instruction_word) & 0xFF;
            //cout<<"DEBUG: "<<operand1<<' '<<operand2<<'\n';
        }
        ::Rd=Rd;
        if(opcode == 0){
            // and instruction
            cout<<"And ";
            exec_func = [] () -> void{
            	res=operand1&operand2;
            };
        }
        if(opcode == 1){
            // xor instruction
            cout<<"Xor ";
            exec_func = [] () -> void{
            	res=operand1^operand2;
            };
        }
        if(opcode == 2){
            // sub instruction
            cout<<"Sub ";
            exec_func = [] () -> void{
            	res=operand1-operand2;
            };
        }
        if(opcode == 3){
            // reverse subtract instruction
            cout<<"Rsb ";
            exec_func = [] () -> void{
            	res=operand2-operand1;
            };
        }
        if(opcode == 4){
            // add instruction
            cout<<"Add ";
            exec_func = [] () -> void{
            	res=operand1+operand2;
            };
        }
        if(opcode == 5){
            // add with constant instruction
            cout<<"Adc ";
            exec_func = [] () -> void{
            	res=operand1+operand2+C;
            };
        }
        if(opcode == 6){
            // subtract with constant instruction
            cout<<"Sbc ";
            exec_func = [] () -> void{
            	res=operand1-operand2+C-1;
            };
        }
        if(opcode == 7){
            // reverse subtract with constant instruction
            cout<<"Rsc ";
            exec_func = [] () -> void{
            	res=operand2-operand1+C-1;
            };
        }
        if(opcode == 8){
            // test Op1 AND Op2 instruction
            cout<<"TST ";
            exec_func = [] () -> void{
            	Z = !(operand1&operand2);
            };
        }
        if(opcode == 9){
            // test Op1 XOR Op2 instruction
            cout<<"TEQ ";
            exec_func = [] () -> void{
            	Z = !(operand1^operand2);
            };
        }
        if(opcode == 10){
            // test Op1 - Op2 instruction
            cout<<"CMP ";
            exec_func = [] () -> void{
            	N = (operand1-operand2<0);
            	Z = (operand1-operand2==0);
            };
        }
        if(opcode == 11){
            // test Op1 + Op2 instruction
            cout<<"CMN ";
            exec_func = [] () -> void{
            	N = (operand1+operand2<0);
            	Z = !(operand1+operand2);
            };
        }
        if(opcode == 12){
            // OR instruction
            cout<<"ORR ";
            exec_func = [] () -> void{
            	res=operand1|operand2;
            };
        }
        if(opcode == 13){
            // Move instruction
            cout<<"Mov ";
            exec_func = [] () -> void{
            	res=operand2;
            };
        }
        if(opcode == 14){
            // Bit Clear: Op1 AND ~Op2 instruction
            cout<<"BIC ";
            exec_func = [] () -> void{
            	res=operand1&(!operand2);
            };
        }
        if(opcode == 15){
            // Negation instruction
            cout<<"MVN ";
            exec_func = [] () -> void{
            	res=~operand2;
            };
        }
	}
	else if(instruction_type == 1){
        // data transfer type instruction
	}
	else if(instruction_type == 2){
        // branching type instruction
	}
	cout<<"\n";
}
//executes the ALU operation based on ALUop
void execute() {
	if(instruction_word == 0xEF000011){
		swi_exit();
	}
	exec_func();
}
//perform the memory operation
void mem(){

}
//writes the results back to register file
void write_back() {
	wb_func();
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
