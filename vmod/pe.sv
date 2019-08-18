
`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/08/16 10:04:49
// Design Name: 
// Module Name: Pe
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
module Pe(
    input wire clk,
    input wire rst,
    input wire from_mult_enable,
    
    //value input
    input wire [31:0] from_mult_weight_value,
    input wire [31:0] from_mult_imap_value,
    input wire [31:0] from_mult_psum_value,
    
    output wire to_mult_ready
    );
    
    enum {IDLE, READ ,CONV, DONE} state_aq, state_next;
        
    //REG in PE
    reg [31:0] in_weight [11:0];
    reg [31:0] in_imap [11:0];
    reg [31:0] in_psum [11:0];
    
    //count for store
    reg [5:0]count_weight;
    reg [5:0]count_imap;
    
    //selected value
    reg [31:0] psum;
    reg [31:0] imap;
    reg [31:0] weight;
    
    //calculation result
    reg [31:0] sum_result;
    wire [31:0] mul_result;
    
    //gogogogogogogogogogo
    wire conv_start;
    wire conv_done;
    
    reg output_size = 3;
    
    //count for calculation
    reg [5:0] count_imap_cal;
    reg [5:0] count_weight_cal;
    reg [5:0] count_one_filter;
    
    assign conv_done = (count_one_filter == 3)? 1:0;
    assign conv_start = (count_imap == 4)? 1:0;
    
        
    // state transition condition
    always @(*)
      begin
          state_next = state_aq;
      case (state_aq)
        IDLE:
          if (from_mult_enable) state_next = READ;
          
        READ:
          if (conv_start) state_next = CONV;
    
        CONV:
          if (conv_done) state_next = DONE;
    
        DONE:
          state_next = IDLE;
      endcase
    end
    
    //state transform
    always @(posedge clk, negedge rst) begin
        if (rst == 1'b0) state_aq <= IDLE;
        else state_aq <= state_next;
    end
      
    //store at weight reg
    always @(posedge clk, negedge rst)
    begin
        if(!rst) begin
        in_weight[0] <= 1'b0;
        in_weight[1] <= 1'b0;
        in_weight[2] <= 1'b0;
        count_weight <= 1'b0;
        end
        
        else if(state_aq == READ) begin
            if(count_weight == 0) begin
            in_weight[0] <= from_mult_weight_value;
            count_weight <= count_weight + 1;
            end
            
            else if(count_weight == 1) begin
            in_weight[1] <= from_mult_weight_value;
            count_weight <= count_weight + 1;
            end
            
            else if(count_weight == 2) begin
            in_weight[2] <= from_mult_weight_value;
            count_weight <= count_weight + 1;
            end
        end
    end
    
    
    //store at imap reg
    always @(posedge clk, negedge rst)
    begin
        if(!rst) begin
        in_imap[0] <= 1'b0;
        in_imap[1] <= 1'b0;
        in_imap[2] <= 1'b0;
        in_imap[3] <= 1'b0;
        in_imap[4] <= 1'b0;
        count_imap <= 1'b0;
        end
        
        else if(state_aq == READ) begin
            if(count_imap == 0) begin
            in_imap[0] <= from_mult_weight_value;
            count_imap <= count_imap + 1;
            end
            
            else if(count_imap == 1) begin
            in_imap[1] <= from_mult_weight_value;
            count_imap <= count_imap + 1;
            end
            
            else if(count_imap == 2) begin
            in_imap[2] <= from_mult_weight_value;
            count_imap <= count_imap + 1;
            end
            
            else if(count_imap == 3) begin
            in_imap[3] <= from_mult_weight_value;
            count_imap <= count_imap + 1;
            end
            
            else if(count_imap == 4) begin
            in_imap[4] <= from_mult_weight_value;
            count_imap <= 0;
            end
       end
    end
    
    always @(posedge clk, negedge rst)
    begin
        if(!rst) begin
        in_psum[0] <= 1'b0;
        in_psum[1] <= 1'b0;
        in_psum[2] <= 1'b0;
        end
        
        else if(count_one_filter == 0) begin
        in_psum[0] <= mul_result + in_psum[0];
        end
        
        else if(count_one_filter == 1) begin
        in_psum[1] <= mul_result + in_psum[1];
        end
        
        else if(count_one_filter == 2) begin
        in_psum[2] <= mul_result + in_psum[2];
        end
    end
    
    
    always @(posedge clk, negedge rst)
    begin
        if(!rst) begin
        count_imap_cal <= 0;
        end
        
        else if(state_aq == CONV && count_imap_cal == 5) begin
            count_imap_cal <= 0;
        end
        
        else if(state_aq == CONV) begin
            count_imap_cal <= count_imap_cal + 1;
        end
        
        
    end
    
    always @(posedge clk, negedge rst)
    begin
        if(!rst) begin
        count_weight_cal <= 0;
        count_one_filter <= 0;
        end
        
        else if(state_aq == CONV && count_one_filter == 2) begin
            count_one_filter <= 0;
        end
        
        else if(state_aq == CONV && count_weight_cal == 2) begin
            count_weight_cal <= 0;
            count_one_filter <= count_one_filter + 1;
        end
        
        
        else if(state_aq == CONV) begin
            count_weight_cal <= count_weight_cal + 1;
        end
        
        
        

    end
    
    //out from reg
    always_comb
    begin
        case(count_imap_cal)
            4'b000: imap =  in_imap[0];
            4'b001: imap =  in_imap[1];
            4'b010: imap =  in_imap[2];
            4'b011: imap =  in_imap[3];
            4'b100: imap =  in_imap[4];
        endcase 
    end
    
    always_comb
    begin
        case(count_weight_cal)
            4'b000: weight =  in_weight[0];
            4'b001: weight =  in_weight[1];
            4'b010: weight =  in_weight[2];
        endcase 
    end
    /*
    always_comb
    begin
        case(count_one_filter)
            4'b000: sum_result = mul_result + in_psum[0];
            4'b001: sum_result = mul_result + in_psum[1];
            4'b010: sum_result = mul_result + in_psum[2];
         endcase
    end
    */
    assign mul_result = weight * imap;

        
endmodule