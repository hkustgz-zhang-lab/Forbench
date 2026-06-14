module addmodule(input [7:0] a, input [7:0] b, input start, output [7:0] result);


assign result = start ? a + b : 0;

endmodule : addmodule
