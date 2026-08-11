// Generator : SpinalHDL v1.4.2    git head : 804c7bd7b7feaddcc1d25ecef6c208fd5f776f79
// Component : RiscV32Core


`define OpASelect_binary_sequential_type [1:0]
`define OpASelect_binary_sequential_opReg1Data 2'b00
`define OpASelect_binary_sequential_opPC 2'b01
`define OpASelect_binary_sequential_opZero 2'b10

`define OpBSelect_binary_sequential_type [1:0]
`define OpBSelect_binary_sequential_opReg2Data 2'b00
`define OpBSelect_binary_sequential_opImmediate 2'b01
`define OpBSelect_binary_sequential_opZero 2'b10

`define CSRAccessType_binary_sequential_type [2:0]
`define CSRAccessType_binary_sequential_CSRidle 3'b000
`define CSRAccessType_binary_sequential_CSRread 3'b001
`define CSRAccessType_binary_sequential_CSRwrite 3'b010
`define CSRAccessType_binary_sequential_CSRset 3'b011
`define CSRAccessType_binary_sequential_CSRclear 3'b100

`define CSRDataSelect_binary_sequential_type [0:0]
`define CSRDataSelect_binary_sequential_reg1Data 1'b0
`define CSRDataSelect_binary_sequential_csrImmData 1'b1

`define InstructionType_binary_sequential_type [4:0]
`define InstructionType_binary_sequential_isUndef 5'b00000
`define InstructionType_binary_sequential_isRegReg 5'b00001
`define InstructionType_binary_sequential_isRegImm 5'b00010
`define InstructionType_binary_sequential_isImm 5'b00011
`define InstructionType_binary_sequential_isBranch 5'b00100
`define InstructionType_binary_sequential_isLoad 5'b00101
`define InstructionType_binary_sequential_isStore 5'b00110
`define InstructionType_binary_sequential_isCT_JAL 5'b00111
`define InstructionType_binary_sequential_isCT_JALR 5'b01000
`define InstructionType_binary_sequential_isLUI 5'b01001
`define InstructionType_binary_sequential_isAUIPC 5'b01010
`define InstructionType_binary_sequential_isECall 5'b01011
`define InstructionType_binary_sequential_isFence 5'b01100
`define InstructionType_binary_sequential_isIllegal 5'b01101
`define InstructionType_binary_sequential_isCSR 5'b01110
`define InstructionType_binary_sequential_isCSRImm 5'b01111
`define InstructionType_binary_sequential_isTrapReturn 5'b10000
`define InstructionType_binary_sequential_isMulDiv 5'b10001

`define MCauseSelect_binary_sequential_type [1:0]
`define MCauseSelect_binary_sequential_trapInstrAddrMisalign 2'b00
`define MCauseSelect_binary_sequential_trapIllegalInstr 2'b01
`define MCauseSelect_binary_sequential_trapECallMachine 2'b10
`define MCauseSelect_binary_sequential_trapMachineTimerIRQ 2'b11

`define MemoryStrobeSelect_binary_sequential_type [1:0]
`define MemoryStrobeSelect_binary_sequential_byte_1 2'b00
`define MemoryStrobeSelect_binary_sequential_halfWord 2'b01
`define MemoryStrobeSelect_binary_sequential_word 2'b10

`define PCSelect_binary_sequential_type [2:0]
`define PCSelect_binary_sequential_incrementPC 3'b000
`define PCSelect_binary_sequential_jalTarget 3'b001
`define PCSelect_binary_sequential_jalrTarget 3'b010
`define PCSelect_binary_sequential_branchTarget 3'b011
`define PCSelect_binary_sequential_trapEntryTarget 3'b100
`define PCSelect_binary_sequential_trapExitTarget 3'b101

`define DestDataSelect_binary_sequential_type [2:0]
`define DestDataSelect_binary_sequential_aluRes 3'b000
`define DestDataSelect_binary_sequential_aluBool 3'b001
`define DestDataSelect_binary_sequential_memReadData 3'b010
`define DestDataSelect_binary_sequential_csrReadData 3'b011
`define DestDataSelect_binary_sequential_muldivData 3'b100

