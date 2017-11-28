/*

The project is developed as part of Computer Organisation class
Project Name: Functional Simulator for subset of ARM Processor

*/


/* main.cpp
   Purpose of this file: The file handles the input and output, and
   invokes the simulator
*/


#include "coolSim.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  char* prog_mem_file;
  if(argc < 2) {
    cout<<"Incorrect number of arguments. Please invoke the simulator \n\t./coolSim <input mem file> \n";
    exit(1);
  }


  int choice = -1;
  while(choice == -1){
    cout<<"Hello There !!!, Welcome to the Cool Arm Simulator, Advanced. What would you like to do ?\n";
    cout<<"1) Run in Active mode. Press enter after each instruction executed. \n";
    cout<<"2) Run in Silent mode. (Output is logged, so you can see it later, input is still expected if needed.).\n>>>\t";
    cin>>choice;

    if(choice <=0 || choice >=3){
        cout<<"Not a valid choice. Choose Again.\n\n";
    }
  }
  cin.ignore(100,'\n');
  if(choice == 2){
    freopen("log.txt","w",stdout);
    waitF = []() -> void{
    };
  }
  else{
    waitF = []() -> void{
        char x[2];
        cin.getline(x,2);
    };
  }

  //reset the processor
  reset_proc();
  //load the program memory
  load_program_memory(argv[1]);
  //run the simulator
  run_armsim();

  return 1;
}
