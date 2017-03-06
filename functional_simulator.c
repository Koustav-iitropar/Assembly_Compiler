# This is a SimpleRISC functional simulator created by Koustav Das.
# This program will be able to process assembly codes written in .mem file.
# While executing the please give the name of the file along with it. Example   ./a.out simple_add.mem

#include <stdio.h>
#include <stdlib.h>

//Register file
static unsigned int R[16];
static int isbranchtaken;        // for finding if it is a branch taken instruction
static int branchPC;
static int PC;
int *inst;                //for storing the instruction
static int isRet;
static int isSt,isLd,isBeq,isBgt,isImmediate,isWb,isUbranch,isCall,isAdd,isSub,isCmp,isMul,isDiv,isMod,isLsl,isLsr,isAsr,isOr,isAnd,isNot,isMov;
int branchtarget;    // This should remain as int and not usigned int
int aluResult,ldResult;
unsigned int i;
//flags
static int gt,eq;
//memory
static unsigned char MEM[4000];

//intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int immx;               //calculated value of immediate
static unsigned int operand1;
static unsigned int operand2;

void reset_proc()
{
int i;
for(i=0;i<16;i++)
R[i]=0;
PC=0;
gt=0;
eq=0;
isbranchtaken=0;
branchPC=0;
isRet=0;
isSt=0;
isLd=0;
isBeq=0;
isBgt=0;
isImmediate=0;
isWb=0;
isUbranch=0;
isCall=0;
isAdd=0;
isSub=0;
isCmp=0;
isMul=0;
isDiv=0;
isMod=0;
isLsl=0;
isLsr=0;
isAsr=0;
isOr=0;
isAnd=0;
isNot=0;
isMov=0;
aluResult=0;
ldResult=0;
}


int read_word(char *mem, unsigned int address) {
  int *data;
  data =  (int*) (mem + address);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int *data_p;
  data_p = (int*) (mem + address);
  *data_p = data;
}


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


void fetch() {
if(isbranchtaken==1)
PC=branchPC;
inst=(int*) (MEM+PC);
instruction_word=*inst;
PC+=4;
}

//control unit
void control()
{
int opcode;
opcode=instruction_word>>27;

i=instruction_word<<5;
i=i>>31;

if(opcode==0)
{isAdd=1;isWb=1;}
if(opcode==1)
{isSub=1;isWb=1;}
if(opcode==2)
{isMul=1;isWb=1;}
if(opcode==3)
{isDiv=1;isWb=1;}
if(opcode==4)
{isMod=1;isWb=1;}
if(opcode==5)
{isCmp=1;}
if(opcode==6)
{isAnd=1;isWb=1;}
if(opcode==7)
{isOr=1;isWb=1;}
if(opcode==8)
{isNot=1;isWb=1;}
if(opcode==9)
{isMov=1;isWb=1;}
if(opcode==10)
{isLsl=1;isWb=1;}
if(opcode==11)
{isLsr=1;isWb=1;}
if(opcode==12)
{isAsr=1;isWb=1;}
if(opcode==14)
{isLd=1;isWb=1;isAdd=1;}
if(opcode==15)
{isSt=1;isAdd=1;}
if(opcode==16)
{isBeq=1;}
if(opcode==17)
{isBgt=1;}
if(opcode==20)
{isRet=1;}
if(opcode==19)
{isCall=1;isWb=1;}
if(opcode==18 || opcode==19 || opcode==20)
isUbranch=1;

if(i==1)
isImmediate=1;

}

//reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode()
{
unsigned int imm,m;  // m=modifier
imm=instruction_word<<16;
m=instruction_word<<14;
m=m>>30;
if(m==0)
imm=imm>>16;         //default value
if(m==1)
imm=imm>>16;
immx=imm;      // Assigning value to immediate

branchtarget=instruction_word<<5;
branchtarget=branchtarget>>3;
if(isRet==0)
{
operand1=instruction_word<<10;
operand1=operand1>>28;
operand1=R[operand1];
}
else
{
operand1=R[15];
}

if(isSt==0)
{
operand2=instruction_word<<14;
operand2=operand2>>28;
operand2=R[operand2];
}

if(isSt==1)
{
operand2=instruction_word<<6;
operand2=operand2>>28;
operand2=R[operand2];
}


}


//executes the ALU operation based on ALUop
void execute()
{

// Branch Unit
if(((eq==1)&&(isBeq==1))||((gt==1)&&(isBgt==1))||(isUbranch==1))
isbranchtaken=1;
else
isbranchtaken=0;
if(isRet==0)
branchPC=branchtarget;
else
branchPC=operand1;

int a=operand1,b;
if(isImmediate==0)
b=operand2;
else
b=immx;



// ALU unit
if(isAdd==1 || isSub==1 || isCmp==1)
{
	aluResult=a+b;
//int j=aluResult>>>31;
	if(isCmp==1)
	{
		if(aluResult==0)
		{eq=1;gt=0;}
		else if(a>b)
		{eq=0;gt=1;}
		else
		;
	}
}
if(isMul==1)
{aluResult=a*b;}
if(isDiv==1)
{aluResult=a/b;}
if(isMod==1)
aluResult=a%b;
if(isMov==1)
aluResult=b;
if(isLsl==1)
aluResult=a<<b;
if(isLsr==1)
{unsigned int c =a;aluResult=c>>b;}
if(isAsr==1)
aluResult==a>>b;
if(isOr==1)
aluResult=a|b;
if(isNot==1)
aluResult=~b;
if(isAnd==1)
aluResult=a&b;

}

//perform the memory operation
void mem()
{
if(isLd==1)
{
ldResult=read_word(MEM,aluResult);
}
if(isSt==1)
{
write_word(MEM, aluResult,operand2);
}
}

//writes the results back to register file
void write_back()
{
if(isWb==1)
{

//GENERATES DATA
int result;
if(isLd==0 && isCall==0)
result=aluResult;
if(isLd==1 && isCall==0)
result=ldResult;
if(isLd==0 && isCall==1)
result=PC;
//GENERATES ADDRESS AND SAVES IN PROPER LOCATION
unsigned int add;
if(isCall==0)
{
add=instruction_word<<6;
add=add>>28;
R[add]=result;
}
if(isCall==1)
{
R[15]=result;
}

}

}


void run_simplesim() {
  while(1) {
    fetch();
    decode();
    execute();
    mem();
    write_back();
  }
}



int main(int argc, char* argv[]) {
  char* prog_mem_file;
  if(argc < 2) {
    printf("Incorrect number of arguments. Please invoke the simulator \n\t./mySimpleSim <input mem file> \n");
    exit(1);
  }

  //reset the processor
  reset_proc();
  //load the program memory
  load_program_memory(argv[1]);
  //run the simulator
  run_simplesim();

  return 1;
}
