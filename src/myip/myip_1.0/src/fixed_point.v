`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/31/2018 10:50:55 AM
// Design Name: 
// Module Name: fixed_point
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fixed_point (
        input           aclk,
        input           aresetn,
        input           s_axis_a_tvalid,
        input   [7:0]  s_axis_a_tdata,
        input           s_axis_b_tvalid,
        input   [7:0]  s_axis_b_tdata,
        input           s_axis_c_tvalid,
        input   [31:0]  s_axis_c_tdata,
        output          m_axis_result_tvalid,
        output  [31:0]  m_axis_result_tdata
    );
    
    reg [31:0]  dout;
    reg         dvalid;    
    wire        valid = s_axis_a_tvalid && s_axis_b_tvalid && s_axis_c_tvalid;
    wire [16:0] ain = {8'h00, s_axis_a_tdata};
    wire [16:0] bin = {8'h00, s_axis_b_tdata};
    
    assign  m_axis_result_tdata = dout;
    assign  m_axis_result_tvalid = dvalid;
    
    always @(posedge aclk)
        if(!aresetn)
            dout <= 'd0;
        else
            dout <= {16'h0000, ((ain * bin) >> 2)} + s_axis_c_tdata;
            
    always @(posedge aclk)
        if(!aresetn)
            dvalid <= 1'b0;
        else
            if(valid)
                dvalid <= 1'b1;
            else
                dvalid <= 1'b0;
endmodule
