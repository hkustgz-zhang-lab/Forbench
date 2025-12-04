module mul(input [7:0] a, input [2:0] b, input start, output valid, output reg [10:0] result);

reg [7:0] rega;
reg [2:0] regb;

initial regb = 0;
assign valid = regb == 0;

always @(posedge clk) begin
	if(start && valid) begin
		rega <= a;
		regb <= b;
		result <= 0;
	end else begin
		if (!valid) begin  // namely, regb > 0
			result <= result + rega;
			regb <= regb - 1;
		end
	end
end

endmodule : mul