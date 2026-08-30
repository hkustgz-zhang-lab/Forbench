`timescale 1ns/1ps

module tb_pair_swapper_random;
    reg        clk;
    reg        reset;
    reg        in_valid;
    reg  [7:0] in_data;
    wire       out_valid;
    wire [7:0] out_data;

    reg [7:0] pending_byte;
    reg       have_pending;
    reg [7:0] expect_q [0:255];
    integer expect_wr;
    integer expect_rd;
    integer cycle;
    integer errors;
    integer seed;

    pair_swapper dut (
        .clk(clk),
        .reset(reset),
        .in_valid(in_valid),
        .in_data(in_data),
        .out_valid(out_valid),
        .out_data(out_data)
    );

    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    initial begin
        $dumpfile("pair_swapper_random.vcd");
        $dumpvars(0, tb_pair_swapper_random);

        reset = 1'b1;
        in_valid = 1'b0;
        in_data = 8'h00;
        pending_byte = 8'h00;
        have_pending = 1'b0;
        expect_wr = 0;
        expect_rd = 0;
        cycle = 0;
        errors = 0;
        seed = 32'h1234abcd;

        repeat (2) @(negedge clk);
        reset = 1'b0;

        repeat (40) begin
            @(negedge clk);
            in_valid = ($random(seed) & 32'h3) != 0;
            in_data = $random(seed);
            $display("drive cycle %0d: in_valid=%0d in_data=%02x",
                     cycle, in_valid, in_data);

            @(posedge clk);
            #1;

            if (in_valid) begin
                if (have_pending) begin
                    expect_q[expect_wr] = in_data;
                    expect_wr = expect_wr + 1;
                    expect_q[expect_wr] = pending_byte;
                    expect_wr = expect_wr + 1;
                    have_pending = 1'b0;
                end else begin
                    pending_byte = in_data;
                    have_pending = 1'b1;
                end
            end

            if (out_valid) begin
                if (expect_rd >= expect_wr) begin
                    $display("ERROR cycle %0d: unexpected output %02x",
                             cycle, out_data);
                    errors = errors + 1;
                end else if (out_data !== expect_q[expect_rd]) begin
                    $display("ERROR cycle %0d: expected %02x got %02x",
                             cycle, expect_q[expect_rd], out_data);
                    errors = errors + 1;
                end else begin
                    $display("check cycle %0d: out_data=%02x",
                             cycle, out_data);
                end
                expect_rd = expect_rd + 1;
            end

            cycle = cycle + 1;
        end

        in_valid = 1'b0;
        repeat (4) begin
            @(posedge clk);
            #1;
            if (out_valid) begin
                if (expect_rd >= expect_wr) begin
                    $display("ERROR drain: unexpected output %02x", out_data);
                    errors = errors + 1;
                end else if (out_data !== expect_q[expect_rd]) begin
                    $display("ERROR drain: expected %02x got %02x",
                             expect_q[expect_rd], out_data);
                    errors = errors + 1;
                end else begin
                    $display("check drain: out_data=%02x", out_data);
                end
                expect_rd = expect_rd + 1;
            end
        end

        if (errors == 0) begin
            $display("PAIR SWAPPER RANDOM TEST PASS");
        end else begin
            $display("PAIR SWAPPER RANDOM TEST FAIL: errors=%0d", errors);
        end
        $finish;
    end
endmodule
