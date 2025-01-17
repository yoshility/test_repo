#include <bits/stdc++.h>
#include "./helper.hpp"
#include "./memory.hpp"
#include "./predictor.hpp"
#include "./fpu.hpp"
#include "./fmul_fdiv.hpp"
#include "./uart.hpp"
using namespace std;

#define BUFSIZE     100

const unordered_map<string, int> op_n = {
    {"addi", 1}, {"add", 2}, {"sub", 3}, {"mul", 4}, {"div", 5}, {"slli", 6}, {"luil", 7}, {"beq", 8}, {"bne", 9}, {"blt", 10}, {"lw", 11}, {"sw", 12}, {"fadd", 13}, {"fsub", 14}, {"fmul", 15}, {"fdiv", 16}, {"flw", 17}, {"fsw", 18}, {"fsqrt", 19}, {"fsgnjn", 20}, {"fsgnjx", 21}, {"fsgnj", 22}, {"fcvtsw", 23}, {"fcvtws", 24}, {"feq", 25}, {"flt", 26}, {"jalr", 27}, {"jal", 28}, {"lui", 29}, {"srli", 30}, {"addil", 31}
};

const unordered_map<int, string> n_op = {
    {1, "addi"}, {2, "add"}, {3, "sub"}, {4, "mul"}, {5, "div"}, {6, "slli"}, {7, "luil"}, {8, "beq"}, {9, "bne"}, {10, "blt"}, {11, "lw"}, {12, "sw"}, {13, "fadd"}, {14, "fsub"}, {15, "fmul"}, {16, "fdiv"}, {17, "flw"}, {18, "fsw"}, {19, "fsqrt"}, {20, "fsgnjn"}, {21, "fsgnjx"}, {22, "fsgnj"}, {23, "fcvtsw"}, {24, "fcvtws"}, {25, "feq"}, {26, "flt"}, {27, "jalr"}, {28, "jal"}, {29, "lui"}, {30, "srli"}, {31, "addil"}
};

const unordered_map<string, int> reg_num = {
    {"x0", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"x3",3}, {"tp", 4}, {"x4", 4 }, {"t0", 5}, {"hp", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31}
};

const unordered_map<string, int> freg_num = {
    {"ft0", 0}, {"ft1", 1}, {"ft2", 2}, {"ft3", 3}, {"ft4", 4}, {"ft5", 5}, {"ft6", 6}, {"ft7", 7}, {"fs0", 8}, {"fs1", 9}, {"fa0", 10}, {"fa1", 11}, {"fa2", 12}, {"fa3", 13}, {"fa4", 14}, {"fa5", 15}, {"fa6", 16}, {"fa7", 17}, {"fs2", 18}, {"fs3", 19}, {"fs4", 20}, {"fs5", 21}, {"fs6", 22}, {"fs7", 23}, {"fs8", 24}, {"fs9", 25}, {"fs10", 26}, {"fs11", 27}, {"ft8", 28}, {"ft9", 29}, {"ft10", 30}, {"ft11", 31}
};

// map<reg_num, reg_name>
const map<int, string> reg_name_ {
    {0, "x0"}, {1, "ra"}, {2, "sp"}, {3, "gp"}, {4, "tp"}, {5, "t0/hp"}, {6, "t1"}, {7, "t2"}, {8, "s0/fp"}, {9, "s1"}, {10, "a0"}, {11, "a1"}, {12, "a2"}, {13, "a3"}, {14, "a4"}, {15, "a5"}, {16, "a6"}, {17, "a7"}, {18, "s2"}, {19, "s3"}, {20, "s4"}, {21, "s5"}, {22, "s6"}, {23, "s7"}, {24, "s8"}, {25, "s9"}, {26, "s10"}, {27, "s11"}, {28, "t3"}, {29, "t4"}, {30, "t5"}, {31, "t6"}
};

// map<freg_num, freg_name>
const map<int, string> freg_name_ {
    {0, "ft0"}, {1, "ft1"}, {2, "ft2"}, {3, "ft3"}, {4, "ft4"}, {5, "ft5"}, {6, "ft6"}, {7, "ft7"}, {8, "fs0"}, {9, "fs1"}, {10, "fa0"}, {11, "fa1"}, {12, "fa2"}, {13, "fa3"}, {14, "fa4"}, {15, "fa5"}, {16, "fa6"}, {17, "fa7"}, {18, "fs2"}, {19, "fs3"}, {20, "fs4"}, {21, "fs5"}, {22, "fs6"}, {23, "fs7"}, {24, "fs8"}, {25, "fs9"}, {26, "fs10"}, {27, "fs11"}, {28, "ft8"}, {29, "ft9"}, {30, "ft10"}, {31, "ft11"}
};

