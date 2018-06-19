`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/31/2018 04:34:06 PM
// Design Name: 
// Module Name: tb_fixed_point
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


module tb_fixed_point();

reg [7:0]  ain;
reg [7:0]  bin;
reg [31:0]  cin;
reg         aclk;
reg         aresetn;
reg         a_tvalid;
reg         b_tvalid;
reg         c_tvalid;
wire    [31:0]  result_data;
wire            result_valid;

always #1   aclk = ~aclk;

fixed_point u_fixed_point(
    .aclk(aclk),
    .aresetn(aresetn),
    .s_axis_a_tvalid(a_tvalid),
    .s_axis_a_tdata(ain),
    .s_axis_b_tvalid(b_tvalid),
    .s_axis_b_tdata(bin),
    .s_axis_c_tvalid(c_tvalid),
    .s_axis_c_tdata(cin),
    .m_axis_result_tvalid(result_valid),
    .m_axis_result_tdata(result_data)
);

initial begin
    ain <= 8'hff;
    bin <= 8'hff;
    cin <= 0;
    aclk <= 1'b1;
    aresetn <= 1'b1;
    a_tvalid <= 1'b0;
    b_tvalid <= 1'b0;
    c_tvalid <= 1'b0;
end

initial begin
    #6;
    a_tvalid = 1'b1;
    b_tvalid = 1'b1;
    c_tvalid = 1'b1;
    #2;
    a_tvalid = 1'b0;
    b_tvalid = 1'b0;
    c_tvalid = 1'b0;
    #10;
    $finish;
end

endmodule
