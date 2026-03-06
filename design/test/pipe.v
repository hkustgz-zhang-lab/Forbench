module pipe(input clk, input rst, input wire [15:0] a, input wire[15:0] b, input wire [15:0] c, output reg [15:0] d1, output reg [15:0] d2);

reg [15:0] r1,r2,r3,r4;

always @(posedge clk) begin
  if(rst) begin
    r1 <= 0;
    r2 <= 0;
    r3 <= 0;
    r4 <= 0;
    d1 <= 0;
    d2 <= 0;
  end else begin
    r1 <= a*c;
    r2 <= b*c;
    r3 <= a+b;
    r4 <= c;
    d1 <= r1 + r2;
    d2 <= r3 * r4;
  end
end

assert property (d1 == d2);

endmodule

