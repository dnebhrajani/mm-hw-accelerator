module mm_mac
  (input wire                clk,
   input wire                rst,
    
   input wire [511:0]        col_rdata,
   input wire [511:0]        row_rdata,
  
   input wire                acc_en, // capture, multiply, accumulate this cycle
   input wire                acc_clr, // when set with acc_en load not add

   output wire [7:0]         yram_addr,
   output wire signed [39:0] yram_wdata, // 40-bit signed running sum
   output wire               yram_ce, // 
   output wire               yram_we   // 
   );

  reg signed [511:0]         row_reg;
  reg signed [511:0]         col_reg;
  
  reg signed [31:0]         prod_r [0:31];
  integer                   i;
  // accumulate the products here
  reg signed [32:0] sum_1 [0:15];
  reg signed [33:0] sum_2 [0:7];
  reg signed [34:0] sum_3 [0:3];
  reg signed [35:0] sum_4 [0:1];
  reg signed [36:0] sum_5;

  reg signed [39:0] acc_out;  // 40-bit signed running sum
  reg               acc_valid;

  reg [7:0]         res_addr;
  
  reg               en_r1; // delayed copies of signals for accumulator stage
  reg               clr_r1;
  reg               en_r2; // delayed copies of signals for accumulator stage
  reg               clr_r2;
  reg               en_s1;
  reg               clr_s1;
  reg               en_s2;
  reg               clr_s2;
  reg               en_s3;
  reg               clr_s3;
  reg               en_s4;
  reg               clr_s4;
  reg               en_s5;
  reg               clr_s5;

  assign yram_addr = res_addr;

  //------------------------------------------------------------------------------
  // Flop in row and col data
  //------------------------------------------------------------------------------
  always @(posedge clk) 
    begin
      en_r1   <= acc_en;
      clr_r1  <= acc_clr;
      row_reg <= row_rdata;
      col_reg <= col_rdata;
    end
      
  //------------------------------------------------------------------------------
  // Multipliers
  //------------------------------------------------------------------------------
  always @(posedge clk) 
    begin
      if (rst == 1'b1)
        begin
          en_r2  <= 1'b0;
          clr_r2 <= 1'b0;
        end

      else
        begin
          en_r2  <= en_r1;
          clr_r2 <= clr_r1;
          for (i = 0; i < 32; i = i + 1)
            begin
              prod_r[i] <= $signed(row_reg[i*16+15 -: 16]) * $signed(col_reg[i*16+15 -: 16]);
            end
        end // else: !if(rst == 1'b1)
    end // always @ (posedge clk)
  

  //------------------------------------------------------------------------------
  // Adder tree
  //------------------------------------------------------------------------------
  // Stage 1
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          en_s1  <= 1'b0;
          clr_s1 <= 1'b0;
        end

      else
        begin
          en_s1 <= en_r2;
          clr_s1 <= clr_r2;
          for (i = 0; i < 16; i = i+1)
            begin
              sum_1[i] <= prod_r[2*i] + prod_r[2*i+1];
            end
        end
    end

  // Stage 2
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          en_s2  <= 1'b0;
          clr_s2 <= 1'b0;
        end

      else
        begin
          en_s2 <= en_s1;
          clr_s2 <= clr_s1;
          for (i = 0; i < 8; i = i+1)
            begin
              sum_2[i] <= sum_1[2*i] + sum_1[2*i+1];
            end
        end
    end

  // Stage 3
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          en_s3  <= 1'b0;
          clr_s3 <= 1'b0;
        end

      else
        begin
          en_s3 <= en_s2;
          clr_s3 <= clr_s2;
          for (i = 0; i < 4; i = i+1)
            begin
              sum_3[i] <= sum_2[2*i] + sum_2[2*i+1];
            end
        end
    end

  // Stage 4
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          en_s4  <= 1'b0;
          clr_s4 <= 1'b0;
        end

      else
        begin
          en_s4 <= en_s3;
          clr_s4 <= clr_s3;
          for (i = 0; i < 2; i = i+1)
            begin
              sum_4[i] <= sum_3[2*i] + sum_3[2*i+1];
            end
        end
    end

  // Stage 5, generate final sum
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          en_s5  <= 1'b0;
          clr_s5 <= 1'b0;
        end

      else
        begin
          en_s5 <= en_s4;
          clr_s5 <= clr_s4;
          sum_5 <= sum_4[0] + sum_4[1];
        end // else: !if(rst == 1'b1)
    end


  // accumulator

  wire signed [39:0] psum;
  assign psum = {{3{sum_5[36]}}, sum_5};

  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          acc_out <= 40'd0;
          acc_valid <= 1'b0;
        end
      else
        begin
          acc_valid <= en_s5;
          if (en_s5 == 1'b1)
            begin
              if (clr_s5 == 1'b1)
                begin
                  acc_out <= $signed(psum);
                end
              else
                begin
                  acc_out <= $signed(acc_out) + $signed(psum);
                end
            end
        end
    end

  assign yram_wdata = acc_out;
  assign yram_ce    = acc_valid;
  assign yram_we    = acc_valid;

  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          res_addr <= 8'hFF;
        end
      else
        begin
          if (en_s5 == 1'b1)
            begin
              if (clr_s5 == 1'b1)
                begin
                  res_addr <= res_addr + 1'b1;
                end
            end
          else
            begin
              res_addr <= 8'hFF;
            end
        end
    end
      

  
endmodule // mm_mac
