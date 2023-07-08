module ID (
    input CLK,
    input RegWriteW,
    input[4:0] WriteRegW,
    input[31:0] ResultW,
    input[31:0] IR,
    input[31:0] PCPlus4F,

    output PCSrcD_wire,
    output JumpD_wire,
    output RegWriteD_wire,
    output MemtoRegD_wire,
    output MemWriteD_wire,
    output ALUSrcAD_wire,
    output ALUSrcBD_wire,
    output RegDstD_wire,
    output[3:0] ALUControlD_wire,

    output[4:0] RsD,
    output[4:0] RtD,
    output[4:0] RdD,
    
    output[31:0] SignImmD_wire,
    output[31:0] PCBranchD_wire,
    output[31:0] PCJumpD_wire,
    output[31:0] A_wire, //content in rs
    output[31:0] B_wire  //content in rt
);
    wire[5:0] op;
    wire[5:0] funct;
    wire[4:0] shamt;
    wire[15:0] immediate;

    assign op = IR[31:26];
    assign RsD = IR[25:21];
    assign RtD = IR[20:16];
    assign RdD = IR[15:11];
    assign shamt = IR[10:6];
    assign funct = IR[5:0];
    assign immediate = IR[15:0];

    reg PCSrcD;
    reg JumpD;
    reg RegWriteD;
    reg MemtoRegD;
    reg MemWriteD;
    reg ALUSrcAD;
    reg ALUSrcBD;
    reg RegDstD;
    reg[3:0] ALUControlD;

    assign PCSrcD_wire = PCSrcD;
    assign JumpD_wire = JumpD;
    assign RegWriteD_wire = RegWriteD;
    assign MemtoRegD_wire = MemtoRegD;
    assign MemWriteD_wire = MemWriteD;
    assign ALUSrcAD_wire = ALUSrcAD;
    assign ALUSrcBD_wire = ALUSrcBD;
    assign RegDstD_wire = RegDstD;
    assign ALUControlD_wire = ALUControlD;

    reg[31:0] SignImmD;
    reg[31:0] PCBranchD;
    reg[31:0] PCJumpD;
    reg[31:0] A; //content in rs
    reg[31:0] B; //content in rt

    assign SignImmD_wire = SignImmD;
    assign PCBranchD_wire = PCBranchD;
    assign PCJumpD_wire = PCJumpD; 
    assign A_wire = A; //content in rs
    assign B_wire = B; //content in rt
    
    //register file
    integer i;
    reg[31:0] registers[31:0];

    initial begin
        for (i = 0; i < 32; i = i + 1) begin
            registers[i] = 32'h0000_0000;
        end
    end

    //control signal
    initial begin
        PCSrcD <= 1'b0;
        JumpD <= 1'b0;
        RegWriteD <= 1'b0;
        MemtoRegD <= 1'b0;
        MemWriteD <= 1'b0;
        ALUSrcAD <= 1'b0;
        ALUSrcBD <= 1'b0;
        RegDstD <= 1'b0;
    end

    always @(posedge CLK) begin
        // $display("RegWriteW = %b", RegWriteW);
        // $display("WriteRegW = %b", WriteRegW);
        // $display("ResultW = %b", ResultW);
        if (RegWriteW === 1'b1) registers[WriteRegW] = ResultW;
        A = registers[RsD];
        B = registers[RtD];
    end
    
    always @(IR or PCPlus4F) begin  //
        // $display("RsD = %b", RsD);
        // $display("RtD = %b", RtD);
        // $display("registers[RsD] = %b", registers[RsD]);
        // $display("registers[RtD] = %b", registers[RtD]);
        case (op)
            6'b000000: begin
                case (funct)
                    6'b100000: begin    //add
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0000;
                    end
                    6'b100001: begin    //addu
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0000;
                    end
                    6'b100010: begin    //sub
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0001;
                    end
                    6'b100011: begin    //subu
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0001;
                    end
                    6'b100100: begin    //and
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0010;
                    end
                    6'b100111: begin    //nor
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0100;
                    end
                    6'b100101: begin    //or
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0011;
                    end
                    6'b100110: begin    //xor
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0101;
                    end
                    6'b000000: begin    //sll
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b1;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1;
                        ALUControlD <= 4'b0110;
                        SignImmD = {27'h0000000, shamt[4:0]};
                    end
                    6'b000100: begin    //sllv
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0110;
                    end
                    6'b000011: begin    //sra
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b1;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0111;
                        SignImmD = {27'h0000000, shamt[4:0]};
                    end
                    6'b000111: begin    //srav
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b0111;
                    end
                    6'b000010: begin    //srl
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b1;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b1000;
                        SignImmD = {27'h0000000, shamt[4:0]};
                    end
                    6'b000110: begin    //srlv
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b1000;
                    end
                    6'b101010: begin    //slt
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b0;
                        RegWriteD <= 1'b1;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b1; 
                        ALUControlD <= 4'b1001;
                    end
                    6'b001000: begin    //jr
                        PCSrcD <= 1'b0;
                        JumpD <= 1'b1;
                        RegWriteD <= 1'b0;
                        MemtoRegD <= 1'b0;
                        MemWriteD <= 1'b0;
                        ALUSrcAD <= 1'b0;
                        ALUSrcBD <= 1'b0;
                        RegDstD <= 1'b0; 
                        ALUControlD <= 4'b0000;
                        PCJumpD = registers[RsD];
                    end
                endcase
            end     
            6'b001000: begin    //addi
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};               
            end
            6'b001001: begin    //addiu
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};  
            end
            6'b100011: begin    //lw
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b1;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};               
            end
            6'b101011: begin    //sw
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b1;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]}; 
            end
            6'b001100: begin    //andi
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0010;
                SignImmD = {16'h0000, immediate[15:0]};
            end
            6'b001101: begin    //ori
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0;
                ALUControlD <= 4'b0011;
                SignImmD = {16'h0000, immediate[15:0]};
            end
            6'b001110: begin    //xori
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b1;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0101;
                SignImmD = {16'h0000, immediate[15:0]};                
            end
            6'b000100: begin    //beq
                JumpD <= 1'b0;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};
                //$display("beq attention!!!");
                if (registers[RsD] == registers[RtD]) begin
                    PCSrcD = 1'b1;
                    PCBranchD = PCPlus4F + (SignImmD << 2);
                end
                else PCSrcD = 1'b0;
                
            end
            6'b000101: begin    //bne
                JumpD <= 1'b0;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b1;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};
                //$display("bne attention!!!");
                if (registers[RsD] != registers[RtD]) begin
                    PCSrcD = 1'b1;
                    PCBranchD = PCPlus4F + (SignImmD << 2);
                end
                else PCSrcD = 1'b0;
                
            end
            6'b000010: begin    //j
                PCSrcD <= 1'b0;
                JumpD <= 1'b1;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b0;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};
                PCJumpD = {PCPlus4F[31:28], IR[25:0], 2'b00};
            end
            6'b000011: begin    //jal
                PCSrcD <= 1'b0;
                JumpD <= 1'b1;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b0;
                RegDstD <= 1'b0; 
                ALUControlD <= 4'b0000;
                SignImmD = immediate[15] == 1'b0 ? {16'h0000, immediate[15:0]} : {16'hffff, immediate[15:0]};              
                PCJumpD = {PCPlus4F[31:28], IR[25:0], 2'b00};
                registers[31] = PCPlus4F;
            end
            default: begin
                PCSrcD <= 1'b0;
                JumpD <= 1'b0;
                RegWriteD <= 1'b0;
                MemtoRegD <= 1'b0;
                MemWriteD <= 1'b0;
                ALUSrcAD <= 1'b0;
                ALUSrcBD <= 1'b0;
                RegDstD <= 1'b0;
            end
        endcase
    end

endmodule