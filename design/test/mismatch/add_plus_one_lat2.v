module add_plus_one_lat2 (
    input clk,
    input [7:0] a,
    input [7:0] b,
    output [7:0] out
);
    reg [7:0] stage0;
    reg [7:0] stage1;

    always @(posedge clk) begin
        stage0 <= a + b + 8'd1;
        stage1 <= stage0;
    end

    assign out = stage1;
endmodule
