module ALU (
    input[31:0] A,//rs
    input[31:0] B,//rt/immediate/shamt
    input[3:0] ALUControlD,

    output [31:0] result_wire
);  

    reg[31:0] result;
    assign result_wire = result;

    always @(A or B or ALUControlD) begin
        case (ALUControlD)
            4'b0000: begin  //add
                result = A + B;
            end
            4'b0001: begin  //subtract
                result = A - B;
            end
            4'b0010: begin  //and
                result = A & B;
            end
            4'b0011: begin  //or
                result = A | B;
            end
            4'b0100: begin  //nor
                result = ~(A | B);
            end
            4'b0101: begin  //xor
                result = A ^ B;
            end
            4'b0110: begin  //sll
                result = B << A;
            end
            4'b0111: begin  //sra
                result = $signed(B) >>> A;
            end
            4'b1000: begin  //srl
                result = B >> A;
            end
            4'b1001: //slt
                result = $signed(A) < $signed(B) ? 32'h0000_0001 : 32'h0000_0000; 
        endcase
    end
endmodule