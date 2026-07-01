//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   mm_dpram.v
// Author: Durga V. Nebhrajani
// Description:
//         Parameterized dual-port block RAM for Matrix Multiplier Core. Port A is
//         16-bit (element-wise access), port B is 512-bit (page-wide
//         access). Used for both XRAM and MRAM.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

module mm_dpram
  #(parameter                        MEM_SZ = 256
   )
  
  (input wire                        clk,

   // port a
   input wire [$clog2(MEM_SZ)-1 : 0] addr_a,
   input wire [15:0]                 wdata_a,
   input wire                        we_a,
   input wire                        ce_a,
   output reg [15:0]                 rdata_a,

   // port b
   input wire [$clog2(MEM_SZ)-6 : 0] addr_b,
   input wire [511:0]                wdata_b,
   input wire                        ce_b,
   input wire                        we_b,
   output reg [511:0]                rdata_b
   );

  (* ram_style = "block" *)
  reg [15:0]          ram [0:MEM_SZ - 1];

  // port a
  always @(posedge clk)
    begin : port_a
      if (ce_a == 1'b1)
        begin
          if (we_a == 1'b1)
            begin
              ram[addr_a] <= wdata_a;
            end
//          rdata_a <= ram[addr_a];
        end

      rdata_a <= ram[addr_a];
      
    end // always @ (posedge clk)

  
  // port b
  always @(posedge clk)
    begin : port_b
      integer i;
      
      if (ce_b == 1'b1)
        begin
          if (we_b == 1'b1)
            begin
              for (i = 0; i < 32; i = i + 1)
                begin
                  ram[{addr_b, i[4:0]}] <= wdata_b[(16 * (i + 1) - 1) -: 16];
                end
            end
        end

      for (i = 0; i < 32; i = i + 1)
        begin
          rdata_b[(16 * (i + 1) - 1) -: 16] <= ram[{addr_b, i[4:0]}];
        end
    end

  
endmodule
