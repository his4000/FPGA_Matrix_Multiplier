`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/31/2018 05:23:46 PM
// Design Name: 
// Module Name: tb_float_to_fixed
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


module tb_float_to_fixed();

reg [31:0]  floating;
wire [31:0] fixed;

float_to_fixed u(
    .float(floating),
    .fixed(fixed)
);

initial begin
    floating <= 32'h40780000;
    #2;
    $finish;
end

endmodule
