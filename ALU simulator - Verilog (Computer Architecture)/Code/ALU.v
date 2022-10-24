module alu(instruction, regA, regB, result, flags);

input[31:0] instruction, regA, regB; 
// the address of regA is 00000, the address of regB is 00001
output[31:0] result;
output[2:0] flags; 
// the first bit(0) is zero flag, the second bit is negative flag, the third bit is overflow flag.
//For the overflow flag, you should only consider add, addi, and sub instruction. 
//For the zero flag, you should only consider beq and bne instruction. 
//For the negative flag, you only need to deal with slt, slti, sltiu, sltu instruction. 
//For example, at any time, when you execute addu instruction, the overflow flag will remain zero. 
//And for subu instruction, even the result is less than 0, the negative flag will remain zero.

reg[5:0] opcode, func;
reg[4:0] rs, rt, shamt;
reg[15:0] immediate, extension;
reg[31:0] reg_A, reg_B, reg_C, reg_D; //result
reg[2:0] tags;  //tags[0] = zero, tags[1] = negative, tags[2] = overflow

always @(instruction, regA, regB)
begin

// Step 1: You should parsing the instruction;
opcode = instruction[31:26];
rs = instruction[25:21];
rt = instruction[20:16];
shamt = instruction[10:6];
func = instruction[5:0];
immediate = instruction[15:0];
extension = immediate[15] ? 16'hffff : 16'h0000;
tags = 3'b000;
reg_A = regA;
reg_B = regB;

