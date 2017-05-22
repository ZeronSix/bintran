/*!
 x86arch.cpp - contains x86 representations of ZVM commands.
 Copyright 2017 Vyacheslav "ZeronSix" Zhdanovskiy <zeronsix@gmail.com>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "bintran.hpp"
#include "exceptions.hpp"
#include "datatools.hpp"
#include <cstring>

namespace zvm {

#define EMIT_CODE() { std::memcpy(ptr, code, sizeof(code)); ptr += sizeof(code); }
#define EMIT_DATA() { EmitAndShiftBuf(ptr, instr.arg); }

void BinTran::WriteCodeHeader(Byte*& ptr) {
    Byte code[] = {
        0x49, 0x89, 0xE2,        // mov r10, rsp
        0x49, 0x83, 0xEA, 0x08,  // sub r10, 8
        0x49, 0x89, 0xFB,        // mov r11, rdi (input func)
        0x49, 0x89, 0xF4,        // mov r12, rsi (output func)
        0x49, 0x89, 0xE5         // mov r13, rsp
    };

    EMIT_CODE();
}

inline void WriteHalt(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x4C, 0x89, 0xEC, // mov rsp, r13
        0xC3              // ret
    };
    EMIT_CODE();
}

void BinTran::WriteCodeFooter(Byte*& ptr) {
    BtInstr instr = { .opcode = OPCODE_HALT };
    WriteHalt(ptr, instr);
}

inline void WritePush(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x68  // push IMM
    };

    EMIT_CODE();
    EMIT_DATA();
}

inline void WriteLoad(Byte*& ptr, const BtInstr& instr) {
    {
        Byte code[] = {
            0x48, 0x31, 0xC0, // xor rax, rax
            0x41, 0x8B, 0x82  // mov [r10+IMM]
        };

        EMIT_CODE();
    }
    EmitAndShiftBuf(ptr, -8 * instr.arg);
    {
        Byte code[] = {
            0x50 // push rax
        };

        EMIT_CODE();
    }
}

inline void WriteStore(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,             // pop rax
        0x49, 0x89, 0x82  // mov [r10+IMM]
    };

    EMIT_CODE();
    EmitAndShiftBuf(ptr, -8 * instr.arg);
}

inline void WritePop(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58  // pop rax
    };

    EMIT_CODE();
}

inline void LoadOperands(Byte*& ptr, const BtInstr& instr) {
    if (instr.op2_loc == DATALOC_STACK) {
        Byte code[] = {
            0x41, 0x58        // pop r8
        };
        EMIT_CODE();
    }
    else if (instr.op2_loc == DATALOC_R9) {
        Byte code[] = {
            0x4D, 0x89, 0xC8  // mov rax, r9
        };
        EMIT_CODE();
    }
    else if (instr.op2_loc == DATALOC_R14) {
        Byte code[] = {
            0x4D, 0x89, 0xF0  // mov rax, r14
        };
        EMIT_CODE();
    }

    if (instr.op1_loc == DATALOC_STACK) {
        Byte code[] = {
            0x58  // pop rax
        };
        EMIT_CODE();
    }
    else if (instr.op1_loc == DATALOC_R9) {
        Byte code[] = {
            0x4C, 0x89, 0xC8  // mov rax, r9
        };
        EMIT_CODE();
    }
    else if (instr.op1_loc == DATALOC_R14) {
        Byte code[] = {
            0x4C, 0x89, 0xF0  // mov rax, r9
        };
        EMIT_CODE();
    }
}

inline void WriteResult(Byte*& ptr, const BtInstr& instr) {
    if (instr.res_loc == DATALOC_STACK) {
        Byte code[] = {
            0x50  // push rax
        };
        EMIT_CODE();
    }
    else if (instr.res_loc == DATALOC_R9) {
        Byte code[] = {
            0x49, 0x89, 0xC1
        };
        EMIT_CODE();
    }
    else if (instr.res_loc == DATALOC_R14) {
        Byte code[] = {
            0x49, 0x89, 0xC6
        };
        EMIT_CODE();
    }
}

inline void WriteAdd(Byte*& ptr, const BtInstr& instr) {
    LoadOperands(ptr, instr);

    Byte code[] = {
        0x4C, 0x01, 0xC0,  // add rax, r8
    };
    EMIT_CODE();

    WriteResult(ptr, instr);
}

inline void WriteSub(Byte*& ptr, const BtInstr& instr) {
    LoadOperands(ptr, instr);

    Byte code[] = {
        0x4C, 0x29, 0xC0,  // sub rax, r8
    };
    EMIT_CODE();

    WriteResult(ptr, instr);
}

inline void WriteMul(Byte*& ptr, const BtInstr& instr) {
    LoadOperands(ptr, instr);

    Byte code[] = {
        0x49, 0x0F, 0xAF, 0xC0  // imul rax, r8
    };
    EMIT_CODE();

    WriteResult(ptr, instr);
}

inline void WriteDiv(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x41, 0x58,        // pop r8
        0x58,              // pop rax
        0x49, 0xF7, 0xF8,  // idiv r8
        0x50               // push rax
    };

    EMIT_CODE();
}

// TODO: refactor this
inline void WriteJump(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0xE9  // jump (relative)
    };
    EMIT_CODE();

    EmitAndShiftBuf(ptr, (int32_t)(0));
}

inline void WriteCall(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0xE8  // call (relative)
    };
    EMIT_CODE();

    EmitAndShiftBuf(ptr, (int32_t)(0));
}

inline void WriteJmc(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                   // pop rax
        0x48, 0x83, 0xF8, 0x00, // cmp rax, 0
        0x0F, 0x85              // jne
    };
    EMIT_CODE();

    EmitAndShiftBuf(ptr, (int32_t)(0));
}

inline void WriteGz(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x4F, 0xC0,             // cmovg rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteGez(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x4D, 0xC0,             // cmovge rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteBz(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x4C, 0xC0,             // cmovl rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteBez(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x4E, 0xC0,             // cmovle rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteEqz(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x44, 0xC0,             // cmove rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteNeqz(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x58,                               // pop rax
        0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, // mov r8, 1
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0
        0xB8, 0x00, 0x00, 0x00, 0x00,       // mov rax, 0
        0x49, 0x0F, 0x45, 0xC0,             // cmovne rax, r8
        0x50                                // push rax
    };

    EMIT_CODE();
}

inline void WriteInput(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x41, 0x52,        // push r10
        0x41, 0x53,        // push r11
        0x41, 0xFF, 0xD3,  // call r11
        0x41, 0x5B,        // pop r11
        0x41, 0x5A,        // pop r10
        0x50               // push rax
    };

    EMIT_CODE();
}

inline void WriteOutput(Byte*& ptr, const BtInstr& instr) {
    Byte code[] = {
        0x5F,               // pop rdi
        0x41, 0x52,         // push r10
        0x41, 0x53,         // push r11
        0x41, 0xFF, 0xD4,   // call r12
        0x41, 0x5B,         // pop r11
        0x41, 0x5A,         // pop r10
    };

    EMIT_CODE();
}

void BinTran::WriteInstr(Byte*& ptr, BtInstr& instr) {
    instr.x86_addr = ptr - (Byte*)translated_code_;
    zvmaddr_map_[instr.zvm_addr] = &instr;

    // write command macro
#define WRT(instrname) Write ## instrname (ptr, instr);
    switch (instr.opcode) {
        case OPCODE_HALT:
            WRT(Halt);
            break;
        case OPCODE_PUSH:
            WRT(Push);
            break;
        case OPCODE_POP:
            WRT(Pop);
            break;
        case OPCODE_ADD:
            WRT(Add);
            break;
        case OPCODE_SUB:
            WRT(Sub);
            break;
        case OPCODE_MUL:
            WRT(Mul);
            break;
        case OPCODE_DIV:
            WRT(Div);
            break;
        case OPCODE_JMP:
            WRT(Jump);
            break;
        case OPCODE_JMC:
            WRT(Jmc);
            break;
        case OPCODE_CALL:
            WRT(Call);
            break;
        case OPCODE_GZ:
            WRT(Gz);
            break;
        case OPCODE_GEZ:
            WRT(Gez);
            break;
        case OPCODE_BZ:
            WRT(Bz);
            break;
        case OPCODE_BEZ:
            WRT(Bez);
            break;
        case OPCODE_LOAD:
            WRT(Load);
            break;
        case OPCODE_STORE:
            WRT(Store);
            break;
        case OPCODE_EQZ:
            WRT(Eqz);
            break;
        case OPCODE_NEQZ:
            WRT(Neqz);
            break;
        case OPCODE_INPUT:
            WRT(Input);
            break;
        case OPCODE_OUTPUT:
            WRT(Output);
            break;
        default:
            throw UndefinedOpcodeException(instr.opcode);
    }
#undef WRT
}
#undef EMIT_DATA
#undef EMIT_CODE

}  // namespace zvm
