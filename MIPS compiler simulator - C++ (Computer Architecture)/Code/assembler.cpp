#include <iostream>
#include <fstream>
#include <string>
#include <string.h>     //for strcpy
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>  //for open, write, read, close function
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>

using namespace std;

const int MB = 1024 * 1024; //byte

int mem_address[6 * MB / 4];
int * real_mem; //unit: word
int txt_seg;    //the start address of text segment; unit: bit
int txt_end;  
int data_seg;   //the start address of data segment; unit: bit
int data_end;
int stack_seg;  //the start address of stack segment; unit: bit
int dynamic_seg;//the start address of dynamic segment; unit: bit
int dynamic_end;
int * PC;
int * HI;
int * LO;

string input_mips;
string syscall_inputs;
string output_file;

ifstream infile;
ofstream outfile;

map<string, int> register_selector; //store the address of the register
map<string, int *> registers;      //store the content of the register
map<string, int> labels;            //store the address of the laber

enum components
{
    op, rs, rt, rd, shamt, funct, immediate, address
    //6, 5, 5, 5, 5, 6, 16, 26 bits
};

struct instruction
{
    int opcode;
    int function;   //>= 0: R, == -1: I, == -2: J
    string name;
    components tags[3];
    int datas[3];
    void (* fp) (int * a1, int * a2, int * a3);
};


int save_address(int * address){    //real address to simulated address
    return mem_address[address - real_mem]; 
}

int * get_real_address(int address){       //simulated address to real address 
    return real_mem + (address - 4 * MB) / 4;  //unit: word 
}



void memory_init(){ //memory initialization
    mem_address[0] = 4 * MB;    
    for (int i = 1; i < 6 * MB / 4; i++){   //simulated memory
        mem_address[i] = mem_address[i - 1] + 4;
    } 
    real_mem = (int *) malloc(6 * MB);  //point to the start of the real memory of 6MB
    for (int j = 1; j < 6 * MB / 4; j++){   // 4 byte = 1 int address
        real_mem[j] = 0;    //initialize this block of memory
    } 
    txt_seg = 4 * MB;   //according to simulated memory
    txt_end = txt_seg;
    data_seg = 5 * MB;  //according to simulated memory
    stack_seg = 10 * MB - 4;    //according to simulated memory
    PC = (int *) malloc(4);
    HI = (int *) malloc(4);
    LO = (int *) malloc(4);
    * PC = txt_seg; //point to the start of text segment, according to simulated memory
    * HI = 0;
    * LO = 0;

    registers["$zero"] = (int *) malloc(4); //1
    registers["$at"] = (int *) malloc(4);   //2
    registers["$v0"] = (int *) malloc(4);   //3
    registers["$v1"] = (int *) malloc(4);   //4
    registers["$a0"] = (int *) malloc(4);   //5 
    registers["$a1"] = (int *) malloc(4);   //6
    registers["$a2"] = (int *) malloc(4);   //7
    registers["$a3"] = (int *) malloc(4);   //8
    registers["$t0"] = (int *) malloc(4);   //9
    registers["$t1"] = (int *) malloc(4);   //10
    registers["$t2"] = (int *) malloc(4);   //11
    registers["$t3"] = (int *) malloc(4);   //12
    registers["$t4"] = (int *) malloc(4);   //13
    registers["$t5"] = (int *) malloc(4);   //14
    registers["$t6"] = (int *) malloc(4);   //15
    registers["$t7"] = (int *) malloc(4);   //16
    registers["$s0"] = (int *) malloc(4);   //17
    registers["$s1"] = (int *) malloc(4);   //18
    registers["$s2"] = (int *) malloc(4);   //19
    registers["$s3"] = (int *) malloc(4);   //20
    registers["$s4"] = (int *) malloc(4);   //21
    registers["$s5"] = (int *) malloc(4);   //22
    registers["$s6"] = (int *) malloc(4);   //23
    registers["$s7"] = (int *) malloc(4);   //24
    registers["$t8"] = (int *) malloc(4);   //25
    registers["$t9"] = (int *) malloc(4);   //26
    registers["$k0"] = (int *) malloc(4);   //27
    registers["$k1"] = (int *) malloc(4);   //28
    registers["$gp"] = (int *) malloc(4);   //29
    registers["$sp"] = (int *) malloc(4);   //30
    registers["$fp"] = (int *) malloc(4);   //31
    registers["$ra"] = (int *) malloc(4);   //32

    register_selector["$zero"] = 0;
    register_selector["$at"] = 1;
    register_selector["$v0"] = 2;
    register_selector["$v1"] = 3;
    register_selector["$a0"] = 4; 
    register_selector["$a1"] = 5;
    register_selector["$a2"] = 6;
    register_selector["$a3"] = 7;
    register_selector["$t0"] = 8;
    register_selector["$t1"] = 9;
    register_selector["$t2"] = 10;
    register_selector["$t3"] = 11;
    register_selector["$t4"] = 12;
    register_selector["$t5"] = 13;
    register_selector["$t6"] = 14;
    register_selector["$t7"] = 15;
    register_selector["$s0"] = 16;
    register_selector["$s1"] = 17;
    register_selector["$s2"] = 18;
    register_selector["$s3"] = 19;
    register_selector["$s4"] = 20;
    register_selector["$s5"] = 21;
    register_selector["$s6"] = 22;
    register_selector["$s7"] = 23;
    register_selector["$t8"] = 24;
    register_selector["$t9"] = 25;
    register_selector["$k0"] = 26;
    register_selector["$k1"] = 27;
    register_selector["$gp"] = 28;
    register_selector["$sp"] = 29;
    register_selector["$fp"] = 30;
    register_selector["$ra"] = 31;
    for (auto &pointer: registers){ //initialization
        * pointer.second = 0;
    };
    * registers["$sp"] = stack_seg; //points to the top of the stack
    * registers["$fp"] = * registers["$sp"]; //initialized using $sp on a call, points to the top of the stack 
    * registers["$gp"] = data_seg;
}

void cut_out_instruction(istream & is, ostream & os) {   //cut out insructions in .text part and save the label
    string buf;
    bool flag = false; //flag_text
    while (getline(is,buf)){
        //if (buf == "EOF") break;
        if (flag){ //in .text part
            if (int(buf.find(".data",0)) == -1){    //not in another part
                int tag = int(buf.find("#", 0));
                if (tag != -1) { //has comments
                    buf = buf.substr(0,tag); //remove comments
                }
                if (int(buf.find(':',0)) != -1){
                    int pointer = 0;
                    while ((buf[pointer] == ' ') || (buf[pointer] == '\t')){
                        pointer ++;
                    }
                    labels[buf.substr(pointer, int(buf.find(':')) - pointer)] = txt_end;//store the simulated address of labels
                    buf = buf.substr(int(buf.find(':')) + 1);//remove labels

                }
                bool flag_instrction = false;
                for (int i = 0; i < int(buf.size()); i++){
                    if ((buf[i] != ' ') && (buf[i] != '\t')){
                        flag_instrction = true;//search instrction each line (after label), skip blank line
                        buf = buf.substr(i);
                        break;
                    }
                }
                if (flag_instrction){
                    os << buf << endl;//cout instrctions
                    txt_end += 4;
                }
            }
            else{
                flag = false;
            }
            
        }  
        else{ //find the start of .text part
            if (int(buf.find(".text",0)) == 0){   
                flag = true;
            }
        }               
    }
    txt_end = txt_seg;  //reset text_end pointer
}

