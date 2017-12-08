/*
The project is developed as part of Computer Organisation class
Project Name: Functional Simulator for subset of ARM Processor
*/


/* coolSim.cpp
Purpose of this file: implementation file for coolSim core functions
*/

#include "coolSim.h"
#include <bits/stdc++.h>

#define MEMSIZE 40000
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
static unsigned char MEM[MEMSIZE];

//intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int operand1;
static unsigned int operand2;
static unsigned int Rd;
static unsigned int opcode;
static unsigned int imm;
static unsigned int res;
static unsigned int START_OF_MEMORY;

void runOnce(){
    exec_func = [&] () -> void{
		cout<<"No Execute Operation.";
	};

	mem_func = [&] () -> void{
        cout<<"No Memory Operation.";
	};

	wb_func = [&] () -> void{
		cout<<"No Write Back Operation.";
	};

    fetch();
    decode();
    execute();
    mem();
    write_back();
    cout<<"\n";
    waitF();
    cout<<"\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n";
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

void printStack(){
	cout<<"\n";
	for(int i=R[13]; i<MEMSIZE; i++)
	{
		cout<<hex<<"0x"<<i<<" : "<<MEM[i]<<"\n";
	}
	cout<<"\n";
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
		if(instruction&0xF0000000) START_OF_MEMORY = R[13];
		cout<<"Writing "<<instruction<<" to "<<address<<'\n';
		write_word(MEM, address, instruction);
	}
	fclose(fp);
	START_OF_MEMORY = R[13];
	R[13] = MEMSIZE - 1;
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

	for(i=START_OF_MEMORY; i < MEMSIZE; i = i+4){
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
	if(R[15] >= START_OF_MEMORY){
		swi_exit();
	}
	instruction_word = read_word(MEM,R[15]); // Load instruction from the address pointed by program counter.
	cout<<"FETCH :\t\tInstruction "<<hex<<instruction_word<<" from address 0x"<<(int)(R[15])<<dec<<"\n";
	R[15]+=4;

	if(R[15] >=MEMSIZE){
		cout<<"Out of Memory Error...\n";
		swi_exit();
	}
}
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode() {
	unsigned int conditions = instruction_word >>28;
	bool takeFwd = false;
	string y = "";

	// a==0 means clear a!=0 means set
	if(conditions == 0 && (Z!=0) ){
		// EQ
		takeFwd = true;
		y = "EQ";
	}
	else if(conditions == 1 && (Z==0) ){
		// NE
		takeFwd = true;
		y = "NE";	
	}
	else if(conditions == 2 && (C!=0)){
		// CS
		takeFwd = true;	
	}
	else if(conditions == 3 && (C==0)){
		// CC
		takeFwd = true;	
	}
	else if(conditions == 4 && (N!=0)){
		takeFwd = true;	
	}
	else if(conditions == 5 && (N==0)){
		takeFwd = true;	
	}
	else if(conditions == 6 && (V!=0)){
		takeFwd = true;	
	}
	else if(conditions == 7 && (V==0)){
		takeFwd = true;	
	}
	else if(conditions == 8 && (C!=0 && Z==0)){
		takeFwd = true;	
	}
	else if(conditions == 9 && (C==0 || Z!=0) ){
		takeFwd = true;	
	}
	else if(conditions == 10 && (N==V) ){
		takeFwd = true;	
		y = "GE";
	}
	else if(conditions == 11 && (N!=V) ){
		takeFwd = true;	
		y = "LT";
	}
	else if(conditions == 12 && (Z==0 && (N==V)) ){
		takeFwd = true;	
		y = "GT";
	}
	else if(conditions == 13 && (Z!=0 || (N!=V)) ){
		takeFwd = true;	
		y = "LE";
	}
	else if(conditions == 14 ){
		takeFwd = true;	
	}

	if(takeFwd == false){
		cout<<"DECODE :\tConditions not met. Ignoring Statement.\n";
		exec_func = [] () -> void{
			cout<<"Not Executing this statement.";
		};
		return;
	}

	unsigned int instruction_type = (instruction_word >>26) & 0x3;
	unsigned int Rn=-1,Rm=-1,Rd=-1;
	// Rn = first operand register
	// Rd = destination register
	// Rm = second operand register
	cout<<"DECODE :\tOperation is ";

	if(!((instruction_word>>21)&0x7F) && (((instruction_word>>4)&0xF)==9)) {
		cout<<"MUL ";
		::Rd = (instruction_word >> 16) & 0xF ;
		operand1 = R[(instruction_word) & 0xF];
		operand2 = R[(instruction_word >> 8) & 0xF];
		exec_func = [] () -> void{
			res=operand1*operand2;
			cout<<operand1<<" MUL "<<operand2<<" = "<<res;
		};
		wb_func = [] () -> void{
			cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
			R[::Rd] = res;
		};
	}
	else if(instruction_type == 0){
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

			if((instruction_word & 0x10) != 0){
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
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 1){
            // xor instruction
			cout<<"XOR ";
			exec_func = [] () -> void{
				res=operand1^operand2;
				cout<<operand1<<" XOR "<<operand2<<" = "<<res;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 2){
            // sub instruction
			cout<<"SUB ";
			exec_func = [] () -> void{
				res=operand1-operand2;
				cout<<operand1<<" - "<<operand2<<" = "<<res;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 3){
            // reverse subtract instruction
			cout<<"RSB ";
			exec_func = [] () -> void{
				res=operand2-operand1;
				cout<<operand2<<" - "<<operand1<<" = "<<res;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 4){
            // add instruction
			cout<<"ADD ";
			exec_func = [] () -> void{
				res=operand1+operand2;
				cout<<operand1<<" + "<<operand2<<" = "<<res;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 5){
            // add with constant instruction
			cout<<"ADC ";
			exec_func = [] () -> void{
				res=operand1+operand2+C;
				cout<<operand1<<" + "<<operand2<<" = "<<res<<" (with Carry)";
			};
            wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 6){
            // subtract with constant instruction
			cout<<"SBC ";
			exec_func = [] () -> void{
				res=operand1-operand2+C-1;
				cout<<operand1<<" - "<<operand2<<" = "<<res<<" (with Carry)";
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 7){
            // reverse subtract with constant instruction
			cout<<"RSC ";
			exec_func = [] () -> void{
				res=operand2-operand1+C-1;
				cout<<operand2<<" - "<<operand1<<" = "<<res<<" (with Carry)";
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
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
				N = (operand1<operand2);
				cout<<" ( ("<<operand1<<" - "<<operand2<<") < 0) = "<<N<<" = N";
				Z = (operand1==operand2);
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
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 13){
            // Move instruction
			cout<<"MOV ";
			exec_func = [] () -> void{
				res=operand2;
				cout<<"Moving "<<operand2<<" to R"<<::Rd;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 14){
            // Bit Clear: Op1 AND ~Op2 instruction
			cout<<"BIC ";
			exec_func = [] () -> void{
				res=operand1&(!operand2);
				cout<<operand1<<" AND  !("<<operand2<<") = "<<res;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
		}
		if(opcode == 15){
            // Negation instruction
			cout<<"MVN ";
			exec_func = [] () -> void{
				res=~operand2;
				cout<<"Moving !"<<operand2<<" = "<<!operand2<<" to R"<<::Rd;
			};
			wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
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
        opcode = (instruction_word & 0x03F00000)>> 20;
        Rn = (instruction_word >> 16) & 0xF ;
        Rd = (instruction_word >> 12) & 0xF ;
        unsigned int offset = instruction_word & 0xFFF;
        if((instruction_word>>25)&1){
        	unsigned int shiftAmt = offset>>7;
        	unsigned int Rt = offset & 0xF;
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
        	operand2 = R[Rt] << shiftAmt;
        }
        else if(((instruction_word>>25)&1 )== 0){
        	operand2 = offset;
        }
        operand2 += R[Rn] + 4;
        ::Rd = Rd;

        // cout<<"lol "<<operand2<<' ';
        if(opcode%2==1){
        	// LDR instruction
        	cout<<"LDR ";
        	mem_func = [] () -> void{
        		res = read_word(MEM, operand2);
        		cout<<"Reading value "<<res<<" from address "<<operand2;
        	};
        	wb_func = [] () -> void{
				cout<<"Writing "<<res<<" to R"<<::Rd<<"\n";
				R[::Rd] = res;
			};
        }
        if(opcode%2==0){
        	// STR instruction
        	cout<<"STR ";
        	mem_func = [] () -> void{
        		write_word(MEM,operand2,R[::Rd]);
        		cout<<"Writing value "<<R[::Rd]<<" into "<<operand2;
        	};
        }

	}
	else if(instruction_type == 2){
        // branching type instruction
        unsigned int offset= (instruction_word&0xFFFFFF);
        if((instruction_word & 0x800000) == 0){	//checking 23rd bit, will be 0 if 24bit offset is positive
        	//positive number do nothing
        }
        else{
        	//sign extend offset to a negative number
        	//offset = offset | 0xFF80000;
        	unsigned int extend = (offset&0x800000)<<1, j;
      		for(int j=8;j--; extend<<=1) offset+=extend;
        }
        unsigned int link = (instruction_word>>24)&0x1;
        offset<<=2;
        if(link==1)	//Checking for branch with link
        {
        	R[14]=R[15];	//Saving the current value of the PC in the Link register
        }
        R[15]+=(int)offset+4;	//Adding the Offset to the program counter
        cout<<"B"+y<<"\n\t\t";
        cout<<"Jump to : "<<hex<<R[15]<<dec<<"\n";
        return;
	}
	else if(instruction_type == 3){
        // software interrupts
        cout<<"SWI : Software interrupt";

        if(instruction_word == 0xEF000011){
            exec_func = []() -> void{
                cout<<"Exiting\n";
                swi_exit();
            };
        }
        else if(instruction_word == 0xEF00006C){
        	// read integer
			exec_func = []() -> void{
                cout<<"Input integer : \n\t\t";
                cin>>R[0];
            };

            wb_func = []() -> void{
                cout<<"Writing "<<R[0]<<" to R0\n";
            };
        }
        else if(instruction_word == 0xEF00006B){
        	// write integer
			exec_func = []() -> void{
				cout<<"Printing R1 \t";
                if(R[0] == 1){
                	cout<<dec<<(unsigned)R[1]<<"\n";
                }
                else if(R[0] == 2){
                	cerr<<dec<<(unsigned)R[1]<<"\n";
                }
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
	R[13]+=4;
}