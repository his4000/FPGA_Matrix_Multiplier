`timescale 1ns / 1ps

module pe_con#(
       parameter VECTOR_SIZE = 64, // vector size
       parameter MATRIX_HEIGHT = 64,
	   parameter L_RAM_SIZE = 4,
	   parameter DATA_WIDTH = 8
    )
    (
        input start,
        output done,
        input aclk,
        input aresetn,
	    
	    // to BRAM
	    output [31:0] BRAM_ADDR,
	    output [31:0] BRAM_WRDATA,
	    output [3:0] BRAM_WE,
	    output BRAM_CLK,
	    input [31:0] BRAM_RDDATA
);
    localparam WD_WIDTH = 32;
    localparam NUM_PE = WD_WIDTH / DATA_WIDTH;
   // PE
    wire [31:0] ain;
    wire [31:0] din;
    wire [L_RAM_SIZE-1:0] addr;
    wire [L_RAM_SIZE-1:0] matrix_addr;
    wire [3:0] we_local;
    wire we_global;
    wire we_output;
    wire valid;
    wire dvalid;
    wire [10:0] rdaddr;
    wire [5:0] wraddr;
    wire [31:0] rddata;
    wire [31:0] dout_val [NUM_PE-1:0];
    wire [NUM_PE-1:0] dvalid_val;
    
    assign dvalid = &dvalid_val;

    reg [31:0] wrdata;
    clk_wiz_0 u_clk (.clk_out1(BRAM_CLK), .clk_in1(aclk));
   
   // global block ram
    reg [31:0] gdout;
    reg [31:0] dout;
    integer i;
    (* ram_style = "block" *) reg [31:0] globalmem [0:VECTOR_SIZE/NUM_PE-1];
    (* ram_style = "block" *) reg [31:0] outputmem [0:MATRIX_HEIGHT-1];
    //FSM
    // transition triggering flags
    wire load_done;
    wire calc_done;
    wire done_done;
    
    wire calc_term;
    wire [5:0] output_addr;
    reg [5:0] output_addr_pre;
    reg [5:0] output_addr_reg;
    
    always @(posedge aclk)
        if (we_global)
            globalmem[addr] <= rddata;
        else
            gdout <= globalmem[addr];
    
    localparam errCorVal = 12'h800;
    
    reg term_pre;
    reg term_reg;
     always @(posedge aclk)
           if(!aresetn)
               term_pre <= 'd0;
           else
               if(calc_term)
                   term_pre <= 'd1;
               else
                   term_pre <= 'd0;
       
    always @(posedge aclk)
        if(!aresetn)
            term_reg <= 'd0;
        else
            term_reg <= term_pre;
               
    always @(posedge aclk)
        if (term_reg)
            outputmem[output_addr_reg] = dout_val[0] + dout_val[1] + dout_val[2] + dout_val[3] + errCorVal;
	    
    // state register
    reg [3:0] state, state_d;
    localparam S_IDLE = 4'd0;
    localparam S_LOAD = 4'd1;
    localparam S_CALC = 4'd2;
    localparam S_DONE = 4'd3;

	//part 1: state transition
    always @(posedge aclk)
        if (!aresetn)
            state <= S_IDLE;
        else
            case (state)
                S_IDLE:
                    state <= (start)? S_LOAD : S_IDLE;
                S_LOAD: // LOAD PERAM
                    state <= (load_done)? S_CALC : S_LOAD;
                S_CALC: // CALCULATE RESULT
                    state <= (calc_done)? S_DONE : S_CALC;
                S_DONE:
                    state <= (done_done)? S_IDLE : S_DONE;
                default:
                    state <= S_IDLE;
            endcase
    
    always @(posedge aclk)
        if (!aresetn)
            state_d <= S_IDLE;
        else
            state_d <= state;

	//part 2: determine state
    // S_LOAD
    reg load_flag;
    wire load_flag_reset = !aresetn || load_done;
    wire load_flag_en = (state_d == S_IDLE) && (state == S_LOAD);
    localparam CNTLOAD1 = VECTOR_SIZE / 2 -1;
    always @(posedge aclk)
        if (load_flag_reset)
            load_flag <= 'd0;
        else
            if (load_flag_en)
                load_flag <= 'd1;
            else
                load_flag <= load_flag;
    
    // S_CALC
    
    reg calc_flag;
    wire calc_flag_reset = !aresetn || calc_done;
    wire calc_flag_en = (state_d == S_LOAD) && (state == S_CALC);  
    localparam CNTCALC1 = VECTOR_SIZE * MATRIX_HEIGHT / 2 - 1;
    always @(posedge aclk)
        if (calc_flag_reset)
            calc_flag <= 'd0;
        else
            if (calc_flag_en)
                calc_flag <= 'd1;
            else
                calc_flag <= calc_flag;
    
    // S_DONE
    reg done_flag;
    wire done_flag_reset = !aresetn || done_done;
    wire done_flag_en = (state_d == S_CALC) && (state == S_DONE);
    localparam CNTDONE = MATRIX_HEIGHT * 2 - 1;
    always @(posedge aclk)
        if (done_flag_reset)
            done_flag <= 'd0;
        else
            if (done_flag_en)
                done_flag <= 'd1;
            else
                done_flag <= done_flag;
    
    // down counter
    reg [31:0] counter;
    wire [31:0] ld_val = (load_flag_en)? CNTLOAD1 :
                         (calc_flag_en)? CNTCALC1 : 
                         (done_flag_en)? CNTDONE  : 'd0;
    wire counter_ld = load_flag_en || calc_flag_en || done_flag_en;
    wire counter_en = load_flag || calc_flag || done_flag;
    wire counter_reset = !aresetn || load_done || calc_done || done_done;
    always @(posedge aclk)
        if (counter_reset)
            counter <= 'd0;
        else
            if (counter_ld)
                counter <= ld_val;
            else if (counter_en)
                counter <= counter - 1;
    
    //part3: update output and internal register
    //S_LOAD: we
	assign we_global = (load_flag && load_flag && !counter[0]) ? 'd1 : 'd0;
	assign we_output = (done_flag && !counter[0]) ? 'd1 : 'd0;
	
	//S_CALC: wrdata
   always @(posedge aclk)
        if (!aresetn)
                wrdata <= 'd0;
        else
            if (done_flag || we_output)
                    wrdata <= outputmem[wraddr];
            else
                    wrdata <= wrdata;
	//S_CALC: valid
    reg valid_pre, valid_reg;
    always @(posedge aclk)
        if (!aresetn)
            valid_pre <= 'd0;
        else
            if (counter_ld || counter_en)
                valid_pre <= 'd1;
    
    always @(posedge aclk)
        if (!aresetn)
            valid_reg <= 'd0;
        else if (calc_flag)
            valid_reg <= valid_pre;
     
    assign valid = (calc_flag) && valid_reg;
    
	//S_CALC: ain
	localparam IPT_ADDR_WIDTH = 10;
	assign ain = (calc_flag)? gdout : 'd0;
    assign rdaddr = (state == S_LOAD)? counter[L_RAM_SIZE:1] :
                    (state == S_CALC)? counter[IPT_ADDR_WIDTH-1:0] : 'd0;

	//S_LOAD&&CALC
    assign addr = (load_flag)? counter[L_RAM_SIZE:1]:
                  (calc_flag)? counter[L_RAM_SIZE-1:0]:'d0;
    assign matrix_addr = (load_flag)? counter[L_RAM_SIZE+L_RAM_SIZE:L_RAM_SIZE+1]: 
                          (calc_flag)? counter[L_RAM_SIZE*2-1:L_RAM_SIZE]:'d0;
    assign wraddr = (done_flag)? counter[6:1]:'d0;
    
	//S_LOAD
	reg [31:0] rddata_reg;
	always @(posedge aclk)
	   if(!aresetn)
	       rddata_reg <= 'd0;
	   else if(calc_flag)
	       rddata_reg <= rddata;
	assign din = (calc_flag)? rddata_reg : 'd0;

	//done signals
	reg delayed_calc_done_reg;
	wire delayed_calc_done = (calc_flag) && (counter == 'd0) && dvalid;
	
    assign load_done = (load_flag) && (counter == 'd0);
    assign calc_done = delayed_calc_done_reg;
    assign done_done = (done_flag) && (counter == 'd0);
    
    always @(posedge aclk)
        if(delayed_calc_done)
            delayed_calc_done_reg <= 'd1;
        else
            delayed_calc_done_reg <= 'd0;    
    assign calc_term = (calc_flag) && (counter[3:0] == 4'h0) && dvalid;
    assign output_addr = (calc_flag)? counter[9:4] : 'd0;
    
    always @(posedge aclk)
        if(!aresetn)
            output_addr_pre <= 'd0;
        else
            output_addr_pre <= output_addr;
    
    always @(posedge aclk)
        if(!aresetn)
            output_addr_reg <= 'd0;
        else
            output_addr_reg <= output_addr_pre;
    
    assign done = (state == S_IDLE) && (state_d == S_DONE);
    
    // BRAM interface
    localparam IPT_BASE_IDX = 1040;
    assign rddata = BRAM_RDDATA;
    assign BRAM_WRDATA = wrdata;
    assign BRAM_ADDR = (load_flag)? {7'h40, rdaddr[L_RAM_SIZE-1:0], 2'b00} :
                        (calc_flag)? {rdaddr, 2'b00} : 
                        (done_flag)? (IPT_BASE_IDX + wraddr) * 4 : 'd0;
    assign BRAM_WE = (done_flag)? 4'hF : 0;
    // Make PE array
    genvar k;
    generate for(k=0;k<NUM_PE;k=k+1) begin
         my_pe u_pe (
           .aclk(aclk),
           .aresetn(aresetn && (state != S_DONE)),
           .ain(ain[8*(k+1)-1:8*k]),
           .din(din[8*(k+1)-1:8*k]),
           .term(term_reg),
           .valid(valid),
           .dvalid(dvalid_val[k]),
           .dout(dout_val[k])
       );

    end
    endgenerate

endmodule