void free_all(){
    free(real_mem);
    for (auto &pointer: registers){ 
        free(pointer.second);
    };
    free(PC);
    free(HI);
    free(LO);
}

//function tool


string decimal_to_binary_unsigned(long decimal, int bit_limit){ //unsigned
    int remainder;
    int binary[bit_limit];
    int pointer = bit_limit;
    string output = "";
    if (decimal < 0){
        decimal = -decimal;
    }
    while (( decimal != 0) && ( pointer > 0)){
        pointer -= 1;
        remainder = decimal % 2;
        binary[pointer] = remainder;
        decimal /= 2;
    }
    while (pointer > 0){
        pointer -= 1;
        binary[pointer] = 0;
    }
    for (int j: binary)
    {
        output += to_string(j);
    }
    return output;
}

string decimal_to_binary_signed(long decimal, int bit_limit){   //signed
    bool flag = false;
    int remainder;
    int binary[bit_limit];
    int pointer = bit_limit;
    string output = "";
    if (decimal < 0){
        flag = true;
        decimal = -decimal;
    }
    while (( decimal != 0) && ( pointer > 0)){
        pointer -= 1;
        remainder = decimal % 2;
        binary[pointer] = remainder;
        decimal /= 2;
    }
    while (pointer > 0){
        pointer -= 1;
        binary[pointer] = 0;
    }
    if (flag){
        for (int i: binary){
            if (i == 1){
                binary[pointer] = 0;
            }
            else{
                binary[pointer] = 1;
            }
            pointer += 1;
        }        
        while (binary[pointer-1] == 1){
            pointer -= 1;
            binary[pointer] = 0;            
        }
        binary[pointer-1] = 1; 
    }
    for (int j: binary)
    {
        output += to_string(j);
    }
    return output;
}

string decimal_to_binary_signed2(long long decimal, int bit_limit){   //signed make up verson
    bool flag = false;
    int remainder;
    int binary[bit_limit];
    int pointer = bit_limit;
    string output = "";
    if (decimal < 0){
        flag = true;
        decimal = -decimal;
    }
    while (( decimal != 0) && ( pointer > 0)){
        pointer -= 1;
        remainder = decimal % 2;
        binary[pointer] = remainder;
        decimal /= 2;
    }
    while (pointer > 0){
        pointer -= 1;
        binary[pointer] = 0;
    }
    if (flag){
        for (int i: binary){
            if (i == 1){
                binary[pointer] = 0;
            }
            else{
                binary[pointer] = 1;
            }
            pointer += 1;
        }        
        while (binary[pointer-1] == 1){
            pointer -= 1;
            binary[pointer] = 0;            
        }
        binary[pointer-1] = 1; 
    }
    for (int j: binary)
    {
        output += to_string(j);
    }
    return output;
}

long binary_to_decimal_unsigned(string input){  //unsigned
    int length = input.size();
    char binary[length];
    int pointer = length;
    long output = 0;
    long adder = 1;
    for (char i: input){
        binary[length - pointer] = i;
        pointer -= 1;
    }
    for (int j = length-1; j >= 0; j--){
        if (binary[j] == '1'){
            output += adder;
        }
        adder *= 2;
    }
    return output;
}

long binary_to_decimal_signed(string input){    //signed
    bool flag = false;
    int length = input.size();
    char binary[length];
    int pointer = length;
    long output = 0;
    long adder = 1;
    for (char i: input){
        binary[length - pointer] = i;
        pointer -= 1;
    }
    if (binary[0] == '1'){ //negative
        flag = true;
        for (char i: binary){
            if (i == '1'){
                binary[pointer] = '0';
            }
            else{
                binary[pointer] = '1';
            }
            pointer += 1;
        }
        while (binary[pointer-1] == '1'){
            pointer -= 1;
            binary[pointer] = '0';            
        }
        binary[pointer-1] = '1';
    }
    for (int j = length-1; j >= 0; j--){
        if (binary[j] == '1'){
            output += adder;
        }
        adder *= 2;
    }
    if (flag){
        output = -output;
    }
    return output;
}

vector<string> split (string line){
    vector <string> elements;
    string element = "";
    line += ' ';
    for (char i: line){
        if ((i == ' ') || (i == '\t') || (i == ',') ||  (i == '(') || (i == ')')){
            if (element != ""){
                elements.push_back(element);
                element = "";
            }
        }
        else{
            element += i;
        }
    }
    return elements;
}

vector<string> split_ver2 (string line){
    vector <string> elements;
    string element = "";
    line += ' ';
    for (int i = 0; i < int(line.size()); i++){
        if (int(line[i]) == 34){  //in " "
            while (int(line[++i]) != 34){
                if (int(line[i]) == 92){    //backslash
                    switch (line[++i]) {
                    case '0':
                        element += '\0';
                        break;
                    case 't':
                        element += '\t';
                        break;
                    case 'n':
                        element += '\n';
                        break;
                    default:
                        element += '\0';
                        break;
                    }
                }
                else{
                    element += line[i];
                }
            }
            elements.push_back(element);
            element = "";
            i ++;
        }
        if ((line[i] == ' ') || (line[i] == '\t') || (line[i] == ',')){
            if (element != ""){
                elements.push_back(element);
                element = "";
            }
        }
        else{
            element += line[i];
        }
    }
    return elements;
}
//instruction fucntion

void add(int * a1, int * a2, int * a3){     //1: rd, rs, rt [R]
    int result = * a2 + * a3;
    if (((* a2 > 0) && (* a3 > 0) && (result < 0)) ||((* a2 < 0) && (* a3 < 0) && (result > 0))){
        cout << "Trap Exception: Overflow caused by add.";
        exit(0);
    }
    else{
        * a1 = result;
    }
}

void addu(int * a1, int * a2, int * a3){    //2: rd, rs, rt [R]
    * a1 = unsigned(* a2) + unsigned (* a3);
}

void addi(int * a1, int * a2, int * a3){    //3: rt, rs, immediate [I]
    int result = * a2 + * a3;
    if (((* a2 > 0) && (* a3 > 0) && (result < 0)) ||((* a2 < 0) && (* a3 < 0) && (result > 0))){
        cout << "Trap Exception: Overflow caused by addi.";
        exit(0);
    }
    else{
        * a1 = result;
    }
}

