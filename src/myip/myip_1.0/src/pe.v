module my_pe (
        // clk/reset
        input aclk,
        input aresetn,        
        // port A
        input [7:0] ain,        
        // peram -> port B 
        input [7:0] din,
        input term,     
        // integrated valid signal
        input valid,        
        // computation result     
        output dvalid,
        output [31:0] dout
    );
    reg reset_reg;
    
    wire [31:0] dout_fb = (dvalid && !term) ? dout : 'd0;
      
    fixed_point u_fixed_dsp (
        .aclk             (aclk),
        .aresetn          (aresetn),
        .s_axis_a_tvalid  (valid),
        .s_axis_a_tdata   (ain),
        .s_axis_b_tvalid  (valid),
        .s_axis_b_tdata   (din),
        .s_axis_c_tvalid  (valid),
        .s_axis_c_tdata   (dout_fb),
        .m_axis_result_tvalid (dvalid),
        .m_axis_result_tdata  (dout)
   );
       
endmodule

 
