`define W 7:0
module wrapper(input wire clk, input wire rst,
    input wire [7:0] inst, input wire inst_valid, output wire inst_ready,
    input wire stallex, input wire stallwb,
    input wire [1:0] dummy_read_rf, output wire [`W] dummy_rf_data,

    input wire __START__
    );

wire FV_if_go;
wire FV_id_go;
wire FV_ex_go;
wire FV_wb_go;

wire      [`W] FV_r0;
wire      [`W] FV_r1;
wire      [`W] FV_r2;
wire      [`W] FV_r3;

pipeline_v dut(
	.clk(clk),.rst(rst),.inst(inst),.inst_valid(inst_valid),.inst_ready(inst_ready),
	.stallex(stallex),.stallwb(stallwb),
	.dummy_read_rf(dummy_read_rf),.dummy_rf_data(dummy_rf_data),

	.FV_if_go(FV_if_go),
	.FV_id_go(FV_id_go),
	.FV_ex_go(FV_ex_go),
	.FV_wb_go(FV_wb_go),

	.FV_r0(FV_r0),
	.FV_r1(FV_r1),
	.FV_r2(FV_r2),
	.FV_r3(FV_r3)
	);

// TODO: phase tracker
// TODO: undetermined start

reg __STARTED__;
reg __ENDED__;

reg if_id_iuv;
reg id_ex_iuv;
reg ex_wb_iuv;
reg wb_iuv;

reg [`W] r0_pre;
reg [`W] r1_pre;
reg [`W] r2_pre;
reg [`W] r3_pre;

reg [`W] r0_post;
reg [`W] r1_post;
reg [`W] r2_post;
reg [`W] r3_post;

wire [`W] r0_post_spec;
wire [`W] r1_post_spec;
wire [`W] r2_post_spec;
wire [`W] r3_post_spec;
reg [`W] inst_hold;

wire __DECODE__ = FV_if_go && inst[7:6]==1;

assume property ((~__STARTED__) || ~ __START__);
assume property ((~ __START__) || (__DECODE__) );


wire __EDCOND__ = ((wb_iuv)==(1))&&(__STARTED__) ;
wire __IEND__ = (((((wb_iuv)==(1))&&(__STARTED__)))&&(!(__ENDED__)))&&(1'b1) ;


always @(posedge clk) begin
	if (rst) __STARTED__ <= 0;
	else if (__START__) __STARTED__ <= 1;
end
always @(posedge clk) begin
	if (rst) __ENDED__ <= 0;
	else if (__IEND__) __ENDED__ <= 1;
end


always @(posedge clk) begin
  if(rst)
    if_id_iuv <= 0;
  else if(__START__)
    if_id_iuv <= 1;
  else if(if_id_iuv && FV_id_go)
    if_id_iuv <= 0;
end

always @(posedge clk) begin
  if(rst)
    id_ex_iuv <= 0;
  else if(if_id_iuv && FV_id_go)
    id_ex_iuv <= 1;
  else if(id_ex_iuv && FV_ex_go)
    id_ex_iuv <= 0;
end

always @(posedge clk) begin
  if(rst)
    ex_wb_iuv <= 0;
  else if(id_ex_iuv && FV_ex_go)
    ex_wb_iuv <= 1;
  else if(ex_wb_iuv && FV_wb_go)
    ex_wb_iuv <= 0;
end

always @(posedge clk) begin
  if(rst)
    wb_iuv <= 0;
  else if(ex_wb_iuv && FV_wb_go)
    wb_iuv <= 1;
  else if(wb_iuv )
    wb_iuv <= 0; // just last for one cycle
end


always @(*) begin
	if(__ENDED__) begin
		assert (r0_post == r0_post_spec);
		assert (r1_post == r1_post_spec);
		assert (r2_post == r2_post_spec);
		assert (r3_post == r3_post_spec);
	end
end

always @(posedge clk) begin
  if (__START__)
    inst_hold <= inst;
	if(ex_wb_iuv && FV_wb_go) begin
		r0_pre <= FV_r0;
		r1_pre <= FV_r1;
		r2_pre <= FV_r2;
		r3_pre <= FV_r3;
	end

	if(wb_iuv) begin
		r0_post <= FV_r0;
		r1_post <= FV_r1;
		r2_post <= FV_r2;
		r3_post <= FV_r3;
	end
end


simplePipe__DOT__ADD chk(
	.inst(inst_hold),
	.r0_pre(r0_pre),
	.r1_pre(r1_pre),
	.r2_pre(r2_pre),
	.r3_pre(r3_pre),
	.r0_post(r0_post_spec),
	.r1_post(r1_post_spec),
	.r2_post(r2_post_spec),
	.r3_post(r3_post_spec)
	);



endmodule