void addiu(int * a1, int * a2, int * a3){   //4: rt, rs, immediate [I]
    * a1 = unsigned(* a2) + unsigned (* a3);
}

void and_logical(int * a1, int * a2, int * a3){ //5: rd, rs, rt [R]
    * a1 = * a2 & * a3;
}

void andi(int * a1, int * a2, int * a3){    //6: rt, rs ,immediate [I]
    * a1 = * a2 & * a3;
}

void clo(int * a1, int * a2, int * a3){     //7: rd, rs, rt(0) [R]
    * a3 = 0;
    int result = 0;
    string binary = decimal_to_binary_signed(long(* a2), 32);
    for (char i: binary){
        if (i == '1'){
            result ++;
        }
        else{
            break;
        }
    }
    * a1 = result;
}

void clz(int * a1, int * a2, int * a3){     //8: rd, rs, rt(0) [R]
    * a3 = 0;
    int result = 0;
    string binary = decimal_to_binary_signed(long(* a2), 32);
    for (char i: binary){
        if (i == '0'){
            result ++;
        }
        else{
            break;
        }
    }
    * a1 = result;
}

void div(int * a1, int * a2, int * a3){     //9: rs, rt, rd(0) [R]
    * a3 = 0;
    * HI = * a1 % * a2;
    * LO = * a1 / * a2;
}

void divu(int * a1, int * a2, int * a3){    //10: rs, rt, rd(0) [R]
    * a3 = 0;
    * HI = unsigned(* a1) % unsigned(* a2);
    * LO = unsigned(* a1) / unsigned(* a2);
}

void mult(int * a1, int * a2, int * a3){    //11: rs, rt, rd(0) [R]
    * a3 = 0;
    long long result = (long long)(* a1) * (long long)(* a2);
    string binary = decimal_to_binary_signed2(result,64);
    * HI = binary_to_decimal_signed(binary.substr(0, 32));
    * LO = binary_to_decimal_signed(binary.substr(32));
}

void multu(int * a1, int * a2, int * a3){   //12: rs, rt, rd(0) [R]
    * a3 = 0;
    long long result = static_cast<long long>(unsigned(* a1)) * static_cast<long long>(unsigned(* a2));
    string binary = decimal_to_binary_signed2(result,64);
    * HI = binary_to_decimal_signed(binary.substr(0, 32));
    * LO = binary_to_decimal_signed(binary.substr(32));
}

void mul(int * a1, int * a2, int * a3){     //13: rd, rs, rt [R]
    long result = long(* a2) * long(* a3);
    string binary = decimal_to_binary_signed(result,64);
    * a1 = binary_to_decimal_signed(binary.substr(32));
}

void madd(int * a1, int * a2, int * a3){    //14: rs, rt, rd(0) [R]
    * a3 = 0;
    string adder1 = decimal_to_binary_signed(* HI, 32) + decimal_to_binary_signed(* LO, 32);
    long sum = long(* a2) * long(* a3) + binary_to_decimal_signed(adder1);
    string result = decimal_to_binary_signed(sum, 64);
    * HI = binary_to_decimal_signed(result.substr(0, 32));
    * LO = binary_to_decimal_signed(result.substr(32));
}

void maddu(int * a1, int * a2, int * a3){   //15: rs, rt, rd(0) [R]
    * a3 = 0;
    string adder1 = decimal_to_binary_signed(* HI, 32) + decimal_to_binary_signed(* LO, 32);
    long sum = long(unsigned(* a2)) * long(unsigned(* a3)) + binary_to_decimal_signed(adder1);
    string result = decimal_to_binary_signed(sum, 64);
    * HI = binary_to_decimal_signed(result.substr(0, 32));
    * LO = binary_to_decimal_signed(result.substr(32));
}

void msub(int * a1, int * a2, int * a3){    //16: rs, rt, rd(0) [R]
    * a3 = 0;
    string adder1 = decimal_to_binary_signed(* HI, 32) + decimal_to_binary_signed(* LO, 32);
    long sum = long(* a2) * long(* a3) - binary_to_decimal_signed(adder1);
    string result = decimal_to_binary_signed(sum, 64);
    * HI = binary_to_decimal_signed(result.substr(0, 32));
    * LO = binary_to_decimal_signed(result.substr(32));
}

void msubu(int * a1, int * a2, int * a3){   //17: rs, rt, rd(0) [R]
    * a3 = 0;
    string adder1 = decimal_to_binary_signed(* HI, 32) + decimal_to_binary_signed(* LO, 32);
    long sum = long(unsigned(* a2)) * long(unsigned(* a3)) - binary_to_decimal_signed(adder1);
    string result = decimal_to_binary_signed(sum, 64);
    * HI = binary_to_decimal_signed(result.substr(0, 32));
    * LO = binary_to_decimal_signed(result.substr(32));
}

void nor(int * a1, int * a2, int * a3){     //18: rd, rs, rt [R]
    * a1 = ~(* a2 | * a3);
}

void or_logical(int * a1, int * a2, int * a3){  //19: rd, rs, rt [R]
    * a1 = * a2 | * a3;
}

void ori(int * a1, int * a2, int * a3){     //20: rt, rs, immediate [I]
    * a1 = * a2 | * a3;
}

void sll(int * a1, int * a2, int * a3){     //21: rd, rt, shamt [R]
    * a1 = * a2 << * a3;
}

void sllv(int * a1, int * a2, int * a3){   //22: rd, rt, rs [R]
    * a1 = * a2 << * a3;
}   

void sra(int * a1, int * a2, int * a3){     //23: rd, rt, shamt [R] 
    * a1 = * a2 >> * a3;
}

void srav(int * a1, int * a2, int * a3){     //24: rd, rt, rs [R] 
    * a1 = * a2 >> * a3;
}

void srl(int * a1, int * a2, int * a3){     //25: rd, rt, shamt [R]
    * a1 = unsigned(* a2) >> * a3;
}

void srlv(int * a1, int * a2, int * a3){    //26: rd, rt, rs [R]
    * a1 = unsigned(* a2) >> * a3;
}

void sub(int * a1, int * a2, int * a3){     //27: rd, rs, rt [R]
    int result = * a2 - * a3;
    if (((* a2 > 0) && (* a3 < 0) && (result < 0)) ||((* a2 < 0) && (* a3 > 0) && (result > 0))){
        cout << "Trap Exception: Overflow caused by sub.";
        exit(0);
    }
    else * a1 = result;
}

void subu(int * a1, int * a2, int * a3){    //28: rd, rs, rt [R]
    * a1 = unsigned(* a2) - unsigned(* a3);
}

void xor_logical(int * a1, int * a2, int * a3){ //29: rd, rs, rt [R]
    * a1 = * a2 ^ * a3;
}

