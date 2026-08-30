module pair_swapper_sva (
    input logic       clk,
    input logic       reset,
    input logic       in_valid,
    input logic [7:0] in_data,
    input logic       out_valid,
    input logic [7:0] out_data
);
    logic       have_first_ref;
    logic [7:0] first_ref;
    logic [7:0] expect_q [0:255];
    int unsigned expect_wr;
    int unsigned expect_rd;

    always_ff @(posedge clk) begin
        if (reset) begin
            have_first_ref <= 1'b0;
            first_ref <= 8'h00;
            expect_wr <= 0;
            expect_rd <= 0;
        end else begin
            if (in_valid) begin
                if (have_first_ref) begin
                    expect_q[expect_wr] <= in_data;
                    expect_q[expect_wr + 1] <= first_ref;
                    expect_wr <= expect_wr + 2;
                    have_first_ref <= 1'b0;
                end else begin
                    first_ref <= in_data;
                    have_first_ref <= 1'b1;
                end
            end

            if (out_valid) begin
                expect_rd <= expect_rd + 1;
            end
        end
    end

    assert property (@(posedge clk) disable iff (reset)
        out_valid |-> (expect_rd < expect_wr));

    assert property (@(posedge clk) disable iff (reset)
        out_valid |-> (out_data == expect_q[expect_rd]));

    assert property (@(posedge clk) disable iff (reset)
        (have_first_ref && !in_valid && expect_rd == expect_wr) |-> !out_valid);
endmodule

bind pair_swapper pair_swapper_sva pair_swapper_sva_i (
    .clk(clk),
    .reset(reset),
    .in_valid(in_valid),
    .in_data(in_data),
    .out_valid(out_valid),
    .out_data(out_data)
);
