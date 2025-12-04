`define W 7:0
module simplePipe__DOT__ADD(
input wire     [7:0]  inst,
input wire      [`W] r0_pre,
input wire      [`W] r1_pre,
input wire      [`W] r2_pre,
input wire      [`W] r3_pre,
output reg      [`W] r0_post,
output reg      [`W] r1_post,
output reg      [`W] r2_post,
output reg      [`W] r3_post
);


wire [1:0] op = inst[7:6];
wire [1:0] rs1= inst[5:4];
wire [1:0] rs2= inst[3:2];
wire [1:0] rd = inst[1:0];
wire is_add = op == 2'b01;

wire [`W] rs1_val = rs1 == 2'b00 ? r0_pre  : 
                     rs1 == 2'b01 ? r1_pre  : 
                     rs1 == 2'b10 ? r2_pre  : 
                                    r3_pre  ;

wire [`W] rs2_val = rs2 == 2'b00 ? r0_pre : 
                     rs2 == 2'b01 ? r1_pre : 
                     rs2 == 2'b10 ? r2_pre : 
                                    r3_pre ;

wire [`W] rd_val = rs1_val + rs2_val;

always @(*) begin
    r0_post = r0_pre;
    r1_post = r1_pre;
    r2_post = r2_pre;
    r3_post = r3_pre;
    
    if(is_add) begin
        case(rd)
            2'd0: r0_post = rd_val;
            2'd1: r1_post = rd_val;
            2'd2: r2_post = rd_val;
            2'd3: r3_post = rd_val;
        endcase
    end
end

endmodule
