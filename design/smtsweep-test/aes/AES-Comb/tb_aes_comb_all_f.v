`timescale 1ns/1ps

module tb_aes_comb_all_f;
    reg  [127:0] in;
    reg  [127:0] key;
    wire [127:0] out;

    AES_Encrypt dut (
        .in(in),
        .key(key),
        .out(out)
    );

    initial begin
        $dumpfile("aes_comb_all_f.vcd");
        $dumpvars(0, tb_aes_comb_all_f);

        in  = 128'hffffffffffffffffffffffffffffffff;
        key = 128'hffffffffffffffffffffffffffffffff;

        #1;
        $display("in  = %032h", in);
        $display("key = %032h", key);
        $display("out = %032h", out);

        #1;
        $finish;
    end
endmodule
