module EX (
    input CLK,
    input MemWriteD,
    input ALUSrcAD,
    input ALUSrcBD,
    input RegDstD,
    input[3:0] ALUControlD,

    input[4:0] RtD,
    input[4:0] RdD,
    
    input[31:0] SignImmD,
    input[31:0] A, //content in rs
    input[31:0] B,  //content in rt

    output[4:0] WriteRegE_wire,
    output[31:0] WriteDataE_wire,
    output[31:0] ALUOutE
);

    reg[31:0] regA;
    reg[31:0] regB;

    //reg[4:0] WriteRegE;
    reg[31:0] WriteDataE;


    assign WriteRegE_wire = (RegDstD === 1'b1) ? RdD : RtD; 
    assign WriteDataE_wire = WriteDataE;
    

    ALU alu(regA, regB, ALUControlD, ALUOutE);
        
    always @(posedge CLK) begin
        if (ALUSrcAD === 1'b1) regA = SignImmD;
        else regA = A;
        if (ALUSrcBD === 1'b1) regB = SignImmD;
        else regB = B;
        
        // if (RegDstD === 1'b1) WriteRegE = RdD;
        // else WriteRegE = RtD;

        if (MemWriteD === 1'b1) WriteDataE = B;
    end

endmodule