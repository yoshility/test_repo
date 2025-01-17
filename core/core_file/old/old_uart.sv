
module uart_input #(CLK_PER_HALF_BIT = 520) (
  input logic rxd,
  output logic [31:0] input_data_32,
  input logic input_req,
  output logic data_ready,
  input logic clk,
  input logic rstn,
  input logic input_start,
  output logic rts,
  output logic [7:0] rdata,
  output logic rx_ready,
  output logic data_valid,
  input logic [15:0] request_position,
  output logic [31:0] data_buf);
  logic state;
  logic input_state;
  logic [15:0] data_count;
  logic [15:0] tmp_count;
  logic [31:0] data_pool [340:0];
  logic [15:0] position_count;
 // logic [31:0] data_buf;
  typedef enum {first,second,third,fourth} position_state;
  position_state status;
  uart_rx #(CLK_PER_HALF_BIT) u2(rdata, rx_ready, ferr, rxd, clk, rstn);//rdata???output
  logic bit_data_ready;
  assign rts = 1'b1;// (~rstn) ? 1'b1 : (data_count < request_position) ? 1'b1 : ((data_count - request_position) <= 16'd62);
  assign data_valid = (request_position < data_count); //&& ((data_count - request_position) <= 16'd62)) ? 1'b1 : 1'b0;
  assign input_data_32 = data_pool[request_position[8:0]]; 
  always_ff @ (posedge clk) begin
    if (~rstn) begin
      //rts <= 1'b1;
      status <= first;
      state <= 2'b0;
      data_buf <= 32'b0;
      data_count <= 16'b0;
      data_ready <= 1'b0;
      position_count <= 16'b0;
      //rts <= 1'b1;
      input_state <= 1'b0;
    end 
    else if (input_start) begin
      state <= 2'b01;
    end
    if (state == 2'b01) begin
      if (rx_ready) begin
        if (status == first) begin
          data_buf <= (data_buf << 32) + (rdata << 24);
          data_ready <= 1'b0;
          status <= second;
        end if (status == second) begin
          data_buf <= data_buf + (rdata << 16) ;
          status <= third;
        end if (status == third) begin
          data_buf <= data_buf + (rdata << 8);
          status <= fourth;
        end if (status == fourth) begin
          data_buf <= data_buf + rdata;
          status <= first;
          bit_data_ready <= 1'b1;
          data_ready <= 1'b1;;
        end
      end
      if (bit_data_ready) begin
            data_pool[data_count[8:0]] <= data_buf;
          data_count <= data_count + 1;
          data_ready <= 1'b0;
          bit_data_ready <= 1'b0;
      end
//      if ((data_count - request_position) > 16'd62) begin
//        //rts <= 1'b0;
//      end
//      else begin
//        //rts <= 1'b1;
//      end
      /*if (input_req) begin
        if (data_count == 7'b0) begin
            data_valid <= 1'b0;
        end
        else begin
            if (data_count <= input_count) begin
                //data_ready <= 1'b0;
                data_valid <= 1'b0;
             end
             if (data_count > position_count && (data_count - position_count) <= 16'd63) begin
                position_count <= position_count + 1;
                input_data_32 <= data_pool[position_count[5:0]];
                input_state <= 1'b1;
                //data_ready <= 1;
                data_valid <= 1'b1;
             end
         end
      end*/
      /*if (state == 2'b10) begin
        if (rx_ready) begin
        if (status == first) begin
          data_buf <= (data_buf << 32) + (rdata << 24);
          status <= second;
        end if (status == second) begin
          data_buf <= data_buf + (rdata << 16) ;
          status <= third;
        end if (status == third) begin
          data_buf <= data_buf + (rdata << 8);
          status <= fourth;
        end if (status == fourth) begin
          data_buf <= data_buf + rdata;
          status <= first;
          bit_data_ready <= 1'b1;
        end
      end
      if (bit_data_ready) begin
          bit_data_ready <= 1'b0;
          data_pool[data_count[5:0]] <= data_buf;
          data_count <= data_count + 1;
      end
      if (input_req) begin
        if (data_count == 7'b0) begin
            data_valid <= 1'b0;
        end
        else begin
            if (data_count <= input_count) begin
                data_ready <= 1'b0;
                data_valid <= 1'b0;
             end
             if (data_count > (position_count + input_count) && (data_count - (position_count + input_count)) <= 16'd63) begin
                tmp_count <= position_count + input_count;
                input_data_32 <= data_pool[tmp_count[5:0]];
                input_state <= 1'b1;
                data_ready <= 1;
                data_valid <= 1'b1;
             end  
         end
      end
      end*/
    end
    end
  
endmodule

module uart_input_sub #(CLK_PER_HALF_BIT = 520) (
  input wire rxd,
  output logic [31:0] input_data_32,
  input wire input_ready,
  output logic data_ready,
  input wire clk,
  input wire rstn,
  input wire input_start,
  output logic rx_ready,
  output logic [7:0] rdata );
  logic [1:0] status;
  logic [31:0] data_buf;
  wire ferr;
  logic state;
  logic data_valid;
  uart_rx #(CLK_PER_HALF_BIT) u2(rdata, rx_ready, ferr, rxd, clk, rstn);//rdata???output
  always_ff @ (posedge clk) begin
    if (~rstn) begin
      status <= 2'b00;
      state <= 1'b0;
      data_buf <= 32'b0;
      data_ready <= 1'b0;
      data_valid <= 1'b0;
    end 
    else if (input_start) begin
      state <= 1'b1;
    end
    if (state == 1'b1) begin
      if (rx_ready) begin
        if (status == 2'b00) begin
          data_buf <= (data_buf << 32) + (rdata << 24);
          status <= 2'b01;
        end if (status == 2'b01) begin
          data_buf <= data_buf + (rdata << 16) ;
          status <= 2'b10;
        end if (status == 2'b10) begin
          data_buf <= data_buf + (rdata << 8);
          status <= 2'b11;
        end if (status == 2'b11) begin
          data_buf <= data_buf + rdata;
          status <= 2'b00;
          data_valid <= 1'b1;
        end
      end
      if (input_ready) begin
        if (data_valid) begin
          input_data_32 <= data_buf;
          data_ready <= 1'b1;
          data_valid <= 1'b0;
        end
        else begin 
          data_ready <= 1'b0;
        end
      end
    end
  end
endmodule 

module uart_output #(CLK_PER_HALF_BIT = 520 ) (
             output logic txd,
             input logic [31:0] core_data,
             input logic [1:0] output_sig,//{output_signal,4byte{1} or 1byte{0}}
             input logic  clk,
             input logic  rstn,
             output logic [31:0] data_count,
             output logic output_stall,
             output logic output_ready
             );
   logic [7:0]  data;
   logic [31:0] position_count;
   logic               data_valid;
   logic               tx_start;
   logic              tx_busy;
   logic tx_ready;
   (*ram_style = "BLOCK"*) logic [32:0] data_buf [63:0];
   logic [1:0] position;
   uart_tx #(CLK_PER_HALF_BIT) u1(data, tx_start, tx_busy, txd, clk, rstn);//data is input
   logic data_come;
   assign output_stall = ((data_count - position_count) > 32'd63);
   always_ff @(posedge output_sig[1],negedge rstn) begin
      if (~rstn) begin
        data_count <= 32'b0;
      end
      else begin
        data_buf[data_count[5:0]] <= {output_sig[0],core_data};
        data_count <= data_count + 1;
      end
   end

   always_ff @(posedge clk) begin
      if (~rstn) begin
        data <= 8'b0;
        data_valid <= 1'b0;
        tx_start <= 1'b0;
        position_count <= 32'd0;
        position <= 2'b00;
      end
      else begin
        //data is ready
        /*if (output_sig[1]) begin
            data_buf[data_count[3:0]] <= {output_sig[0],core_data};// << (32 * (data_count - position_count)));
            data_count <= data_count + 32'd1;
        end*/
      if ( ~tx_busy && position_count < data_count && (~data_valid)) begin
            data_valid <= 1'b1;
            if (data_buf[position_count[5:0]][32]) begin 
              if (position == 2'b00) begin
                data <= data_buf[position_count[3:0]];
              end
              if (position == 2'b01) begin
                data <= data_buf[position_count[5:0]] >> 8;
              end
              if (position == 2'b10) begin
                data <= data_buf[position_count[5:0]] >> 16;
              end
              if (position == 2'b11) begin
                data <= data_buf[position_count[5:0]] >> 24;
              end
            end
            else begin
              data <= data_buf[position_count[5:0]][7:0];
              position <= 2'b11;
            end
           tx_start <= 1'b1;
        end if (~tx_busy && data_valid) begin
            tx_start <= 1'b0;
            data_valid <= 1'b0;
            if (position == 2'b00) begin
                    position <= 2'b01;
                end
                else if (position == 2'b01) begin
                    position <= 2'b10;
                end
                else if (position == 2'b10) begin
                    position <= 2'b11;
                end
                else if (position == 2'b11) begin
                    position <= 2'b00;
                    position_count <= position_count + 1;
                end
        end
      end
   end
endmodule
