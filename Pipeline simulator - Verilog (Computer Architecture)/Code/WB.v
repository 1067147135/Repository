module WB (
    input MemtoRegW,
    input[31:0] ReadDataW,
    input[31:0] ALUOutW,
    
    output[31:0] ResultW
);

    assign ResultW = MemtoRegW === 1'b1 ? ReadDataW : ALUOutW;
    
endmodule