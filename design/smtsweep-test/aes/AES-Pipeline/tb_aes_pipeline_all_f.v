`timescale 1ns/1ps

module tb_aes_pipeline_all_f;
    reg          clk;
    reg  [127:0] state;
    reg  [127:0] key;
    wire [127:0] out;

    integer cycle;

    aes_128 dut (
        .clk(clk),
        .state(state),
        .key(key),
        .out(out)
    );

    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    initial begin
        $dumpfile("aes_pipeline_all_f.vcd");
        $dumpvars(0, tb_aes_pipeline_all_f);

        cycle = 0;
        state = 128'hffffffffffffffffffffffffffffffff;
        key   = 128'hffffffffffffffffffffffffffffffff;

        $display("state = %032h", state);
        $display("key   = %032h", key);

        repeat (25) begin
            @(posedge clk);
            #1;
                       //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
            state = 128'hxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
            key   = 128'hxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
            $display("cycle %0d out = %032h", cycle, out);
            cycle = cycle + 1;
        end

        $finish;
    end
endmodule
