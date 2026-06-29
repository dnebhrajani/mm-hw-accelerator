module mult_test;
  
  integer i;
  integer j;
  integer n;
  reg [31:0] id, cycle_count;
  reg     err_flag;
  
  
  initial
    begin
      //----------------------------------------------------------------------
      // Set simulation variables
      //----------------------------------------------------------------------
      err_flag = 1'b0;
      n = mm_simtop.N;

      // create x vector
      for (i = 0; i < n; i = i+1) 
        begin
          mm_simtop.X[i] = (i+1);
        end

      // create m matrix
      for (i = 0; i < n; i = i+1)
        begin
          for (j = 0; j < n; j = j+1)
            begin
              if (i == j)
                begin
                  mm_simtop.M[i][j] = 1;
                end
              
              else
                begin
                  mm_simtop.M[i][j] = 0;
                end
            end
        end // for (i = 0; i < n; i = i+1)

      //----------------------------------------------------------------------
      // Program and run the device
      //----------------------------------------------------------------------
      // Reset the device
      mm_simtop.sys_reset;

      // Read version number
      mm_simtop.axi_read(32'h4030_0000, id);
      
      // set n
      mm_simtop.axi_write(32'h4030_000C, n);

      // load xram
      mm_simtop.load_x(n);

      // load mram
      mm_simtop.load_m(n);
      
      // run multiplier
      mm_simtop.axi_write(32'h4030_0004, 1);

      // wait till done bit is set
      mm_simtop.poll_for_done(n);

      // get values
      mm_simtop.read_y(n);

      for (i = 0; i < n; i = i+1)
        begin
          if (mm_simtop.Y[i] !== (i+1))
            begin
              $display("Incorrect Y[%d] = %h, expected %h", i, mm_simtop.Y[i], (i+1));
              err_flag = 1'b1;
            end
        end

      mm_simtop.axi_read(32'h4030_0010, cycle_count);
      $display("Matrix multiplier took %0d clocks for matrix size %0dx%0d", cycle_count, n, n);

      mm_simtop.endsim(err_flag);
    end

endmodule
