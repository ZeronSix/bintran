/*!
 bintran.cpp - binary translator from stack process architecture to x86.
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
#include "datatools.hpp"
#include "exceptions.hpp"
#include "x86arch.hpp"
#include <experimental/filesystem>
#include <sys/mman.h>

namespace fs = std::experimental::filesystem;

namespace zvm {

BinTran::BinTran(std::size_t alloc_size)
    : zvmbinary_(nullptr),
      zvmbinary_size_(0),
      translated_code_(AllocWriteableMemory(alloc_size)),
      allocated_size_(alloc_size),
      actual_x86_size_(0) {}

BinTran::~BinTran() {
    delete[] zvmbinary_;
    if (translated_code_)
        munmap((void*)translated_code_, allocated_size_);
}

void BinTran::LoadBinary(const std::string& filename) {
    if (!fs::exists(filename))
        throw IoException(filename, ERR_FILE_DOESNT_EXIST);

    std::size_t filesize = fs::file_size(filename);
    std::FILE* f = std::fopen(filename.c_str(), "rb");
    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    if (zvmbinary_)
        delete[] zvmbinary_;
    zvmbinary_ = new Byte[filesize]();
    if (!zvmbinary_)
        throw AllocException();

    std::fread(zvmbinary_, 1, filesize, f);
    std::fclose(f);

    zvmbinary_size_ = filesize;
}

inline void InitDataLocations(BtInstr& btinstr) {
    switch (btinstr.opcode) {
        case OPCODE_POP:
        case OPCODE_HALT:
        case OPCODE_POPBP:
        case OPCODE_PUSHBP:
            btinstr.op1_loc = DATALOC_NONE;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_NONE;
            break;
        case OPCODE_PUSH:
        case OPCODE_LOAD:
        case OPCODE_STORE:
            btinstr.op1_loc = DATALOC_IMM;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_STACK;
            break;
        case OPCODE_CALL:
        case OPCODE_JMP:
        case OPCODE_JMC:
            btinstr.op1_loc = DATALOC_IMM;
            btinstr.op2_loc = DATALOC_STACK;
            btinstr.res_loc = DATALOC_NONE;
            break;
        case OPCODE_RET:
            btinstr.op1_loc = DATALOC_STACK;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_NONE;
            break;
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
            btinstr.op1_loc = DATALOC_STACK;
            btinstr.op2_loc = DATALOC_STACK;
            btinstr.res_loc = DATALOC_STACK;
            break;
        case OPCODE_GZ:
        case OPCODE_BZ:
        case OPCODE_BEZ:
        case OPCODE_GEZ:
        case OPCODE_EQZ:
        case OPCODE_NEQZ:
            btinstr.op1_loc = DATALOC_STACK;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_STACK;
            break;
        case OPCODE_INPUT:
            btinstr.op1_loc = DATALOC_STDIN;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_STACK;
            break;
        case OPCODE_OUTPUT:
            btinstr.op1_loc = DATALOC_STACK;
            btinstr.op2_loc = DATALOC_NONE;
            btinstr.res_loc = DATALOC_STDOUT;
            break;
        default:
            throw UndefinedOpcodeException(btinstr.opcode);
    }
}

void BinTran::Translate() {
    Register pc = 0;

    while (pc < zvmbinary_size_) {
        Register bpc = pc;
        DecodedInstr instr = FetchInstr(zvmbinary_, pc);
        BtInstr btinstr = { .opcode = instr.opcode,
                            .arg = instr.args[0],
                            .zvm_addr = std::size_t(bpc) };

        InitDataLocations(btinstr);
        program_.push_back(btinstr);
        zvmaddr_map_[bpc] = &program_.back();
    }

    // scan for jumps
    for (auto it = program_.begin(); it != program_.end(); it++) {
        if (it->opcode == OPCODE_JMP || it->opcode == OPCODE_JMC ||
            it->opcode == OPCODE_CALL) {
            jmp_map_[&(*it)] = zvmaddr_map_[it->arg];
        }
    }

    Optimize();

    Byte* program_ptr = (Byte*)translated_code_;
    WriteCodeHeader(program_ptr);
    for (auto it = program_.begin(); it != program_.end(); it++) {
        WriteInstr(program_ptr, *it);
    }
    WriteCodeFooter(program_ptr);

    // patch jumps
    for (const auto& it: jmp_map_) {
        const int PATCH_OFFSET = 1;
        const int JMC_ADD_OFFSET = 6;
        const int JUMP_OFFSET = -5;

        BtInstr* source = it.first;
        BtInstr* dest = it.second;

        int src_x86_addr = source->x86_addr;
        int dest_x86_addr = dest->x86_addr;
        int jmp_dist = dest_x86_addr - src_x86_addr + JUMP_OFFSET;
        Byte* patch_addr = (Byte*)translated_code_ + src_x86_addr + PATCH_OFFSET;
        if (source->opcode == OPCODE_JMC) {
            patch_addr += JMC_ADD_OFFSET;
            jmp_dist -= JMC_ADD_OFFSET;
        }
        *(int*)patch_addr = jmp_dist;
    }

    actual_x86_size_ = program_ptr - (Byte*)translated_code_;
}

#define IT_PTR() (&(*it))
void BinTran::Optimize() {
    if (program_.size() < 2)
        return;

    auto it = program_.begin();

    BtInstr* instr_prev = IT_PTR();
    it++;
    BtInstr* instr = IT_PTR();

    while (it != program_.end()) {
        it++;
        if (jmp_map_.find(instr) == jmp_map_.end() && instr->IsArithmetic() &&
            jmp_map_.find(instr_prev) == jmp_map_.end() &&
            instr_prev->IsArithmetic())
        {
            instr_prev->res_loc = DATALOC_R9;
            instr->op2_loc = DATALOC_R9;
        }

        instr_prev = instr;
        instr = IT_PTR();
    }
}
#undef IT_PTR

Data Input() {
    Data d = 0;
    std::scanf("%d", &d);
    return d;
}

void Output(Data val) {
    std::printf("%d\n", val);
}

void BinTran::Execute() {
    translated_code_(&Input, &Output, NULL);
}

void BinTran::LoadX86CodeFromFile(const std::string& filename) {
    if (!fs::exists(filename))
        throw IoException(filename, ERR_FILE_DOESNT_EXIST);

    std::size_t filesize = fs::file_size(filename);
    std::FILE* f = std::fopen(filename.c_str(), "rb");
    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    std::fread((void*)translated_code_, 1, allocated_size_, f);
    std::fclose(f);

    actual_x86_size_ = filesize;
}

void BinTran::SaveX86CodeToFile(const std::string& filename) {
    std::FILE* f = std::fopen(filename.c_str(), "wb");
    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    std::fwrite((void*)translated_code_, 1, actual_x86_size_, f);
    std::fclose(f);
}

BinTran::JittedCode BinTran::AllocWriteableMemory(std::size_t size) const {
    void* ptr = mmap(0, size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == (void*)-1) {
        throw AllocException();
    }

    return (JittedCode)ptr;
}

}  // namespace zvm
