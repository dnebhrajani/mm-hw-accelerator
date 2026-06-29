module mm_mram
  (input wire         clk,

   // port a
   input wire [15:0]  addr_a,
   input wire [15:0]  wdata_a,
   input wire         we_a,
   input wire         ce_a,
   output reg [15:0]  rdata_a,

   // port b
   input wire [10:0]  addr_b,
   input wire         ce_b,
   output reg [511:0] rdata_b
   );

  reg [15:0]          ram [0:65535];

  // port a
  always @(posedge clk)
    begin : port_a
      if (ce_a == 1'b1)
        begin
          if (we_a == 1'b1)
            begin
              ram[addr_a] <= wdata_a;
            end
          rdata_a <= ram[addr_a];
        end
    end // always @ (posedge clk)

  
  // port b
  always @(posedge clk)
    begin : port_b
      integer i;
      
      if (ce_b == 1'b1)
        begin          
          for (i = 0; i < 32; i = i + 1)
            begin
              rdata_b[(16 * (i + 1) - 1) -: 16] <= ram[{addr_b, i[4:0]}];
            end
        end
    end

  
endmodule
