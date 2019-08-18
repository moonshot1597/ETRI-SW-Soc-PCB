`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/08/15 19:07:58
// Design Name: 
// Module Name: Y_mults
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
    
module Y_mults(
    input wire clk,
    input wire rst,
    input wire [8:0] y_id,
    
    input wire [8:0] tag_y,
    
    input wire [8:0] from_control_tag_x,
    output reg [8:0] to_Xbus_tag_x,
       
    input wire from_Xbus_ready,
    output wire to_control_ready,
    
    input wire from_control_enable,
    output wire to_Xbus_enable,
    
    input wire [31:0] from_control_value,
    output reg [31:0] to_Xbus_value
    ); 
    
    reg [8:0] y_id_save;
    
    //y_id_save D-ff
    always @(posedge clk, negedge rst)
    begin
        if(!rst)y_id_save <= 0;
        else if(y_id_save != y_id)y_id_save <= y_id;
    end
    
    assign to_Xbus_enable = from_control_enable && to_control_ready && (y_id_save == tag_y);
    
    always @(*)
      begin
         case (to_Xbus_enable)
            1'b0: to_Xbus_value = 0;
            1'b1: to_Xbus_value = from_control_value;
        endcase
    end   

    //pass or non pass to Xbus
    always @(*)
      begin
         case (to_Xbus_enable)
            1'b0: to_Xbus_tag_x = 0;
            1'b1: to_Xbus_tag_x = from_control_tag_x;
        endcase
    end
        
endmodule