`define fsm_enumDefinition_binary_sequential_type [3:0]
`define fsm_enumDefinition_binary_sequential_fsm_BOOT 4'b0000
`define fsm_enumDefinition_binary_sequential_fsm_stateInit 4'b0001
`define fsm_enumDefinition_binary_sequential_fsm_stateFetch 4'b0010
`define fsm_enumDefinition_binary_sequential_fsm_stateDecode 4'b0011
`define fsm_enumDefinition_binary_sequential_fsm_stateExecute 4'b0100
`define fsm_enumDefinition_binary_sequential_fsm_stateWriteBack 4'b0101
`define fsm_enumDefinition_binary_sequential_fsm_stateTrap 4'b0110
`define fsm_enumDefinition_binary_sequential_fsm_stateCSR 4'b0111
`define fsm_enumDefinition_binary_sequential_fsm_stateInterrupt 4'b1000
`define fsm_enumDefinition_binary_sequential_fsm_stateHalt 4'b1001


module RiscV32Core (
  input      [31:0]   io_memIF_IMem_instruction,
  output     [31:0]   io_memIF_IMem_address,
  output              io_memIF_IMem_fetchEnable,
  input               io_memIF_IMem_instructionReady,
  output     [31:0]   io_memIF_DMem_address,
  input      [31:0]   io_memIF_DMem_readData,
  output     [31:0]   io_memIF_DMem_writeData,
  output              io_memIF_DMem_readWrite,
  output              io_memIF_DMem_enable,
  output     [3:0]    io_memIF_DMem_wrStrobe,
  input               io_memIF_DMem_dataReady,
  output              io_halted,
  output              io_fetchSync,
  input               io_halt,
  input               io_haltErr,
  output     [3:0]    io_dbgState,
  input               io_irqTimer,
  output     [63:0]   rvfi_order,
  output     [31:0]   rvfi_insn,
  output              rvfi_trap,
  output     [0:0]    rvfi_intr,
  output     [1:0]    rvfi_mode,
  output     [1:0]    rvfi_ixl,
  output     [4:0]    rvfi_rs1_addr,
  output     [4:0]    rvfi_rs2_addr,
  output     [31:0]   rvfi_rs1_rdata,
  output     [31:0]   rvfi_rs2_rdata,
  output     [4:0]    rvfi_rd_addr,
  output     [31:0]   rvfi_rd_wdata,
  output     [31:0]   rvfi_pc_rdata,
  output reg [31:0]   rvfi_pc_wdata,
  output     [31:0]   rvfi_mem_addr,
  output     [3:0]    rvfi_mem_rmask,
  output     [3:0]    rvfi_mem_wmask,
  output     [31:0]   rvfi_mem_rdata,
  output     [31:0]   rvfi_mem_wdata,
  output reg [0:0]    rvfi_valid,
  output              rvfi_halt,
  input               clk,
  input               reset
);
  wire                _zz_44;
  wire                _zz_45;
  wire       [4:0]    _zz_46;
  wire       [4:0]    _zz_47;
  wire       [4:0]    _zz_48;
  wire                ctrlLogic_io_pcCtrl_enablePC;
  wire       `PCSelect_binary_sequential_type ctrlLogic_io_pcCtrl_pcValSel;
  wire                ctrlLogic_io_fetchCtrl_sample;
  wire       `OpASelect_binary_sequential_type ctrlLogic_io_aluCtrl_opA;
  wire       `OpBSelect_binary_sequential_type ctrlLogic_io_aluCtrl_opB;
  wire                ctrlLogic_io_regCtrl_regFileWR;
  wire       `DestDataSelect_binary_sequential_type ctrlLogic_io_regCtrl_regDestSel;
  wire       `CSRDataSelect_binary_sequential_type ctrlLogic_io_csrCtrl_writeSelect;
  wire                ctrlLogic_io_csrCtrl_enable;
  wire                ctrlLogic_io_csrCtrl_newFetch;
  wire       `MCauseSelect_binary_sequential_type ctrlLogic_io_csrCtrl_mcauseSelect;
  wire                ctrlLogic_io_memCtrl_fetchEna;
  wire                ctrlLogic_io_memCtrl_readWriteData;
  wire                ctrlLogic_io_memCtrl_dataEna;
  wire       `MemoryStrobeSelect_binary_sequential_type ctrlLogic_io_memCtrl_strobeSelect;
  wire                ctrlLogic_io_trapEntry;
  wire                ctrlLogic_io_trapExit;
  wire                ctrlLogic_io_irqEntry;
  wire                ctrlLogic_io_halted;
  wire                ctrlLogic_io_fetchSync;
  wire       [3:0]    ctrlLogic_io_dbgState;
  wire       [31:0]   fetchUnit_1_io_instruction;
  wire       [6:0]    decoder_io_fields_opcode;
  wire       [4:0]    decoder_io_fields_src1;
  wire       [4:0]    decoder_io_fields_src2;
  wire       [4:0]    decoder_io_fields_dest;
  wire       [2:0]    decoder_io_fields_funct3;
  wire       [6:0]    decoder_io_fields_funct7;
  wire       [11:0]   decoder_io_fields_funct12;
  wire       [4:0]    decoder_io_fields_shamt;
  wire       [11:0]   decoder_io_fields_csr;
  wire       [31:0]   decoder_io_immediate;
  wire       [4:0]    decoder_io_csr_uimm;
  wire                decoder_io_decodeValid;
  wire       `InstructionType_binary_sequential_type decoder_io_instType;
  wire       `CSRAccessType_binary_sequential_type decoder_io_csrType;
  wire       [31:0]   regs_io_rs1Data;
  wire       [31:0]   regs_io_rs2Data;
  wire       [31:0]   alu_io_output;
  wire                alu_io_output_bool;
  wire                _zz_49;
  wire                _zz_50;
  wire                _zz_51;
  wire       [63:0]   _zz_52;
  wire       [63:0]   _zz_53;
  wire       [31:0]   _zz_54;
  wire       [31:0]   _zz_55;
  wire       [31:0]   _zz_56;
  wire       [31:0]   _zz_57;
  wire       [31:0]   _zz_58;
  wire       [31:0]   _zz_59;
  wire       [31:0]   _zz_60;
  wire       [31:0]   _zz_61;
  wire       [31:0]   _zz_62;
  wire       [31:0]   _zz_63;
  wire       [7:0]    _zz_64;
  wire       [31:0]   _zz_65;
  wire       [15:0]   _zz_66;
  wire       [31:0]   _zz_67;
  wire       [7:0]    _zz_68;
  wire       [31:0]   _zz_69;
  wire       [15:0]   _zz_70;
  wire       [31:0]   _zz_71;
  wire       [0:0]    _zz_72;
  reg        [31:0]   programCounter;
  wire       [31:0]   pcValMux;
  wire       [31:0]   rdDataMux;
  wire       [31:0]   csrValMux;
  wire       [3:0]    strobeMux;
  wire                irqPending;
  reg        [31:0]   _zz_1;
  reg        [31:0]   _zz_2;
  wire       [31:0]   muldivResult;
  wire                muldivReady;
  wire                muldivBusy;
  wire       [11:0]   CSRLogic_addr;
  wire       `CSRAccessType_binary_sequential_type CSRLogic_accessType;
  wire                CSRLogic_ena;
  wire       [31:0]   CSRLogic_wval;
  reg        [31:0]   CSRLogic_rval;
  wire                CSRLogic_newFetch;
  reg                 CSRLogic_isIllegalAccess;
  wire                CSRLogic_newTimerIRQ;
  wire                CSRLogic_rdX0;
  wire                CSRLogic_rs1X0;
  wire                CSRLogic_uimmZero;
  wire                CSRLogic_chooseOperand;
  wire                CSRLogic_wrCSRcnd;
  wire       [31:0]   CSRLogic_mvendorid;
  wire       [31:0]   CSRLogic_marchid;
  wire       [31:0]   CSRLogic_mimpid;
  wire       [31:0]   CSRLogic_mhartid;
  reg        [31:0]   CSRLogic_mstatus;
  reg        [31:0]   _zz_3;
  wire       [31:0]   CSRLogic_misa;
  reg        [31:0]   _zz_4;
  reg        [31:0]   CSRLogic_medeleg;
  reg        [31:0]   CSRLogic_mideleg;
  reg        [31:0]   CSRLogic_mie;
  reg        [31:0]   CSRLogic_mtvec;
  reg        [31:0]   CSRLogic_mepc;
  reg        [31:0]   CSRLogic_mcause;
  reg        [31:0]   CSRLogic_mtval;
  reg        [31:0]   CSRLogic_mip;
  wire       [31:0]   CSRLogic_mtinst;
  reg        [63:0]   CSRLogic_minstret;
  reg        [63:0]   CSRLogic_mcycle;
  wire       [31:0]   _zz_5;
  wire       [31:0]   _zz_6;
  wire       [31:0]   _zz_7;
  wire       [31:0]   _zz_8;
  wire       [31:0]   _zz_9;
  wire       [31:0]   _zz_10;
  wire       [31:0]   _zz_11;
  wire       [31:0]   _zz_12;
  wire       [31:0]   _zz_13;
  wire       [31:0]   _zz_14;
  reg        [31:0]   _zz_15;
  wire       [31:0]   _zz_16;
  reg        [31:0]   _zz_17;
  reg        [31:0]   _zz_18;
  reg        [31:0]   _zz_19;
  reg        [31:0]   _zz_20;
  reg        [31:0]   _zz_21;
  reg        [3:0]    _zz_22;
  wire       [31:0]   incrPC;
  wire       [31:0]   jalTarget;
  wire       [31:0]   jalrTarget;
  wire       [31:0]   branchTarget;
  wire       [31:0]   trapTarget;
  wire       [31:0]   mretTarget;
  reg        [31:0]   _zz_23;
  wire                jalMisalign;
  wire                jalrMisalign;
  wire                branchMisalign;
  reg        [31:0]   extMemData;
  reg        [31:0]   _zz_24;
  reg        [63:0]   _zz_25;
  reg        [31:0]   _zz_26;
  reg                 _zz_27;
  wire       [0:0]    _zz_28;
  wire       [1:0]    _zz_29;
  wire       [1:0]    _zz_30;
  reg        [4:0]    _zz_31;
  reg        [4:0]    _zz_32;
  reg        [31:0]   _zz_33;
  reg        [31:0]   _zz_34;
  reg        [4:0]    _zz_35;
  reg        [31:0]   _zz_36;
  reg        [31:0]   _zz_37;
  reg        [31:0]   _zz_38;
  reg        [3:0]    _zz_39;
  reg        [3:0]    _zz_40;
  reg        [31:0]   _zz_41;
  reg        [31:0]   _zz_42;
  reg                 _zz_43;
  `ifndef SYNTHESIS
  reg [63:0] CSRLogic_accessType_string;
  `endif

  function [31:0] zz__zz_3(input dummy);
    begin
      zz__zz_3 = 32'h0;
      zz__zz_3[12 : 11] = 2'b11;
    end
  endfunction
  wire [31:0] _zz_73;
  function [31:0] zz__zz_4(input dummy);
    begin
      zz__zz_4 = 32'h0;
      zz__zz_4[31 : 30] = 2'b01;
      zz__zz_4[8] = 1'b1;
    end
  endfunction
  wire [31:0] _zz_74;
  function [31:0] zz__zz_17(input dummy);
    begin
      zz__zz_17 = 32'h0;
      zz__zz_17[31] = 1'b0;
    end
  endfunction
  wire [31:0] _zz_75;
  function [31:0] zz__zz_18(input dummy);
    begin
      zz__zz_18 = 32'h0;
      zz__zz_18[31] = 1'b0;
    end
  endfunction
  wire [31:0] _zz_76;
  function [31:0] zz__zz_19(input dummy);
    begin
      zz__zz_19 = 32'h0;
      zz__zz_19[31] = 1'b0;
    end
  endfunction
  wire [31:0] _zz_77;
  function [31:0] zz__zz_20(input dummy);
    begin
      zz__zz_20 = 32'h0;
      zz__zz_20[31] = 1'b1;
    end
  endfunction
  wire [31:0] _zz_78;

  assign _zz_49 = (io_dbgState == 4'b0001);
  assign _zz_50 = (! _zz_43);
  assign _zz_51 = (io_dbgState == 4'b0001);
  assign _zz_52 = (CSRLogic_minstret + 64'h0000000000000001);
  assign _zz_53 = (CSRLogic_mcycle + 64'h0000000000000001);
  assign _zz_54 = (programCounter - 32'h00000004);
  assign _zz_55 = (programCounter - 32'h00000004);
  assign _zz_56 = (programCounter - 32'h00000004);
  assign _zz_57 = (decoder_io_immediate + regs_io_rs1Data);
  assign _zz_58 = (programCounter + decoder_io_immediate);
  assign _zz_59 = _zz_60;
  assign _zz_60 = ({2'd0,CSRLogic_mtvec[31 : 2]} <<< 2);
  assign _zz_61 = (jalTarget % 3'b100);
  assign _zz_62 = (jalrTarget % 3'b100);
  assign _zz_63 = (branchTarget % 3'b100);
  assign _zz_64 = io_memIF_DMem_readData[7 : 0];
  assign _zz_65 = {{24{_zz_64[7]}}, _zz_64};
  assign _zz_66 = io_memIF_DMem_readData[15 : 0];
  assign _zz_67 = {{16{_zz_66[15]}}, _zz_66};
  assign _zz_68 = io_memIF_DMem_readData[7 : 0];
  assign _zz_69 = {24'd0, _zz_68};
  assign _zz_70 = io_memIF_DMem_readData[15 : 0];
  assign _zz_71 = {16'd0, _zz_70};
  assign _zz_72 = alu_io_output_bool;
  ControlUnit ctrlLogic (
    .io_validDecode                            (decoder_io_decodeValid                  ), //i
    .io_instrType                              (decoder_io_instType[4:0]                ), //i
    .io_instrFields_opcode                     (decoder_io_fields_opcode[6:0]           ), //i
    .io_instrFields_src1                       (decoder_io_fields_src1[4:0]             ), //i
    .io_instrFields_src2                       (decoder_io_fields_src2[4:0]             ), //i
    .io_instrFields_dest                       (decoder_io_fields_dest[4:0]             ), //i
    .io_instrFields_funct3                     (decoder_io_fields_funct3[2:0]           ), //i
    .io_instrFields_funct7                     (decoder_io_fields_funct7[6:0]           ), //i
    .io_instrFields_funct12                    (decoder_io_fields_funct12[11:0]         ), //i
    .io_instrFields_shamt                      (decoder_io_fields_shamt[4:0]            ), //i
    .io_instrFields_csr                        (decoder_io_fields_csr[11:0]             ), //i
    .io_pcCtrl_enablePC                        (ctrlLogic_io_pcCtrl_enablePC            ), //o
    .io_pcCtrl_pcValSel                        (ctrlLogic_io_pcCtrl_pcValSel[2:0]       ), //o
    .io_fetchCtrl_sample                       (ctrlLogic_io_fetchCtrl_sample           ), //o
    .io_aluCtrl_opA                            (ctrlLogic_io_aluCtrl_opA[1:0]           ), //o
    .io_aluCtrl_opB                            (ctrlLogic_io_aluCtrl_opB[1:0]           ), //o
    .io_aluCtrl_aluBranch                      (alu_io_output_bool                      ), //i
    .io_regCtrl_regFileWR                      (ctrlLogic_io_regCtrl_regFileWR          ), //o
    .io_regCtrl_regDestSel                     (ctrlLogic_io_regCtrl_regDestSel[2:0]    ), //o
    .io_csrCtrl_writeSelect                    (ctrlLogic_io_csrCtrl_writeSelect        ), //o
    .io_csrCtrl_enable                         (ctrlLogic_io_csrCtrl_enable             ), //o
    .io_csrCtrl_newFetch                       (ctrlLogic_io_csrCtrl_newFetch           ), //o
    .io_csrCtrl_illegalAccess                  (_zz_44                                  ), //i
    .io_csrCtrl_mcauseSelect                   (ctrlLogic_io_csrCtrl_mcauseSelect[1:0]  ), //o
    .io_memCtrl_fetchEna                       (ctrlLogic_io_memCtrl_fetchEna           ), //o
    .io_memCtrl_instrRdy                       (io_memIF_IMem_instructionReady          ), //i
    .io_memCtrl_readWriteData                  (ctrlLogic_io_memCtrl_readWriteData      ), //o
    .io_memCtrl_dataEna                        (ctrlLogic_io_memCtrl_dataEna            ), //o
    .io_memCtrl_dataRdy                        (io_memIF_DMem_dataReady                 ), //i
    .io_memCtrl_strobeSelect                   (ctrlLogic_io_memCtrl_strobeSelect[1:0]  ), //o
    .io_irqPending                             (irqPending                              ), //i
    .io_trapEntry                              (ctrlLogic_io_trapEntry                  ), //o
    .io_trapExit                               (ctrlLogic_io_trapExit                   ), //o
    .io_irqEntry                               (ctrlLogic_io_irqEntry                   ), //o
    .io_exceptions_misalignedJumpTarget        (jalMisalign                             ), //i
    .io_exceptions_misalignedJumpLinkTarget    (jalrMisalign                            ), //i
    .io_exceptions_misalignedBranchTarget      (branchMisalign                          ), //i
    .io_halt                                   (_zz_45                                  ), //i
    .io_halted                                 (ctrlLogic_io_halted                     ), //o
    .io_fetchSync                              (ctrlLogic_io_fetchSync                  ), //o
    .io_dbgState                               (ctrlLogic_io_dbgState[3:0]              ), //o
    .clk                                       (clk                                     ), //i
    .reset                                     (reset                                   )  //i
  );
  FetchUnit fetchUnit_1 (
    .io_data           (io_memIF_IMem_instruction[31:0]   ), //i
    .io_sample         (ctrlLogic_io_fetchCtrl_sample     ), //i
    .io_instruction    (fetchUnit_1_io_instruction[31:0]  ), //o
    .clk               (clk                               ), //i
    .reset             (reset                             )  //i
  );
  DecodeUnit decoder (
    .io_instruction       (fetchUnit_1_io_instruction[31:0]  ), //i
    .io_fields_opcode     (decoder_io_fields_opcode[6:0]     ), //o
    .io_fields_src1       (decoder_io_fields_src1[4:0]       ), //o
    .io_fields_src2       (decoder_io_fields_src2[4:0]       ), //o
    .io_fields_dest       (decoder_io_fields_dest[4:0]       ), //o
    .io_fields_funct3     (decoder_io_fields_funct3[2:0]     ), //o
    .io_fields_funct7     (decoder_io_fields_funct7[6:0]     ), //o
    .io_fields_funct12    (decoder_io_fields_funct12[11:0]   ), //o
    .io_fields_shamt      (decoder_io_fields_shamt[4:0]      ), //o
    .io_fields_csr        (decoder_io_fields_csr[11:0]       ), //o
    .io_immediate         (decoder_io_immediate[31:0]        ), //o
    .io_csr_uimm          (decoder_io_csr_uimm[4:0]          ), //o
    .io_decodeValid       (decoder_io_decodeValid            ), //o
    .io_instType          (decoder_io_instType[4:0]          ), //o
    .io_csrType           (decoder_io_csrType[2:0]           )  //o
  );
  RV32RegisterFile regs (
    .io_rs1        (_zz_46[4:0]                     ), //i
    .io_rs2        (_zz_47[4:0]                     ), //i
    .io_rs1Data    (regs_io_rs1Data[31:0]           ), //o
    .io_rs2Data    (regs_io_rs2Data[31:0]           ), //o
    .io_wrEna      (ctrlLogic_io_regCtrl_regFileWR  ), //i
    .io_rd         (_zz_48[4:0]                     ), //i
    .io_rdData     (rdDataMux[31:0]                 ), //i
    .clk           (clk                             ), //i
    .reset         (reset                           )  //i
  );
  ArithmeticLogicUnit alu (
    .io_opA                (_zz_1[31:0]                    ), //i
    .io_opB                (_zz_2[31:0]                    ), //i
    .io_operation_f3       (decoder_io_fields_funct3[2:0]  ), //i
    .io_operation_f7       (decoder_io_fields_funct7[6:0]  ), //i
    .io_operation_shamt    (decoder_io_fields_shamt[4:0]   ), //i
    .io_operation_instr    (decoder_io_instType[4:0]       ), //i
    .io_output             (alu_io_output[31:0]            ), //o
    .io_output_bool        (alu_io_output_bool             )  //o
  );
  `ifndef SYNTHESIS
  always @(*) begin
    case(CSRLogic_accessType)
      `CSRAccessType_binary_sequential_CSRidle : CSRLogic_accessType_string = "CSRidle ";
      `CSRAccessType_binary_sequential_CSRread : CSRLogic_accessType_string = "CSRread ";
      `CSRAccessType_binary_sequential_CSRwrite : CSRLogic_accessType_string = "CSRwrite";
      `CSRAccessType_binary_sequential_CSRset : CSRLogic_accessType_string = "CSRset  ";
      `CSRAccessType_binary_sequential_CSRclear : CSRLogic_accessType_string = "CSRclear";
      default : CSRLogic_accessType_string = "????????";
    endcase
  end
  `endif

  assign io_fetchSync = ctrlLogic_io_fetchSync;
  assign io_halted = ctrlLogic_io_halted;
  assign _zz_45 = (io_halt || io_haltErr);
  assign io_dbgState = ctrlLogic_io_dbgState;
  assign io_memIF_IMem_address = programCounter;
  assign io_memIF_IMem_fetchEnable = ctrlLogic_io_memCtrl_fetchEna;
  assign _zz_46 = decoder_io_fields_src1;
  assign _zz_47 = decoder_io_fields_src2;
  assign _zz_48 = decoder_io_fields_dest;
  always @ (*) begin
    case(ctrlLogic_io_aluCtrl_opA)
      `OpASelect_binary_sequential_opReg1Data : begin
        _zz_1 = regs_io_rs1Data;
      end
      `OpASelect_binary_sequential_opPC : begin
        _zz_1 = programCounter;
      end
      default : begin
        _zz_1 = 32'h0;
      end
    endcase
  end

  always @ (*) begin
    case(ctrlLogic_io_aluCtrl_opB)
      `OpBSelect_binary_sequential_opReg2Data : begin
        _zz_2 = regs_io_rs2Data;
      end
      `OpBSelect_binary_sequential_opImmediate : begin
        _zz_2 = decoder_io_immediate;
      end
      default : begin
        _zz_2 = 32'h0;
      end
    endcase
  end

  assign muldivResult = 32'h0;
  assign muldivReady = 1'b0;
  assign muldivBusy = 1'b0;
  assign CSRLogic_mvendorid = 32'h0;
  assign CSRLogic_marchid = 32'h0;
  assign CSRLogic_mimpid = 32'h0;
  assign CSRLogic_mhartid = 32'h0;
  assign _zz_73 = zz__zz_3(1'b0);
  always @ (*) _zz_3 = _zz_73;
  assign _zz_74 = zz__zz_4(1'b0);
  always @ (*) _zz_4 = _zz_74;
  assign CSRLogic_misa = _zz_4;
  assign CSRLogic_mtinst = 32'h0;
  assign CSRLogic_wrCSRcnd = ((CSRLogic_rs1X0 && CSRLogic_chooseOperand) || (CSRLogic_uimmZero && (! CSRLogic_chooseOperand)));
  always @ (*) begin
    CSRLogic_isIllegalAccess = 1'b0;
    if(CSRLogic_ena)begin
      case(CSRLogic_addr)
        12'hf11 : begin
        end
        12'hf12 : begin
        end
        12'hf13 : begin
        end
        12'hf14 : begin
        end
        12'h300 : begin
        end
        12'h301 : begin
        end
        12'h302 : begin
        end
        12'h303 : begin
        end
        12'h304 : begin
        end
        12'h305 : begin
        end
        12'h341 : begin
        end
        12'h342 : begin
        end
        12'h343 : begin
        end
        12'h344 : begin
        end
        12'hb00 : begin
        end
        12'hb02 : begin
        end
        12'hb80 : begin
        end
        12'hb82 : begin
        end
        default : begin
          CSRLogic_isIllegalAccess = 1'b1;
        end
      endcase
    end
  end

  assign _zz_5 = 32'hffffffff;
  assign _zz_6 = 32'hffffffff;
  assign _zz_7 = 32'hffffffff;
  assign _zz_8 = 32'h00000888;
  assign _zz_9 = 32'hfffffffc;
  assign _zz_10 = (CSRLogic_wval & 32'hfffffffc);
  assign _zz_11 = 32'hfffffffc;
  assign _zz_12 = (CSRLogic_wval & 32'hfffffffc);
  assign _zz_13 = 32'hffffffff;
  assign _zz_14 = 32'hffffffff;
  assign CSRLogic_addr = decoder_io_fields_csr;
  assign CSRLogic_accessType = decoder_io_csrType;
  assign CSRLogic_newFetch = ctrlLogic_io_csrCtrl_newFetch;
  assign CSRLogic_ena = ctrlLogic_io_csrCtrl_enable;
  assign irqPending = ((CSRLogic_mip[7] && CSRLogic_mie[7]) && CSRLogic_mstatus[3]);
  always @ (*) begin
    case(ctrlLogic_io_csrCtrl_writeSelect)
      `CSRDataSelect_binary_sequential_reg1Data : begin
        _zz_15 = regs_io_rs1Data;
      end
      default : begin
        _zz_15 = {27'd0, decoder_io_csr_uimm};
      end
    endcase
  end

  assign csrValMux = _zz_15;
  assign CSRLogic_wval = csrValMux;
  assign CSRLogic_newTimerIRQ = io_irqTimer;
  assign CSRLogic_rdX0 = (decoder_io_fields_dest == 5'h0);
  assign CSRLogic_rs1X0 = (decoder_io_fields_src1 == 5'h0);
  assign CSRLogic_uimmZero = (decoder_io_csr_uimm == 5'h0);
  assign CSRLogic_chooseOperand = (decoder_io_instType == `InstructionType_binary_sequential_isCSR);
  assign _zz_75 = zz__zz_17(1'b0);
  always @ (*) _zz_17 = _zz_75;
  assign _zz_76 = zz__zz_18(1'b0);
  always @ (*) _zz_18 = _zz_76;
  assign _zz_77 = zz__zz_19(1'b0);
  always @ (*) _zz_19 = _zz_77;
  assign _zz_78 = zz__zz_20(1'b0);
  always @ (*) _zz_20 = _zz_78;
  always @ (*) begin
    case(ctrlLogic_io_csrCtrl_mcauseSelect)
      `MCauseSelect_binary_sequential_trapInstrAddrMisalign : begin
        _zz_21 = (_zz_17 | 32'h0);
      end
      `MCauseSelect_binary_sequential_trapIllegalInstr : begin
        _zz_21 = (_zz_18 | 32'h00000002);
      end
      `MCauseSelect_binary_sequential_trapECallMachine : begin
        _zz_21 = (_zz_19 | 32'h0000000b);
      end
      default : begin
        _zz_21 = (_zz_20 | 32'h00000007);
      end
    endcase
  end

  assign _zz_16 = _zz_21;
  assign io_memIF_DMem_address = alu_io_output;
  assign io_memIF_DMem_writeData = regs_io_rs2Data;
  assign io_memIF_DMem_readWrite = ctrlLogic_io_memCtrl_readWriteData;
  assign io_memIF_DMem_enable = ctrlLogic_io_memCtrl_dataEna;
  always @ (*) begin
    case(ctrlLogic_io_memCtrl_strobeSelect)
      `MemoryStrobeSelect_binary_sequential_byte_1 : begin
        _zz_22 = 4'b0001;
      end
      `MemoryStrobeSelect_binary_sequential_halfWord : begin
        _zz_22 = 4'b0011;
      end
      default : begin
        _zz_22 = 4'b1111;
      end
    endcase
  end

  assign io_memIF_DMem_wrStrobe = _zz_22;
  assign incrPC = (programCounter + 32'h00000004);
  assign jalTarget = (_zz_56 + decoder_io_immediate);
  assign jalrTarget = (_zz_57 & (~ 32'h00000001));
  assign branchTarget = (_zz_58 - 32'h00000004);
  assign trapTarget = _zz_59;
  assign mretTarget = CSRLogic_mepc;
  always @ (*) begin
    case(ctrlLogic_io_pcCtrl_pcValSel)
      `PCSelect_binary_sequential_incrementPC : begin
        _zz_23 = incrPC;
      end
      `PCSelect_binary_sequential_jalTarget : begin
        _zz_23 = jalTarget;
      end
      `PCSelect_binary_sequential_jalrTarget : begin
        _zz_23 = jalrTarget;
      end
      `PCSelect_binary_sequential_branchTarget : begin
        _zz_23 = branchTarget;
      end
      `PCSelect_binary_sequential_trapEntryTarget : begin
        _zz_23 = trapTarget;
      end
      default : begin
        _zz_23 = mretTarget;
      end
    endcase
  end

  assign pcValMux = _zz_23;
  assign jalMisalign = ((_zz_61 == 32'h0) ? 1'b0 : 1'b1);
  assign jalrMisalign = ((_zz_62 == 32'h0) ? 1'b0 : 1'b1);
  assign branchMisalign = ((_zz_63 == 32'h0) ? 1'b0 : 1'b1);
  always @ (*) begin
    case(decoder_io_instType)
      `InstructionType_binary_sequential_isLoad : begin
        if((((decoder_io_fields_funct3 & 3'b111) == 3'b000))) begin
            extMemData = _zz_65;
        end else if((((decoder_io_fields_funct3 & 3'b111) == 3'b001))) begin
            extMemData = _zz_67;
        end else if((((decoder_io_fields_funct3 & 3'b111) == 3'b100))) begin
            extMemData = _zz_69;
        end else if((((decoder_io_fields_funct3 & 3'b111) == 3'b101))) begin
            extMemData = _zz_71;
        end else begin
            extMemData = io_memIF_DMem_readData;
        end
      end
      default : begin
        extMemData = io_memIF_DMem_readData;
      end
    endcase
  end

  always @ (*) begin
    case(ctrlLogic_io_regCtrl_regDestSel)
      `DestDataSelect_binary_sequential_aluRes : begin
        _zz_24 = alu_io_output;
      end
      `DestDataSelect_binary_sequential_aluBool : begin
        _zz_24 = {31'd0, _zz_72};
      end
      `DestDataSelect_binary_sequential_memReadData : begin
        _zz_24 = extMemData;
      end
      `DestDataSelect_binary_sequential_csrReadData : begin
        _zz_24 = CSRLogic_rval;
      end
      default : begin
        _zz_24 = muldivResult;
      end
    endcase
  end

  assign rdDataMux = _zz_24;
  assign _zz_28 = 1'b0;
  assign _zz_29 = 2'b11;
  assign _zz_30 = 2'b01;
  always @ (*) begin
    if(_zz_49)begin
      if(_zz_50)begin
        rvfi_valid = 1'b1;
      end else begin
        rvfi_valid = 1'b0;
      end
    end else begin
      rvfi_valid = 1'b0;
    end
  end

  always @ (*) begin
    if(_zz_51)begin
      rvfi_pc_wdata = io_memIF_IMem_address;
    end else begin
      rvfi_pc_wdata = _zz_37;
    end
  end

  assign rvfi_insn = _zz_26;
  assign rvfi_halt = io_halted;
  assign rvfi_trap = _zz_27;
  assign rvfi_intr = _zz_28;
  assign rvfi_mode = _zz_29;
  assign rvfi_ixl = _zz_30;
  assign rvfi_order = _zz_25;
  assign rvfi_rs1_addr = _zz_31;
  assign rvfi_rs2_addr = _zz_32;
  assign rvfi_rs1_rdata = _zz_33;
  assign rvfi_rs2_rdata = _zz_34;
  assign rvfi_rd_addr = _zz_35;
  assign rvfi_rd_wdata = _zz_36;
  assign rvfi_pc_rdata = _zz_37;
  assign rvfi_mem_addr = _zz_38;
  assign rvfi_mem_rmask = _zz_39;
  assign rvfi_mem_wmask = _zz_40;
  assign rvfi_mem_rdata = _zz_41;
  assign rvfi_mem_wdata = _zz_42;
  always @ (posedge clk or posedge reset) begin
    if (reset) begin
      programCounter <= 32'h80000000;
      CSRLogic_rval <= 32'h0;
      CSRLogic_mstatus <= _zz_3;
      CSRLogic_medeleg <= 32'h0;
      CSRLogic_mideleg <= 32'h0;
      CSRLogic_mie <= 32'h0;
      CSRLogic_mtvec <= 32'h0;
      CSRLogic_mepc <= 32'h0;
      CSRLogic_mcause <= 32'h0;
      CSRLogic_mtval <= 32'h0;
      CSRLogic_mip <= 32'h0;
      CSRLogic_minstret <= 64'h0;
      CSRLogic_mcycle <= 64'h0;
      _zz_25 <= 64'h0;
      _zz_26 <= 32'h0;
      _zz_27 <= 1'b0;
      _zz_31 <= 5'h0;
      _zz_32 <= 5'h0;
      _zz_33 <= 32'h0;
      _zz_34 <= 32'h0;
      _zz_35 <= 5'h0;
      _zz_36 <= 32'h0;
      _zz_37 <= 32'h0;
      _zz_38 <= 32'h0;
      _zz_39 <= 4'b0000;
      _zz_40 <= 4'b0000;
      _zz_41 <= 32'h0;
      _zz_42 <= 32'h0;
      _zz_43 <= 1'b1;
    end else begin
      if(ctrlLogic_io_pcCtrl_enablePC)begin
        programCounter <= pcValMux;
      end
      CSRLogic_rval <= 32'h0;
      if(CSRLogic_ena)begin
        case(CSRLogic_addr)
          12'hf11 : begin
            CSRLogic_rval <= CSRLogic_mvendorid;
          end
          12'hf12 : begin
            CSRLogic_rval <= CSRLogic_marchid;
          end
          12'hf13 : begin
            CSRLogic_rval <= CSRLogic_mimpid;
          end
          12'hf14 : begin
            CSRLogic_rval <= CSRLogic_mhartid;
          end
          12'h300 : begin
            CSRLogic_rval <= (CSRLogic_mstatus & 32'h00001888);
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mstatus <= (CSRLogic_wval & _zz_5);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mstatus <= ((CSRLogic_mstatus & (~ _zz_5)) | (CSRLogic_wval & _zz_5));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mstatus <= ((CSRLogic_mstatus & (~ _zz_5)) | ((~ CSRLogic_wval) & _zz_5));
                end
              end
            end
          end
          12'h301 : begin
            CSRLogic_rval <= CSRLogic_misa;
          end
          12'h302 : begin
            CSRLogic_rval <= CSRLogic_medeleg;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_medeleg <= (CSRLogic_wval & _zz_6);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_medeleg <= ((CSRLogic_medeleg & (~ _zz_6)) | (CSRLogic_wval & _zz_6));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_medeleg <= ((CSRLogic_medeleg & (~ _zz_6)) | ((~ CSRLogic_wval) & _zz_6));
                end
              end
            end
          end
          12'h303 : begin
            CSRLogic_rval <= CSRLogic_mideleg;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mideleg <= (CSRLogic_wval & _zz_7);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mideleg <= ((CSRLogic_mideleg & (~ _zz_7)) | (CSRLogic_wval & _zz_7));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mideleg <= ((CSRLogic_mideleg & (~ _zz_7)) | ((~ CSRLogic_wval) & _zz_7));
                end
              end
            end
          end
          12'h304 : begin
            CSRLogic_rval <= (CSRLogic_mie & 32'h00000888);
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mie <= (CSRLogic_wval & _zz_8);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mie <= ((CSRLogic_mie & (~ _zz_8)) | (CSRLogic_wval & _zz_8));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mie <= ((CSRLogic_mie & (~ _zz_8)) | ((~ CSRLogic_wval) & _zz_8));
                end
              end
            end
          end
          12'h305 : begin
            CSRLogic_rval <= CSRLogic_mtvec;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mtvec <= (_zz_10 & _zz_9);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mtvec <= ((CSRLogic_mtvec & (~ _zz_9)) | (_zz_10 & _zz_9));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mtvec <= ((CSRLogic_mtvec & (~ _zz_9)) | ((~ _zz_10) & _zz_9));
                end
              end
            end
          end
          12'h341 : begin
            CSRLogic_rval <= CSRLogic_mepc;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mepc <= (_zz_12 & _zz_11);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mepc <= ((CSRLogic_mepc & (~ _zz_11)) | (_zz_12 & _zz_11));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mepc <= ((CSRLogic_mepc & (~ _zz_11)) | ((~ _zz_12) & _zz_11));
                end
              end
            end
          end
          12'h342 : begin
            CSRLogic_rval <= CSRLogic_mcause;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mcause <= (CSRLogic_wval & _zz_13);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mcause <= ((CSRLogic_mcause & (~ _zz_13)) | (CSRLogic_wval & _zz_13));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mcause <= ((CSRLogic_mcause & (~ _zz_13)) | ((~ CSRLogic_wval) & _zz_13));
                end
              end
            end
          end
          12'h343 : begin
            CSRLogic_rval <= CSRLogic_mtval;
            if((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRwrite))begin
              CSRLogic_mtval <= (CSRLogic_wval & _zz_14);
            end else begin
              if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRset) && (! CSRLogic_uimmZero)))begin
                CSRLogic_mtval <= ((CSRLogic_mtval & (~ _zz_14)) | (CSRLogic_wval & _zz_14));
              end else begin
                if(((CSRLogic_accessType == `CSRAccessType_binary_sequential_CSRclear) && (! CSRLogic_uimmZero)))begin
                  CSRLogic_mtval <= ((CSRLogic_mtval & (~ _zz_14)) | ((~ CSRLogic_wval) & _zz_14));
                end
              end
            end
          end
          12'h344 : begin
            CSRLogic_rval <= CSRLogic_mip;
          end
          12'hb00 : begin
            CSRLogic_rval <= CSRLogic_mcycle[31 : 0];
          end
          12'hb02 : begin
            CSRLogic_rval <= CSRLogic_minstret[31 : 0];
          end
          12'hb80 : begin
            CSRLogic_rval <= CSRLogic_mcycle[63 : 32];
          end
          12'hb82 : begin
            CSRLogic_rval <= CSRLogic_minstret[63 : 32];
          end
          default : begin
            CSRLogic_rval <= 32'h0;
          end
        endcase
      end
      if(CSRLogic_newTimerIRQ)begin
        CSRLogic_mip[7] <= 1'b1;
      end else begin
        CSRLogic_mip[7] <= 1'b0;
      end
      if(CSRLogic_newFetch)begin
        CSRLogic_minstret <= _zz_52;
      end
      CSRLogic_mcycle <= _zz_53;
      if(ctrlLogic_io_trapEntry)begin
        CSRLogic_mcause <= _zz_16;
        CSRLogic_mtval <= _zz_54;
      end
      if(ctrlLogic_io_trapExit)begin
        CSRLogic_mstatus[3] <= CSRLogic_mstatus[7];
        CSRLogic_mstatus[7] <= 1'b1;
      end
      if(ctrlLogic_io_irqEntry)begin
        CSRLogic_mstatus[7] <= CSRLogic_mstatus[3];
        CSRLogic_mstatus[3] <= 1'b0;
        CSRLogic_mcause <= _zz_16;
        CSRLogic_mtval <= _zz_55;
        CSRLogic_mepc <= programCounter;
      end
      if(_zz_49)begin
        _zz_38 <= 32'h0;
        _zz_42 <= 32'h0;
        _zz_41 <= 32'h0;
        _zz_39 <= 4'b0000;
        _zz_40 <= 4'b0000;
        if(_zz_50)begin
          _zz_43 <= 1'b1;
          _zz_25 <= (_zz_25 + 64'h0000000000000001);
        end
        if(io_memIF_IMem_instructionReady)begin
          _zz_26 <= io_memIF_IMem_instruction;
        end
      end else begin
        if((io_dbgState == 4'b0010))begin
          _zz_43 <= 1'b0;
        end
      end
      if((rvfi_valid == 1'b1))begin
        _zz_27 <= 1'b0;
      end
      if((io_dbgState == 4'b0110))begin
        _zz_27 <= 1'b1;
      end
      if((io_dbgState == 4'b0111))begin
        _zz_27 <= 1'b1;
      end
      if(_zz_51)begin
        _zz_37 <= rvfi_pc_wdata;
      end
      if((io_memIF_DMem_dataReady && (io_dbgState == 4'b0100)))begin
        _zz_38 <= io_memIF_DMem_address;
        if(io_memIF_DMem_readWrite)begin
          _zz_42 <= io_memIF_DMem_writeData;
          _zz_40 <= io_memIF_DMem_wrStrobe;
        end else begin
          _zz_41 <= io_memIF_DMem_readData;
          _zz_39 <= io_memIF_DMem_wrStrobe;
        end
      end
      if((io_dbgState == 4'b0001))begin
        _zz_35 <= 5'h0;
        _zz_36 <= 32'h0;
        _zz_31 <= 5'h0;
        _zz_32 <= 5'h0;
        _zz_33 <= 32'h0;
        _zz_34 <= 32'h0;
      end
      if((io_dbgState == 4'b0011))begin
        _zz_31 <= _zz_46;
        _zz_32 <= _zz_47;
        _zz_33 <= regs_io_rs1Data;
        _zz_34 <= regs_io_rs2Data;
      end
      if(ctrlLogic_io_regCtrl_regFileWR)begin
        _zz_35 <= _zz_48;
        if((_zz_48 == 5'h0))begin
          _zz_36 <= 32'h0;
        end else begin
          _zz_36 <= rdDataMux;
        end
      end
    end
  end


endmodule

module ArithmeticLogicUnit (
  input      [31:0]   io_opA,
  input      [31:0]   io_opB,
  input      [2:0]    io_operation_f3,
  input      [6:0]    io_operation_f7,
  input      [4:0]    io_operation_shamt,
  input      `InstructionType_binary_sequential_type io_operation_instr,
  output reg [31:0]   io_output,
  output reg          io_output_bool
);
  wire       [31:0]   _zz_1;
  wire       [31:0]   _zz_2;
  wire       [31:0]   _zz_3;
  wire       [31:0]   _zz_4;
  wire       [31:0]   _zz_5;
  wire       [31:0]   _zz_6;
  wire       [31:0]   _zz_7;
  wire       [31:0]   _zz_8;
  wire       [31:0]   _zz_9;
  wire       [31:0]   _zz_10;
  wire       [31:0]   _zz_11;
  wire       [31:0]   add;
  wire       [31:0]   sub;
  wire                equal;
  wire                unequal;
  wire                lt_u;
  wire                lt_s;
  wire                ge_u;
  wire                ge_s;
  wire       [31:0]   bitAnd;
  wire       [31:0]   bitOr;
  wire       [31:0]   bitXor;
  wire       [31:0]   shiftL;
  wire       [31:0]   shiftR;
  wire       [31:0]   shiftRA;
  wire       [31:0]   shiftLI;
  wire       [31:0]   shiftRI;
  wire       [31:0]   shiftRAI;
  `ifndef SYNTHESIS
  reg [95:0] io_operation_instr_string;
  `endif


  assign _zz_1 = (io_opA + io_opB);
  assign _zz_2 = (io_opA - io_opB);
  assign _zz_3 = io_opA;
  assign _zz_4 = io_opB;
  assign _zz_5 = io_opB;
  assign _zz_6 = io_opA;
  assign _zz_7 = ($signed(_zz_8) >>> io_opB[4 : 0]);
  assign _zz_8 = io_opA;
  assign _zz_9 = ($signed(_zz_10) >>> io_operation_shamt);
  assign _zz_10 = io_opA;
  assign _zz_11 = (add - 32'h00000004);
  `ifndef SYNTHESIS
  always @(*) begin
    case(io_operation_instr)
      `InstructionType_binary_sequential_isUndef : io_operation_instr_string = "isUndef     ";
      `InstructionType_binary_sequential_isRegReg : io_operation_instr_string = "isRegReg    ";
      `InstructionType_binary_sequential_isRegImm : io_operation_instr_string = "isRegImm    ";
      `InstructionType_binary_sequential_isImm : io_operation_instr_string = "isImm       ";
      `InstructionType_binary_sequential_isBranch : io_operation_instr_string = "isBranch    ";
      `InstructionType_binary_sequential_isLoad : io_operation_instr_string = "isLoad      ";
      `InstructionType_binary_sequential_isStore : io_operation_instr_string = "isStore     ";
      `InstructionType_binary_sequential_isCT_JAL : io_operation_instr_string = "isCT_JAL    ";
      `InstructionType_binary_sequential_isCT_JALR : io_operation_instr_string = "isCT_JALR   ";
      `InstructionType_binary_sequential_isLUI : io_operation_instr_string = "isLUI       ";
      `InstructionType_binary_sequential_isAUIPC : io_operation_instr_string = "isAUIPC     ";
      `InstructionType_binary_sequential_isECall : io_operation_instr_string = "isECall     ";
      `InstructionType_binary_sequential_isFence : io_operation_instr_string = "isFence     ";
      `InstructionType_binary_sequential_isIllegal : io_operation_instr_string = "isIllegal   ";
      `InstructionType_binary_sequential_isCSR : io_operation_instr_string = "isCSR       ";
      `InstructionType_binary_sequential_isCSRImm : io_operation_instr_string = "isCSRImm    ";
      `InstructionType_binary_sequential_isTrapReturn : io_operation_instr_string = "isTrapReturn";
      `InstructionType_binary_sequential_isMulDiv : io_operation_instr_string = "isMulDiv    ";
      default : io_operation_instr_string = "????????????";
    endcase
  end
  `endif

  assign add = _zz_1;
  assign sub = _zz_2;
  assign equal = (io_opA == io_opB);
  assign unequal = (! equal);
  assign lt_u = (io_opA < io_opB);
  assign lt_s = ($signed(_zz_3) < $signed(_zz_4));
  assign ge_u = (io_opB <= io_opA);
  assign ge_s = ($signed(_zz_5) <= $signed(_zz_6));
  assign bitAnd = (io_opA & io_opB);
  assign bitOr = (io_opA | io_opB);
  assign bitXor = (io_opA ^ io_opB);
  assign shiftL = (io_opA <<< io_opB[4 : 0]);
  assign shiftR = (io_opA >>> io_opB[4 : 0]);
  assign shiftRA = _zz_7;
  assign shiftLI = (io_opA <<< io_operation_shamt);
  assign shiftRI = (io_opA >>> io_operation_shamt);
  assign shiftRAI = _zz_9;
  always @ (*) begin
    io_output = 32'h0;
    case(io_operation_instr)
      `InstructionType_binary_sequential_isCT_JAL, `InstructionType_binary_sequential_isCT_JALR, `InstructionType_binary_sequential_isStore, `InstructionType_binary_sequential_isLoad, `InstructionType_binary_sequential_isLUI : begin
        io_output = add;
      end
      `InstructionType_binary_sequential_isAUIPC : begin
        io_output = _zz_11;
      end
      `InstructionType_binary_sequential_isRegReg : begin
        if((((io_operation_f3 & 3'b111) == 3'b000)) || (((io_operation_f3 & 3'b111) == 3'b000))) begin
            io_output = (((io_operation_f7 & 7'h7f) == 7'h0) ? add : sub);
        end else if((((io_operation_f3 & 3'b111) == 3'b010))) begin
            io_output = ((lt_s == 1'b1) ? 32'h00000001 : 32'h0);
        end else if((((io_operation_f3 & 3'b111) == 3'b011))) begin
            io_output = ((lt_u == 1'b1) ? 32'h00000001 : 32'h0);
        end else if((((io_operation_f3 & 3'b111) == 3'b111))) begin
            io_output = bitAnd;
        end else if((((io_operation_f3 & 3'b111) == 3'b110))) begin
            io_output = bitOr;
        end else if((((io_operation_f3 & 3'b111) == 3'b100))) begin
            io_output = bitXor;
        end else if((((io_operation_f3 & 3'b111) == 3'b001))) begin
            io_output = shiftL;
        end else if((((io_operation_f3 & 3'b111) == 3'b101)) || (((io_operation_f3 & 3'b111) == 3'b101))) begin
            io_output = (((io_operation_f7 & 7'h7f) == 7'h0) ? shiftR : shiftRA);
        end
      end
      `InstructionType_binary_sequential_isRegImm : begin
        if((((io_operation_f3 & 3'b111) == 3'b000))) begin
            io_output = {add[31 : 1],1'b0};
        end else if((((io_operation_f3 & 3'b111) == 3'b010))) begin
            io_output = ((lt_s == 1'b1) ? 32'h00000001 : 32'h0);
        end else if((((io_operation_f3 & 3'b111) == 3'b011))) begin
            io_output = ((lt_u == 1'b1) ? 32'h00000001 : 32'h0);
        end else if((((io_operation_f3 & 3'b111) == 3'b111))) begin
            io_output = bitAnd;
        end else if((((io_operation_f3 & 3'b111) == 3'b110))) begin
            io_output = bitOr;
        end else if((((io_operation_f3 & 3'b111) == 3'b100))) begin
            io_output = bitXor;
        end else if((((io_operation_f3 & 3'b111) == 3'b001))) begin
            io_output = shiftLI;
        end else if((((io_operation_f3 & 3'b111) == 3'b101)) || (((io_operation_f3 & 3'b111) == 3'b101))) begin
            io_output = (((io_operation_f7 & 7'h7f) == 7'h0) ? shiftRI : shiftRAI);
        end
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_output_bool = 1'b0;
    case(io_operation_instr)
      `InstructionType_binary_sequential_isBranch : begin
        if((((io_operation_f3 & 3'b111) == 3'b000))) begin
            io_output_bool = equal;
        end else if((((io_operation_f3 & 3'b111) == 3'b001))) begin
            io_output_bool = unequal;
        end else if((((io_operation_f3 & 3'b111) == 3'b100))) begin
            io_output_bool = lt_s;
        end else if((((io_operation_f3 & 3'b111) == 3'b101))) begin
            io_output_bool = ge_s;
        end else if((((io_operation_f3 & 3'b111) == 3'b110))) begin
            io_output_bool = lt_u;
        end else if((((io_operation_f3 & 3'b111) == 3'b111))) begin
            io_output_bool = ge_u;
        end
      end
      default : begin
      end
    endcase
  end


endmodule

module RV32RegisterFile (
  input      [4:0]    io_rs1,
  input      [4:0]    io_rs2,
  output     [31:0]   io_rs1Data,
  output     [31:0]   io_rs2Data,
  input               io_wrEna,
  input      [4:0]    io_rd,
  input      [31:0]   io_rdData,
  input               clk,
  input               reset
);
  reg        [31:0]   _zz_2;
  reg        [31:0]   _zz_3;
  wire                _zz_4;
  wire                _zz_5;
  reg                 _zz_1;
  reg [31:0] regFile [0:31];

  assign _zz_4 = 1'b1;
  assign _zz_5 = 1'b1;
  always @ (posedge clk) begin
    if(_zz_4) begin
      _zz_2 <= regFile[io_rs1];
    end
  end

  always @ (posedge clk) begin
    if(_zz_5) begin
      _zz_3 <= regFile[io_rs2];
    end
  end

  always @ (posedge clk) begin
    if(_zz_1) begin
      regFile[io_rd] <= io_rdData;
    end
  end

  always @ (*) begin
    _zz_1 = 1'b0;
    if(((io_rd != 5'h0) && io_wrEna))begin
      _zz_1 = 1'b1;
    end
  end

  assign io_rs1Data = _zz_2;
  assign io_rs2Data = _zz_3;

endmodule

module DecodeUnit (
  input      [31:0]   io_instruction,
  output     [6:0]    io_fields_opcode,
  output     [4:0]    io_fields_src1,
  output     [4:0]    io_fields_src2,
  output     [4:0]    io_fields_dest,
  output     [2:0]    io_fields_funct3,
  output     [6:0]    io_fields_funct7,
  output     [11:0]   io_fields_funct12,
  output     [4:0]    io_fields_shamt,
  output     [11:0]   io_fields_csr,
  output     [31:0]   io_immediate,
  output     [4:0]    io_csr_uimm,
  output              io_decodeValid,
  output     `InstructionType_binary_sequential_type io_instType,
  output     `CSRAccessType_binary_sequential_type io_csrType
);
  wire       [31:0]   extender_io_i_imm;
  wire       [31:0]   extender_io_j_imm;
  wire       [31:0]   extender_io_s_imm;
  wire       [31:0]   extender_io_b_imm;
  wire       [31:0]   extender_io_u_imm;
  wire       [4:0]    extender_io_csr_imm;
  wire                _zz_1;
  wire                _zz_2;
  wire                _zz_3;
  wire                _zz_4;
  wire                _zz_5;
  wire                _zz_6;
  wire                _zz_7;
  wire                _zz_8;
  wire                _zz_9;
  wire                _zz_10;
  wire       [31:0]   instruction;
  wire       [6:0]    opcode;
  wire       [4:0]    source1;
  wire       [4:0]    source2;
  wire       [4:0]    destination;
  reg        [31:0]   immediate;
  wire       [2:0]    funct3;
  wire       [6:0]    funct7;
  wire       [11:0]   funct12;
  wire       [4:0]    shamt;
  wire       [11:0]   csr;
  wire       [4:0]    csr_uimm;
  reg                 decoded;
  reg        `InstructionType_binary_sequential_type iType;
  reg        `CSRAccessType_binary_sequential_type csr_accType;
  `ifndef SYNTHESIS
  reg [95:0] io_instType_string;
  reg [63:0] io_csrType_string;
  reg [95:0] iType_string;
  reg [63:0] csr_accType_string;
  `endif


  assign _zz_1 = (((funct7 & 7'h7f) == 7'h0) || (((funct7 & 7'h7f) == 7'h20) && (((funct3 & 3'b111) == 3'b000) || ((funct3 & 3'b111) == 3'b101))));
  assign _zz_2 = ((((funct3 != 3'b001) && (funct3 != 3'b101)) || ((funct3 == 3'b001) && ((funct7 & 7'h7f) == 7'h0))) || ((funct3 == 3'b101) && (((funct7 & 7'h7f) == 7'h0) || ((funct7 & 7'h7f) == 7'h20))));
  assign _zz_3 = (((((((funct3 & 3'b111) == 3'b000) || ((funct3 & 3'b111) == 3'b001)) || ((funct3 & 3'b111) == 3'b100)) || ((funct3 & 3'b111) == 3'b101)) || ((funct3 & 3'b111) == 3'b110)) || ((funct3 & 3'b111) == 3'b111));
  assign _zz_4 = ((((((funct3 & 3'b111) == 3'b000) || ((funct3 & 3'b111) == 3'b001)) || ((funct3 & 3'b111) == 3'b010)) || ((funct3 & 3'b111) == 3'b100)) || ((funct3 & 3'b111) == 3'b101));
  assign _zz_5 = ((((funct3 & 3'b111) == 3'b000) || ((funct3 & 3'b111) == 3'b001)) || ((funct3 & 3'b111) == 3'b010));
  assign _zz_6 = ((funct3 & 3'b111) == 3'b000);
  assign _zz_7 = (((funct3 & 3'b111) == 3'b000) || ((funct3 & 3'b111) == 3'b001));
  assign _zz_8 = (((((funct12 & 12'hfff) == 12'h0) && (source1 == 5'h0)) && (funct3 == 3'b000)) && (destination == 5'h0));
  assign _zz_9 = (((((funct12 & 12'hfff) == 12'h302) && (source1 == 5'h0)) && (funct3 == 3'b000)) && (destination == 5'h0));
  assign _zz_10 = (! ((funct3 & 3'b011) == 3'b000));
  ExtensionUnit extender (
    .io_instruction    (instruction[31:0]         ), //i
    .io_i_imm          (extender_io_i_imm[31:0]   ), //o
    .io_j_imm          (extender_io_j_imm[31:0]   ), //o
    .io_s_imm          (extender_io_s_imm[31:0]   ), //o
    .io_b_imm          (extender_io_b_imm[31:0]   ), //o
    .io_u_imm          (extender_io_u_imm[31:0]   ), //o
    .io_csr_imm        (extender_io_csr_imm[4:0]  )  //o
  );
  `ifndef SYNTHESIS
  always @(*) begin
    case(io_instType)
      `InstructionType_binary_sequential_isUndef : io_instType_string = "isUndef     ";
      `InstructionType_binary_sequential_isRegReg : io_instType_string = "isRegReg    ";
      `InstructionType_binary_sequential_isRegImm : io_instType_string = "isRegImm    ";
      `InstructionType_binary_sequential_isImm : io_instType_string = "isImm       ";
      `InstructionType_binary_sequential_isBranch : io_instType_string = "isBranch    ";
      `InstructionType_binary_sequential_isLoad : io_instType_string = "isLoad      ";
      `InstructionType_binary_sequential_isStore : io_instType_string = "isStore     ";
      `InstructionType_binary_sequential_isCT_JAL : io_instType_string = "isCT_JAL    ";
      `InstructionType_binary_sequential_isCT_JALR : io_instType_string = "isCT_JALR   ";
      `InstructionType_binary_sequential_isLUI : io_instType_string = "isLUI       ";
      `InstructionType_binary_sequential_isAUIPC : io_instType_string = "isAUIPC     ";
      `InstructionType_binary_sequential_isECall : io_instType_string = "isECall     ";
      `InstructionType_binary_sequential_isFence : io_instType_string = "isFence     ";
      `InstructionType_binary_sequential_isIllegal : io_instType_string = "isIllegal   ";
      `InstructionType_binary_sequential_isCSR : io_instType_string = "isCSR       ";
      `InstructionType_binary_sequential_isCSRImm : io_instType_string = "isCSRImm    ";
      `InstructionType_binary_sequential_isTrapReturn : io_instType_string = "isTrapReturn";
      `InstructionType_binary_sequential_isMulDiv : io_instType_string = "isMulDiv    ";
      default : io_instType_string = "????????????";
    endcase
  end
  always @(*) begin
    case(io_csrType)
      `CSRAccessType_binary_sequential_CSRidle : io_csrType_string = "CSRidle ";
      `CSRAccessType_binary_sequential_CSRread : io_csrType_string = "CSRread ";
      `CSRAccessType_binary_sequential_CSRwrite : io_csrType_string = "CSRwrite";
      `CSRAccessType_binary_sequential_CSRset : io_csrType_string = "CSRset  ";
      `CSRAccessType_binary_sequential_CSRclear : io_csrType_string = "CSRclear";
      default : io_csrType_string = "????????";
    endcase
  end
  always @(*) begin
    case(iType)
      `InstructionType_binary_sequential_isUndef : iType_string = "isUndef     ";
      `InstructionType_binary_sequential_isRegReg : iType_string = "isRegReg    ";
      `InstructionType_binary_sequential_isRegImm : iType_string = "isRegImm    ";
      `InstructionType_binary_sequential_isImm : iType_string = "isImm       ";
      `InstructionType_binary_sequential_isBranch : iType_string = "isBranch    ";
      `InstructionType_binary_sequential_isLoad : iType_string = "isLoad      ";
      `InstructionType_binary_sequential_isStore : iType_string = "isStore     ";
      `InstructionType_binary_sequential_isCT_JAL : iType_string = "isCT_JAL    ";
      `InstructionType_binary_sequential_isCT_JALR : iType_string = "isCT_JALR   ";
      `InstructionType_binary_sequential_isLUI : iType_string = "isLUI       ";
      `InstructionType_binary_sequential_isAUIPC : iType_string = "isAUIPC     ";
      `InstructionType_binary_sequential_isECall : iType_string = "isECall     ";
      `InstructionType_binary_sequential_isFence : iType_string = "isFence     ";
      `InstructionType_binary_sequential_isIllegal : iType_string = "isIllegal   ";
      `InstructionType_binary_sequential_isCSR : iType_string = "isCSR       ";
      `InstructionType_binary_sequential_isCSRImm : iType_string = "isCSRImm    ";
      `InstructionType_binary_sequential_isTrapReturn : iType_string = "isTrapReturn";
      `InstructionType_binary_sequential_isMulDiv : iType_string = "isMulDiv    ";
      default : iType_string = "????????????";
    endcase
  end
  always @(*) begin
    case(csr_accType)
      `CSRAccessType_binary_sequential_CSRidle : csr_accType_string = "CSRidle ";
      `CSRAccessType_binary_sequential_CSRread : csr_accType_string = "CSRread ";
      `CSRAccessType_binary_sequential_CSRwrite : csr_accType_string = "CSRwrite";
      `CSRAccessType_binary_sequential_CSRset : csr_accType_string = "CSRset  ";
      `CSRAccessType_binary_sequential_CSRclear : csr_accType_string = "CSRclear";
      default : csr_accType_string = "????????";
    endcase
  end
  `endif

  always @ (*) begin
    iType = `InstructionType_binary_sequential_isUndef;
    if((((opcode & 7'h7f) == 7'h33))) begin
        if(_zz_1)begin
          iType = `InstructionType_binary_sequential_isRegReg;
        end
    end else if((((opcode & 7'h7f) == 7'h13))) begin
        if(_zz_2)begin
          iType = `InstructionType_binary_sequential_isRegImm;
        end
    end else if((((opcode & 7'h7f) == 7'h63))) begin
        if(_zz_3)begin
          iType = `InstructionType_binary_sequential_isBranch;
        end
    end else if((((opcode & 7'h7f) == 7'h03))) begin
        if(_zz_4)begin
          iType = `InstructionType_binary_sequential_isLoad;
        end
    end else if((((opcode & 7'h7f) == 7'h23))) begin
        if(_zz_5)begin
          iType = `InstructionType_binary_sequential_isStore;
        end
    end else if((((opcode & 7'h7f) == 7'h37))) begin
        iType = `InstructionType_binary_sequential_isLUI;
    end else if((((opcode & 7'h7f) == 7'h17))) begin
        iType = `InstructionType_binary_sequential_isAUIPC;
    end else if((((opcode & 7'h7f) == 7'h6f))) begin
        iType = `InstructionType_binary_sequential_isCT_JAL;
    end else if((((opcode & 7'h7f) == 7'h67))) begin
        if(_zz_6)begin
          iType = `InstructionType_binary_sequential_isCT_JALR;
        end
    end else if((((opcode & 7'h7f) == 7'h0f))) begin
        if(_zz_7)begin
          iType = `InstructionType_binary_sequential_isFence;
        end
    end else if((((opcode & 7'h7f) == 7'h73)) || (((opcode & 7'h7f) == 7'h73))) begin
        if(_zz_8)begin
          iType = `InstructionType_binary_sequential_isECall;
        end else begin
          if(_zz_9)begin
            iType = `InstructionType_binary_sequential_isTrapReturn;
          end else begin
            if(_zz_10)begin
              iType = `InstructionType_binary_sequential_isCSR;
              if(funct3[2])begin
                iType = `InstructionType_binary_sequential_isCSRImm;
              end
            end
          end
        end
    end else if((((opcode & 7'h7f) == 7'h0))) begin
        iType = `InstructionType_binary_sequential_isIllegal;
    end else begin
        iType = `InstructionType_binary_sequential_isUndef;
    end
  end

  always @ (*) begin
    csr_accType = `CSRAccessType_binary_sequential_CSRidle;
    if((((opcode & 7'h7f) == 7'h33))) begin
    end else if((((opcode & 7'h7f) == 7'h13))) begin
    end else if((((opcode & 7'h7f) == 7'h63))) begin
    end else if((((opcode & 7'h7f) == 7'h03))) begin
    end else if((((opcode & 7'h7f) == 7'h23))) begin
    end else if((((opcode & 7'h7f) == 7'h37))) begin
    end else if((((opcode & 7'h7f) == 7'h17))) begin
    end else if((((opcode & 7'h7f) == 7'h6f))) begin
    end else if((((opcode & 7'h7f) == 7'h67))) begin
    end else if((((opcode & 7'h7f) == 7'h0f))) begin
    end else if((((opcode & 7'h7f) == 7'h73)) || (((opcode & 7'h7f) == 7'h73))) begin
        if(! _zz_8) begin
          if(! _zz_9) begin
            if(_zz_10)begin
              if((((funct3 & 3'b111) == 3'b001)) || (((funct3 & 3'b111) == 3'b101))) begin
                  csr_accType = `CSRAccessType_binary_sequential_CSRwrite;
              end else if((((funct3 & 3'b111) == 3'b010)) || (((funct3 & 3'b111) == 3'b110))) begin
                  csr_accType = `CSRAccessType_binary_sequential_CSRset;
              end else if((((funct3 & 3'b111) == 3'b011)) || (((funct3 & 3'b111) == 3'b111))) begin
                  csr_accType = `CSRAccessType_binary_sequential_CSRclear;
              end
            end
          end
        end
    end else if((((opcode & 7'h7f) == 7'h0))) begin
    end else begin
    end
  end

  always @ (*) begin
    decoded = 1'b0;
    if((((opcode & 7'h7f) == 7'h33))) begin
        if(_zz_1)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h13))) begin
        if(_zz_2)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h63))) begin
        if(_zz_3)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h03))) begin
        if(_zz_4)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h23))) begin
        if(_zz_5)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h37))) begin
        decoded = 1'b1;
    end else if((((opcode & 7'h7f) == 7'h17))) begin
        decoded = 1'b1;
    end else if((((opcode & 7'h7f) == 7'h6f))) begin
        decoded = 1'b1;
    end else if((((opcode & 7'h7f) == 7'h67))) begin
        if(_zz_6)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h0f))) begin
        if(_zz_7)begin
          decoded = 1'b1;
        end
    end else if((((opcode & 7'h7f) == 7'h73)) || (((opcode & 7'h7f) == 7'h73))) begin
        if(_zz_8)begin
          decoded = 1'b1;
        end else begin
          if(_zz_9)begin
            decoded = 1'b1;
          end else begin
            if(_zz_10)begin
              decoded = 1'b1;
            end
          end
        end
    end else if((((opcode & 7'h7f) == 7'h0))) begin
        decoded = 1'b0;
    end else begin
        decoded = 1'b0;
    end
  end

  assign instruction = io_instruction;
  assign opcode = instruction[6 : 0];
  assign source1 = instruction[19 : 15];
  assign source2 = instruction[24 : 20];
  assign destination = instruction[11 : 7];
  always @ (*) begin
    immediate = 32'h0;
    if((((opcode & 7'h7f) == 7'h33))) begin
    end else if((((opcode & 7'h7f) == 7'h13))) begin
        if(_zz_2)begin
          immediate = extender_io_i_imm;
        end
    end else if((((opcode & 7'h7f) == 7'h63))) begin
        if(_zz_3)begin
          immediate = extender_io_b_imm;
        end
    end else if((((opcode & 7'h7f) == 7'h03))) begin
        if(_zz_4)begin
          immediate = extender_io_i_imm;
        end
    end else if((((opcode & 7'h7f) == 7'h23))) begin
        if(_zz_5)begin
          immediate = extender_io_s_imm;
        end
    end else if((((opcode & 7'h7f) == 7'h37))) begin
        immediate = extender_io_u_imm;
    end else if((((opcode & 7'h7f) == 7'h17))) begin
        immediate = extender_io_u_imm;
    end else if((((opcode & 7'h7f) == 7'h6f))) begin
        immediate = extender_io_j_imm;
    end else if((((opcode & 7'h7f) == 7'h67))) begin
        if(_zz_6)begin
          immediate = extender_io_i_imm;
        end
    end else if((((opcode & 7'h7f) == 7'h0f))) begin
    end else if((((opcode & 7'h7f) == 7'h73)) || (((opcode & 7'h7f) == 7'h73))) begin
    end else if((((opcode & 7'h7f) == 7'h0))) begin
    end else begin
    end
  end

  assign funct3 = instruction[14 : 12];
  assign funct7 = instruction[31 : 25];
  assign funct12 = instruction[31 : 20];
  assign shamt = instruction[24 : 20];
  assign csr = instruction[31 : 20];
  assign csr_uimm = instruction[19 : 15];
  assign io_fields_opcode = opcode;
  assign io_fields_src1 = source1;
  assign io_fields_src2 = source2;
  assign io_fields_dest = destination;
  assign io_fields_funct3 = funct3;
  assign io_fields_funct7 = funct7;
  assign io_fields_funct12 = funct12;
  assign io_fields_shamt = shamt;
  assign io_fields_csr = csr;
  assign io_instType = iType;
  assign io_immediate = immediate;
  assign io_csr_uimm = csr_uimm;
  assign io_decodeValid = decoded;
  assign io_csrType = csr_accType;

endmodule

module FetchUnit (
  input      [31:0]   io_data,
  input               io_sample,
  output     [31:0]   io_instruction,
  input               clk,
  input               reset
);
  reg        [31:0]   instructionBuffer;

  assign io_instruction = instructionBuffer;
  always @ (posedge clk or posedge reset) begin
    if (reset) begin
      instructionBuffer <= 32'h0;
    end else begin
      if(io_sample)begin
        instructionBuffer <= io_data;
      end
    end
  end


endmodule

module ControlUnit (
  input               io_validDecode,
  input      `InstructionType_binary_sequential_type io_instrType,
  input      [6:0]    io_instrFields_opcode,
  input      [4:0]    io_instrFields_src1,
  input      [4:0]    io_instrFields_src2,
  input      [4:0]    io_instrFields_dest,
  input      [2:0]    io_instrFields_funct3,
  input      [6:0]    io_instrFields_funct7,
  input      [11:0]   io_instrFields_funct12,
  input      [4:0]    io_instrFields_shamt,
  input      [11:0]   io_instrFields_csr,
  output reg          io_pcCtrl_enablePC,
  output reg `PCSelect_binary_sequential_type io_pcCtrl_pcValSel,
  output reg          io_fetchCtrl_sample,
  output reg `OpASelect_binary_sequential_type io_aluCtrl_opA,
  output reg `OpBSelect_binary_sequential_type io_aluCtrl_opB,
  input               io_aluCtrl_aluBranch,
  output reg          io_regCtrl_regFileWR,
  output reg `DestDataSelect_binary_sequential_type io_regCtrl_regDestSel,
  output reg `CSRDataSelect_binary_sequential_type io_csrCtrl_writeSelect,
  output reg          io_csrCtrl_enable,
  output reg          io_csrCtrl_newFetch,
  input               io_csrCtrl_illegalAccess,
  output reg `MCauseSelect_binary_sequential_type io_csrCtrl_mcauseSelect,
  output reg          io_memCtrl_fetchEna,
  input               io_memCtrl_instrRdy,
  output reg          io_memCtrl_readWriteData,
  output reg          io_memCtrl_dataEna,
  input               io_memCtrl_dataRdy,
  output reg `MemoryStrobeSelect_binary_sequential_type io_memCtrl_strobeSelect,
  input               io_irqPending,
  output reg          io_trapEntry,
  output reg          io_trapExit,
  output reg          io_irqEntry,
  input               io_exceptions_misalignedJumpTarget,
  input               io_exceptions_misalignedJumpLinkTarget,
  input               io_exceptions_misalignedBranchTarget,
  input               io_halt,
  output reg          io_halted,
  output reg          io_fetchSync,
  output reg [3:0]    io_dbgState,
  input               clk,
  input               reset
);
  wire                _zz_1;
  wire                _zz_2;
  wire                _zz_3;
  wire                _zz_4;
  wire                _zz_5;
  wire                fsm_wantExit;
  reg                 fsm_wantStart;
  reg        `fsm_enumDefinition_binary_sequential_type fsm_stateReg;
  reg        `fsm_enumDefinition_binary_sequential_type fsm_stateNext;
  `ifndef SYNTHESIS
  reg [95:0] io_instrType_string;
  reg [119:0] io_pcCtrl_pcValSel_string;
  reg [79:0] io_aluCtrl_opA_string;
  reg [87:0] io_aluCtrl_opB_string;
  reg [87:0] io_regCtrl_regDestSel_string;
  reg [79:0] io_csrCtrl_writeSelect_string;
  reg [167:0] io_csrCtrl_mcauseSelect_string;
  reg [63:0] io_memCtrl_strobeSelect_string;
  reg [143:0] fsm_stateReg_string;
  reg [143:0] fsm_stateNext_string;
  `endif


  assign _zz_1 = (((((io_instrFields_funct12 & 12'hfff) == 12'h0) && (io_instrFields_src1 == 5'h0)) && (io_instrFields_funct3 == 3'b000)) && (io_instrFields_dest == 5'h0));
  assign _zz_2 = (((((io_instrFields_funct12 & 12'hfff) == 12'h302) && (io_instrFields_src1 == 5'h0)) && (io_instrFields_funct3 == 3'b000)) && (io_instrFields_dest == 5'h0));
  assign _zz_3 = (((io_instrFields_funct7 & 7'h7f) == 7'h0) || ((io_instrFields_funct7 & 7'h7f) == 7'h20));
  assign _zz_4 = (io_memCtrl_dataRdy && (! io_halt));
  assign _zz_5 = (! ((io_instrFields_funct3 & 3'b011) == 3'b000));
  `ifndef SYNTHESIS
  always @(*) begin
    case(io_instrType)
      `InstructionType_binary_sequential_isUndef : io_instrType_string = "isUndef     ";
      `InstructionType_binary_sequential_isRegReg : io_instrType_string = "isRegReg    ";
      `InstructionType_binary_sequential_isRegImm : io_instrType_string = "isRegImm    ";
      `InstructionType_binary_sequential_isImm : io_instrType_string = "isImm       ";
      `InstructionType_binary_sequential_isBranch : io_instrType_string = "isBranch    ";
      `InstructionType_binary_sequential_isLoad : io_instrType_string = "isLoad      ";
      `InstructionType_binary_sequential_isStore : io_instrType_string = "isStore     ";
      `InstructionType_binary_sequential_isCT_JAL : io_instrType_string = "isCT_JAL    ";
      `InstructionType_binary_sequential_isCT_JALR : io_instrType_string = "isCT_JALR   ";
      `InstructionType_binary_sequential_isLUI : io_instrType_string = "isLUI       ";
      `InstructionType_binary_sequential_isAUIPC : io_instrType_string = "isAUIPC     ";
      `InstructionType_binary_sequential_isECall : io_instrType_string = "isECall     ";
      `InstructionType_binary_sequential_isFence : io_instrType_string = "isFence     ";
      `InstructionType_binary_sequential_isIllegal : io_instrType_string = "isIllegal   ";
      `InstructionType_binary_sequential_isCSR : io_instrType_string = "isCSR       ";
      `InstructionType_binary_sequential_isCSRImm : io_instrType_string = "isCSRImm    ";
      `InstructionType_binary_sequential_isTrapReturn : io_instrType_string = "isTrapReturn";
      `InstructionType_binary_sequential_isMulDiv : io_instrType_string = "isMulDiv    ";
      default : io_instrType_string = "????????????";
    endcase
  end
  always @(*) begin
    case(io_pcCtrl_pcValSel)
      `PCSelect_binary_sequential_incrementPC : io_pcCtrl_pcValSel_string = "incrementPC    ";
      `PCSelect_binary_sequential_jalTarget : io_pcCtrl_pcValSel_string = "jalTarget      ";
      `PCSelect_binary_sequential_jalrTarget : io_pcCtrl_pcValSel_string = "jalrTarget     ";
      `PCSelect_binary_sequential_branchTarget : io_pcCtrl_pcValSel_string = "branchTarget   ";
      `PCSelect_binary_sequential_trapEntryTarget : io_pcCtrl_pcValSel_string = "trapEntryTarget";
      `PCSelect_binary_sequential_trapExitTarget : io_pcCtrl_pcValSel_string = "trapExitTarget ";
      default : io_pcCtrl_pcValSel_string = "???????????????";
    endcase
  end
  always @(*) begin
    case(io_aluCtrl_opA)
      `OpASelect_binary_sequential_opReg1Data : io_aluCtrl_opA_string = "opReg1Data";
      `OpASelect_binary_sequential_opPC : io_aluCtrl_opA_string = "opPC      ";
      `OpASelect_binary_sequential_opZero : io_aluCtrl_opA_string = "opZero    ";
      default : io_aluCtrl_opA_string = "??????????";
    endcase
  end
  always @(*) begin
    case(io_aluCtrl_opB)
      `OpBSelect_binary_sequential_opReg2Data : io_aluCtrl_opB_string = "opReg2Data ";
      `OpBSelect_binary_sequential_opImmediate : io_aluCtrl_opB_string = "opImmediate";
      `OpBSelect_binary_sequential_opZero : io_aluCtrl_opB_string = "opZero     ";
      default : io_aluCtrl_opB_string = "???????????";
    endcase
  end
  always @(*) begin
    case(io_regCtrl_regDestSel)
      `DestDataSelect_binary_sequential_aluRes : io_regCtrl_regDestSel_string = "aluRes     ";
      `DestDataSelect_binary_sequential_aluBool : io_regCtrl_regDestSel_string = "aluBool    ";
      `DestDataSelect_binary_sequential_memReadData : io_regCtrl_regDestSel_string = "memReadData";
      `DestDataSelect_binary_sequential_csrReadData : io_regCtrl_regDestSel_string = "csrReadData";
      `DestDataSelect_binary_sequential_muldivData : io_regCtrl_regDestSel_string = "muldivData ";
      default : io_regCtrl_regDestSel_string = "???????????";
    endcase
  end
  always @(*) begin
    case(io_csrCtrl_writeSelect)
      `CSRDataSelect_binary_sequential_reg1Data : io_csrCtrl_writeSelect_string = "reg1Data  ";
      `CSRDataSelect_binary_sequential_csrImmData : io_csrCtrl_writeSelect_string = "csrImmData";
      default : io_csrCtrl_writeSelect_string = "??????????";
    endcase
  end
  always @(*) begin
    case(io_csrCtrl_mcauseSelect)
      `MCauseSelect_binary_sequential_trapInstrAddrMisalign : io_csrCtrl_mcauseSelect_string = "trapInstrAddrMisalign";
      `MCauseSelect_binary_sequential_trapIllegalInstr : io_csrCtrl_mcauseSelect_string = "trapIllegalInstr     ";
      `MCauseSelect_binary_sequential_trapECallMachine : io_csrCtrl_mcauseSelect_string = "trapECallMachine     ";
      `MCauseSelect_binary_sequential_trapMachineTimerIRQ : io_csrCtrl_mcauseSelect_string = "trapMachineTimerIRQ  ";
      default : io_csrCtrl_mcauseSelect_string = "?????????????????????";
    endcase
  end
  always @(*) begin
    case(io_memCtrl_strobeSelect)
      `MemoryStrobeSelect_binary_sequential_byte_1 : io_memCtrl_strobeSelect_string = "byte_1  ";
      `MemoryStrobeSelect_binary_sequential_halfWord : io_memCtrl_strobeSelect_string = "halfWord";
      `MemoryStrobeSelect_binary_sequential_word : io_memCtrl_strobeSelect_string = "word    ";
      default : io_memCtrl_strobeSelect_string = "????????";
    endcase
  end
  always @(*) begin
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_BOOT : fsm_stateReg_string = "fsm_BOOT          ";
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : fsm_stateReg_string = "fsm_stateInit     ";
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : fsm_stateReg_string = "fsm_stateFetch    ";
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : fsm_stateReg_string = "fsm_stateDecode   ";
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : fsm_stateReg_string = "fsm_stateExecute  ";
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : fsm_stateReg_string = "fsm_stateWriteBack";
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : fsm_stateReg_string = "fsm_stateTrap     ";
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : fsm_stateReg_string = "fsm_stateCSR      ";
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : fsm_stateReg_string = "fsm_stateInterrupt";
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : fsm_stateReg_string = "fsm_stateHalt     ";
      default : fsm_stateReg_string = "??????????????????";
    endcase
  end
  always @(*) begin
    case(fsm_stateNext)
      `fsm_enumDefinition_binary_sequential_fsm_BOOT : fsm_stateNext_string = "fsm_BOOT          ";
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : fsm_stateNext_string = "fsm_stateInit     ";
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : fsm_stateNext_string = "fsm_stateFetch    ";
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : fsm_stateNext_string = "fsm_stateDecode   ";
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : fsm_stateNext_string = "fsm_stateExecute  ";
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : fsm_stateNext_string = "fsm_stateWriteBack";
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : fsm_stateNext_string = "fsm_stateTrap     ";
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : fsm_stateNext_string = "fsm_stateCSR      ";
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : fsm_stateNext_string = "fsm_stateInterrupt";
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : fsm_stateNext_string = "fsm_stateHalt     ";
      default : fsm_stateNext_string = "??????????????????";
    endcase
  end
  `endif

  always @ (*) begin
    io_pcCtrl_enablePC = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(io_memCtrl_instrRdy)begin
            io_pcCtrl_enablePC = 1'b1;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            if(! io_exceptions_misalignedJumpTarget) begin
              io_pcCtrl_enablePC = 1'b1;
            end
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            if(! io_exceptions_misalignedJumpLinkTarget) begin
              io_pcCtrl_enablePC = 1'b1;
            end
          end
          `InstructionType_binary_sequential_isBranch : begin
            if(io_aluCtrl_aluBranch)begin
              if(! io_exceptions_misalignedBranchTarget) begin
                io_pcCtrl_enablePC = 1'b1;
              end
            end
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(_zz_1)begin
              io_pcCtrl_enablePC = 1'b1;
            end else begin
              if(_zz_2)begin
                io_pcCtrl_enablePC = 1'b1;
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
        io_pcCtrl_enablePC = 1'b1;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
        io_pcCtrl_enablePC = 1'b1;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_pcCtrl_pcValSel = `PCSelect_binary_sequential_incrementPC;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(io_memCtrl_instrRdy)begin
            io_pcCtrl_pcValSel = `PCSelect_binary_sequential_incrementPC;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            io_pcCtrl_pcValSel = `PCSelect_binary_sequential_jalTarget;
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            io_pcCtrl_pcValSel = `PCSelect_binary_sequential_jalrTarget;
          end
          `InstructionType_binary_sequential_isBranch : begin
            if(io_aluCtrl_aluBranch)begin
              io_pcCtrl_pcValSel = `PCSelect_binary_sequential_branchTarget;
            end
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(_zz_1)begin
              io_pcCtrl_pcValSel = `PCSelect_binary_sequential_trapEntryTarget;
            end else begin
              if(_zz_2)begin
                io_pcCtrl_pcValSel = `PCSelect_binary_sequential_trapExitTarget;
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
        io_pcCtrl_pcValSel = `PCSelect_binary_sequential_trapEntryTarget;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
        io_pcCtrl_pcValSel = `PCSelect_binary_sequential_trapEntryTarget;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_fetchCtrl_sample = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(io_memCtrl_instrRdy)begin
            io_fetchCtrl_sample = 1'b1;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
          end
          `InstructionType_binary_sequential_isRegImm : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
          end
          `InstructionType_binary_sequential_isAUIPC : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opPC;
          end
          `InstructionType_binary_sequential_isLUI : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opZero;
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opPC;
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opPC;
          end
          `InstructionType_binary_sequential_isBranch : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
          end
          `InstructionType_binary_sequential_isLoad : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
          end
          `InstructionType_binary_sequential_isStore : begin
            io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        io_aluCtrl_opA = `OpASelect_binary_sequential_opReg1Data;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_aluCtrl_opB = `OpBSelect_binary_sequential_opReg2Data;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opReg2Data;
          end
          `InstructionType_binary_sequential_isRegImm : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
          end
          `InstructionType_binary_sequential_isAUIPC : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
          end
          `InstructionType_binary_sequential_isLUI : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opZero;
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opZero;
          end
          `InstructionType_binary_sequential_isBranch : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opReg2Data;
          end
          `InstructionType_binary_sequential_isLoad : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
          end
          `InstructionType_binary_sequential_isStore : begin
            io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        io_aluCtrl_opB = `OpBSelect_binary_sequential_opImmediate;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_regCtrl_regFileWR = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
            io_regCtrl_regFileWR = 1'b1;
            if(! _zz_3) begin
              io_regCtrl_regFileWR = 1'b0;
            end
          end
          `InstructionType_binary_sequential_isRegImm : begin
            io_regCtrl_regFileWR = 1'b1;
          end
          `InstructionType_binary_sequential_isAUIPC : begin
            io_regCtrl_regFileWR = 1'b1;
          end
          `InstructionType_binary_sequential_isLUI : begin
            io_regCtrl_regFileWR = 1'b1;
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            if(! io_exceptions_misalignedJumpTarget) begin
              io_regCtrl_regFileWR = 1'b1;
            end
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            if(! io_exceptions_misalignedJumpLinkTarget) begin
              io_regCtrl_regFileWR = 1'b1;
            end
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        if(_zz_4)begin
          case(io_instrType)
            `InstructionType_binary_sequential_isLoad : begin
              io_regCtrl_regFileWR = 1'b1;
            end
            default : begin
            end
          endcase
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
        io_regCtrl_regFileWR = 1'b1;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isRegImm : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isAUIPC : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isLUI : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_aluRes;
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(! _zz_1) begin
              if(! _zz_2) begin
                if(_zz_5)begin
                  io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_csrReadData;
                end
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        if(_zz_4)begin
          case(io_instrType)
            `InstructionType_binary_sequential_isLoad : begin
              io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_memReadData;
            end
            default : begin
            end
          endcase
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
        io_regCtrl_regDestSel = `DestDataSelect_binary_sequential_csrReadData;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_csrCtrl_enable = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(! _zz_1) begin
              if(! _zz_2) begin
                if(_zz_5)begin
                  io_csrCtrl_enable = 1'b1;
                end
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_csrCtrl_writeSelect = `CSRDataSelect_binary_sequential_reg1Data;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(! _zz_1) begin
              if(! _zz_2) begin
                if(_zz_5)begin
                  case(io_instrType)
                    `InstructionType_binary_sequential_isCSR : begin
                      io_csrCtrl_writeSelect = `CSRDataSelect_binary_sequential_reg1Data;
                    end
                    `InstructionType_binary_sequential_isCSRImm : begin
                      io_csrCtrl_writeSelect = `CSRDataSelect_binary_sequential_csrImmData;
                    end
                    default : begin
                    end
                  endcase
                end
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_csrCtrl_newFetch = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(io_memCtrl_instrRdy)begin
            io_csrCtrl_newFetch = 1'b1;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapECallMachine;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isCT_JAL, `InstructionType_binary_sequential_isCT_JALR, `InstructionType_binary_sequential_isBranch : begin
            io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapInstrAddrMisalign;
          end
          `InstructionType_binary_sequential_isRegReg : begin
            if((! (((io_instrFields_funct7 & 7'h7f) == 7'h0) || ((io_instrFields_funct7 & 7'h7f) == 7'h20))))begin
              io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapIllegalInstr;
            end
          end
          `InstructionType_binary_sequential_isIllegal, `InstructionType_binary_sequential_isUndef : begin
            io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapIllegalInstr;
          end
          `InstructionType_binary_sequential_isECall : begin
            io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapECallMachine;
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
        io_csrCtrl_mcauseSelect = `MCauseSelect_binary_sequential_trapMachineTimerIRQ;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_memCtrl_fetchEna = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(! io_memCtrl_instrRdy) begin
            io_memCtrl_fetchEna = 1'b1;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
    if(((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateFetch) && (! (fsm_stateNext == `fsm_enumDefinition_binary_sequential_fsm_stateFetch))))begin
      io_memCtrl_fetchEna = 1'b0;
    end
  end

  always @ (*) begin
    io_memCtrl_readWriteData = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
            io_memCtrl_readWriteData = 1'b0;
          end
          `InstructionType_binary_sequential_isStore : begin
            io_memCtrl_readWriteData = 1'b1;
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isStore : begin
            io_memCtrl_readWriteData = 1'b1;
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_memCtrl_dataEna = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
            io_memCtrl_dataEna = 1'b1;
          end
          `InstructionType_binary_sequential_isStore : begin
            io_memCtrl_dataEna = 1'b1;
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isLoad : begin
            io_memCtrl_dataEna = 1'b1;
          end
          `InstructionType_binary_sequential_isStore : begin
            io_memCtrl_dataEna = 1'b1;
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
            if((((io_instrFields_funct3 & 3'b111) == 3'b000)) || (((io_instrFields_funct3 & 3'b111) == 3'b100))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_byte_1;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b001)) || (((io_instrFields_funct3 & 3'b111) == 3'b101))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_halfWord;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b010))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end else begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end
          end
          `InstructionType_binary_sequential_isStore : begin
            if((((io_instrFields_funct3 & 3'b111) == 3'b000))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_byte_1;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b001))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_halfWord;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b010))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end else begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isLoad : begin
            if((((io_instrFields_funct3 & 3'b111) == 3'b000)) || (((io_instrFields_funct3 & 3'b111) == 3'b100))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_byte_1;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b001)) || (((io_instrFields_funct3 & 3'b111) == 3'b101))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_halfWord;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b010))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end else begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end
          end
          `InstructionType_binary_sequential_isStore : begin
            if((((io_instrFields_funct3 & 3'b111) == 3'b000))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_byte_1;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b001))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_halfWord;
            end else if((((io_instrFields_funct3 & 3'b111) == 3'b010))) begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end else begin
                io_memCtrl_strobeSelect = `MemoryStrobeSelect_binary_sequential_word;
            end
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_trapEntry = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(_zz_1)begin
              io_trapEntry = 1'b1;
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
        io_trapEntry = 1'b1;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_trapExit = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
          end
          `InstructionType_binary_sequential_isRegImm : begin
          end
          `InstructionType_binary_sequential_isAUIPC : begin
          end
          `InstructionType_binary_sequential_isLUI : begin
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
          end
          `InstructionType_binary_sequential_isBranch : begin
          end
          `InstructionType_binary_sequential_isLoad : begin
          end
          `InstructionType_binary_sequential_isStore : begin
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(! _zz_1) begin
              if(_zz_2)begin
                io_trapExit = 1'b1;
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
          end
          `InstructionType_binary_sequential_isIllegal : begin
          end
          default : begin
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_irqEntry = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
        io_irqEntry = 1'b1;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_halted = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
        io_halted = 1'b1;
      end
      default : begin
      end
    endcase
  end

  always @ (*) begin
    io_fetchSync = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(! io_irqPending) begin
          if(! io_memCtrl_instrRdy) begin
            io_fetchSync = 1'b1;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
  end

  assign fsm_wantExit = 1'b0;
  always @ (*) begin
    fsm_wantStart = 1'b0;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
        fsm_wantStart = 1'b1;
      end
    endcase
  end

  always @ (*) begin
    io_dbgState = 4'b0000;
    if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateInit))begin
      io_dbgState = 4'b0000;
    end else begin
      if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateFetch))begin
        io_dbgState = 4'b0001;
      end else begin
        if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateDecode))begin
          io_dbgState = 4'b0010;
        end else begin
          if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateExecute))begin
            io_dbgState = 4'b0011;
          end else begin
            if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack))begin
              io_dbgState = 4'b0100;
            end else begin
              if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateCSR))begin
                io_dbgState = 4'b0101;
              end else begin
                if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateTrap))begin
                  io_dbgState = 4'b0110;
                end else begin
                  if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateHalt))begin
                    io_dbgState = 4'b0111;
                  end else begin
                    if((fsm_stateReg == `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt))begin
                      io_dbgState = 4'b1001;
                    end
                  end
                end
              end
            end
          end
        end
      end
    end
  end

  always @ (*) begin
    fsm_stateNext = fsm_stateReg;
    case(fsm_stateReg)
      `fsm_enumDefinition_binary_sequential_fsm_stateInit : begin
        fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateFetch : begin
        if(io_irqPending)begin
          fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt;
        end else begin
          if(io_memCtrl_instrRdy)begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateDecode;
          end
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateDecode : begin
        if(io_validDecode)begin
          fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateExecute;
        end else begin
          fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateExecute : begin
        case(io_instrType)
          `InstructionType_binary_sequential_isRegReg : begin
            if(_zz_3)begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
            end else begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
            end
          end
          `InstructionType_binary_sequential_isRegImm : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
          end
          `InstructionType_binary_sequential_isAUIPC : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
          end
          `InstructionType_binary_sequential_isLUI : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
          end
          `InstructionType_binary_sequential_isCT_JAL : begin
            if(io_exceptions_misalignedJumpTarget)begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
            end else begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
            end
          end
          `InstructionType_binary_sequential_isCT_JALR : begin
            if(io_exceptions_misalignedJumpLinkTarget)begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
            end else begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
            end
          end
          `InstructionType_binary_sequential_isBranch : begin
            if(io_aluCtrl_aluBranch)begin
              if(io_exceptions_misalignedBranchTarget)begin
                fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
              end else begin
                fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
              end
            end else begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
            end
          end
          `InstructionType_binary_sequential_isLoad : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack;
          end
          `InstructionType_binary_sequential_isStore : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack;
          end
          `InstructionType_binary_sequential_isECall, `InstructionType_binary_sequential_isCSR, `InstructionType_binary_sequential_isCSRImm, `InstructionType_binary_sequential_isTrapReturn : begin
            if(_zz_1)begin
              fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
            end else begin
              if(_zz_2)begin
                fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
              end else begin
                if(_zz_5)begin
                  fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateCSR;
                end
              end
            end
          end
          `InstructionType_binary_sequential_isFence : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
          end
          `InstructionType_binary_sequential_isIllegal : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateTrap;
          end
          default : begin
            fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateHalt;
          end
        endcase
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateWriteBack : begin
        if(_zz_4)begin
          fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
        end
        if(io_halt)begin
          fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateHalt;
        end
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateTrap : begin
        fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateCSR : begin
        fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateInterrupt : begin
        fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateFetch;
      end
      `fsm_enumDefinition_binary_sequential_fsm_stateHalt : begin
      end
      default : begin
      end
    endcase
    if(fsm_wantStart)begin
      fsm_stateNext = `fsm_enumDefinition_binary_sequential_fsm_stateInit;
    end
  end

  always @ (posedge clk or posedge reset) begin
    if (reset) begin
      fsm_stateReg <= `fsm_enumDefinition_binary_sequential_fsm_BOOT;
    end else begin
      fsm_stateReg <= fsm_stateNext;
    end
  end


endmodule

module ExtensionUnit (
  input      [31:0]   io_instruction,
  output     [31:0]   io_i_imm,
  output     [31:0]   io_j_imm,
  output     [31:0]   io_s_imm,
  output     [31:0]   io_b_imm,
  output     [31:0]   io_u_imm,
  output     [4:0]    io_csr_imm
);
  wire       [0:0]    _zz_1;
  wire       [20:0]   _zz_2;
  wire       [20:0]   _zz_3;
  wire       [31:0]   _zz_4;
  wire       [0:0]    _zz_5;
  wire       [20:0]   _zz_6;
  wire       [0:0]    _zz_7;
  wire       [19:0]   _zz_8;

  assign _zz_1 = io_instruction[31];
  assign _zz_2 = {{20{_zz_1[0]}}, _zz_1};
  assign _zz_3 = {{{{{io_instruction[31],io_instruction[19 : 12]},io_instruction[20]},io_instruction[30 : 25]},io_instruction[24 : 21]},1'b0};
  assign _zz_4 = {{11{_zz_3[20]}}, _zz_3};
  assign _zz_5 = io_instruction[31];
  assign _zz_6 = {{20{_zz_5[0]}}, _zz_5};
  assign _zz_7 = io_instruction[31];
  assign _zz_8 = {{19{_zz_7[0]}}, _zz_7};
  assign io_i_imm = {{{_zz_2,io_instruction[30 : 25]},io_instruction[24 : 21]},io_instruction[20]};
  assign io_j_imm = _zz_4;
  assign io_s_imm = {{{_zz_6,io_instruction[30 : 25]},io_instruction[11 : 8]},io_instruction[7]};
  assign io_b_imm = {{{{_zz_8,io_instruction[7]},io_instruction[30 : 25]},io_instruction[11 : 8]},1'b0};
  assign io_u_imm = {{{io_instruction[31],io_instruction[30 : 20]},io_instruction[19 : 12]},12'h0};
  assign io_csr_imm = io_instruction[19 : 15];

endmodule
