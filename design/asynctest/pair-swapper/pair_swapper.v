module pair_swapper (
    input         clk,
    input         reset,
    input         in_valid,
    input  [7:0]  in_data,
    output reg    out_valid,
    output reg [7:0] out_data
);
    reg       have_first;
    reg [7:0] first_byte;
    reg       emit_first;
    reg [7:0] emit_byte;

    reg       have_first_n;
    reg [7:0] first_byte_n;
    reg       emit_first_n;
    reg [7:0] emit_byte_n;
    reg       out_valid_n;
    reg [7:0] out_data_n;

    always @(*) begin
        have_first_n = have_first;
        first_byte_n = first_byte;
        emit_first_n = emit_first;
        emit_byte_n = emit_byte;
        out_valid_n = 1'b0;
        out_data_n = out_data;

        if (emit_first) begin
            out_valid_n = 1'b1;
            out_data_n = emit_byte;
            emit_first_n = 1'b0;
        end

        if (in_valid) begin
            if (have_first) begin
                out_valid_n = 1'b1;
                out_data_n = in_data;
                emit_first_n = 1'b1;
                emit_byte_n = first_byte;
                have_first_n = 1'b0;
            end else begin
                first_byte_n = in_data;
                have_first_n = 1'b1;
            end
        end
    end

    always @(posedge clk) begin
        if (reset) begin
            out_valid <= 1'b0;
            out_data <= 8'b0;
            have_first <= 1'b0;
            first_byte <= 8'b0;
            emit_first <= 1'b0;
            emit_byte <= 8'b0;
        end else begin
            out_valid <= out_valid_n;
            out_data <= out_data_n;
            have_first <= have_first_n;
            first_byte <= first_byte_n;
            emit_first <= emit_first_n;
            emit_byte <= emit_byte_n;
        end
    end
endmodule
