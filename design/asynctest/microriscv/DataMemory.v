/* HZ: Actually this is not in use for now */

module SimpleDMem #(
    parameter MEM_BYTES = 4096
) (
    input               clk,
    input               rst,

    input      [31:0]   io_memIF_DMem_address,
    output reg [31:0]   io_memIF_DMem_readData,
    input      [31:0]   io_memIF_DMem_writeData,
    input               io_memIF_DMem_readWrite,
    input               io_memIF_DMem_enable,
    input      [3:0]    io_memIF_DMem_wrStrobe,
    output reg          io_memIF_DMem_dataReady
);

    // Byte-addressable memory
    reg [7:0] mem [0:MEM_BYTES-1];

    // Pipeline one request, response returned next cycle
    reg        req_valid;
    reg        req_is_write;
    reg [31:0] req_addr;
    reg [31:0] req_wdata;
    reg [3:0]  req_wstrb;

    wire is_write = io_memIF_DMem_readWrite;

    always @(posedge clk) begin
        if (rst) begin
            req_valid               <= 1'b0;
            req_is_write            <= 1'b0;
            req_addr                <= 32'b0;
            req_wdata               <= 32'b0;
            req_wstrb               <= 4'b0;
            io_memIF_DMem_readData  <= 32'b0;
            io_memIF_DMem_dataReady <= 1'b0;
        end else begin
            // default: no response unless a request from previous cycle completes
            io_memIF_DMem_dataReady <= 1'b0;

            // Complete previous cycle's request
            if (req_valid) begin
                if (req_is_write) begin
                    if (req_addr + 3 < MEM_BYTES) begin
                        if (req_wstrb[0]) mem[req_addr + 0] <= req_wdata[7:0];
                        if (req_wstrb[1]) mem[req_addr + 1] <= req_wdata[15:8];
                        if (req_wstrb[2]) mem[req_addr + 2] <= req_wdata[23:16];
                        if (req_wstrb[3]) mem[req_addr + 3] <= req_wdata[31:24];
                    end
                end else begin
                    if (req_addr + 3 < MEM_BYTES) begin
                        io_memIF_DMem_readData <= {
                            mem[req_addr + 3],
                            mem[req_addr + 2],
                            mem[req_addr + 1],
                            mem[req_addr + 0]
                        };
                    end else begin
                        io_memIF_DMem_readData <= 32'b0;
                    end
                end

                io_memIF_DMem_dataReady <= 1'b1;
            end

            // Capture new request
            req_valid    <= io_memIF_DMem_enable;
            req_is_write <= is_write;
            req_addr     <= io_memIF_DMem_address;
            req_wdata    <= io_memIF_DMem_writeData;
            req_wstrb    <= io_memIF_DMem_wrStrobe;
        end
    end

endmodule

