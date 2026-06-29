//--------------------------------------------------------------------------------
// Module: hwacc
// File:   mm_ctrl.v
// Author: Durga V. Nebhrajani
// Description:
//         Simulation top level for Ising Machine Core.
//      
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

module mm_ctrl
  (input wire         clk,
   input wire         rst,

   input wire         mm_start,
   input wire [8:0]   n, // 0-256 range for value of n
   output reg         mm_done,
   
   output wire [2:0]  col_addr,
   output reg         col_ce,

   output wire [10:0] row_addr,
   output reg         row_ce,

   output reg         acc_en,
   output reg         acc_clr,

   output reg [31:0]  ccnt,
   input wire         ccnt_clr
   );

  localparam IDLE  = 3'd0;
  localparam OPER  = 3'd2;
  localparam WAIT  = 3'd3;
  localparam DONE  = 3'd4;

  reg [2:0]          mm_state;
  reg [3:0]          n_pages;  // (n/32) ceiling, max 8
  reg [7:0]          row_cnt;  // 0 to 255, curr row index
  reg [2:0]          page_cnt; // pages fetched till now
  reg [2:0]          wait_cnt; // wait countdown for MAC 7 cycle pipeline
  reg                is_first; // flag if first page of row for acc_clr

  assign col_addr = page_cnt;
  assign row_addr = {row_cnt, page_cnt};
  
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          mm_state <= IDLE;
          mm_done  <= 1'b1;
          col_ce   <= 1'b0;
          row_ce   <= 1'b0;
          acc_en   <= 1'b0;
          acc_clr  <= 1'b0;          
          n_pages  <= 4'd0;
          row_cnt  <= 8'd0;
          page_cnt <= 3'd0;
          wait_cnt <= 3'd0;
          is_first <= 1'b0;
        end // if (rst == 1'b1)

      else
        begin
          acc_clr    <= is_first;

          case (mm_state)
            IDLE:
              begin
                if ((mm_start == 1'b1) && (n != 9'd0))
                  begin
                    n_pages  <= (n + 9'd31) >> 5; // ceil n/32
                    row_cnt  <= 8'd0;
                    col_ce   <= 1'b1;
                    row_ce   <= 1'b1;
                    page_cnt <= 4'd0;
                    is_first <= 1'b1;
                    mm_done  <= 1'b0;
                    mm_state <= OPER;
                  end
              end // case: IDLE

            OPER:
              begin
                acc_en   <= 1'b1;
                
                if ((row_cnt == (n - 1'b1)) &&      // Last row
                    (page_cnt == (n_pages - 1'b1))) // Last page
                  begin
                    // Terminal condition, all done, wait and end
                    page_cnt <= 4'd0;
                    row_cnt  <= 8'd0;
//                    acc_en   <= 1'b0;
                    is_first <= 1'b0;
                    row_ce   <= 1'b0;
                    col_ce   <= 1'b0;
                    mm_state <= WAIT;
                  end

                else if (page_cnt == (n_pages - 1'b1)) // Last page
                  begin
                    // Pages of current row are done
                    page_cnt <= 4'd0;
                    is_first <= 1'b1;
                    row_cnt  <= row_cnt + 1'b1;
                  end

                else
                  begin
                    is_first <= 1'b0;
                    page_cnt <= page_cnt + 1'b1;
                  end
              end // case: OPER
            
            WAIT:
              begin
                acc_en <= 1'b0;
                if (wait_cnt == 3'd0)
                  begin
                    mm_state <= DONE;
                  end
                else
                  begin
                    wait_cnt <= wait_cnt - 3'd1; // decrement wait count
                  end

              end // case: WAIT

            DONE:
              begin
                mm_done  <= 1'b1;
                mm_state <= IDLE;
              end // case: DONE

            default:
              begin
                mm_state <= IDLE;
              end
          endcase
        end
    end // always @ (posedge clk)

  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          ccnt <= 32'd0;
        end

      else
        begin
          if (ccnt_clr == 1'b1)
            begin
              ccnt <= 32'd0;
            end
          else
            begin
              if (mm_state != IDLE)
                begin
                  ccnt <= ccnt + 1'b1;
                end
            end
        end
    end

endmodule
