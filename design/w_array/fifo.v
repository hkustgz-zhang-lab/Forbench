`timescale 1ns/1ps

// Single-clock, synchronous FIFO.
// A write/read is accepted only when wr_en && !full / rd_en && !empty.
module fifo (
    input  wire       clk,
    input  wire       rst,
    input  wire       wr_en,
    input  wire [7:0] din,
    input  wire       rd_en,
    output reg  [7:0] dout,
    output wire       full,
    output wire       empty,
    output wire [4:0] count
);

    reg [7:0] mem [0:15];
    reg [3:0] write_ptr;
    reg [3:0] read_ptr;
    reg [4:0] item_count;

    wire write_accept = wr_en && !full;
    wire read_accept  = rd_en && !empty;

    assign empty = (item_count == 0);
    assign full  = (item_count == 16);
    assign count = item_count;

    always @(posedge clk) begin
        if (rst) begin
            write_ptr  <= 4'd0;
            read_ptr   <= 4'd0;
            item_count <= 5'd0;
            dout       <= 8'd0;
        end else begin
            if (write_accept) begin
                write_ptr <= write_ptr + 1'b1;
                mem[write_ptr] <= din;
            end

            if (read_accept) begin
                read_ptr <= read_ptr + 1'b1;
                dout <= mem[read_ptr];
            end

            case ({write_accept, read_accept})
                2'b10: item_count <= item_count + 1'b1;
                2'b01: item_count <= item_count - 1'b1;
                default: item_count <= item_count;
            endcase
        end
    end

endmodule
