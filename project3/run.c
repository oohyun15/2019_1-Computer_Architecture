/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   run.c                                                     */
/*   Adapted from Computer Architecture@KAIST                  */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

#define MEM_NREGIONS (sizeof(MEM_REGIONS) / sizeof(mem_region_t))

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction *get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction()
{

    if (CURRENT_STATE.PIPE_NUM < 5)
        CURRENT_STATE.PIPE_NUM++;

    for (int index = CURRENT_STATE.PIPE_NUM - 1; index >= 0; index--)
    {
        switch (CURRENT_STATE.CURRENT_STAGE[index])
        {
        case IF_STAGE:
            IF_Stage();
            break;

        case ID_STAGE:
            ID_Stage();
            break;

        case EX_STAGE:
            EX_Stage();
            break;

        case MEM_STAGE:
            MEM_Stage();
            break;

        case WB_STAGE:
            WB_Stage();
            break;
        
        case SKIP:
            CURRENT_STATE.CURRENT_STAGE[index] = index;
            break;

        default:
            printf("ERROR!\n");
            assert(0);
        }
    }
    if ((CURRENT_STATE.PC - MEM_TEXT_START) < 4 * NUM_INST && CURRENT_STATE.flag[0] == FALSE) CURRENT_STATE.PC += 0x4;
    else if (CURRENT_STATE.flag[0] == TRUE) CURRENT_STATE.flag[0] = FALSE;
}

