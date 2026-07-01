//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   mm_reg.v
// Author: Durga V. Nebhrajani
// Description:
//         Control and status register file for Matrix Multiplier Core. Provides
//         ID, control, status, matrix dimension (N), and cycle count
//         registers accessible via the AXI slave interface.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

module mm_reg
  (input wire        clk,
   input wire        rst,
   input wire [9:0]  addr,
   input wire [31:0] wdata,
   input wire        ce, 
   input wire        we,
   output reg [31:0] rdata,

   output reg        mm_start,
   input wire        mm_done,
   output reg [8:0]  n,
   input wire [31:0] ccnt,
   output reg        ccnt_clr,

   output wire [7:0] led_o
  );

  localparam ID_OFFSET   = 10'h000;
  localparam CTL_OFFSET  = 10'h004;
  localparam STS_OFFSET  = 10'h008;
  localparam N_OFFSET    = 10'h00C;
  localparam CCNT_OFFSET = 10'h010;

  assign led_o   = n[7:0];

  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          n        <= 9'd0;
          mm_start <= 1'b0;
          ccnt_clr <= 1'b0;
        end
      else
        begin
          // Self clearing bits
          mm_start <= 1'b0;
          ccnt_clr <= 1'b0;
          
          if (ce == 1'b1)
            begin
              // writes
              if (we == 1'b1)
                begin
                  case (addr)
                    CTL_OFFSET:
                      begin
                        mm_start <= wdata[0];
                      end

                    
                    N_OFFSET:
                      begin
                        n <= wdata[8:0];
                      end

                    default:
                      begin
                      end
                    
                  endcase
                end 
              
              // reads
              else 
                begin
                  case (addr)
                    ID_OFFSET:
                      begin
                        rdata <= {5'd16, 4'd6, 11'd2026, 12'd1};
                      end
                    
                    CTL_OFFSET:
                      begin
                        rdata <= {31'd0, mm_start};
                      end

                    STS_OFFSET:
                      begin
                        rdata <= {31'd0, mm_done};
                      end

                    N_OFFSET:
                      begin
                        rdata <= {23'd0, n};
                      end

                    CCNT_OFFSET:
                      begin
                        rdata    <= ccnt;
                        ccnt_clr <= 1'b1;
                      end
                    
                    
                    default:
                      begin
                        rdata <= 32'hDEAD_BEEF;
                      end
                  endcase
                end
            end // if (ce == 1'b1)
        end // else: !if(rst == 1'b1)
    end // always @ (posedge clk)
  
endmodule
