/*
 zvm.cpp - ZeronSix's stack-based virtual machine.
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

#include <stack>
#include <vector>
#include <string>
#include <experimental/filesystem>
#include "exceptions.hpp"
#include "zvmarch.hpp"
#include "datatools.hpp"

namespace fs = std::experimental::filesystem;

namespace zvm {

class Zvm {
public:
    Zvm();
    ~Zvm();

    void LoadBinary(const std::string& filename);
    void Run();
private:
    Byte* program_memory_;
    std::size_t program_size_;
    std::vector<Data> data_stack_;
    std::stack<Register> call_stack_;
    std::stack<Register> bp_stack_;

    Register pc_;
    Register sp_;
    Register bp_;

    bool halt_flag_;

    void Execute(Opcode opcode, Data arg);
    void Push(Data val);
    Data Pop();
    void PushBp();
    void PopBp();
    void PushAddr(Register val);
    Register PopAddr();
};

Zvm::Zvm(): program_memory_(nullptr),
            program_size_(0),
            pc_(0),
            sp_(0),
            bp_(0),
            halt_flag_(false) {}

Zvm::~Zvm() {
    delete program_memory_;
}

void Zvm::LoadBinary(const std::string& filename) {
    if (!fs::exists(filename))
        throw IoException(filename, ERR_FILE_DOESNT_EXIST);

    if (program_memory_)
        delete[] program_memory_;

    std::size_t filesize = fs::file_size(filename);
    std::FILE* f = std::fopen(filename.c_str(), "rb");
    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    program_memory_ = new Byte[filesize]();
    if (!program_memory_)
        throw AllocException();

    std::fread(program_memory_, sizeof(*program_memory_), filesize, f);

    std::fclose(f);
    program_size_ = filesize;
}

void Zvm::Run() {
    pc_ = sp_ = bp_ = 0;

    while (!halt_flag_) {
        if (pc_ >= program_size_)
            throw OutOfBoundsException("PC out of bounds");

        DecodedInstr instr = FetchInstr(program_memory_, pc_);
        Execute(instr.opcode, instr.args[0]);
    }
}

void Zvm::Execute(Opcode opcode, Data arg) {
    Data op1 = 0, op2 = 0;

    switch (opcode) {
        case OPCODE_HALT:
            halt_flag_ = true;
            break;
        case OPCODE_PUSH:
            Push(arg);
            break;
        case OPCODE_POP:
            Pop();
            break;
        case OPCODE_ADD:
            op1 = Pop();
            op2 = Pop();
            Push(op1 + op2);
            break;
        case OPCODE_SUB:
            op1 = Pop();
            op2 = Pop();
            Push(op2 - op1);
            break;
        case OPCODE_MUL:
            op1 = Pop();
            op2 = Pop();
            Push(op1 * op2);
            break;
        case OPCODE_DIV:
            op1 = Pop();
            op2 = Pop();
            if (op1 == 0)
                throw DivisionByZeroException("division by zero");
            Push(op2 / op1);
            break;
        case OPCODE_LOAD:
            Push(data_stack_.at(bp_ + arg));
            break;
        case OPCODE_STORE:
            data_stack_.at(bp_ + arg) = Pop();
            break;
        case OPCODE_INPUT:
            scanf("%d", &op1);
            Push(op1);
            break;
        case OPCODE_OUTPUT:
            op1 = Pop();
            printf("%d\n", op1);
            break;
        case OPCODE_JMP:
            if (arg < 0 || std::size_t(arg) >= program_size_)
                throw OutOfBoundsException("JMP out of bounds");
            pc_ = arg;
            break;
        case OPCODE_JMC:
            if (arg < 0 || std::size_t(arg) >= program_size_)
                throw OutOfBoundsException("JMP out of bounds");
            op1 = Pop();
            if (op1)
                pc_ = arg;
            break;
        case OPCODE_GZ:
            op1 = Pop();
            Push(op1 > 0);
            break;
        case OPCODE_BZ:
            op1 = Pop();
            Push(op1 < 0);
            break;
        case OPCODE_GEZ:
            op1 = Pop();
            Push(op1 >= 0);
            break;
        case OPCODE_BEZ:
            op1 = Pop();
            Push(op1 <= 0);
            break;
        case OPCODE_EQZ:
            op1 = Pop();
            Push(op1 == 0);
            break;
        case OPCODE_NEQZ:
            op1 = Pop();
            Push(op1 != 0);
            break;
        case OPCODE_CALL:
            if (arg < 0 || std::size_t(arg) >= program_size_)
                throw OutOfBoundsException("JMP out of bounds");
            PushAddr(pc_);
            pc_ = arg;
            break;
        case OPCODE_RET:
            pc_ = PopAddr();
            break;
        case OPCODE_PUSHBP:
            PushBp();
            break;
        case OPCODE_POPBP:
            PopBp();
            break;
        default:
            throw UndefinedOpcodeException(opcode);
    }
}

void Zvm::Push(Data val) {
    data_stack_.push_back(val);
    sp_++;
}

Data Zvm::Pop() {
    if (sp_ == 0)
        throw StackUnderflowException("data stack underflow");
    Data val = data_stack_.back();
    data_stack_.pop_back();

    return val;
}

void Zvm::PushBp() {
    bp_stack_.push(bp_);
}

void Zvm::PopBp() {
    if (bp_stack_.size() == 0)
        throw StackUnderflowException("bp stack underflow");
    bp_ = bp_stack_.top();
    bp_stack_.pop();
}

void Zvm::PushAddr(Register val) {
    call_stack_.push(val);
}

Register Zvm::PopAddr() {
    if (call_stack_.size() == 0)
        throw StackUnderflowException("call stack underflow");
    Register addr = call_stack_.top();
    call_stack_.pop();
    return addr;
}

} // namespace zvm

inline void DisplayUsage() {
    std::printf("Usage: zvm PROGRAM\n");
}

int main(int argc, char* argv[]) {
    using namespace zvm;

    if (argc != 2) {
        DisplayUsage();
        return ERR_WRONG_CMD_LINE_ARGS;
    }

    try {
        Zvm zvm;
        zvm.LoadBinary(argv[1]);
        zvm.Run();
    } catch (const IoException& ioerr) {
        std::fprintf(stderr, "IO error: %s\n", ioerr.what());
        return ioerr.GetErrorCode();
    } catch (const AllocException& allocerr) {
        std::fprintf(stderr, "Allocation error: %s\n", allocerr.what());
        return ERR_FAILED_MEM_ALLOC;
    } catch (const OutOfBoundsException& bnderr) {
        std::fprintf(stderr, "Runtime error: %s\n", bnderr.what());
        return ERR_OUT_OF_BOUNDS;
    } catch (const StackUnderflowException& stackerr) {
        std::fprintf(stderr, "Runtime error: %s\n", stackerr.what());
        return ERR_STACK_UNDERFLOW;
    } catch (const UndefinedOpcodeException& opcerr) {
        std::fprintf(stderr, "Runtime error: %s\n", opcerr.what());
        return ERR_OUT_OF_BOUNDS;
    } catch (const DivisionByZeroException& diverr) {
        std::fprintf(stderr, "Runtime error: %s\n", diverr.what());
        return ERR_OUT_OF_BOUNDS;
    }

    return ERR_OK;
}
