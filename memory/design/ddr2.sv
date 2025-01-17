`default_nettype wire
module ddr2 (
    // DDR2
    output wire [12:0] ddr2_addr,
    output wire [2:0] ddr2_ba,
    output wire ddr2_cas_n,
    output wire [0:0] ddr2_ck_n,
    output wire [0:0] ddr2_ck_p,
    output wire [0:0] ddr2_cke,
    output wire ddr2_ras_n,
    output wire ddr2_we_n,
    inout  wire [15:0] ddr2_dq,
    inout  wire [1:0] ddr2_dqs_n,
    inout  wire [1:0] ddr2_dqs_p,
    output wire [0:0] ddr2_cs_n,
    output wire [1:0] ddr2_dm,
    output wire [0:0] ddr2_odt,
    // others
    input logic clk,
    output logic led
);
    // clock
    logic cpu_clk;
    logic mig_clk;
    clk_wiz_0 clk_gen (
        .clk_in1(clk),
        .clk_out1(mig_clk),
        .clk_out2(cpu_clk)
    );

    // interfaces
    master_fifo master_fifo ();
    slave_fifo slave_fifo ();

    // master
    // 使う線
    wire [27-1:0] addr;
    wire [32-1:0] data;
    wire read_or_write; //read : 1, write : 0
    wire finish; // アクセス完了（出力
    wire rstn;
    //
    cache_controller cache_controller (
        .fifo(master_fifo),
        .clk(cpu_clk),
        .led(led),
        .addr(addr),
        .data(data),
        .read_or_write(read_or_write),
        .finish(finish)
    );

    // fifo
    dram_buf dram_buf (
        .master(master_fifo),
        .slave(slave_fifo)
    );

    // slave
    dram_controller dram_controller (
        // DDR2
        .*,
        // others
        .sys_clk(mig_clk),
        .fifo(slave_fifo)
    );
endmodule
