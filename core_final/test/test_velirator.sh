verilator --top-module fadd -cc fadd.sv
verilator --top-module fmul -cc fmul.sv
verilator --top-module fdiv -cc finv.sv
verilator --top-module sqrt -cc sqrt.sv
verilator --top-module flt -cc fcomp.sv
verilator --top-module feq -cc fcomp.sv
verilator --top-module fle -cc fcomp.sv
verilator --top-module fabs -cc fsign.sv
verilator --top-module fneg -cc fsign.sv
verilator --top-module fsgnj -cc fsign.sv
verilator --top-module fsgnjn -cc fsign.sv
verilator --top-module fsgnjx -cc fsign.sv
verilator --top-module ftoi -cc convert.sv
verilator --top-module itof -cc convert.sv