void IF_Stage()
{
    int i = (CURRENT_STATE.PC - MEM_TEXT_START) / 4;
    if (i >= NUM_INST)
    {
        CURRENT_STATE.PIPE_STALL[IF_STAGE] = TRUE;
    }

    if (CURRENT_STATE.PIPE_STALL[IF_STAGE] == TRUE)
    {
        CURRENT_STATE.PIPE[IF_STAGE] = 0x0;
        CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PIPE[IF_STAGE];
        CURRENT_STATE.CURRENT_STAGE[1] = ID_STAGE;
        if (CURRENT_STATE.BRANCH_PC != 0x0)
        {
            CURRENT_STATE.PC = CURRENT_STATE.BRANCH_PC;
            CURRENT_STATE.BRANCH_PC = 0x0;
        }
        CURRENT_STATE.PIPE_STALL[IF_STAGE] = FALSE;
        CURRENT_STATE.PIPE_STALL[ID_STAGE] = TRUE;
        return;
    }
    CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
    CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
    CURRENT_STATE.CURRENT_STAGE[1] = ID_STAGE;
    CURRENT_STATE.IF_ID_INST = CURRENT_STATE.PC;
}
void ID_Stage()
{
    if (CURRENT_STATE.PIPE_STALL[ID_STAGE] == TRUE)
    {
        CURRENT_STATE.PIPE[ID_STAGE] = 0x0;
        CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.PIPE[ID_STAGE];
        CURRENT_STATE.CURRENT_STAGE[2] = EX_STAGE;
        CURRENT_STATE.PIPE_STALL[ID_STAGE] = FALSE;
        CURRENT_STATE.PIPE_STALL[EX_STAGE] = TRUE;
        return;
    }
    
    CURRENT_STATE.PIPE[ID_STAGE] = CURRENT_STATE.IF_ID_NPC;

    int i = (CURRENT_STATE.PIPE[1] - MEM_TEXT_START) / 4;

    switch (INST_INFO[i].opcode)
    {
    //Type I
    case 0x9:  //(0x001001)ADDIU
    case 0xc:  //(0x001100)ANDI
    case 0xf:  //(0x001111)LUI
    case 0xd:  //(0x001101)ORI
    case 0xb:  //(0x001011)SLTIU
    case 0x23: //(0x100011)LW
    case 0x2b: //(0x101011)SW
    case 0x4:  //(0x000100)BEQ
    case 0x5:  //(0x000101)BNE
        CURRENT_STATE.ID_EX_IMM = INST_INFO[i].r_t.r_i.r_i.imm;
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs];
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
        break;
    //TYPE R
    case 0x0: //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
        CURRENT_STATE.ID_EX_SHAMT = INST_INFO[i].r_t.r_i.r_i.r.shamt;
        CURRENT_STATE.ID_EX_FUNCT = INST_INFO[i].func_code;
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs];
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
        CURRENT_STATE.ID_EX_REG3 = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
        break;

    //TYPE J
    case 0x2: //(0x000010)J
    case 0x3: //(0x000011)JAL
        CURRENT_STATE.ID_EX_TARGET = INST_INFO[i].r_t.target;
        break;

    default:
        printf("%d Not available instruction\n", i);
        assert(0);
    }
    CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;
    CURRENT_STATE.CURRENT_STAGE[2] = EX_STAGE;
}
void EX_Stage()
{
    if (CURRENT_STATE.PIPE_STALL[EX_STAGE] == TRUE)
    {
        CURRENT_STATE.PIPE[EX_STAGE] = 0x0;
        CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.PIPE[EX_STAGE];
        CURRENT_STATE.CURRENT_STAGE[3] = MEM_STAGE;
        CURRENT_STATE.PIPE_STALL[EX_STAGE] = FALSE;
        CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
        return;
    }

    CURRENT_STATE.PIPE[2] = CURRENT_STATE.ID_EX_NPC;
    int i = (CURRENT_STATE.PIPE[2] - MEM_TEXT_START) / 4;
    int iindex = (CURRENT_STATE.PIPE[3] - MEM_TEXT_START) / 4;
    if (INST_INFO[i].r_t.r_i.rs == CURRENT_STATE.EX_MEM_FORWARD_REG)
    {
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    }
    else if (INST_INFO[i].r_t.r_i.rs == CURRENT_STATE.MEM_WB_FORWARD_REG)
    {
        CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }
    if (INST_INFO[i].r_t.r_i.rt == CURRENT_STATE.EX_MEM_FORWARD_REG)
    {
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
    }
    else if (INST_INFO[i].r_t.r_i.rt == CURRENT_STATE.MEM_WB_FORWARD_REG)
    {
        CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }

    switch (INST_INFO[i].opcode)
    {
    //Type I
    case 0x9: //(0x001001)ADDIU
        if (CURRENT_STATE.flag[1] == TRUE)
        {
            CURRENT_STATE.flag[1] = FALSE;
            CURRENT_STATE.PIPE[EX_STAGE] = 0x0;
            CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
            CURRENT_STATE.CURRENT_STAGE[0] = SKIP;
            CURRENT_STATE.CURRENT_STAGE[1] = SKIP;
            CURRENT_STATE.EX_MEM_FORWARD_REG = 0x0;
            CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0x0;
            break;
        }
        CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
        if (CURRENT_STATE.EX_MEM_ALU_OUT < 0)
            CURRENT_STATE.EX_MEM_ALU_OUT *= (-1);
        INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.EX_MEM_ALU_OUT;
        break;
    case 0xc: //(0x001100)ANDI
        CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & CURRENT_STATE.ID_EX_IMM;
        INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.EX_MEM_ALU_OUT;
        break;
    case 0xf: //(0x001111)LUI
        CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_IMM << 16;
        break;
    case 0xd: //(0x001101)ORI
        CURRENT_STATE.EX_MEM_ALU_OUT = (CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_IMM);
        break;
    case 0xb: //(0x001011)SLTIU

        if ((CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_IMM))
        {
            CURRENT_STATE.EX_MEM_ALU_OUT = 1;
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 1;
        }
        else
        {
            CURRENT_STATE.EX_MEM_ALU_OUT = 0;
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 0;
        }
        break;
    case 0x23: //(0x100011)LW
        CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
        if (CURRENT_STATE.PIPE[0] < MEM_TEXT_START) break;
        int temp = (CURRENT_STATE.PIPE[0] - MEM_TEXT_START) / 4;
        if (((INST_INFO[temp].func_code == 0x0 || INST_INFO[temp].func_code == 0x2 || INST_INFO[temp].func_code == 0x21) 
            && INST_INFO[temp].opcode == 0x0) || INST_INFO[temp].opcode == 0x9
            && (INST_INFO[i].r_t.r_i.rt == INST_INFO[temp].r_t.r_i.rt || INST_INFO[i].r_t.r_i.rt == INST_INFO[temp].r_t.r_i.rs))
        {
            CURRENT_STATE.flag[0] = TRUE;
            CURRENT_STATE.flag[1] = TRUE;
        }        
        break;
    case 0x2b: //(0x101011)SW
        CURRENT_STATE.EX_MEM_W_VALUE = INST_INFO[INST_INFO[i].r_t.r_i.rt].value;
        CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
        break;
    case 0x4: //(0x000100)BEQ
        if (CURRENT_STATE.ID_EX_REG1 == CURRENT_STATE.ID_EX_REG2)
        {
            CURRENT_STATE.BRANCH_PC = CURRENT_STATE.ID_EX_NPC + 4 * (CURRENT_STATE.ID_EX_IMM);
        }
        break;
    case 0x5: //(0x000101)BNE
        if (CURRENT_STATE.ID_EX_REG1 != CURRENT_STATE.ID_EX_REG2)
        {
            CURRENT_STATE.BRANCH_PC = CURRENT_STATE.ID_EX_NPC + 4 * (CURRENT_STATE.ID_EX_IMM);
        }
        break;
    //TYPE R
    case 0x0: //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
        switch (CURRENT_STATE.ID_EX_FUNCT)
        {
        case 0x00: //(0x000000)SLL       
        if (CURRENT_STATE.flag[1] == TRUE)
        {
            CURRENT_STATE.flag[1] = FALSE;
            CURRENT_STATE.PIPE[EX_STAGE] = 0x0;
            CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
            CURRENT_STATE.CURRENT_STAGE[0] = SKIP;
            CURRENT_STATE.CURRENT_STAGE[1] = SKIP;
            CURRENT_STATE.EX_MEM_FORWARD_REG = 0x0;
            CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0x0;
            break;
        }
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 << CURRENT_STATE.ID_EX_SHAMT;
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x02: //(0x000010)SRL
        if (CURRENT_STATE.flag[1] == TRUE)
        {
            CURRENT_STATE.flag[1] = FALSE;
            CURRENT_STATE.PIPE[EX_STAGE] = 0x0;
            CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
            CURRENT_STATE.CURRENT_STAGE[0] = SKIP;
            CURRENT_STATE.CURRENT_STAGE[1] = SKIP;
            CURRENT_STATE.EX_MEM_FORWARD_REG = 0x0;
            CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0x0;
            break;
        }
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 >> CURRENT_STATE.ID_EX_SHAMT;
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x08: //(0x001000)JR
            CURRENT_STATE.PC = CURRENT_STATE.ID_EX_REG1;
            CURRENT_STATE.PIPE_STALL[1] = TRUE;
            break;
        case 0x20: //(0x100000)ADD
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_REG2;
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x21: //(0x100001)ADDU
        if (CURRENT_STATE.flag[1] == TRUE)
        {
            CURRENT_STATE.flag[1] = FALSE;
            CURRENT_STATE.PIPE[EX_STAGE] = 0x0;
            CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
            CURRENT_STATE.CURRENT_STAGE[0] = SKIP;
            CURRENT_STATE.CURRENT_STAGE[1] = SKIP;
            CURRENT_STATE.EX_MEM_FORWARD_REG = 0x0;
            CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0x0;
            break;
        }
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_REG2;
            if (CURRENT_STATE.EX_MEM_ALU_OUT < 0)
                CURRENT_STATE.EX_MEM_ALU_OUT *= (-1);
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x23: //(0x100011)SUBU
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 - CURRENT_STATE.ID_EX_REG2;
            if (CURRENT_STATE.EX_MEM_ALU_OUT < 0)
                CURRENT_STATE.EX_MEM_ALU_OUT *= (-1);
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x24: //(0x100100)AND
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & CURRENT_STATE.ID_EX_REG2;
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x25: //(0x100101)OR
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2;
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x27: //(0x100111)NOR
            CURRENT_STATE.EX_MEM_ALU_OUT = ~(CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2);
            INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
        case 0x2b: //(0x101011)SLTU
            if ((CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_REG2))
            {
                CURRENT_STATE.EX_MEM_ALU_OUT = 1;
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 1;
            }
            else
            {
                CURRENT_STATE.EX_MEM_ALU_OUT = 0;
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 0; // 필요 없을 거 같음
            }
            break;
        }
        break;
    //TYPE J
    case 0x2:                                                                                                            //(0x000010)J
        CURRENT_STATE.PC = (CURRENT_STATE.ID_EX_NPC / 268435456) * 268435456 + (CURRENT_STATE.ID_EX_TARGET << 2);

        CURRENT_STATE.PIPE_STALL[1] = TRUE;
        break;
    case 0x3: //(0x000011)JAL
        CURRENT_STATE.REGS[31] = CURRENT_STATE.ID_EX_NPC + 0x8;
        CURRENT_STATE.PC = (CURRENT_STATE.ID_EX_NPC / 268435456) * 268435456 + (CURRENT_STATE.ID_EX_TARGET << 2);
        CURRENT_STATE.PIPE_STALL[1] = TRUE;
        break;

    default:
        printf("%d Not available instruction\n", INST_INFO[i].opcode);
        assert(0);
    }

    CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.ID_EX_NPC;
    CURRENT_STATE.CURRENT_STAGE[3] = MEM_STAGE;
}
void MEM_Stage()
{
    if (CURRENT_STATE.PIPE_STALL[MEM_STAGE] == TRUE)
    {
        CURRENT_STATE.PIPE[MEM_STAGE] = 0x0;
        CURRENT_STATE.MEM_WB_NPC = CURRENT_STATE.PIPE[MEM_STAGE];
        CURRENT_STATE.CURRENT_STAGE[4] = WB_STAGE;
        CURRENT_STATE.PIPE_STALL[MEM_STAGE] = FALSE;
        CURRENT_STATE.PIPE_STALL[WB_STAGE] = TRUE;
        return;
    }
    CURRENT_STATE.PIPE[3] = CURRENT_STATE.EX_MEM_NPC;
    int i = (CURRENT_STATE.PIPE[3] - MEM_TEXT_START) / 4;

    switch (INST_INFO[i].opcode)
    {
    case 0xf:  //(0x001111)LUI
    case 0xd:  //(0x001101)ORI
        for (int iii = 0; iii < MEM_NREGIONS; iii++)
        {
            if (CURRENT_STATE.EX_MEM_ALU_OUT >= MEM_REGIONS[iii].start && CURRENT_STATE.EX_MEM_ALU_OUT < (MEM_REGIONS[iii].start + MEM_REGIONS[iii].size))
            {
                CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.MEM_WB_MEM_OUT;
                CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
                CURRENT_STATE.EX_MEM_FORWARD_REG = INST_INFO[i].r_t.r_i.rt;
                break;
            }
        }
        CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
        break;    
    case 0x23: //(0x100011)LW
        for (int iii = 0; iii < MEM_NREGIONS; iii++)
        {
            if (CURRENT_STATE.EX_MEM_ALU_OUT >= MEM_REGIONS[iii].start && CURRENT_STATE.EX_MEM_ALU_OUT < (MEM_REGIONS[iii].start + MEM_REGIONS[iii].size))
            {
                CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.MEM_WB_MEM_OUT;
                CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.MEM_WB_MEM_OUT;
                CURRENT_STATE.EX_MEM_FORWARD_REG = INST_INFO[i].r_t.r_i.rt;
                break;
            }
        }
        break;
    case 0x2b: //(0x101011)SW
        mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.EX_MEM_W_VALUE);
        CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
        break;
    case 0xb:
    case 0xc:
    case 0x9:
        CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
        CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
        CURRENT_STATE.EX_MEM_FORWARD_REG = INST_INFO[i].r_t.r_i.rt;
        break;
    case 0x0:
        CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
        CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;
        CURRENT_STATE.EX_MEM_FORWARD_REG = INST_INFO[i].r_t.r_i.r_i.r.rd;
        break;

    case 0x2:
    case 0x3:
        CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0x0;
        CURRENT_STATE.EX_MEM_FORWARD_REG = 0x0;
    }
    if (CURRENT_STATE.BRANCH_PC != 0)
    {
        for (int k = 0; k < 3; k++)
        {
            CURRENT_STATE.PIPE_STALL[k] = TRUE;
        }
    }
    CURRENT_STATE.MEM_WB_NPC = CURRENT_STATE.EX_MEM_NPC;
    CURRENT_STATE.CURRENT_STAGE[4] = WB_STAGE;
}
void WB_Stage()
{
    if (CURRENT_STATE.PIPE_STALL[WB_STAGE] == TRUE)
    {
        CURRENT_STATE.PIPE[WB_STAGE] = 0x0;
        CURRENT_STATE.PIPE_STALL[WB_STAGE] = FALSE;
        return;
    }
    int j = (CURRENT_STATE.MEM_WB_NPC - MEM_TEXT_START) / 4 + 1;
    if (j >= NUM_INST)
    {
        RUN_BIT = FALSE;
    }

    CURRENT_STATE.PIPE[4] = CURRENT_STATE.MEM_WB_NPC;
    int i = (CURRENT_STATE.PIPE[4] - MEM_TEXT_START) / 4;
    switch (INST_INFO[i].opcode)
    {
    //Type I
    case 0x9:  //(0x001001)ADDIU
    case 0xc:  //(0x001100)ANDI
    case 0xf:  //(0x001111)LUI
    case 0xd:  //(0x001101)ORI
    case 0xb:  //(0x001011)SLTIU
        CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = CURRENT_STATE.MEM_WB_ALU_OUT;
        CURRENT_STATE.MEM_WB_FORWARD_VALUE = CURRENT_STATE.MEM_WB_ALU_OUT;
        CURRENT_STATE.MEM_WB_FORWARD_REG = INST_INFO[i].r_t.r_i.rt;
        break;
    case 0x23: //(0x100011)LW
        CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = CURRENT_STATE.MEM_WB_MEM_OUT;
        CURRENT_STATE.MEM_WB_FORWARD_VALUE = CURRENT_STATE.MEM_WB_MEM_OUT;
        CURRENT_STATE.MEM_WB_FORWARD_REG = INST_INFO[i].r_t.r_i.rt;
        break;
    case 0x2b: //(0x101011)SW
        break;

    //TYPE R
    case 0x0: //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
        switch (INST_INFO[i].func_code)
        {
        case 0x00: //(0x000000)SLL
        case 0x02: //(0x000010)SRL
        case 0x20: //(0x100000)ADD
        case 0x21: //(0x100001)ADDU
        case 0x23: //(0x100011)SUBU
        case 0x24: //(0x100100)AND
        case 0x25: //(0x100101)OR
        case 0x27: //(0x100111)NOR
        case 0x2b: // SLTU
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.MEM_WB_ALU_OUT;
            CURRENT_STATE.MEM_WB_FORWARD_VALUE = CURRENT_STATE.MEM_WB_ALU_OUT;
            CURRENT_STATE.MEM_WB_FORWARD_REG = INST_INFO[i].r_t.r_i.r_i.r.rd;
            break;
        default:
            CURRENT_STATE.MEM_WB_FORWARD_VALUE = 0x0;
            CURRENT_STATE.MEM_WB_FORWARD_REG = 0x0;
        }
        break;
    }
    INSTRUCTION_COUNT++;
}