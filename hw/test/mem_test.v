module mem_test;

  reg [31:0] rd_data;
  reg        err_flag;

  integer    i;
  
  initial
    begin
      err_flag = 1'b0;
      
      mm_simtop.sys_reset;

      // Test one register
      mm_simtop.axi_write(19'h000C, 16'hDFAD);
      mm_simtop.axi_read(19'h000C, rd_data);

      if (rd_data [15:0] !== 16'h1AD)
        begin
          err_flag = 1'b1;
        end


      for (i = 0; i < 256; i = i + 1)
        begin
          // Test one location of XRAM
          mm_simtop.axi_write(19'h2_0000 + 4*i, i[15:0]);
          mm_simtop.axi_read(19'h2_0000 + 4*i, rd_data);

          if (rd_data [15:0] !== i[15:0])
            begin
              $display("error: read back data incorrect. expected %h, got %h", rd_data[15:0], i[15:0]);
              
              err_flag = 1'b1;
            end
        end
      
      // Test one location of MRAM
      mm_simtop.axi_write(19'h4_0000, 16'h7075);
      mm_simtop.axi_read(19'h4_0000, rd_data);

      if (rd_data [15:0] !== 16'h7075)
        begin
          err_flag = 1'b1;
        end

      // Test one location of YRAM
      mm_simtop.axi_write(19'h3_0004, 32'hBADA_BABA);
      mm_simtop.axi_read(19'h3_0004, rd_data);

      if (rd_data !== 32'hBADA_BABA)
        begin
          err_flag = 1'b1;
        end

      mm_simtop.endsim(err_flag);
    end

endmodule
