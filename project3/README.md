# Project 3. MIPS Pipelined Simulator
Skeleton developed by CMU,
modified for AJOU SCE212.

## Instructions
There are three files you may modify: `util.h`, `run.h`, and `run.c`.

### 1. util.h

We have setup the basic CPU\_State that is sufficient to implement the project.
However, you may decide to add more variables, and modify/remove any misleading variables.

<MODIFY VARIABLES>

* #define SKIP 5
: stall을 구현하는 데 있어 각각의 스테이지를 스킵하기 위한 변수로 사용

[CPU_State_Struct]
* int PIPE_NUM
: 현재 파이프라인 내에 있는 레지스터 수를 나타냄

* int flag[2]
: flag[0]은 stall되는 경우, pc 값 조절을 위해 사용한 플래그,
  flag[1]은 이후 EX_Stage 내에서 stall이 됐는지 확인하는 플래그

* ID_EX_REG1 ~ 3 
: 각각 rs, rt, rd에 해당하는 레지스터를 나타냄

* ID_EX_SHAMT, ID_EX_FUNCT, ID_EX_TARGET
: 각각 R type, J type에 사용할 값


### 2. run.h

You may add any additional functions that will be called by your implementation of `process_instruction()`.
In fact, we encourage you to split your implementation of `process_instruction()` into many other helping functions.
You may decide to have functions for each stages of the pipeline.
Function(s) to handle flushes (adding bubbles into the pipeline), etc.

### 3. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` function to simulate a `cycle` of the pipelined simulator.
Each `cycle()` the pipeline will advance to the next instruction (if there are no stalls/hazards, etc.).
Your internal register, memory, and pipeline register state should be updated according to the instruction
that is being executed at each stage.
