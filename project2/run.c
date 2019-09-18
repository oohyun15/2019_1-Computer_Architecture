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

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc)
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
    int i = INSTRUCTION_COUNT;
    //printf("%dth instruction, PC: 0x%08x\n", i, CURRENT_STATE.PC);
    if (i >= NUM_INST)
    {
        RUN_BIT = FALSE;
        return;
    }
	switch(INST_INFO[i].opcode)
    {
        //Type I
        case 0x9:		//(0x001001)ADDIU
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] + INST_INFO[i].r_t.r_i.r_i.imm;
            if (CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] < 0) CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] *=(-1);  
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
            break;
        case 0xc:		//(0x001100)ANDI
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] & INST_INFO[i].r_t.r_i.r_i.imm;
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
            break;        
        case 0xf:		//(0x001111)LUI
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = INST_INFO[i].r_t.r_i.r_i.imm*65536;
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = mem_read_32(CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt]);
            break;
        case 0xd:		//(0x001101)ORI
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = (CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] | INST_INFO[i].r_t.r_i.r_i.imm);
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = mem_read_32(CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt]);         
            break;
        case 0xb:		//(0x001011)SLTIU
            
            if ((CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] <  INST_INFO[i].r_t.r_i.r_i.imm))
            {
                CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = 1;
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 1;
            }  
            else
            {
                CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = 0;
                INST_INFO[INST_INFO[i].r_t.r_i.rt].value = 0;
            }               
            if (CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] < 0) CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] *=(-1);  
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
            break;
        case 0x23:		//(0x100011)LW
            CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] = mem_read_32(CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs]+INST_INFO[i].r_t.r_i.r_i.imm);
            INST_INFO[INST_INFO[i].r_t.r_i.rt].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
            break;
        case 0x2b:		//(0x101011)SW
            mem_write_32( CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs]+INST_INFO[i].r_t.r_i.r_i.imm, CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt]);
        case 0x4:		//(0x000100)BEQ
            if (INST_INFO[INST_INFO[i].r_t.r_i.rs].value == INST_INFO[INST_INFO[i].r_t.r_i.rt].value)
                {
                    CURRENT_STATE.PC += 4*(INST_INFO[i].r_t.r_i.r_i.imm);
                    INSTRUCTION_COUNT += INST_INFO[i].r_t.r_i.r_i.imm;
                }
            break;
        case 0x5:		//(0x000101)BNE
            if (INST_INFO[INST_INFO[i].r_t.r_i.rs].value != INST_INFO[INST_INFO[i].r_t.r_i.rt].value)
                {
                    CURRENT_STATE.PC += 4*(INST_INFO[i].r_t.r_i.r_i.imm);
                    INSTRUCTION_COUNT += INST_INFO[i].r_t.r_i.r_i.imm;
                }
            break;
        //TYPE R
        case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            switch(INST_INFO[i].func_code)
            {
                case 0x00:  //(0x000000)SLL
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] << INST_INFO[i].r_t.r_i.r_i.r.shamt;
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x02:  //(0x000010)SRL
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt] >> INST_INFO[i].r_t.r_i.r_i.r.shamt;
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x08:  //(0x001000)JR
                    CURRENT_STATE.PC = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] - 0x4;
                    INSTRUCTION_COUNT = ((CURRENT_STATE.PC - MEM_TEXT_START)/4);
                    break;
                case 0x20:  //(0x100000)ADD
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] + CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x21:  //(0x100001)ADDU
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] + CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
                    if (CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] < 0) CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] *=(-1);  
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x23:  //(0x100011)SUBU
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] - CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
                    if (CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] < 0) CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] *=(-1);                 
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x24:  //(0x100100)AND
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] & CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x25:  //(0x100101)OR
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] | CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt];                    
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x27:  //(0x100111)NOR
                    CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd] = ~(CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rs] | CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.rt]);                    
                    INST_INFO[INST_INFO[i].r_t.r_i.r_i.r.rd].value = CURRENT_STATE.REGS[INST_INFO[i].r_t.r_i.r_i.r.rd];
                    break;
                case 0x2b:  //(0x101011)SLTU
                    break;
            }
            break;

        //TYPE J
        case 0x2:		//(0x000010)J
            CURRENT_STATE.PC = (CURRENT_STATE.PC/268435456)*268435456 + (INST_INFO[i].r_t.target << 2)-0x4;
            INSTRUCTION_COUNT = ((CURRENT_STATE.PC - MEM_TEXT_START)/4);
            break;
        case 0x3:		//(0x000011)JAL
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 0x8;
            CURRENT_STATE.PC = (CURRENT_STATE.PC/268435456)*268435456 + (INST_INFO[i].r_t.target << 2)-0x4;
            INSTRUCTION_COUNT = (int)(CURRENT_STATE.PC - MEM_TEXT_START)/4;
            break;

        default:
            printf("Not available instruction\n");
            //assert(0);
    }
CURRENT_STATE.PC += 0x4;
}
