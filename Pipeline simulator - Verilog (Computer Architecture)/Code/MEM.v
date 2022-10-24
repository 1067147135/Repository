module MEM (
    input CLK,
    input MemWriteM,
    input[31:0] ALUOutM,
    input[31:0] WriteDataM,

    output[31:0] ReadDataM
);
    reg reset, enable;
    wire[31:0] fetch_address;
    wire[64:0] edit_serial;
    
    assign fetch_address = ALUOutM / 4;
    assign edit_serial = {MemWriteM, fetch_address, WriteDataM};

    initial begin
        reset <= 1'b0;
        enable <= 1'b1;
    end

    MainMemory main_memory(CLK, reset, enable, fetch_address, edit_serial, ReadDataM);
    
endmodule