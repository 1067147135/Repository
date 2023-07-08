module IF (
    input CLK,
    input PCSrcD, 
    input JumpD,
    input stall,
    input[31:0] PCBranchD, 
    input[31:0] PCJumpD,

    output[31:0] PCPlus4F, 
    output[31:0] IR
);
    reg reset;
    reg enable;
    reg[31:0] PC;
    wire[31:0] fetch_address;
    
    InstructionRAM instruction_memory(PC, reset, enable, fetch_address, IR);

    assign fetch_address = PC / 4;  //Address by word
    assign PCPlus4F = PC + 4;       //next PC

    initial begin
        reset <= 1'b0;  //always not reset
        enable <= 1'b1; //always enable
        PC <= -32'h0000_0004;   // then PC can start from 0
    end

    always @(posedge CLK) begin
        if (stall !== 1'b1) begin   //update
            if (JumpD === 1'b1) PC = PCJumpD;
            else if (PCSrcD === 1'b1) PC = PCBranchD;
            else PC = PCPlus4F;   
        end 
    end

endmodule