void xori(int * a1, int * a2, int * a3){ //30: rd, rs, immediate [I]
    * a1 = * a2 ^ * a3;
}

void lui(int * a1, int * a2, int * a3){ //31: rt, immediate, rs [I] 
    * a3 = 0;
    string half = decimal_to_binary_signed(long(* a2), 16);
    half += "0000000000000000";
    * a1 = binary_to_decimal_signed(half);
}

void slt(int * a1, int * a2, int * a3){    //32: rd, rs, rt [R]
    if (* a2 < * a3){
        * a1 = 1;
    } 
    else {
    * a1 = 0;
    }
}

void sltu(int * a1, int * a2, int * a3){    //33: rd, rs, rt [R]
    if (unsigned(* a2) < unsigned(* a3)){
        * a1 = 1;
    } 
    else {
    * a1 = 0;
    }
}

void slti(int * a1, int * a2, int * a3){    //34: rt, rs, immediate [I]
    if (* a2 < * a3){
        * a1 = 1;
    } 
    else {
    * a1 = 0;
    }
}

void sltiu(int * a1, int * a2, int * a3){   //35: rs, rt, immediate [R]
    if (unsigned(* a2) < unsigned(* a3)){
        * a1 = 1;
    } 
    else {
    * a1 = 0;
    }
}

void beq(int * a1, int * a2, int * a3){     //36: rs, rt, offset(immediate) [I]
    if (* a1 == * a2){
        * PC += * a3 * 4 + 4;
    }
    else{
        * PC += 4;
    }
}

