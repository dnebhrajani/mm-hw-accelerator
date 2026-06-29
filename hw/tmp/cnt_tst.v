module cnt_tst;


  reg       rst;
  reg       clk;
  reg [7:0] ldval;
  reg       ld;
  reg       en;
  wire [7:0] q;
  wire [7:0] q_struct;
  
 


  cnt cnt_1
    (.rst(rst),
     .clk(clk),
     .ldval(ldval),
     .ld(ld),
     .en(en),
     .q(q)
    );

  
  cnt_struct cnt_2
    (.rst(rst),
     .clk(clk),
     .ldval(ldval),
     .ld(ld),
     .en(en),
     .q(q_struct)
    );
  
  always
    begin
      clk = 0;
      #5;
      clk = 1;
      #5;
    end

  initial
    begin
      $dumpvars;
      
      rst = 1;
      ldval = 8'd0;
      ld = 0;
      en = 0;
      
      repeat (3) @(posedge clk);
      rst = 0;

      repeat (3) @(posedge clk);
      en = 1;

      repeat (16) @(posedge clk);

      ldval = 8'd128;
      ld = 1;
      @(posedge clk);
      ld = 0;
      
      repeat (16) @(posedge clk);

      $finish;
    end
  
  

endmodule
