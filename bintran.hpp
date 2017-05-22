/*!
 bintran.hpp - binary translator from stack process architecture to x86.
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

#ifndef ZVM_BINTRAN_HPP_
#define ZVM_BINTRAN_HPP_

#include <string>
#include <map>
#include <vector>
#include <list>
#include "zvmarch.hpp"

namespace zvm {

enum DataLocation {
    DATALOC_NONE,
    DATALOC_STACK,
    DATALOC_RAX,
    DATALOC_R8,
    DATALOC_R9,
    DATALOC_R14,
    DATALOC_IMM,
    DATALOC_STDIN,
    DATALOC_STDOUT
};

struct BtInstr {
    Opcode opcode;
    Data arg;

    std::size_t zvm_addr;
    std::size_t x86_addr;

    DataLocation op1_loc;
    DataLocation op2_loc;
    DataLocation res_loc;

    bool IsArithmetic() {
        return opcode == OPCODE_ADD || opcode == OPCODE_SUB ||
               opcode == OPCODE_MUL;
    }
};

class BinTran {
public:
    BinTran(std::size_t alloc_size = MAX_OUTPUT_SIZE);
    ~BinTran();

    void LoadBinary(const std::string& filename);
    void Translate();
    void Optimize();
    void Execute();
    void LoadX86CodeFromFile(const std::string& filename);
    void SaveX86CodeToFile(const std::string& filename);

    const static std::size_t MAX_OUTPUT_SIZE = 4096 * 16;
private:
    typedef Data (*InputFunc)();
    typedef void (*OutputFunc)(Data val);
    typedef void (*JittedCode)(InputFunc inputfun,
                               OutputFunc outputfun,
                               Byte* bp_stack);

    Byte* zvmbinary_;
    std::size_t zvmbinary_size_;

    JittedCode translated_code_;
    std::size_t allocated_size_;
    std::size_t actual_x86_size_;

    std::list<BtInstr> program_;
    std::map<std::size_t, BtInstr*> zvmaddr_map_;
    std::map<BtInstr*, BtInstr*> jmp_map_;

    JittedCode AllocWriteableMemory(std::size_t size) const;
    void WriteCodeHeader(Byte*& ptr);
    void WriteCodeFooter(Byte*& ptr);
    void WriteInstr(Byte*& ptr, BtInstr& instr);
};

}  // namespace zvm

#endif /* ifndef ZVM_BINTRAN_HPP_ */
