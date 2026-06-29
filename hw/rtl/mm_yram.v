module mm_yram
  (input wire         clk,

   // port a
   input wire [8:0]   addr_a,
   input wire [31:0]  wdata_a,
   input wire         we_a,
   input wire         ce_a,
   output wire [31:0] rdata_a,

   // port b
   input wire [7:0]   addr_b,
   input wire         ce_b,
   input wire         we_b,
   input wire [39:0]  wdata_b
   );

  (* ram_style = "block" *)
  reg [31:0]          ram_l [0:255];
  (* ram_style = "block" *)
  reg [7:0]           ram_u [0:255];

  wire [7:0]          addr_a_u = addr_a[8:1];
  wire [7:0]          addr_a_l = addr_a[8:1];

  wire                ce_a_u = ce_a && addr_a[0];
  wire                ce_a_l = ce_a && ~addr_a[0];

  reg [31:0]          rdata_a_l;
  reg [7:0]          rdata_a_u;

  assign rdata_a = addr_a[0] ? {{24{rdata_a_u[7]}}, rdata_a_u} : rdata_a_l;
  
  //----------------------------------------------------------------------
  // Lower RAM
  //----------------------------------------------------------------------
  // port a
  always @(posedge clk)
    begin : port_a_lower
      
      if (ce_a_l == 1'b1)
        begin          
          if (we_a == 1'b1)
            begin
              ram_l[addr_a_l] <= wdata_a;
            end // if (we_a == 1'b1)

          // else
          //   begin
          //     rdata_a = ram_l[addr_a_l];
          //   end
        end // if (ce_a_l == 1'b1)

      rdata_a_l <= ram_l[addr_a_l];      
    end
  
  // port b
  always @(posedge clk)
    begin : port_b_lower
      if (ce_b == 1'b1)
        begin
          if (we_b == 1'b1)
            begin
              ram_l[addr_b] <= wdata_b[31:0];
            end
        end
    end // always @ (posedge clk)

  //----------------------------------------------------------------------
  // Upper RAM
  //----------------------------------------------------------------------
  // port a
  always @(posedge clk)
    begin : port_a_upper
      
      if (ce_a_u == 1'b1)
        begin          
          if (we_a == 1'b1)
            begin
              ram_u[addr_a_u] <= wdata_a[7:0];
            end // if (we_a == 1'b1)

          // else
          //   begin
          //     rdata_a = {{24{ram_u[addr_a_u][7]}},  ram_u[addr_a_u]};
          //   end
        end // if (ce_a_u == 1'b1)

      rdata_a_u <= ram_u[addr_a_u];
    end

  // port b
  always @(posedge clk)
    begin : port_b_upper
      if (ce_b == 1'b1)
        begin
          if (we_b == 1'b1)
            begin
              ram_u[addr_b] <= wdata_b[39:32];
            end
        end
    end // always @ (posedge clk)

  
endmodule
