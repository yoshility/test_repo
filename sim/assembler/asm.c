#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./helper.h"

#define BUFSIZE 100

int main(int argc, char* argv[]) {
    FILE *in, *out;
    if ((in = fopen(argv[1], "r")) == NULL) {
        printf("cannot open file\n");
        exit(1);
    }
    if ((out = fopen("../test/binary.txt", "w")) == NULL) {
        printf("cannot open file\n");
        exit(1);
    }

    // options
    int debug = (argv[2] == NULL) ? 0 : 1; // default: no debug

    char line[BUFSIZE];
    char opcode[10];
    char r0[30];
    char r1[30];
    char r2[30];

    char* inst;

    char str[100];

    // 1回目の読みでラベルを探してアドレスを振る /////////////////////////////////////////////////////////////////////
    int addr = -4;
    char label[1000][10]; // ラベル保管庫

    while (fgets(line, BUFSIZE, in) != NULL) {
        addr += 4;

        strcpy(r0, "\0");
        strcpy(r1, "\0");
        strcpy(r2, "\0");
        inst = eliminate_comma_and_comment(line);
        sscanf(inst, "%s%s%s%s", opcode, r0, r1, r2);

        if (opcode[strlen(opcode)-1] == ':') {
            // 配列のaddr/4番目にラベル名を保管
            strcpy(label[addr/4], opcode);
            // 命令アドレスも一緒に出力
            printf("%08X %s\n", addr, opcode);
            addr -= 4;
            continue;
        }

        // 命令アドレスも一緒に出力
        printf("%08X\t%s %s %s %s\n", addr, opcode, r0, r1, r2);
    }

    // 一回閉じてもう一回開く
    fclose(in);
    if ((in = fopen(argv[1], "r")) == NULL) {
        printf("[in] cannot open file\n");
        exit(1);
    }

    addr = -4;
    // 2回目の読みでバイナリに変換 /////////////////////////////////////////////////////////////////////
    while (fgets(line, BUFSIZE, in) != NULL) {
        addr += 4;

        strcpy(r0, "\0");
        strcpy(r1, "\0");
        strcpy(r2, "\0");
        inst = eliminate_comma_and_comment(line);
        sscanf(inst, "%s%s%s%s", opcode, r0, r1, r2);

        // labelだったら
        if (opcode[strlen(opcode)-1] == ':') {
            addr -= 4;
            continue;
        }

        // addi
        if (strncmp(opcode, "addi", 4) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            long long int imm = imm_11_0(r2);
            if (debug) {
                sprintf(str, "%08X %s %012lld %05d %03d %05d %07d", addr, opcode, imm, rs1, F3_ADDI, rd, OP_ADDI);
            } else {
                sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_ADDI, rd, OP_ADDI);  
            }
            fprintf(out, "%s\n", str);
        }
        // li
        else if (strncmp(opcode, "li", 2) == 0) {
            int rd = reg(r0);
            int rs1 = 0;
            long long int imm = imm_11_0(r1);
            sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_ADDI, rd, OP_ADDI);
            fprintf(out, "%s\n", str);
        }
        // add
        else if (strncmp(opcode, "add", 3) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07d %05d %05d %03d %05d %07d", addr, opcode, F7_ADD, rs2, rs1, F3_ADD, rd, OP_ADD);
            } else {
                sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_ADD, rs2, rs1, F3_ADD, rd, OP_ADD);
            }
            fprintf(out, "%s\n", str);
        }
        // mv
        else if (strncmp(opcode, "mv", 2) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = 0;
            sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_ADD, rs2, rs1, F3_ADD, rd, OP_ADD);
            fprintf(out, "%s\n", str);
        }
        // sub
        else if (strncmp(opcode, "sub", 3) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07d %05d %05d %03d %05d %07d", addr, opcode, F7_SUB, rs2, rs1, F3_SUB, rd, OP_SUB);
            } else {
                sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_SUB, rs2, rs1, F3_SUB, rd, OP_SUB);
            }
            fprintf(out, "%s\n", str);
        }
        // fmul
        else if (strncmp(opcode, "fmul", 4) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07d %05d %05d %03d %05d %07d", addr, opcode, F7_FMUL, rs2, rs1, F3_FMUL, rd, OP_FMUL);
            } else {
                sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_FMUL, rs2, rs1, F3_FMUL, rd, OP_FMUL);
            }
            fprintf(out, "%s\n", str);
        }
        // fdiv
        else if (strncmp(opcode, "fdiv", 4) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07d %05d %05d %03d %05d %07d", addr, opcode, F7_FDIV, rs2, rs1, F3_FDIV, rd, OP_FDIV);
            } else {
                sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_FDIV, rs2, rs1, F3_FDIV, rd, OP_FDIV);
            }
            fprintf(out, "%s\n", str);
        }
        // and
        else if (strncmp(opcode, "and", 3) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            int rs2 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07d %05d %05d %03d %05d %07d", addr, opcode, F7_AND, rs2, rs1, F3_AND, rd, OP_AND);
            } else {
                sprintf(str, "%07d%05d%05d%03d%05d%07d", F7_AND, rs2, rs1, F3_AND, rd, OP_AND);
            }
        }
        // beq
        else if (strncmp(opcode, "beq", 3) == 0) {
            int rs1 = reg(r0);
            int rs2 = reg(r1);
            long long int jmp_addr;
            // find label's address
            for (int i=0; i<1000; i++) {
                eliminate_colon(label[i]);
                if (strncmp(label[i], r2, strlen(r2)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            // オフセットは(分岐先アドレス-分岐命令自体のアドレス) -> 本当は/2
            long long int imm = jmp_addr - addr;
            printf("[beq] jmp_addr = %lld, addr = %d\n", jmp_addr, addr);
            // imm[12|10:5]
            long long int imm1 = imm_12_10_5(imm);
            // imm[4:1|11]
            int imm2 = imm_4_1_11(imm);
            if (debug) {
                sprintf(str, "%08X %s %07lld %05d %05d %03d %05d %07d", addr, opcode, imm1, rs2, rs1, F3_BEQ, imm2, OP_BEQ);
            } else {
                sprintf(str, "%07lld%05d%05d%03d%05d%07d", imm1, rs2, rs1, F3_BEQ, imm2, OP_BEQ);
            }
            fprintf(out, "%s\n", str);
        }
        // blt rs1, rs2, label
        else if (strncmp(opcode, "blt", 3) == 0) {
            int rs1 = reg(r0);
            int rs2 = reg(r1);
            long long int jmp_addr;
            for (int i=0; i<1000; i++) {
                eliminate_colon(label[i]);
                if (strncmp(label[i], r2, strlen(r2)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            long long int imm = jmp_addr - addr;
            printf("[blt] jmp_addr = %lld, addr = %d\n", jmp_addr, addr);
            long long int imm1 = imm_12_10_5(imm);
            int imm2 = imm_4_1_11(imm);
            if (debug) {
                sprintf(str, "%08X %s %07lld %05d %05d %03d %05d %07d", addr, opcode, imm1, rs2, rs1, F3_BLT, imm2, OP_BLT);
            } else {
                sprintf(str, "%07lld%05d%05d%03d%05d%07d", imm1, rs2, rs1, F3_BLT, imm2, OP_BLT);
            }
            fprintf(out, "%s\n", str);
        }
        // bge
        else if (strncmp(opcode, "bge", 3) == 0) {
            int rs1 = reg(r0);
            int rs2 = reg(r1);
            // まずラベルr2をアドレスオフセットに変換する
            long long int jmp_addr;
            for (int i=0; i<1000; i++) {
                // ラベルの最後のコロンを消去
                eliminate_colon(label[i]);
                // ラベル名が見つかったら、そのアドレスを返す
                if (strncmp(label[i], r2, strlen(r2)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            // オフセットは(分岐先アドレス-分岐命令自体のアドレス) -> 本当は/2
            long long int imm = jmp_addr - addr;
            // imm[12|10:5]
            long long int imm1 = imm_12_10_5(imm);
            // imm[4:1|11]
            int imm2 = imm_4_1_11(imm);
            if (debug) {
                sprintf(str, "%08X %s %07lld %05d %05d %03d %05d %07d", addr, opcode, imm1, rs2, rs1, F3_BGE, imm2, OP_BGE);
            } else {
                sprintf(str, "%07lld%05d%05d%03d%05d%07d", imm1, rs2, rs1, F3_BGE, imm2, OP_BGE);
            }
            fprintf(out, "%s\n", str);
        }
        // bgt rs1, rs2, label = blt rs2, rs1, label
        else if (strncmp(opcode, "bgt", 3) == 0) {
            int rs2 = reg(r0);
            int rs1 = reg(r1);
            // まずラベルr2をアドレスオフセットに変換する
            long long int jmp_addr;
            for (int i=0; i<1000; i++) {
                // ラベルの最後のコロンを消去
                eliminate_colon(label[i]);
                // ラベル名が見つかったら、そのアドレスを返す
                if (strncmp(label[i], r2, strlen(r2)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            // オフセットは(分岐先アドレス-分岐命令自体のアドレス) -> 本当は/2
            long long int imm = jmp_addr - addr;
            // imm[12|10:5]
            long long int imm1 = imm_12_10_5(imm);
            // imm[4:1|11]
            int imm2 = imm_4_1_11(imm);
            if (debug) {
                sprintf(str, "%08X %s %07lld %05d %05d %03d %05d %07d", addr, opcode, imm1, rs2, rs1, F3_BLT, imm2, OP_BLT);
            } else {
                sprintf(str, "%07lld%05d%05d%03d%05d%07d", imm1, rs2, rs1, F3_BLT, imm2, OP_BLT);
            }
            fprintf(out, "%s\n", str);
        }
        // lw rd, imm(rs1)
        else if (strncmp(opcode, "lw", 2) == 0) {
            int rd = reg(r0);
            long long int imm = imm_11_0(r1);
            int rs1 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %012lld %05d %03d %05d %07d", addr, opcode, imm, rs1, F3_LW, rd, OP_LW);
            } else {
                sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_LW, rd, OP_LW);
            }
            fprintf(out, "%s\n", str);
        }
        // sw rs2, imm(rs1)
        else if (strncmp(opcode, "sw", 2) == 0) {
            int rs2 = reg(r0);
            long long int imm1 = imm_11_5(r1);
            int imm2 = imm_4_0(r1);
            int rs1 = reg(r2);
            if (debug) {
                sprintf(str, "%08X %s %07lld %05d %05d %03d %05d %07d", addr, opcode, imm1, rs2, rs1, F3_SW, imm2, OP_SW);
            } else {
                sprintf(str, "%07lld%05d%05d%03d%05d%07d", imm1, rs2, rs1, F3_SW, imm2, OP_SW);
            }
            fprintf(out, "%s\n", str);
        }
        // jalr(注意：jalの前に置く！)
        else if (strncmp(opcode, "jalr", 4) == 0) {
            int rd = reg(r0);
            int rs1 = reg(r1);
            long long int imm = imm_11_0(r2);
            if (debug) {
                sprintf(str, "%08X %s %012lld %05d %03d %05d %07d", addr, opcode, imm, rs1, F3_JALR, rd, OP_JALR);
            } else {
                sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_JALR, rd, OP_JALR);
            }
            fprintf(out, "%s\n", str);
        }
        // ret = jalr x0, ra, 0
        else if (strncmp(opcode, "ret", 3) == 0) {
            int rd = 0;
            int rs1 = 1; // ra
            long long int imm = 0;
            sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_JALR, rd, OP_JALR);
            fprintf(out, "%s\n", str);
        }
        // jal rd, label
        else if (strncmp(opcode, "jal", 3) == 0) {
            int rd = reg(r0);
            long long int jmp_addr;
            for (int i=0; i<1000; i++) {
                eliminate_colon(label[i]);
                if (strncmp(label[i], r1, strlen(r1)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            printf("label: %s\n", r1);
            printf("[jal] jmp_addr = %lld, addr = %d\n", jmp_addr, addr);
            // imm[20,10:1,11,19:12]
            long long int imm = imm_20_10_1_11_19_12(jmp_addr - addr);
            if (debug) {
                sprintf(str, "%08X %s %020llu %05d %07d", addr, opcode, imm, rd, OP_JAL);
            } else {
                sprintf(str, "%020llu%05d%07d", imm, rd, OP_JAL);
            }
            fprintf(out, "%s\n", str);
        }
        // jr rs1 = jalr x0, rs1, 0
        else if (strncmp(opcode, "jr", 2) == 0) {
            int rd = 0;
            int rs1 = reg(r0);
            long long int imm = 0;
            sprintf(str, "%012lld%05d%03d%05d%07d", imm, rs1, F3_JALR, rd, OP_JALR);
            fprintf(out, "%s\n", str);
        }
        // j
        else if (strncmp(opcode, "j", 1) == 0) {
            int rd = 0;
            long long int jmp_addr;
            for (int i=0; i<1000; i++) {
                eliminate_colon(label[i]);
                if (strncmp(label[i], r0, strlen(r0)) == 0) {
                    jmp_addr = i*4;
                    break;
                }
            }
            long long int imm = imm_20_10_1_11_19_12(jmp_addr - addr);
            printf("[j] jmp_addr = %lld, addr = %d\n", jmp_addr, addr);
            sprintf(str, "%020llu%05d%07d", imm, rd, OP_JAL);
            fprintf(out, "%s\n", str);
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}