iverilog -o wave led_demo_tb.v led_demo.v 
iverilog -o wave test_ALU.v ALU.v 
对源文件和仿真文件，进行语法规则检查和编译，如果编译成功，会在当前目录下生成名称为wave的文件。

vvp -n wave -lxt2
生成vcd波形文件，运行之后，会在当前目录下生成.vcd文件。

gtkwave wave.vcd
在图形化界面中查看仿真的波形图。