// Step 2: You may fetch values in mem;
case(opcode)
    6'b000000:
    begin
        //R_format
        case(func)
            6'b000000:  //sll
            begin
                case(rt)
                    5'b00000: reg_C = reg_A << shamt;
                    5'b00001: reg_C = reg_B << shamt;
                endcase
            end
            6'b000010:  //srl
            begin
                case(rt)
                    5'b00000: reg_C = reg_A >> shamt;
                    5'b00001: reg_C = reg_B >> shamt;
                endcase
            end
            6'b000011:  //sra
            begin
                case(rt)
                    5'b00000: reg_C = ($signed(reg_A)) >>> shamt;
                    5'b00001: reg_C = ($signed(reg_B)) >>> shamt;
                endcase
            end
            6'b000100:  //sllv
            begin
                case(rt)
                    5'b00000: reg_C = reg_A << reg_B;
                    5'b00001: reg_C = reg_B << reg_A;
                endcase
            end
            6'b000110:  //srlv
            begin
                case(rt)
                    5'b00000: reg_C = reg_A >> reg_B;
                    5'b00001: reg_C = reg_B >> reg_A;
                endcase
            end
            6'b000111:  //srav
            begin
                case(rt)
                    5'b00000: reg_C = ($signed(reg_A)) >>> reg_B;
                    5'b00001: reg_C = ($signed(reg_B)) >>> reg_A;
                endcase
            end
            6'b100000:  //add
            begin
                reg_D = reg_A + reg_B;
                if (((reg_A[31] == 1'b0)&&(reg_B[31] == 1'b0)&&(reg_D[31] == 1'b1))||
                ((reg_A[31] == 1'b1)&&(reg_B[31] == 1'b1)&&(reg_D[31] == 1'b0)))
                begin
                    tags[2] = 1'b1;
                end
                else reg_C = reg_D;
            end
            6'b100001:  //addu
            begin
                reg_C = reg_A + reg_B;
            end
            6'b100010:  //sub
            begin
                case(rs)
                    5'b00000: 
                    begin
                        reg_D = reg_A - reg_B;
                        if (((reg_A[31] == 1'b0)&&(reg_B[31] == 1'b1)&&(reg_D[31] == 1'b1))||
                        ((reg_A[31] == 1'b1)&&(reg_B[31] == 1'b0)&&(reg_D[31] == 1'b0)))
                        begin
                            tags[2] = 1'b1;
                        end
                        else reg_C = reg_D;
                    end
                    5'b00001:
                    begin
                        reg_D = reg_B - reg_A;
                        if (((reg_B[31] == 1'b0)&&(reg_A[31] == 1'b1)&&(reg_D[31] == 1'b1))||
                        ((reg_B[31] == 1'b1)&&(reg_A[31] == 1'b0)&&(reg_D[31] == 1'b0)))
                        begin
                            tags[2] = 1'b1;
                        end
                        else reg_C = reg_D;
                    end
                endcase
            end
            6'b100011:  //subu
            begin
                case(rs)
                    5'b00000: 
                    begin
                        reg_C = reg_A - reg_B;
                    end
                    5'b00001:
                    begin
                        reg_C = reg_B - reg_A;
                    end
                endcase
            end
            6'b100100:  //and
            begin
                reg_C = reg_A & reg_B;
            end
            6'b100101:  //or
            begin
                reg_C = reg_A | reg_B;
            end
            6'b100110:  //xor
            begin
                reg_C = reg_A ^ reg_B;
            end
            6'b100111:  //nor
            begin
                reg_C = ~ (reg_A | reg_B);
            end
            6'b101010:  //slt
            begin
                case(rs)
                    5'b00000: 
                    begin
                        if ($signed(reg_A) < $signed(reg_B)) 
                        begin
                            tags[1] = 1'b1;
                            reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
                        end
                        else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;   
                    end
                    5'b00001:
                    begin
                        if ($signed(reg_B) < $signed(reg_A)) 
                        begin
                            tags[1] = 1'b1;
                            reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
                        end
                        else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
                    end
                endcase
            end
            6'b101011:  //sltu
            begin
                case(rs)
                    5'b00000: 
                    begin
                        if ($unsigned(reg_A) < $unsigned(reg_B))
                        begin
                            tags[1] = 1'b1;
                            reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
                        end
                        else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
                    end
                    5'b00001:
                    begin
                        if ($unsigned(reg_B) < $unsigned(reg_A))
                        begin
                            tags[1] = 1'b1;
                            reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
                        end
                        else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
                    end
                endcase
            end
        endcase
    end
    6'b000100:  //beq
    begin
        if (reg_A == reg_B) 
        begin
            tags[0] = 1'b1;
        end
    end
    6'b000101:  //bne
    begin
        if (reg_A != reg_B) 
        begin
            tags[0] = 1'b1;
        end
    end
    6'b001000:  //addi
    begin
        case(rs)
        5'b00000: 
        begin
            reg_D = reg_A + {extension, immediate};
            if (((reg_A[31] == 1'b0)&&(immediate[15] == 1'b0)&&(reg_D[31] == 1'b1))||
            ((reg_A[31] == 1'b1)&&(immediate[15] == 1'b1)&&(reg_D[31] == 1'b0)))
            begin
                tags[2] = 1'b1;
            end
            else reg_C = reg_D;
        end
        5'b00001: 
        begin
            reg_D = reg_B + {extension, immediate};
            if (((reg_B[31] == 1'b0)&&(immediate[15] == 1'b0)&&(reg_D[31] == 1'b1))||
            ((reg_B[31] == 1'b1)&&(immediate[15] == 1'b1)&&(reg_D[31] == 1'b0)))
            begin
                tags[2] = 1'b1;
            end
            else reg_C = reg_D;
        end
        endcase
    end
    6'b001001:  //addiu
    begin
        case(rs)
        5'b00000: reg_C = reg_A + {extension, immediate};
        5'b00001: reg_C = reg_B + {extension, immediate};
        endcase
    end
    6'b001010:  //slti
    begin
        case(rs)
        5'b00000: 
        begin
            if ($signed(reg_A) < $signed({extension, immediate}))
            begin
                tags[1] = 1'b1;
                reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
            end
            else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
        end
        5'b00001:
        begin
            if ($signed(reg_B) < $signed({extension, immediate}))
            begin
                tags[1] = 1'b1;
                reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
            end
            else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
        end
        endcase
    end
    6'b001011:  //sltiu
    begin
        case(rs)
        5'b00000: 
        begin
            if ($unsigned(reg_A) < $unsigned({16'b0000_0000_0000_0000, immediate}))
            begin
                tags[1] = 1'b1;
                reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
            end
            else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
        end
        5'b00001:
        begin
            if ($unsigned(reg_B) < $unsigned({16'b0000_0000_0000_0000, immediate}))
            begin
                tags[1] = 1'b1;
                reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0001;
            end
            else reg_C = 32'b0000_0000_0000_0000_0000_0000_0000_0000;  
        end
        endcase
    end
    6'b001100:  //andi
    begin
        case(rs)
        5'b00000: reg_C = reg_A & {16'b0000_0000_0000_0000, immediate};
        5'b00001: reg_C = reg_B & {16'b0000_0000_0000_0000, immediate};
        endcase
    end
    6'b001101:  //ori
    begin
        case(rs)
        5'b00000: reg_C = reg_A | {16'b0000_0000_0000_0000, immediate};
        5'b00001: reg_C = reg_B | {16'b0000_0000_0000_0000, immediate};
        endcase
    end
    6'b001110:  //xori
    begin
        case(rs)
        5'b00000: reg_C = reg_A ^ {16'b0000_0000_0000_0000, immediate};
        5'b00001: reg_C = reg_B ^ {16'b0000_0000_0000_0000, immediate};
        endcase
    end
    6'b100011:  //lw
    begin
        case(rs)
        5'b00000: reg_C = reg_A + {extension, immediate};
        5'b00001: reg_C = reg_B + {extension, immediate}; 
        endcase       
    end
    6'b101011:  //sw
    begin
        case(rs)
        5'b00000: reg_C = reg_A + {extension, immediate};
        5'b00001: reg_C = reg_B + {extension, immediate};  
        endcase
    end
endcase
end

// - add, addi, addu, addiu
// - sub, subu
// - and, andi, nor, or, ori, xor, xori
// - beq, bne, slt, slti, sltiu, sltu
// - lw, sw
// - sll, sllv, srl, srlv, sra, srav


// Step 3: You should output the correct value of result and correct status of flags
assign result = reg_C[31:0];
assign flags = tags[2:0];
endmodule






