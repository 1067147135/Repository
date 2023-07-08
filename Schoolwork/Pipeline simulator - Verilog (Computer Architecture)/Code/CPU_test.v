`timescale 100fs/100fs
module CPU_test();
    //clock
    reg CLK;
    integer count;
    integer file_wr; //define data write pointer
    wire finish;
    

    initial begin
        CLK = 0;
        count = 0;
    end

    always #10 begin
        CLK = ~CLK;
        count = count + 1;
        file_wr = $fopen("main_memory.txt","w"); //open file to write
    end
   
    CPU Cpu(CLK, finish);
  
    integer i;
    always @(finish) begin
        if (finish === 1'b1) begin
            for (i = 0; i < 512; i = i + 1) begin
                $fwrite(file_wr,"%b\n",Cpu.Mem.main_memory.DATA_RAM[i]); //write line by line
            end
            $fwrite(file_wr,"The number of clock cycles:%d", count / 2); 
            $fclose(file_wr);   //close the file
            $finish;
        end
    end
endmodule