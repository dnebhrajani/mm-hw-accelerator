module cnt
  (input wire       rst,
   input wire       clk,
   input wire [7:0] ldval,
   input wire       ld,
   input wire       en,
   output reg [7:0] q
   );

  always @(posedge clk)
    begin
      if (rst == 1)
        begin
          q <= 8'd0;
        end
      else
        begin
          if (ld == 1)
            begin
              q <= ldval;
            end
          else if (en == 1)
            begin
              q <= q + 1'b1;
            end
          else
            begin
              q <= q;
            end
        end
    end

endmodule // cnt
