`timescale 100fs/100fs
module CPU (
    input CLK,

    output finish_wire
);

    reg finish;
    assign finish_wire = finish;

    always @(IRD) begin
        if (IRD === 32'hffff_ffff) #60 finish = 1'b1;
    end
    
    //IF STAGE
    wire PCSrcD;    //Whether to branch 
    wire JumpD;     //Whether to jump
    wire stall;     //Whether to update PC in IF stage

    wire[31:0] PCBranchD;   //Address where the branch goes 
    wire[31:0] PCJumpD;     //Address where the jump goes 

    wire[31:0] PCPlus4F;    //PC + 4
    wire[31:0] IRF;         //Instruction machine code 

    IF If(CLK, PCSrcD, JumpD, stall, PCBranchD, PCJumpD, PCPlus4F, IRF);

    //Register between IF stage and EX stage 
    reg[31:0] IR_reg;
    reg[31:0] PCPlus4F_reg;
    
    //The wire connecting the register IF/EX and the input of the EX stage 
    assign IRD = IR_reg;
    assign PCPlus4D = PCPlus4F_reg;

    //ID STAGE   
    wire RegWriteW;         //Whether to write
    wire[4:0] WriteRegW;    //Write to which register
    wire[31:0] ResultW;     //Data to be written
    wire[31:0] IRD;         
    wire[31:0] PCPlus4D;

    //more information about control signal 
    //-> see the report "High level implementation ideas" part figure 4 ID part
    wire RegWriteD;         
    wire MemtoRegD;
    wire MemWriteD;
    wire ALUSrcAD;
    wire ALUSrcBD;
    wire RegDstD;
    wire[3:0] ALUControlD;

    wire[4:0] RsD;
    wire[4:0] RtD;
    wire[4:0] RdD;
    
    wire[31:0] SignImmD;
    wire[31:0] AD; //content in rs
    wire[31:0] BD; //content in rt

    ID Id(CLK, RegWriteW, WriteRegW, ResultW, IRD, PCPlus4D, PCSrcD, JumpD, RegWriteD, MemtoRegD, MemWriteD, ALUSrcAD, ALUSrcBD, RegDstD, ALUControlD, RsD, RtD, RdD, SignImmD, PCBranchD, PCJumpD, AD, BD);

    reg RegWriteD_reg;
    reg MemtoRegD_reg;
    reg MemWriteD_reg;
    reg ALUSrcAD_reg;
    reg ALUSrcBD_reg;
    reg RegDstD_reg;
    reg[3:0] ALUControlD_reg;

    reg[4:0] RsD_reg;
    reg[4:0] RtD_reg;
    reg[4:0] RdD_reg;
    
    reg[31:0] SignImmD_reg;
    reg[31:0] AD_reg; //content in rs
    reg[31:0] BD_reg; //content in rt

    assign RegWriteE = RegWriteD_reg;
    assign MemtoRegE = MemtoRegD_reg;
    assign MemWriteE = MemWriteD_reg;
    assign ALUSrcAE = ALUSrcAD_reg;
    assign ALUSrcBE = ALUSrcBD_reg;
    assign RegDstE = RegDstD_reg;
    assign ALUControlE = ALUControlD_reg;

    assign RsE = RsD_reg;
    assign RtE = RtD_reg;
    assign RdE = RdD_reg;
    
    assign SignImmE = SignImmD_reg;
    assign AE = AD_reg; 
    assign BE = BD_reg;

    //EX
    wire RegWriteE;
    wire MemtoRegE;
    wire MemWriteE;
    wire ALUSrcAE;
    wire ALUSrcBE;
    wire RegDstE;
    wire[3:0] ALUControlE;

    wire[4:0] RsE;
    wire[4:0] RtE;
    wire[4:0] RdE;

    wire[31:0] SignImmE;
    wire[31:0] AE;
    wire[31:0] BE;
    wire[4:0] WriteRegE;
    wire[31:0] WriteDataE;
    wire[31:0] ALUOutE;
    
    EX Ex(CLK, MemWriteE, ALUSrcAE, ALUSrcBE, RegDstE, ALUControlE, RtE, RdE, SignImmE, AE, BE, WriteRegE, WriteDataE, ALUOutE);

    reg RegWriteE_reg;
    reg MemtoRegE_reg;
    reg MemWriteE_reg;

    reg[4:0] WriteRegE_reg;
    reg[31:0] WriteDataE_reg;
    reg[31:0] ALUOutE_reg;

    assign RegWriteM = RegWriteE_reg;
    assign MemtoRegM = MemtoRegE_reg;
    assign MemWriteM = MemWriteE_reg;
    assign WriteRegM = WriteRegE_reg;
    assign WriteDataM = WriteDataE_reg;
    assign ALUOutM = ALUOutE_reg;

    //MEM
    wire RegWriteM;
    wire MemtoRegM;
    wire MemWriteM;

    wire[4:0] WriteRegM;
    wire[31:0] WriteDataM;
    wire[31:0] ALUOutM;
    wire[31:0] ReadDataM;

    MEM Mem(CLK, MemWriteM, ALUOutM, WriteDataM, ReadDataM);

    reg RegWriteM_reg;
    reg MemtoRegM_reg;
    reg[4:0] WriteRegM_reg;
    reg[31:0] ReadDataM_reg;
    reg[31:0] ALUOutM_reg;
    
    assign RegWriteW = RegWriteM_reg;
    assign MemtoRegW = MemtoRegM_reg;
    assign WriteRegW = WriteRegM_reg;
    assign ReadDataW = ReadDataM_reg;
    assign ALUOutW = ALUOutM_reg;

    //WB
    wire MemtoRegW;
    wire[31:0] ReadDataW;
    wire[31:0] ALUOutW;

    WB Wb(MemtoRegW, ReadDataW, ALUOutW, ResultW);

    //only use stall to solve hazard
    assign stall = ((RegWriteM === 1'b1)&&((WriteRegM == RsD)||(WriteRegM == RtD)))||
        ((RegWriteW === 1'b1)&&((WriteRegW == RsD)||(WriteRegW == RtD)))||
        ((RegWriteE === 1'b1)&&((WriteRegE == RsD)||(WriteRegE == RtD)));


    always @(negedge CLK) begin //update outputs to registers between stages in reverse order
        //MEM
        RegWriteM_reg = RegWriteM;
        MemtoRegM_reg = MemtoRegM;
        WriteRegM_reg = WriteRegM;
        ReadDataM_reg = ReadDataM;
        ALUOutM_reg = ALUOutM;

        //EX
        RegWriteE_reg = RegWriteE;
        MemtoRegE_reg = MemtoRegE;
        MemWriteE_reg = MemWriteE;

        WriteRegE_reg = WriteRegE;
        WriteDataE_reg = WriteDataE;
        ALUOutE_reg = ALUOutE;

        //ID
        if (stall === 1'b1) begin   //flush
            RegWriteD_reg = 1'b0;
            MemtoRegD_reg = 1'b0;
            MemWriteD_reg = 1'b0;
            ALUSrcAD_reg = 1'b0;
            ALUSrcBD_reg = 1'b0;
            RegDstD_reg = 1'b0;
        end
        else begin
            RegWriteD_reg = RegWriteD;
            MemtoRegD_reg = MemtoRegD;
            MemWriteD_reg = MemWriteD;
            ALUSrcAD_reg = ALUSrcAD;
            ALUSrcBD_reg = ALUSrcBD;
            RegDstD_reg = RegDstD;          
        end
        ALUControlD_reg = ALUControlD;

        RsD_reg = RsD;
        RtD_reg = RtD;
        RdD_reg = RdD;
        
        SignImmD_reg = SignImmD;
        AD_reg = AD; 
        BD_reg = BD; 
        
        //IF
        IR_reg = IRF;
        PCPlus4F_reg = PCPlus4F;
        
        

        

        


    end
endmodule