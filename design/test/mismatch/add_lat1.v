module add_lat1 (
    input clk,
    input [7:0] a,
    input [7:0] b,
    output [7:0] out
);
    reg [7:0] stage0;

    always @(posedge clk) begin
        stage0 <= a + b;
    end

    assign out = stage0;
endmodule