int main(int argc, char* argv[]) {
    FILE *in, *in_sld, *out_ppm;
    // assembly
    if ((in = fopen(argv[1], "r")) == NULL) {
        printf("[in] cannot open file\n");
        exit(1);
    }
    // sld
    if ((in_sld = fopen(argv[2], "rb")) == NULL) {
        printf("[in] cannot open file\n");
        exit(1);
    }
    // ppm
    if ((out_ppm = fopen(argv[3], "wb")) == NULL) {
        printf("[out] cannot open file\n");
        exit(1);
    }

    // options
    bool debug = atoi(argv[4]);
    bool use_cache = atoi(argv[5]);
    bool step_by_step = atoi(argv[6]);

    // devices
    Inst *inst_memory;                  // instr memory 構造体版
    inst_memory = (Inst*)malloc(sizeof(Inst) * INST_MEMORY_SIZE);

    int reg[32] = {0};                  // integer register
    float freg[32] = {0.0};             // float register
    Memory memory;                      // data memory
    Cache cache;                        // data cache
    Predictor predictor;                // bimodal predictor
    InputBuffer inBuf;                  // input buffer
    OutputBuffer outBuf;                // output buffer

    // support variables
    char line[BUFSIZE];
    char opcode[30];
    char r0[30];
    char r1[30];
    char r2[30];

    char* inst_cleaned;

    // <step 1> ラベルを探してデータラベルテーブルと関数ラベルテーブルを作る作業
    int addr = 0;                   // 命令アドレス
    int data_addr = 0;              // データセクションでのアドレス
    unordered_map<string, int> func_label;      // 関数ラベル
    unordered_map<string, int> data_label;      // データラベル
    int pc = 0;
    bool is_data = 0;               // 現在データセクションかどうか

    // 時間計測
    chrono::system_clock::time_point start_time, end_time;
    start_time = chrono::system_clock::now();

    while (fgets(line, BUFSIZE, in) != NULL) {
        strcpy(r0, "\0");
        strcpy(r1, "\0");
        strcpy(r2, "\0");       
        inst_cleaned = eliminate_comma_and_comment(line);
        sscanf(inst_cleaned, "%s%s%s%s", opcode, r0, r1, r2);

        // 関数のラベル
        if (opcode[strlen(opcode)-1] == ':' && (!is_data)) {
            // ラベル名とアドレスの組をmapに格納
            opcode[strlen(opcode)-1] = '\0';
            func_label[opcode] = addr;
            if (debug) {
                printf("0x%08X\t%s:\n", addr, opcode);
            }
            // ラベルがmin_caml_startなら、このときのaddrをpcの初期値にする
            if (strncmp(opcode, "min_caml_start", 14) == 0) {
                pc = addr;
                printf("initial pc: 0x%X\n", pc);
            }
        }
        // データのラベル
        else if (opcode[strlen(opcode)-1] == ':' && (is_data)) {
            opcode[strlen(opcode)-1] = '\0';
            data_label[opcode] = data_addr;
        }
        // アセンブリ指令
        else if (opcode[0] == '.') {
            if (strcmp(r0, "\".rodata\"") == 0) {
                is_data = 1;
            } else if (strcmp(r0, "\".text\"") == 0) {
                is_data = 0;
            } else if (strcmp(opcode, ".long") == 0) {
                memory.d[data_addr/4].i = atof(r0);
                data_addr += 4;
            }
        }
        // 普通の命令
        else {
            // また何もしない。後で命令メモリに格納する
            if (debug) {
                printf("0x%08X\t\t%s %s %s %s\n", addr, opcode, r0, r1, r2);
            }
            addr += 4;
        }
    }

    fclose(in);
    // print float table
    // cout << "------ float table ------" << endl;
    // for (auto itr = data_label.begin(); itr != data_label.end(); ++itr) {
    //     cout << "[label] " << itr->first << "\t[addr] " << itr->second << "\t[value] " << memory.d[itr->second/4].f << endl;
    // }

    // print func_label
    // cout << "------ function label ------" << endl << endl;
    // for (auto itr = func_label.begin(); itr != func_label.end(); ++itr) {
    //     cout << itr->first << " : " << itr->second << endl;
    // }
    
    // <step 2> 命令メモリに格納する作業
    // もう一回開く
    if ((in = fopen(argv[1], "r")) == NULL) {
        printf("[in] cannot open file\n");
        exit(1);
    }
    addr = 0;                   // 命令アドレズを0に戻す
    int line_n = 1;             // アセンブリでの行番号
    int opcode_n = 0;           // opcode 番号
    int off = 0;                // offsetのチェック用
    while (fgets(line, BUFSIZE, in) != NULL) {
        strcpy(r0, "\0");
        strcpy(r1, "\0");
        strcpy(r2, "\0");
        inst_cleaned = eliminate_comma_and_comment(line);
        sscanf(inst_cleaned, "%s%s%s%s", opcode, r0, r1, r2);

        // ラベルなら無視
        if (opcode[strlen(opcode)-1] == ':') {
            line_n++;
            continue;
        }
        // アセンブリ指令も無視
        else if (opcode[0] == '.') {
            line_n++;
            continue;
        }

        // 普通の命令なら構造体を作って命令メモリに格納
        opcode_n = op_n.at(opcode);
        switch(opcode_n) {
            case 1: // addi rd, rs1, imm
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), atoi(r2), line_n);
                break;
            case 2: // add rd, rs1, rs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), reg_num.at(r2), line_n);
                break;
            case 3: // sub rd, rs1, rs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), reg_num.at(r2), line_n);
                break;
            case 4: // mul rd, rs1, rs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), reg_num.at(r2), line_n);
                break;
            case 5: // div rd, rs1, rs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), reg_num.at(r2), line_n);
                break;
            case 6: // slli rd, rs1, uimm
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), atoi(r2), line_n);
                break;
            case 7: // luil rd, label
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), data_label[r1], -1, line_n);
                break;
            case 8: // beq rs1, rs2, label
                off = func_label[r2] - addr;
                if (off >= 4096 || off < -4096) {
                    printf("[Step 2] Warning: beq too far offset!\n");
                    printf("         inst addr: %X    line: %d\n", addr, line_n);
                }
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), func_label[r2], line_n);
                break;
            case 9: // bne rs1, rs2, label
                off = func_label[r2] - addr;
                if (off >= 4096 || off < -4096) {
                    printf("[Step 2] Warning: bne too far offset!\n");
                    printf("         inst addr: %X    line: %d\n", addr, line_n);
                }
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), func_label[r2], line_n);
                break;
            case 10: // blt rs1, rs2, label
                off = func_label[r2] - addr;
                if (off >= 4096 || off < -4096) {
                    printf("[Step 2] Warning: blt too far offset!\n");
                    printf("         inst addr: %X    line: %d\n", addr, line_n);
                }
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), func_label[r2], line_n);
                break;
            case 11: // lw rd, imm(rs1)
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), atoi(r1), reg_num.at(r2), line_n);
                break;
            case 12: // sw rs2, imm(rs1)
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), atoi(r1), reg_num.at(r2), line_n);
                break;
            case 13: // fadd fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 14: // fsub fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 15: // fmul fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 16: // fdiv fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 17: // flw fd, imm(rs1)
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), atoi(r1), reg_num.at(r2), line_n);
                break;
            case 18: // fsw fs2, imm(rs1)
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), atoi(r1), reg_num.at(r2), line_n);
                break;
            case 19: // fsqrt fd, fs1
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), -1, line_n);
                break;
            case 20: // fsgnjn fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 21: // fsgnjx fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 22: // fsgnj fd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 23: // fcvtsw fd, rs1
                inst_memory[addr/4] = inst_of(opcode_n, freg_num.at(r0), reg_num.at(r1), -1, line_n);
                break;
            case 24: // fcvtws rd, fs1
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), freg_num.at(r1), -1, line_n);
                break;
            case 25: // feq rd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 26: // flt rd, fs1, fs2
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), freg_num.at(r1), freg_num.at(r2), line_n);
                break;
            case 27: // jalr rd, rs1, imm
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), atoi(r2), line_n);
                break;
            case 28: // jal rd, label
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), func_label[r1], -1, line_n);
                break;
            case 29: // lui rd, imm (big_addiの展開用)
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), atoi(r1), -1, line_n);
                break;
            case 30: // srli rd, rs1, imm (ラベルluiとbig_addiの展開用)
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), atoi(r2), line_n);
                break;
            case 31: // addil rd, rs1, label (ラベルluiの展開用)
                inst_memory[addr/4] = inst_of(opcode_n, reg_num.at(r0), reg_num.at(r1), data_label[r2], line_n);
                break;
            default:
                cout << "[Step 2] Error: unknown inst: " << opcode_n << endl;
                exit(1);
                break;
        }
    
        addr += 4;
        line_n++;
    }
    fclose(in);

    // <step 3> あとは命令メモリを逐次実行
    reg[1] = 1025;                      // first ra = 1025
    reg[2] = MEMORY_SIZE;               // sp = MEMORY_SIZE
    int pre_pc = 0;                     // 変化前のpc
    opcode_n = 0;                       // opcode 番号
    unsigned long long inst_count = 0;  // 命令数
    unsigned long long clk = 4;         // クロック数
    int input_clk = 0;                  // inputバッファ用のクロック（いろんなところで参照するのでここで宣言）
    int output_clk = 0;                 // outputバッファ用のクロック（同上）
    bool pre_inst_is_load = 0;          // 前の命令がロードか
    int pre_load_rd = -1;               // ロードされた目的レジスタ
    int data_hazard_stall = 0;          // ロードによるストール回数
    Inst op;                            // 命令
    char enter;                         // ステップ実行用

    printf("Processing...\n");
    while (1) {
        if (pc == 1025) { // 大元のra
            cout << "pc = initial ra!" << endl;
            break;
        } 
        op = inst_memory[pc/4];

        inst_count++;

        // 書き変わる前のpcとclkを保持
        pre_pc = pc;
        opcode_n = op._opcode;
        switch(opcode_n) {
            case 1: // addi rd, rs1, imm
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] + op._r2;
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 2: // add rd, rs1, rs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << reg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] + reg[op._r2];
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 3: // sub rd, rs1, rs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << reg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] - reg[op._r2];
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 4: // mul rd, rs1, rs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << reg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] * reg[op._r2];
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 5: // div rd, rs1, rs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << reg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] / reg[op._r2];
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 6: // slli rd, rs1, uimm
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] << op._r2;
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 7: // luil rd, label
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << op._r1;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = ((op._r1 / 2048) << 12);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, -2, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 8: // beq rs1, rs2, label
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                if (reg[op._r0] == reg[op._r1]) {
                    pc = op._r2;
                } else {
                    pc += 4;
                }
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r0, op._r1, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                predictor.predict((reg[op._r0]==reg[op._r1]), &clk, debug, &input_clk, &output_clk, outBuf.busy);
                break;
            case 9: // bne rs1, rs2, label
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                if (reg[op._r0] != reg[op._r1]) {
                    pc = op._r2;
                } else {
                    pc += 4;
                }
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r0, op._r1, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                predictor.predict((reg[op._r0]!=reg[op._r1]), &clk, debug, &input_clk, &output_clk, outBuf.busy);
                break;
            case 10: // blt rs1, rs2, label
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                if (reg[op._r0] < reg[op._r1]) {
                    pc = op._r2;
                } else {
                    pc += 4;
                }
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r0, op._r1, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                predictor.predict((reg[op._r0]<reg[op._r1]), &clk, debug, &input_clk, &output_clk, outBuf.busy);
                break;
            case 11: // lw rd, imm(rs1) (input=s10/x26)
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << op._r1 << "(" << reg_name_.at(op._r2) << ")";
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                    printf("\t[lw] addr: %d\n", reg[op._r2]+op._r1);
                }
                // input
                if (op._r2 == 26) {
                    // input++;
                    if (debug) {
                        cout << "\t[lw] int input!" << endl;
                    }
                    char i[10];
                    if ((fscanf(in_sld, "%s", i)) != EOF) {
                        reg[op._r0] = atoi(i);
                    } else {
                        perror("sld file is over!\n");
                    }
                    inBuf.dequeue(&clk, &input_clk);
                }
                // cache
                else if (use_cache) {
                    cache.use_cache(0, reg[op._r2]+op._r1, memory, reg, freg, op._r0, debug, &clk, &input_clk, &output_clk, outBuf.busy);
                }
                // regular lw
                else {
                    reg[op._r0] = memory.d[(reg[op._r2]+op._r1)/4].i;
                }
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r2, -2, 1, op._r0, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            // sw rs2, imm(rs1) (int output=s10/x26; char output=s11/x27)
            case 12:
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << op._r1 << "(" << reg_name_.at(op._r2) << ")";
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                    printf("\t[sw] addr: %d\n", reg[op._r2]+op._r1);
                }
                // int output
                if (op._r2 == 26) {
                    if (debug) {
                        cout << "\t[sw] int output!" << endl;
                    }
                    fprintf(out_ppm, "%d", reg[op._r0]);
                }
                // char output
                else if (op._r2 == 27) {
                    if (debug) {
                        cout << "\t[sw] char output!" << endl;
                    }
                    fprintf(out_ppm, "%c", reg[op._r0]);
                    outBuf.enqueue(&clk, &output_clk);
                }
                // cache
                else if (use_cache) {
                    cache.use_cache(1, reg[op._r2]+op._r1, memory, reg, freg, op._r0, debug, &clk, &input_clk, &output_clk, outBuf.busy);
                }
                // regular sw
                else {
                    memory.d[(reg[op._r2]+op._r1)/4].i = reg[op._r0];
                }
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r0, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 13: // fadd fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = freg[op._r1] + freg[op._r2];
                freg[op._r0] = fadd(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 14: // fsub fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = freg[op._r1] - freg[op._r2];
                freg[op._r0] = fadd(freg[op._r1], -freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 15: // fmul fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }

                // freg[op._r0] = freg[op._r1] * freg[op._r2];
                freg[op._r0] = fmul(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 16: // fdiv fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = freg[op._r1] / freg[op._r2];
                freg[op._r0] = fdiv(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 17: // flw fd, imm(rs1) (input=s11/x27)
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << op._r1 << "(" << reg_name_.at(op._r2) << ")";
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                    printf("\t[flw] addr: %d\n", reg[op._r2]+op._r1);
                }
                // input
                if (op._r2 == 27) {
                    // input++;
                    if (debug) {
                        cout << "\t[flw] float input!" << endl;
                    }
                    char i[10];
                    if ((fscanf(in_sld, "%s", i)) != EOF) {
                        freg[op._r0] = atof(i);
                    } else {
                        perror("sld file is over!\n");
                        exit(1);
                    }
                    inBuf.dequeue(&clk, &input_clk);
                }
                // data section
                else if (reg[op._r2]+op._r1 < 256) {
                    freg[op._r0] = memory.d[(reg[op._r2]+op._r1)/4].f;
                    pc += 4;
                    clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r2, -2, 1, (op._r0+32), &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                    break;
                }
                // cache
                else if (use_cache) {
                    cache.use_cache(2, reg[op._r2]+op._r1, memory, reg, freg, op._r0, debug, &clk, &input_clk, &output_clk, outBuf.busy);
                }
                // regular flw
                else {
                    freg[op._r0] = memory.d[(reg[op._r2]+op._r1)/4].f;
                }
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r2, -2, 1, (op._r0+32), &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 18: // fsw fs2, imm(rs1) (outputには使わない)
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << op._r1 << "(" << reg_name_.at(op._r2) << ")";
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                    printf("\t[fsw] addr: %d\n", reg[op._r2]+op._r1);
                }
                // cache
                if (use_cache) {
                    cache.use_cache(3, reg[op._r2]+op._r1, memory, reg, freg, op._r0, debug, &clk, &input_clk, &output_clk, outBuf.busy);
                }
                // no cache
                else {
                memory.d[(reg[op._r2]+op._r1)/4].f = freg[op._r0];
                }
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r0+32, op._r2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 19: // fsqrt fd, fs1
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = sqrt(freg[op._r1]);
                freg[op._r0] = fsqrt(freg[op._r1]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 20: // fsgnjn fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = fabs(freg[op._r1]) * (-sign(freg[op._r2]));
                freg[op._r0] = fsgnjn(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 21: // fsgnjx fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = freg[op._r1] * sign(freg[op._r2]);
                freg[op._r0] = fsgnjx(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 22: // fsgnj fd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = fabs(freg[op._r1]) * sign(freg[op._r2]);
                freg[op._r0] = fsgnj(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 23: // fcvtsw fd, rs1
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << freg_name_.at(op._r0) << ", " << reg_name_.at(op._r1);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // freg[op._r0] = (float)(reg[op._r1]);
                freg[op._r0] = itof(reg[op._r1]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 24: // fcvtws rd, fs1
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << freg_name_.at(op._r1);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // reg[op._r0] = (int)round(freg[op._r1]); // 最近傍
                reg[op._r0] = ftoi(freg[op._r1]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 25: // feq rd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // reg[op._r0] = (freg[op._r1] == freg[op._r2]);
                reg[op._r0] = feq(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 26: // flt rd, fs1, fs2
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << freg_name_.at(op._r1) << ", " << freg_name_.at(op._r2);
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                // reg[op._r0] = (freg[op._r1] < freg[op._r2]);
                reg[op._r0] = flt(freg[op._r1], freg[op._r2]);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1+32, op._r2+32, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 27: // jalr rd, rs1, imm
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = pc + 4;
                pc = reg[op._r1] + op._r2;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                clk += 2;
                break;
            case 28: // jal rd, label
                if (debug) {
                    printf("####[pc: 0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << op._r1;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = pc + 4;
                pc = op._r1;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, -2, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 29: // lui rd, imm
                if (debug) {
                    printf("####[pc :0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << op._r1;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = (op._r1 << 12);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, -2, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 30: // srli rd, rs1, imm
                if (debug) {
                    printf("####[pc :0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] >> op._r2;
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            case 31: // addil rd, rs1, label -> addi rd, rs1, {0,label[10:0]}
                if (debug) {
                    printf("####[pc :0x%08X | ", pc);
                    cout << n_op.at(op._opcode) << " " << reg_name_.at(op._r0) << ", " << reg_name_.at(op._r1) << ", " << op._r2;
                    printf(" | line: %d | inst_count: %lld]##############################################################################\n", op._line_n, inst_count);
                }
                reg[op._r0] = reg[op._r1] + (op._r2 % 2048);
                pc += 4;
                clk_count(&clk, &pre_inst_is_load, &pre_load_rd, op._r1, -2, 0, -1, &data_hazard_stall, &input_clk, &output_clk, outBuf.busy);
                break;
            default: // others
                printf("[Step 3] Error: unknown inst: %d\tinst_count: %lld\n", opcode_n, inst_count);
                exit(1);
        }

        // print integer register
        if (debug) {
            print_reg(reg);
        }

        // print float register
        if (debug) {
            print_freg(freg);
        }

        reg[0] = 0;
        
        // if (inst_count % 100000000 == 0) {
        //     cout << "now inst count: " << inst_count << endl;
        // }

        // hazardチェック
        // printf("[inst_count|%lld] [clk|%lld] [line|%d] [op|%d] [pre_inst_is_load|%d] [pre_load_rd|%d]\n", inst_count, clk, op._line_n, op._opcode, pre_inst_is_load, pre_load_rd);

        inBuf.enqueue(&input_clk);
        outBuf.dequeue(&output_clk);

        // inputチェック
        // printf("[inst_count|%lld]\t[line|%d]\t[op|%d]\t[clk|%lld]\t[data_n|%d]\t[input_clk|%d]\t[input_count|%d]\t[input_stall|%d]\n", inst_count, op._line_n, op._opcode, clk, inBuf.data_n, input_clk, inBuf.input_count, inBuf.input_stall);

        // outputチェック
        // printf("[inst_count|%lld]\t[line|%d]\t[op|%d]\t[clk|%lld]\t[busy|%d]\t[data_n|%d]\t[output_clk|%d]\t[output_count|%d]\t[output_stall|%d]\n\n", inst_count, op._line_n, op._opcode, clk, outBuf.busy, outBuf.data_n, output_clk, outBuf.output_count, outBuf.output_stall);

        // ステップ実行
        if (step_by_step) {
            if(scanf("%c", &enter) < 0) {
                printf("step by step: enter error\n");
                exit(1);
            }
        }

        // 無限ループ検知
        if (pc == pre_pc) {
            cout << "same pc!: " << pc << endl;
            break;
        }
    }

    end_time = chrono::system_clock::now();
    double elapsed = chrono::duration_cast<chrono::milliseconds>(end_time-start_time).count();

    printf("Finished!\n");
    if (!debug) {
        print_reg(reg);
        print_freg(freg);
    }
    if (use_cache) {
        cache.print_stat();
    }
    predictor.print_stat();
    
    printf("inst_count:\t\t%lld\n", inst_count);
    printf("clk_count:\t\t%lld\n", clk);
    printf("estimated time:\t\t%lf(s) / %lf(min)\n", (double)clk/CLK_HZ, (double)clk/CLK_HZ/60);
    printf("lw stall count:\t\t%d\n", data_hazard_stall);
    printf("sim elapsed time:\t%lf(ms)\n", elapsed);

    return 0;
}