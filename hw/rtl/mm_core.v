module mm_core
  (input wire         clk,
   input wire         rst,

   input wire [31:0]  sys_addr,
   input wire [31:0]  sys_wdata,
   input wire         sys_wen,
   input wire         sys_ren,
   output wire [31:0] sys_rdata,
   output wire        sys_ack,
   output wire        sys_err,
   output wire [7:0]  led_o
   );
  
  // Local signals for mm_reg
  wire [9:0]          reg_addr;
  wire [31:0]         reg_wdata;
  wire                reg_ce;
  wire                reg_we;
  wire [31:0]         reg_rdata;
  wire                mm_start;
  wire                mm_done;
  wire [8:0]          n;
  wire [31:0]         ccnt;
  wire                ccnt_clr;

  // Local signals for mm_xram
  wire [7:0]          xram_addr;
  wire [15:0]         xram_wdata;
  wire                xram_we;
  wire                xram_ce;
  wire [15:0]         xram_rdata;

  wire [2:0]          col_addr;
  wire                col_ce;
  wire [511:0]        col_rdata;

  // Local signals for mm_mram
  wire [15:0]         mram_addr;
  wire [15:0]         mram_wdata;
  wire                mram_we;
  wire                mram_ce;
  wire [15:0]         mram_rdata;

  wire [10:0]         row_addr;
  wire                row_ce;
  wire [511:0]        row_rdata;

  // Local signals for mm_yram
  wire [8:0]          yram_addr;
  wire [31:0]         yram_wdata;
  wire                yram_we;
  wire                yram_ce;
  wire [31:0]         yram_rdata;

  wire [7:0]          acc_out_addr;
  wire                acc_out_ce;
  wire                acc_out_we;  
  wire signed [39:0]  acc_out_wdata;
  
  // Local signals for mm_mac
  wire                acc_en;
  wire                acc_clr;

  //----------------------------------------------------------------------
  // MM_AXI_SLAVE
  //----------------------------------------------------------------------
  mm_axi_slave mm_axi_slave
    (.clk(clk),
     .rst(rst),
     .sys_addr(sys_addr),
     .sys_wdata(sys_wdata),
     .sys_wen(sys_wen),
     .sys_ren(sys_ren),
     .sys_rdata(sys_rdata),
     .sys_ack(sys_ack),
     .sys_err(sys_err),
     .reg_addr(reg_addr),
     .reg_wdata(reg_wdata),
     .reg_ce(reg_ce), 
     .reg_we(reg_we),
     .reg_rdata(reg_rdata),
     .xram_addr(xram_addr),
     .xram_wdata(xram_wdata),
     .xram_we(xram_we),
     .xram_ce(xram_ce),
     .xram_rdata(xram_rdata),
     .mram_addr(mram_addr),
     .mram_wdata(mram_wdata),
     .mram_we(mram_we),
     .mram_ce(mram_ce),
     .mram_rdata(mram_rdata),
     .yram_addr(yram_addr),
     .yram_wdata(yram_wdata),
     .yram_we(yram_we),
     .yram_ce(yram_ce),
     .yram_rdata(yram_rdata)
    );

  //----------------------------------------------------------------------
  // MM_REG
  //----------------------------------------------------------------------
  mm_reg mm_reg
    (.clk(clk),
     .rst(rst),
     .addr(reg_addr),
     .wdata(reg_wdata),
     .ce(reg_ce), 
     .we(reg_we),
     .rdata(reg_rdata),
     .mm_start(mm_start),
     .mm_done(mm_done),
     .n(n),
     .ccnt(ccnt),
     .ccnt_clr(ccnt_clr),
     .led_o(led_o)
    );

  //----------------------------------------------------------------------
  // MM_XRAM
  //----------------------------------------------------------------------
  mm_dpram
    #(.MEM_SZ(256)
     ) mm_xram
     (.clk(clk),
      .addr_a(xram_addr),
      .wdata_a(xram_wdata),
      .we_a(xram_we),
      .ce_a(xram_ce),
      .rdata_a(xram_rdata),
      .addr_b(col_addr),
      .wdata_b(),
      .we_b(1'b0),
      .ce_b(col_ce),
      .rdata_b(col_rdata)
     );

  //----------------------------------------------------------------------
  // MM_MRAM
  //----------------------------------------------------------------------
  mm_dpram
    #(.MEM_SZ(65536)
     ) mm_mram
     (.clk(clk),
      .addr_a(mram_addr),
      .wdata_a(mram_wdata),
      .we_a(mram_we),
      .ce_a(mram_ce),
      .rdata_a(mram_rdata),
      .addr_b(row_addr),
      .wdata_b(),
      .we_b(1'b0),
      .ce_b(row_ce),
      .rdata_b(row_rdata)
     );

  //----------------------------------------------------------------------
  // MM_YRAM
  //----------------------------------------------------------------------
  mm_yram mm_yram
    (.clk(clk),
     .addr_a(yram_addr),
     .wdata_a(yram_wdata),
     .we_a(yram_we),
     .ce_a(yram_ce),
     .rdata_a(yram_rdata),
     .addr_b(acc_out_addr),
     .ce_b(acc_out_ce),
     .we_b(acc_out_we),
     .wdata_b(acc_out_wdata)
     );

  //----------------------------------------------------------------------
  // MM_MAC
  //----------------------------------------------------------------------
  mm_mac mm_mac
    (.clk(clk),
     .rst(rst),
     .col_rdata(col_rdata),
     .row_rdata(row_rdata),
     .acc_en(acc_en),
     .acc_clr(acc_clr),
     .yram_addr(acc_out_addr),
     .yram_wdata(acc_out_wdata),
     .yram_ce(acc_out_ce),
     .yram_we(acc_out_we)
    );

  //----------------------------------------------------------------------
  // MM_CTRL
  //----------------------------------------------------------------------
  mm_ctrl mm_ctrl
    (.clk(clk),
     .rst(rst),
     .mm_start(mm_start),
     .n(n),
     .mm_done(mm_done),
     .col_addr(col_addr),
     .col_ce(col_ce),
     .row_addr(row_addr),
     .row_ce(row_ce),
     .acc_en(acc_en),
     .acc_clr(acc_clr),
     .ccnt(ccnt),
     .ccnt_clr(ccnt_clr)
    );

endmodule
