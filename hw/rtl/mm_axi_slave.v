module mm_axi_slave
  (input wire        clk,
   input wire        rst,

   input wire [31:0] sys_addr,
   input wire [31:0] sys_wdata,
   input wire        sys_wen,
   input wire        sys_ren,
   output reg [31:0] sys_rdata,
   output reg        sys_ack,
   output reg        sys_err,
 
   // Register interface
   output reg [9:0]  reg_addr,
   output reg [31:0] reg_wdata,
   output reg        reg_ce, 
   output reg        reg_we,
   input wire [31:0] reg_rdata,

   // XRAM interface
   output reg [7:0]  xram_addr,
   output reg [15:0] xram_wdata,
   output reg        xram_we,
   output reg        xram_ce,
   input wire [15:0] xram_rdata,

   // MRAM interface
   output reg [15:0] mram_addr,
   output reg [15:0] mram_wdata,
   output reg        mram_we,
   output reg        mram_ce,
   input wire [15:0] mram_rdata,

   // YRAM interface
   output reg [8:0]  yram_addr,
   output reg [31:0] yram_wdata,
   output reg        yram_we,
   output reg        yram_ce,
   input wire [31:0] yram_rdata
  );

  localparam IDLE   = 3'b000;
  localparam WAIT_1 = 3'b001;
  localparam WAIT_2 = 3'b010;
  localparam ACK    = 3'b011;
  localparam WBI1   = 3'b100;
  localparam WBI2   = 3'b101;
  
  reg [2:0]          axi_state;
  
  always @(posedge clk)
    begin
      if (rst == 1'b1)
        begin
          sys_ack   <= 1'b0;
          sys_err   <= 1'b0;
          
          reg_ce    <= 1'b0;
          reg_we    <= 1'b0;
          xram_ce   <= 1'b0;
          xram_we   <= 1'b0;
          mram_ce   <= 1'b0;
          mram_we   <= 1'b0;
          yram_ce   <= 1'b0;
          yram_we   <= 1'b0;

          sys_rdata <= 32'hEEEE_EEEE;
          axi_state <= IDLE;
        end

      else
        begin
          sys_ack <= 1'b0;
          
          case (axi_state)
            IDLE:
              begin
                // Write
                if ((sys_wen == 1'b1) && (sys_addr[31:20] == 12'h403))
                  begin
                    if (sys_addr[18] == 1'b1) // Selects MRAM
                      begin
                        mram_ce    <= 1'b1;
                        mram_we    <= 1'b1;
                        mram_addr  <= sys_addr[17:2];
                        mram_wdata <= sys_wdata[15:0];
                      end
                    
                    else
                      begin
                        if (sys_addr[17] == 1'b1) // Selects XRAM or YRAM
                          begin
                            if (sys_addr[16] == 1'b1) // Selects YRAM
                              begin
                                yram_ce    <= 1'b1;
                                yram_we    <= 1'b1;
                                yram_addr  <= sys_addr[10:2];
                                yram_wdata <= sys_wdata[31:0];
                              end
                            
                            else // Selects XRAM
                              begin
                                xram_ce    <= 1'b1;
                                xram_we    <= 1'b1;
                                xram_addr  <= sys_addr[9:2];
                                xram_wdata <= sys_wdata[15:0];
                              end
                          end
                        
                        else  // Selects REG
                          begin
                            reg_ce    <= 1'b1;
                            reg_we    <= 1'b1;
                            reg_addr  <= sys_addr[9:0];
                            reg_wdata  <= sys_wdata[31:0];
                          end
                      end // else: !if(sys_addr[18] == 1'b1)

                    sys_ack    <= 1'b1;                    
                    axi_state  <= ACK;
                  end // if (sys_wen == 1'b0)

                // Read
                else if ((sys_ren == 1'b1) && (sys_addr[31:20] == 12'h403))
                  begin
                    if (sys_addr[18] == 1'b1) // Selects MRAM
                      begin
                        mram_ce    <= 1'b1;
                        mram_addr  <= sys_addr[17:2];
                      end
                    
                    else
                      begin
                        if (sys_addr[17] == 1'b1) // Selects XRAM or YRAM
                          begin
                            if (sys_addr[16] == 1'b1) // Selects YRAM
                              begin
                                yram_ce    <= 1'b1;
                                yram_addr  <= sys_addr[10:2];
                              end
                            
                            else // Selects XRAM
                              begin
                                xram_ce    <= 1'b1;
                                xram_addr  <= sys_addr[9:2];
                              end
                          end
                        
                        else  // Selects REG
                          begin
                            reg_ce    <= 1'b1;
                            reg_addr  <= sys_addr[9:0];
                          end
                      end // else: !if(sys_addr[18] == 1'b1)

                    axi_state <= WAIT_1;
                  end
              end // case: IDLE

            WAIT_1:
              begin
                axi_state <= WAIT_2;
              end
            
            WAIT_2:
              begin
                case ({reg_ce, xram_ce, mram_ce, yram_ce})
                  4'b1000:
                    begin
                      sys_rdata <= reg_rdata;
                      sys_ack   <= 1'b1;                
                      axi_state <= ACK;
                    end

                  4'b0100:
                    begin
                      sys_rdata <= {16'd0, xram_rdata};
                      sys_ack   <= 1'b1;                
                      axi_state <= ACK;
                    end

                  4'b0010:
                    begin
                      sys_rdata <= {16'd0, mram_rdata};
                      sys_ack   <= 1'b1;                
                      axi_state <= ACK;
                    end

                  4'b0001:
                    begin
                      sys_rdata <= yram_rdata;
                      sys_ack   <= 1'b1;                
                      axi_state <= ACK;
                    end

                  default:
                    begin
                      sys_rdata <= 32'hDEAD_DA7A;
                      sys_ack   <= 1'b1;
                      sys_err   <= 1'b1;
                      axi_state <= ACK;
                    end
                endcase // case ({reg_ce, xram_ce, mram_ce, yram_ce})

              end
            
            ACK:
              begin
                sys_ack   <= 1'b0;
                
                reg_ce    <= 1'b0;
                reg_we    <= 1'b0;
                xram_ce   <= 1'b0;
                xram_we   <= 1'b0;
                mram_ce   <= 1'b0;
                mram_we   <= 1'b0;
                yram_ce   <= 1'b0;
                yram_we   <= 1'b0;

                axi_state <= WBI1;
              end // case: ACK

            WBI1:
              begin
                if ((sys_wen == 1'b0) && (sys_ren == 1'b0))
                  begin
                    sys_ack <= 1'b0;
                    axi_state <= IDLE;
                  end
                axi_state <= WBI2;
              end
            
            WBI2:
              begin
                reg_ce    <= 1'b0;
                reg_we    <= 1'b0;
                xram_ce   <= 1'b0;
                xram_we   <= 1'b0;
                mram_ce   <= 1'b0;
                mram_we   <= 1'b0;
                yram_ce   <= 1'b0;
                yram_we   <= 1'b0;

                axi_state <= IDLE;
              end
            
            default:
              begin
                sys_ack   <= 1'b1;
                sys_err   <= 1'b1;
                reg_ce    <= 1'b0;
                reg_we    <= 1'b0;
                xram_ce   <= 1'b0;
                xram_we   <= 1'b0;
                mram_ce   <= 1'b0;
                mram_we   <= 1'b0;
                yram_ce   <= 1'b0;
                yram_we   <= 1'b0;

                axi_state <= IDLE;                
              end
            
          endcase // case (axi_state)
          
        end // else: !if(rst == 1'b1)
    end

  
endmodule