void bgez(int * a1, int * a2, int * a3){     //37: rs, offset(immediate), rt [I]
    * a3 = 1;
    if (* a1 >= 0){
        * PC += * a2 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void bgezal(int * a1, int * a2, int * a3){  //38: rs, offset(immediate), rt [I]
    * a3 = 17;
    * registers["$ra"] = * PC + 4;
    if (* a1 >= 0){
        * PC += * a2 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void bgtz(int * a1, int * a2, int * a3){    //39: rs, offset(immediate), rt [I]
    * a3 = 0;
    if (* a1 > 0){
        * PC += * a2 * 4 + 4;
    }
    else{
        * PC += 4;
    }
}

void blez(int * a1, int * a2, int * a3){    //40: rs, offset(immediate), rt [I]
    * a3 = 0;
    if (* a1 <= 0){
        * PC += * a2 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void bltzal(int * a1, int * a2, int * a3){  //41: rs, offset(immediate), rt [I]
    * a3 = 16;
    * registers["$ra"] = * PC + 4;
    if (* a1 < 0){
        * PC += * a2 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void bltz(int * a1, int * a2, int * a3){    //42: rs, offset(immediate), rt [I]
    * a3 = 0;
    if (* a1 < 0){
        * PC += * a2 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void bne(int * a1, int * a2, int * a3){     //43: rs, rt, offset(immediate) [I] 
    if (* a1 != * a2){
        * PC += * a3 * 4 + 4;
    } 
    else{
        * PC += 4;
    }
}

void j(int * a1, int * a2, int * a3){       //44: address, op(placeholder), op(placeholder) [J]
    * a2 = 2;
    * a3 = 2;
    string part1 = decimal_to_binary_signed(long(* PC), 32).substr(0, 4);
    string part2 = decimal_to_binary_signed(long(* a1),26);
    * PC = int(binary_to_decimal_signed(part1 + part2 + "00"));
}

void jal(int * a1, int * a2, int * a3){     //45: address, op(placeholder), op(placeholder) [J]
    * registers["$ra"] = * PC + 4;
    * a2 = 3;
    * a3 = 3;
    string part1 = decimal_to_binary_signed(long(* PC), 32).substr(0, 4);
    string part2 = decimal_to_binary_signed(long(* a1),26);
    * PC = int(binary_to_decimal_signed(part1 + part2 + "00"));
}

void jalr(int * a1, int * a2, int * a3){    //46: rs, rd, rt [R]
    * a3 = 0;
    * a2 = * PC + 4; //usually $ra
    * PC = * a1;
}

void jr(int * a1, int * a2, int * a3){      //47: rs, rt, rt [R]
    * a2 = 0;
    * a3 = 0;
    * PC = * a1;
}

void teq(int * a1, int * a2, int * a3){     //48: rs, rt, rd [R]
    * a3 = 0;
    if (* a1 == * a2) {     
        cout << "Trap Exception.";
        exit(0);
    }
}

void teqi(int * a1, int * a2, int * a3){    //49: rs, immediate, rt [I]
    * a3 = 12;
    if (* a1 == * a2) {     
        cout << "Trap Exception.";
        exit(0);
    }
}

void tne(int * a1, int * a2, int * a3){     //50: rs, rt, rd [R]
    * a3 = 0;
    if (* a1 != * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tnei(int * a1, int * a2, int * a3){    //51: rs, immediate, rt [i]
    * a3 = 14;
    if (* a1 != * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tge(int * a1, int * a2, int * a3){     //52: rs, rt, rd [R]
    * a3 = 0;
    if (* a1 >= * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tgeu(int * a1, int * a2, int * a3){     //53: rs, rt, rd [R]
    * a3 = 0;
    if (unsigned(* a1) >= unsigned(* a2)) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tgei(int * a1, int * a2, int * a3){     //54: rs, immediate, rt [I]
    * a3 = 8;
    if (* a1 >= * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tgeiu(int * a1, int * a2, int * a3){     //55: rs, immediate, rt [I]
    * a3 = 9;
    if (unsigned(* a1) >= unsigned(* a2)) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tlt(int * a1, int * a2, int * a3){     //56: rs, rt, rd [R]  
    * a3 = 0;
    if (* a1 < * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tltu(int * a1, int * a2, int * a3){     //57: rs, rt, rd [R]  
    * a3 = 0;
    if (unsigned(* a1) < unsigned(* a2)) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tlti(int * a1, int * a2, int * a3){    //58: rs, immediate, rt [I]
    * a3 = 10;
    if (* a1 < * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void tltiu(int * a1, int * a2, int * a3){    //59: rs, immediate, rt [I]
    * a3 = 11;
    if (* a1 < * a2) {
        cout << "Trap Exception.";
        exit(0);
    }
}

void lb(int * a1, int * a2, int * a3){      //60: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2; 
    string binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string byte = binary.substr(24 - (addr - 4 * MB) % 4 * 8, 8);
    * a1 = int(binary_to_decimal_signed(byte));
}

void lbu(int * a1, int * a2, int * a3){     //61: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    string binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string byte = binary.substr(24 - (addr - 4 * MB) % 4 * 8, 8);
    * a1 = int(binary_to_decimal_unsigned(byte));
}

void lh(int * a1, int * a2, int * a3){      //62: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    string binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string byte = binary.substr(16 - (addr - 4 * MB) % 4 * 8, 16);
    * a1 = int(binary_to_decimal_signed(byte));
}

void lhu(int * a1, int * a2, int * a3){     //63: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    string binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string byte = binary.substr(16 - (addr - 4 * MB) % 4 * 8, 16);
    * a1 = int(binary_to_decimal_unsigned(byte));
}

void lw(int * a1, int * a2, int * a3){      //64: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2; //should be a multiple of 4
    * a1 = * get_real_address(addr);
}

void lwl(int * a1, int * a2, int * a3){     //65: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    string load_data = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string register_data = decimal_to_binary_signed(* get_real_address(* a1), 32);
    int length = (4 - addr % 4) * 8;
    * a1 = binary_to_decimal_signed(load_data.substr(32 - length, length) + register_data.substr(length));
}

void lwr(int * a1, int * a2, int * a3){     //66: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    string load_data = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    string register_data = decimal_to_binary_signed(* get_real_address(* a1), 32);
    int length = addr % 4 * 8;
    * a1 = binary_to_decimal_signed(register_data.substr(0, 32 - length) + load_data.substr(0, length));
}

void ll(int * a1, int * a2, int * a3){      //67: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2; //should be a multiple of 4
    * a1 = * get_real_address(addr);
}

void sb(int * a1, int * a2, int * a3){      //68: rt, offset(immediate), rs [I]
    string byte = decimal_to_binary_signed(long(* a1), 32).substr(24);
    int addr = * a3 + * a2;
    string word = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    int start = 24 - (addr - 4 * MB) % 4 * 8;
    word = word.substr(0, start) + byte + word.substr(start + 8);
    * get_real_address(addr) = binary_to_decimal_signed(word);
}

void sh(int * a1, int * a2, int * a3){      //69: rt, offset(immediate), rs [I]
    string byte = decimal_to_binary_signed(long(* a1), 32).substr(16);
    int addr = * a3 + * a2;
    string word = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
    int start = 16 - (addr - 4 * MB) % 4 * 8;
    word = word.substr(0, start) + byte + word.substr(start + 8);
    * get_real_address(addr) = binary_to_decimal_signed(word);
}

void sw(int * a1, int * a2, int * a3){      //70: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    * get_real_address(addr) = * a1;
}

void swl(int * a1, int * a2, int * a3){     //71: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    int length = 32 - addr % 4 * 8;
    string bytes = decimal_to_binary_signed(* a1, 32).substr(0,length);
    string word = decimal_to_binary_signed(* get_real_address(addr), 32);
    word = word.substr(0, 32 - length) + bytes;
    * get_real_address(addr) = binary_to_decimal_signed(word);
}

void swr(int * a1, int * a2, int * a3){     //72: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    int length = addr % 4 * 8;
    string bytes = decimal_to_binary_signed(* a1, 32).substr(32 - length);
    string word = decimal_to_binary_signed(* get_real_address(addr), 32);
    word = bytes + word.substr(length);
    * get_real_address(addr) = binary_to_decimal_signed(word);
}

void sc(int * a1, int * a2, int * a3){      //73: rt, offset(immediate), rs [I]
    int addr = * a3 + * a2;
    * get_real_address(addr) = * a1;
}

void mfhi(int * a1, int * a2, int * a3){    //74: rd, rs, rt [R]
    * a2 = 0;
    * a3 = 0;
    * a1 = * HI;
}

void mflo(int * a1, int * a2, int * a3){    //75: rd, rs, rt [R]
    * a2 = 0;
    * a3 = 0;
    * a1 = * LO;    
}

void mthi(int * a1, int * a2, int * a3){    //76: rs, rt, rd [R]
    * a2 = 0;
    * a3 = 0;
    * HI = * a1;
}

void mtlo(int * a1, int * a2, int * a3){    //77: rs, rt, rd [R]
    * a2 = 0;
    * a3 = 0;
    * LO = * a1;
}

void syscall(int * a1, int * a2, int * a3){ //78: rs, rt, rd [R] 
    * a1 = 0;
    * a2 = 0;
    * a3 = 0;
    int addr;
    int length;
    int base = 10;
    char c;
    char * pointer;
    string binary;
    string byte;
    string word = "";
    switch (* registers["$v0"])
    {
    case 1:
        //cout << "come 1" << endl;
        outfile << * registers["$a0"];
        break;
    case 4:
        //cout << "come 4" << endl;
        addr = * registers["$a0"];
        while ((c = binary_to_decimal_unsigned(decimal_to_binary_signed(long(* get_real_address(addr)), 32).substr((24 - (addr - 4 * MB) % 4 * 8),8))) != '\0'){
            outfile << c;
            addr ++;
        };
        break;
    case 5:
        //cin >> * registers["$v0"];
        getline(infile,word);
        if (word.substr(0,2) == "0x"){
            base = 16;
        }
        * registers["$v0"] = stoi(word, 0, base);
        break;
    case 8: //to cater to the 64-bits equipment, store char by char in little endian
        addr = * registers["$a0"];
        length = * registers["$a1"];
        pointer = (char *) malloc(length);
        //fgets(pointer, length, stdin);
        infile.getline(pointer, length);
        for (int i = 0; i < length; i++){
            binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
            byte = decimal_to_binary_signed(pointer[i], 8);
            binary = binary.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + byte + binary.substr(32 - (addr - 4 * MB) % 4 * 8);
            * get_real_address(addr) = int(binary_to_decimal_unsigned(binary));
            addr ++;
        }
        binary = decimal_to_binary_signed(long(* get_real_address(addr)), 32);  
        byte = decimal_to_binary_signed('\0', 8);   //store the end of the string
        binary = binary.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + byte + binary.substr(32 - (addr - 4 * MB) % 4 * 8);
        * get_real_address(addr) = int(binary_to_decimal_unsigned(binary));
        free(pointer);
        break;
    case 9:
        * registers["$v0"] = dynamic_end;
        dynamic_end += * registers["$a0"];
        break;
    case 10:
        //cout << "come 10" << endl;
        exit(0);
        break;
    case 11:
        outfile << char(* registers["$a0"]);
        break;
    case 12: 
        getline(infile,word) >> c;
        * registers["$v0"] = c;
        break;
    case 13:
        addr = * registers["$a0"];
        while ((c = binary_to_decimal_unsigned(decimal_to_binary_signed(long(* get_real_address(addr)), 32).substr((24 - (addr - 4 * MB) % 4 * 8), 8))) != '\0'){
            word += c;
        } 
        pointer = (char *) malloc(word.size());
        strcpy(pointer, word.c_str());  //strcpy: copies a string containing the terminator '\0' into another address space and returns a value of type char*.
        * registers["$v0"] = open(pointer, * registers["$a1"], * registers["$a2"]); //int open(const char *pathname, int flags, mode_t mode);
        break;
    case 14:
        * registers["$v0"] = read(* registers["$a0"], get_real_address(* registers["$a1"]), * registers["$a2"]);    //ssize_t read(int fd, void *buf, size_t count);
        break;
    case 15:
        * registers["$v0"] = write(* registers["$a0"], get_real_address(* registers["$a1"]), * registers["$a2"]);   //ssize_t write(int fd, const void *buf, size_t count);
        break;
    case 16: 
        close(* registers["$a0"]);  //int close(int fd);
        break;
    case 17:
        exit(* registers["$a0"]);
        break;
    default:
        cout << "syscall order error." << endl;
        break;  
    }  
}


instruction instruction_set[78] = {
    {0, 32, "add", {rd, rs, rt}, {0 ,0 ,0}, add},
    {0, 33, "addu", {rd, rs, rt}, {0 ,0 ,0}, addu},
    {8, -1, "addi", {rt, rs, immediate}, {0 ,0 ,0}, addi},
    {9, -1, "addiu", {rt, rs, immediate}, {0 ,0 ,0}, addiu},
    {0, 36, "and", {rd, rs, rt}, {0 ,0 ,0}, and_logical},
    {12, -1, "andi", {rt, rs ,immediate}, {0 ,0 ,0}, andi},
    {28, 33, "clo", {rd, rs, rt}, {0 ,0 ,0}, clo},
    {28, 32, "clz", {rd, rs, rt}, {0 ,0 ,0}, clz},
    {0, 26, "div", {rs, rt ,rd}, {0 ,0 ,0}, div},
    {0, 27, "divu", {rs, rt, rd}, {0 ,0 ,0}, divu},
    {0, 24, "mult", {rs, rt, rd}, {0 ,0 ,0}, mult},
    {0, 25, "multu", {rs, rt, rd}, {0 ,0 ,0}, multu},
    {28, 2, "mul", {rd, rs, rt}, {0 ,0 ,0}, mul},
    {28, 0, "madd", {rs, rt, rd}, {0 ,0 ,0}, madd},
    {28, 1, "maddu", {rs, rt, rd}, {0 ,0 ,0}, maddu},
    {28, 4, "msub", {rs, rt, rd}, {0 ,0 ,0}, msub},
    {28, 5, "maddu", {rs, rt, rd}, {0 ,0 ,0}, maddu},
    {0, 39, "nor", {rd, rs, rt}, {0 ,0 ,0}, nor},
    {0, 37, "or", {rd, rs, rt}, {0 ,0 ,0}, or_logical},
    {13, -1, "ori", {rt, rs, immediate}, {0 ,0 ,0}, ori},
    {0, 0, "sll", {rd, rt, shamt}, {0 ,0 ,0}, sll},
    {0, 4, "sllv", {rd, rt, rs}, {0 ,0 ,0}, sllv},
    {0, 3, "sra", {rd, rt, shamt}, {0 ,0 ,0}, sra},
    {0, 7, "srav", {rd, rt, rs}, {0 ,0 ,0}, srav},
    {0, 2, "srl", {rd, rt, shamt}, {0 ,0 ,0}, srl},
    {0, 6, "srlv", {rd, rt, rs}, {0 ,0 ,0}, srlv},
    {0, 34, "sub", {rd, rs, rt}, {0 ,0 ,0}, sub},
    {0, 35, "subu", {rd, rs, rt}, {0 ,0 ,0}, subu},
    {0, 38, "xor", {rd, rs, rt}, {0 ,0 ,0}, xor_logical},
    {14, -1, "xori", {rt, rs, immediate}, {0 ,0 ,0}, xori},
    {15, -1, "lui", {rt, immediate, rs}, {0 ,0 ,0}, lui},
    {0, 42, "slt", {rd, rs, rt}, {0 ,0 ,0}, slt},
    {0, 43, "sltu", {rd, rs, rt}, {0 ,0 ,0}, sltu},
    {10, -1, "slti", {rt, rs, immediate}, {0 ,0 ,0}, slti},
    {11, -1, "sltiu", {rt, rs, immediate}, {0 ,0 ,0}, sltiu},
    {4, -1, "beq", {rs, rt, immediate}, {0 ,0 ,0}, beq},
    {1, -1, "bgez", {rs, immediate, rt}, {0 ,0 ,1}, bgez},
    {1, -1, "bgezal", {rs, immediate, rt}, {0 ,0 ,17}, bgezal},
    {7, -1, "bgtz", {rs, immediate, rt}, {0 ,0 ,0}, bgtz},
    {6, -1, "blez", {rs, immediate, rt}, {0 ,0 ,0}, blez},
    {1, -1, "bltzal", {rs, immediate, rt}, {0 ,0 ,16}, bltzal},
    {1, -1, "bltz", {rs, immediate, rt}, {0 ,0 ,0}, bltz},
    {5, -1, "bne", {rs, rt, immediate}, {0 ,0 ,0}, bne},
    {2, -2, "j", {address, op, op}, {0 ,0 ,0}, j},
    {3, -2, "jal", {address, op, op}, {0 ,0 ,0}, jal},
    {0, 9, "jalr", {rs, rd, rt}, {0 ,0 ,0}, jalr},
    {0, 8, "jr", {rs, rt, rt}, {0 ,0 ,0}, jr},
    {0, 52, "teq", {rs, rt, rd}, {0 ,0 ,0}, teq},
    {1, -1, "teqi", {rs, immediate, rt}, {0 ,0 ,12}, teqi},
    {0, 54, "tne", {rs, rt, rd}, {0 ,0 ,0}, tne},
    {1, -1, "tnei", {rs, immediate, rt}, {0 ,0 ,14}, tnei},
    {0, 48, "tge", {rs, rt, rd}, {0 ,0 ,0}, tge},
    {0, 49, "tgeu", {rs, rt, rd}, {0 ,0 ,0}, tgeu},
    {1, -1, "tgei", {rs, immediate, rt}, {0 ,0 ,8}, tgei},
    {1, -1, "tgeiu", {rs, immediate, rt}, {0 ,0 ,9}, tgeiu},
    {0, 50, "tlt", {rs, rt, rd}, {0 ,0 ,0}, tlt},
    {0, 51, "tltu", {rs, rt, rd}, {0 ,0 ,0}, tltu},
    {1, -1, "tlti", {rs, immediate, rt}, {0 ,0 ,10}, tlti},
    {1, -1, "tltiu", {rs, immediate, rt}, {0 ,0 ,11}, tltiu},
    {32, -1, "lb", {rt, immediate, rs}, {0 ,0 ,0}, lb},
    {36, -1, "lbu", {rt, immediate, rs}, {0 ,0 ,0}, lbu},
    {33, -1, "lh", {rt, immediate, rs}, {0 ,0 ,0}, lh},
    {37, -1, "lhu", {rt, immediate, rs}, {0 ,0 ,0}, lhu},
    {35, -1, "lw", {rt, immediate, rs}, {0 ,0 ,0}, lw},
    {34, -1, "lwl", {rt, immediate, rs}, {0 ,0 ,0}, lwl},
    {38, -1, "lwr", {rt, immediate, rs}, {0 ,0 ,0}, lwr},
    {48, -1, "ll", {rt, immediate, rs}, {0 ,0 ,0}, ll},
    {40, -1, "sb", {rt, immediate, rs}, {0 ,0 ,0}, sb},
    {41, -1, "sh", {rt, immediate, rs}, {0 ,0 ,0}, sh},
    {43, -1, "sw", {rt, immediate, rs}, {0 ,0 ,0}, sw},
    {42, -1, "swl", {rt, immediate, rs}, {0 ,0 ,0}, swl},
    {46, -1, "swr", {rt, immediate, rs}, {0 ,0 ,0}, swr},
    {56, -1, "sc", {rt, immediate, rs}, {0 ,0 ,0}, sc},
    {0, 16, "mfhi", {rd, rs, rt}, {0 ,0 ,0}, mfhi},
    {0, 18, "mflo", {rd, rs, rt}, {0 ,0 ,0}, mflo},
    {0, 17, "mthi", {rs, rt, rd}, {0 ,0 ,0}, mthi},
    {0, 19, "mtlo", {rs, rt, rd}, {0 ,0 ,0}, mtlo},
    {0, 12, "syscall", {rs, rt, rd}, {0 ,0 ,0}, syscall}
};

void translator_to_code(components c, int decimal, string & base){
    switch(c){
        case op: 
            base = decimal_to_binary_unsigned(long(decimal), 6) + base.substr(6);
            break;
        case funct:
            base = base.substr(0, 26) + decimal_to_binary_unsigned(long(decimal), 6);
            break;
        case rd: 
            base = base.substr(0, 16) + decimal_to_binary_unsigned(long(decimal), 5) + base.substr(21);
            break;
        case rs: 
            base = base.substr(0, 6) + decimal_to_binary_unsigned(long(decimal), 5) + base.substr(11);
            break;
        case rt: 
            base = base.substr(0, 11) + decimal_to_binary_unsigned(long(decimal), 5) + base.substr(16);
            break;
        case shamt:
            base = base.substr(0, 21) + decimal_to_binary_signed(long(decimal), 5) + base.substr(26);
            break;
        case immediate:
            base = base.substr(0, 16) + decimal_to_binary_signed(long(decimal), 16);
            break;
        case address:
            base = base.substr(0, 6) + decimal_to_binary_signed(long(decimal), 32).substr(4, 26);
            break;
    }
}

int code_translator(components c, string binary){
    switch(c){
        case op: 
            return binary_to_decimal_unsigned(binary.substr(0, 6));
        case funct:
            return binary_to_decimal_unsigned(binary.substr(26, 6));
        case rd: 
            return binary_to_decimal_unsigned(binary.substr(16, 5));
        case rs: 
            return binary_to_decimal_unsigned(binary.substr(6, 5));
        case rt: 
            return binary_to_decimal_unsigned(binary.substr(11, 5));
        case shamt:
            return binary_to_decimal_signed(binary.substr(21, 5));
        case immediate:
            return binary_to_decimal_signed(binary.substr(16, 16));
        case address:
            return binary_to_decimal_signed(binary.substr(6, 26));
        default:
            cout << "fail to translate code" << endl;
            return 0;
    }
}

instruction identity_instruction(string binary){
    instruction instruct;
    int opcode = code_translator(op, binary);
    int function;
    if ((opcode == 0)||(opcode == 28)){
        function = code_translator(funct, binary);
        for (int i = 0; i < 78; ++i) {
            if ((instruction_set[i].opcode == opcode)&&(instruction_set[i].function == function)) {
                return instruction_set[i];
            }
        }
    }
    else if(opcode == 1){
        function = code_translator(rt, binary);
        for (int i = 0; i < 78; ++i) {
            if ((instruction_set[i].opcode == opcode)&&(instruction_set[i].datas[2] == function)) {
                return instruction_set[i];
            }
        }
    }
    else{
        for (int i = 0; i < 78; ++i) {
            if (instruction_set[i].opcode == opcode) {
                return instruction_set[i];
            }
        }
    }
    cout << "fail to identify the instruction" << endl;
    return instruction_set[0];
}

void pass_argument(components c, int i, int ** a){
    if ((c == rd)||(c == rs)||(c == rt)){
        for (auto j: register_selector){
            if (j.second == i){
                * a = registers[j.first];
                break;
            }
        }
    }
    else if ((c == immediate)||(c == shamt)||(c == address)){
        ** a = i;
    } 
    else ** a = 0;
}

vector < instruction > tokenize(istream & is){
    string line;
    vector < string > elements;
    vector < instruction > all_instructions;
    instruction instruct;
    while (getline(is, line)){
        int pointer = -1;
        instruction instruct;
        elements = split(line);
        for (string str: elements){
            if (pointer == -1){  //str is the name of the instruction
                for (int i = 0; i < 78; i++) {
                    if (instruction_set[i].name == str) {
                        instruct = instruction_set[i];  //determine the type of the instruction
                    }
                }
            }
            else if (pointer <= int(elements.size())){   //set register data
                if (instruct.tags[pointer] < 4){  //data in registers: rs, rt, rd
                    instruct.datas[pointer] = register_selector[str]; 
                }
                else if (instruct.tags[pointer] == 4){
                    instruct.datas[pointer] = stoi(str);
                }
                else if (instruct.tags[pointer] == 6){  //immediate
                    if (isdigit(str[0]) || str[0] == '-'){  //is number
                        int base = 10;
                        if (str.substr(0,2) == "0x"){
                            base = 16;
                        }
                        instruct.datas[pointer] = stoi(str, 0, base);
                    }
                    else{   //label
                        instruct.datas[pointer] =  (labels[str] - txt_end) / 4 - 1;
                    }
                }
                else if (instruct.tags[pointer] == 7){  //address(label)
                    instruct.datas[pointer] = labels[str];
                }   
            }
            pointer++;
        }
        all_instructions.push_back(instruct);
        txt_end += 4;
    }
    txt_end = txt_seg;  //reset text_end pointer
    return all_instructions;
}

void assembler(vector < instruction > all_instructions, ostream & os){
    for (instruction instruct: all_instructions){
        string base = "00000000000000000000000000000000";
        translator_to_code(op, instruct.opcode, base);
        if (instruct.function >= 0){    //R-format
            translator_to_code(funct, instruct.function, base);
            for (int i = 0; i < 3; i++){
                translator_to_code(instruct.tags[i], instruct.datas[i], base); 
            }
        }
        else if (instruct.function == -1){   //I-format
            for (int i = 0; i < 3; i++){
                translator_to_code(instruct.tags[i], instruct.datas[i], base); 
            }
        }
        else if (instruct.function == -2){  //J-format
            translator_to_code(address, instruct.datas[0], base);
        }
    os << base << endl;
    * get_real_address(txt_end) = binary_to_decimal_unsigned(base); //store machine code in the memory
    txt_end += 4; 
    }
}

void save_data(){
    vector<string> elements;
    string buf = ""; //line buffer
    string str; //string buffer
    int addr = data_seg;
    bool flag = false; //flag_data
    while (getline(infile,buf)){
        //if (buf == "EOF") break;
        int start = -1;
        if (flag){ //in .data part
            int tag = int(buf.find("#", 0));
            if (tag != -1) { //has comments
                buf = buf.substr(0,tag); //remove comments
            }
            elements = split_ver2(buf);
            if (elements.size() == 0){  //an empty line, or a line consists of comments
                continue;   
            }
            for (int i = 0; i < int(elements.size()); i++){
                if (int(elements[i].find(".",0)) == 0){ //find data type
                    start = i;
                    break;
                }
            }
            if ((int(buf.find(".text",0)) == 0)||(start == -1)){
                break;
            }
            if (elements[start] == ".ascii"){
                string buffer;
                for (int i = start + 1; i < int(elements.size()); i++ ){
                    str = elements[i];
                    for (char c: str){
                        buffer = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
                        buffer = buffer.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed(long(c), 8) + buffer.substr(32 - (addr - 4 * MB) % 4 * 8);
                        * get_real_address(addr) = binary_to_decimal_signed(buffer);
                        addr += 1;
                    }
                }
                //cout<<addr % 4<<endl;
                while ((addr - 4 * MB) % 4 * 8 != 0){
                    addr += 1;
                }
            }
            else if (elements[start] == ".asciiz"){
                string buffer;
                str = elements[start + 1];
                //cout<<str<<endl;
                for (char c: str){
                    buffer = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
                    buffer = buffer.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed(long(c), 8) + buffer.substr(32 - (addr - 4 * MB) % 4 * 8);
                    * get_real_address(addr) = binary_to_decimal_signed(buffer);
                    addr += 1;
                }
                buffer = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
                buffer = buffer.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed('\0', 8) + buffer.substr(32 - (addr - 4 * MB) % 4 * 8);
                * get_real_address(addr) = binary_to_decimal_signed(buffer);
                addr += 1;
                while ((addr - 4 * MB) % 4 * 8 != 0){
                    addr += 1;
                }
            }
            else if (elements[start] == ".word"){
                for (int i = start + 1; i < int(elements.size()); i++ ){
                    int base = 10;
                    if (elements[start + 1].substr(0,2) == "0x"){
                        base = 16;
                    }
                    * get_real_address(addr) = stoi(elements[start + 1], 0, base);
                    addr += 4;
                }
            }
            else if (elements[start] == ".half"){
                for (int i = start + 1; i < int(elements.size()); i++ ){
                    str = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
                    int base = 10;
                    if (elements[i].substr(0,2) == "0x"){
                        base = 16;
                    }
                    str = str.substr(0, 16 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed(long(stoi((elements[i]), 0, base)), 16) + str.substr(32 - (addr - 4 * MB) % 4 * 8);
                    * get_real_address(addr) = binary_to_decimal_signed(str);
                    addr += 2;
                }
                if ((addr - 4 * MB) % 4 * 8 == 2){  //(addr - 4 * MB) % 4 * 8 can only be 2 or 0
                    addr += 2;
                }
            }
            else if (elements[start] == ".byte"){   //can only store char
                for (int i = start + 1; i < int(elements.size()); i++ ){
                    str = decimal_to_binary_signed(long(* get_real_address(addr)), 32);
                    if (isdigit(elements[i][0])){
                        int base = 10;
                        if (elements[i].substr(0,2) == "0x"){
                            base = 16;
                        }
                        str = str.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed(long(stoi((elements[i]), 0, base)), 8) + str.substr(32 - (addr - 4 * MB) % 4 * 8);
                    }
                    else{
                        str = str.substr(0, 24 - (addr - 4 * MB) % 4 * 8) + decimal_to_binary_signed(long(elements[i][1]), 8) + str.substr(32 - (addr - 4 * MB) % 4 * 8);
                    }
                    * get_real_address(addr) = binary_to_decimal_signed(str);
                    addr += 1;
                }
                while ((addr - 4 * MB) % 4 * 8 != 0){
                    addr += 1;
                }
            }
            else {
                cout << "data type error!" << endl;
            }        
        }
        else{ //find the start of .data part
            if (int(buf.find(".data",0)) == 0){   
                flag = true;
            }
        } 
    }
    dynamic_seg = addr;
    dynamic_end = addr;
}

void simulater(){
    string binary;
    instruction instruct;
    while (* PC != txt_end) {
        int i1 = 0, i2 = 0, i3 = 0;
        int *a1, *a2, *a3;
        a1 = &i1;
        a2 = &i2;
        a3 = &i3;
        binary = decimal_to_binary_signed(long(* get_real_address(* PC)), 32);
        instruct = identity_instruction(binary);
        //cout<<binary<<endl;
        for (int i = 0; i < 3; i++){
            instruct.datas[i] = code_translator(instruct.tags[i], binary);
        }

        //cout << instruct.name <<endl;

        pass_argument(instruct.tags[0], instruct.datas[0], & a1);
        pass_argument(instruct.tags[1], instruct.datas[1], & a2);
        pass_argument(instruct.tags[2], instruct.datas[2], & a3);

        //cout<<*a1<<"|"<<*a2<<"|"<<*a3<<endl;

        instruct.fp(a1, a2, a3);
        //cout<<*a1<<"|"<<*a2<<"|"<<*a3<<endl;

        if ((instruct.name[0] != 'b')&&(instruct.name[0] != 'j')){
            (* PC) += 4;
        } 
    }
    
}


int main(int argc, char** argv) {    
    if(argc < 4){
        cout << "wrong number of arguments" << endl;
    }
    input_mips = argv[1];
    syscall_inputs = argv[2];
    output_file = argv[3];

    //input_mips = "a.asm";
    //syscall_inputs = "a.in";
    //output_file = "a.out";
    memory_init();

    infile.open(input_mips);    //start assembler part
    outfile.open("step1.txt");
    cut_out_instruction(infile, outfile);
    infile.close();
    outfile.close();
    infile.open("step1.txt");
    vector < instruction > all_instructions;
    all_instructions = tokenize(infile);
    infile.close();
    outfile.open("stdout.out");
    assembler(all_instructions, outfile);
    outfile.close();            //finish assembler part

    infile.open(input_mips);    //start simulater part
    save_data();
    infile.close();  
    infile.open(syscall_inputs);
    outfile.open(output_file);
    simulater();
    infile.close();
    outfile.close();            //finish simulater part

    free_all();

    return 0;
}


