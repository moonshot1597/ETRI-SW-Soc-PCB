
`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/08/15 19:07:58
// Design Name: 
// Module Name: X_mults
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


module X_mults(
    input wire clk,
    input wire rst,
    
    input wire [8:0] x_id,
    
    input wire [8:0] tag_x,
    
    input wire from_pe_ready,
    output wire to_bus_ready,
    
    input wire from_bus_enable,
    output wire to_pe_enable,
    
    input wire [31:0] from_bus_value,
    output reg [31:0] to_pe_value
    );
    
    reg [8:0] x_id_save;
    
    always @(posedge clk, negedge rst)
    begin
        if(!rst)x_id_save <= 0;
        else if(x_id_save != x_id)x_id_save <= x_id;
    end
    
    assign to_pe_enable = from_bus_enable && from_pe_ready && (x_id_save == tag_x);
    
    always @(*)
      begin
         case (to_pe_enable)
            1'b0: to_pe_value = 0;
            1'b1: to_pe_value = from_bus_value;
        endcase
    end   
        
endmodule