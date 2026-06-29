module cnt_struct
  (input wire       rst,
   input wire       clk,
   input wire [7:0] ldval,
   input wire       ld,
   input wire       en,
   output reg [7:0] q
   );

  wire [7:0]        d;
  wire [7:0]        q_inc;  
  wire [7:0]        q_inc_en;
    
  // dff
  always @(posedge clk)
    begin
      if (rst == 1)
        begin
          q <= 8'd0;
        end
      else
        begin
          q <= d;
        end
    end

  // incrementer
  assign q_inc = q + 1'b1;

  // en mux
  assign q_inc_en = (en == 1) ? q_inc : q;

  // ld mux
  assign d = (ld == 1) ? ldval : q_inc_en;

 
endmodule // cnt
