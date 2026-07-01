//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   mm_simtop.v
// Author: Durga V. Nebhrajani
// Description:
//         Simulation top level for Matrix Multiplier Core.     
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

`timescale 1ns/1ns

module mm_simtop;

  // Timing parameters
  parameter t_ck = 10.000;                    // 10 ns = 100 MHz clock
  parameter t_s  = 0.20 * t_ck;               // Setup
  parameter t_h  = 0.10 * t_ck;               // Hold
  parameter t_cq = 0.15 * t_ck;               // Clock to q
  parameter duty = 0.5;                       // Clock duty cycle, 50%

  localparam REG_BA      = 32'h4030_0000;
  localparam XRAM_BA     = 32'h4032_0000;
  localparam YRAM_BA     = 32'h4033_0000;
  localparam MRAM_BA     = 32'h4034_0000;

  localparam ID_OFFSET   = 8'h00;
  localparam CTL_OFFSET  = 8'h01;
  localparam STS_OFFSET  = 8'h02;
  localparam N_OFFSET    = 8'h03;
  localparam CCNT_OFFSET = 8'h04;

  localparam ID_ADDR     = REG_BA + ID_OFFSET;
  localparam CTL_ADDR    = REG_BA + CTL_OFFSET;
  localparam STS_ADDR    = REG_BA + STS_OFFSET;
  localparam N_ADDR      = REG_BA + N_OFFSET;
  localparam CCNT_ADDR   = REG_BA + CCNT_OFFSET;
  
  reg                              clk;
  reg                              rst;
  reg [31:0]                       sys_addr;  // bus address
  reg [31:0]                       sys_wdata; // bus write data
  reg                              sys_wen;   // bus write enable
  reg                              sys_ren;   // bus read enable
  wire [31:0]                      sys_rdata; // bus read data
  wire                             sys_err;   // bus error indicator
  wire                             sys_ack;   // bus acknowledge signal
  wire [7:0]                       led_o;

  reg signed [15:0]                M [0:255][0:255];
  reg signed [15:0]                X [0:255];
  reg signed [39:0]                Y [0:255];
  reg [8:0]                        N;

  reg                              DEBUG_MODE;
  
  // Simulation events
  event                            write_ok;
  event                            write_err;
  event                            read_ok;
  event                            read_err;

  // DUT
  mm_core mm_core
     (.clk(clk),
      .rst(rst),
      .sys_addr(sys_addr),
      .sys_wdata(sys_wdata),
      .sys_wen(sys_wen),
      .sys_ren(sys_ren),
      .sys_rdata(sys_rdata),
      .sys_err(sys_err),
      .sys_ack(sys_ack),
      .led_o(led_o)
     );
  
  //----------------------------------------------------------------------
  // Clock
  //----------------------------------------------------------------------
  always
    begin
      clk = 1'b0;
      #(t_ck * (1 - duty));
      clk = 1'b1;
      #(t_ck * duty);
    end

  initial
    begin
      if ($test$plusargs("DEBUG"))
        begin
          $display("DEBUG mode ON");
          DEBUG_MODE = 1;
          $dumpvars();
        end
      else
        begin
          DEBUG_MODE = 0;
        end

      if ($value$plusargs("N=%d", N))
        begin
          $display("Setting N to %0d", N);
        end
      else
        begin
          N = 10;
          $display("Setting N to 10");          
        end
    end
  
  //----------------------------------------------------------------------
  // Tasks
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  // System Reset task
  task sys_reset;
    begin
      sys_wen = 0;
      sys_ren = 0;

      // Apply reset right away
      rst = 1'b1;
      // Wait for two clock edges and hold time
      repeat (2) @(posedge clk);
      #t_h;
      // Remove reset
      rst = 1'b0;
    end
  endtask

  //----------------------------------------------------------------------
  // AXI write task
  task axi_write;
    input [31:0] addr;
    input [31:0] data;
    
    begin
      @(posedge clk);
      #(t_ck - t_s);
      sys_addr  = addr;
      sys_wdata = data;
      sys_wen   = 1'b1;
      sys_ren   = 1'b0;
      @(posedge clk);

      while (sys_ack == 1'b0)
        begin
          @(posedge clk);
        end

      if (sys_err == 1'b0)
        begin
          if (DEBUG_MODE)
            begin
              $display("At time %t: Addr [0x%h] : Data written successfully [0x%h]", 
                       $time, addr, data);
            end
          -> write_ok;
        end
      else
        begin
          if (DEBUG_MODE)
            begin
              $display("At time %t: Addr [0x%h] : Data write ERROR.", $time, addr);
            end
          -> write_err;
          endsim(1);
        end

      #t_h;
      sys_addr  = 32'bxxxx_xxxx_xxxx_xxxx_xxxx;
      sys_wdata = 32'bxxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx;
      sys_wen   = 1'b0;
      sys_ren   = 1'b0;

      @(posedge clk);
    end
  endtask // axi_write

  //----------------------------------------------------------------------
  // AXI read task
  task axi_read;
    input [31:0]  addr;
    output [31:0] data;

    begin
      @(posedge clk);
      #(t_ck - t_s);
      sys_addr  = addr;
      sys_wdata = 32'bxxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx;
      sys_wen   = 1'b0;
      sys_ren   = 1'b1;
      @(posedge clk);
      #t_h;
      sys_addr  = 32'bxxxx_xxxx_xxxx_xxxx_xxxx;
      sys_wdata = 32'bxxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx;
      sys_wen   = 1'b0;
      @(posedge clk);

      while (sys_ack != 1'b1)
        begin
          @(posedge clk);
        end

      if (sys_err == 1'b0)
        begin
          data = sys_rdata;
          if (DEBUG_MODE)
            begin
              $display("At time %t: Addr [0x%h] : Data read successfully [0x%h]", $time, addr, data);
            end
          -> read_ok;
        end
      else
        begin
          if (DEBUG_MODE)
            begin
              $display("At time %t: Addr [0x%h] : Data read ERROR.", $time, addr);
            end
          -> read_err;
          endsim(1);
        end

      #t_h;
      sys_ren  = 1'b0;
      @(posedge clk);
    end
  endtask // axi_read

  //----------------------------------------------------------------------
  // Load M matrix to MRAM
  task load_m;
    input [8:0]   n;

    reg [31:0]    mram_addr;
    integer       ra, ca;
    integer       npages;

    begin
      npages = (n + 31) >> 5;
      
      for (ra = 0; ra < n; ra = ra + 1)
        begin
          for (ca = 0; ca < n; ca = ca + 1)          
            begin
              mram_addr = MRAM_BA + {{ra[7:0], ca[7:0]}, 2'b00};
              mm_simtop.axi_write(mram_addr, M[ra][ca]);
            end
          
          for (ca = n; ca < 32*npages; ca = ca + 1)          
            begin
              mram_addr = MRAM_BA + {{ra[7:0], ca[7:0]}, 2'b00};
              mm_simtop.axi_write(mram_addr, 16'd0);
            end
        end
    end
  endtask

  //----------------------------------------------------------------------
  // Load X vector to XRAM
  task load_x;
    input [8:0]    n;
    
    reg [31:0]     xram_addr;
    integer        ra;
    
    begin
      for (ra = 0; ra < n; ra = ra + 1)
        begin
          xram_addr = XRAM_BA + {ra[7:0], 2'b00};
          mm_simtop.axi_write(xram_addr, {16'd0, X[ra]});
        end

      // Zero out remaining locations
      for (ra = n; ra < 256; ra = ra + 1)
        begin
          xram_addr = XRAM_BA + {ra[7:0], 2'b00};
          mm_simtop.axi_write(xram_addr, 16'd0);
        end
    end
  endtask // load_x

  //----------------------------------------------------------------------
  // Read Y vector
  task read_y;
    input [8:0]    n;
    
    reg [31:0]     yram_addr;
    reg [31:0]     rdata;
    reg [39:0]     y;
    integer        ra;
    integer        i;
    
    begin
      i = 0;
      for (ra = 0; ra < 2*n; ra = ra + 2)
        begin
          yram_addr = YRAM_BA + {ra[8:0], 2'b00};
          mm_simtop.axi_read(yram_addr, rdata);
          y[31:0] = rdata;

          yram_addr = YRAM_BA + {ra[8:0]+1'b1, 2'b00};
          mm_simtop.axi_read(yram_addr, rdata);
          y[39:32] = rdata[7:0];
          
          Y[i] = y;
          i = i + 1;
        end
    end
  endtask // read_y

  //----------------------------------------------------------------------
  // Task to poll mm_done status inside Matrix Multiplier Core
  task poll_for_done;
    input [8:0] n;

    reg [31:0] rdata;
    reg mm_done;
    integer gap;

    begin
      gap = (n[8:5] + 1)*n + 10;

      mm_done = 1'b0;

      while (mm_done == 1'b0)
        begin
          delay(gap);
          axi_read(STS_ADDR,  rdata);
          mm_done = rdata[0];
          if (DEBUG_MODE)
            begin
              if (mm_done == 1'b1)
                begin
                  $display("\nMatrix multiplication FINISHED - mm_done = %b", mm_done);
                end
              else
                begin
                  $display("\nMatrix multiplication RUNNING  - mm_done = %b\n", mm_done);              
                end
            end
        end
    end
  endtask // poll_for_done
  
  //----------------------------------------------------------------------
  // A delay task that whiles away some clocks
  task delay;
    input [31:0]    nclocks;

    begin
      repeat(nclocks) @(posedge clk);
    end    
  endtask // delay
  
  //----------------------------------------------------------------------
  // End simulation cleanly task
  task endsim;
    input fail_flag;
    
    begin
      if (fail_flag !== 1'b0)
        begin
          $display("Test FAILED.");
        end
      else
        begin
          $display("Test PASSED.");
        end

      repeat (10) @(posedge clk);
      $display("Ending simulation");
      $finish;
    end
  endtask // endsim
  
endmodule // mm_simtop

