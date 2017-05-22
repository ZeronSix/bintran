/*!
 datatools.cpp - contains implementations of functions from datatools.hpp.
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

#include "datatools.hpp"

namespace zvm {

DecodedInstr FetchInstr(const Byte* ptr, Register& pc) {
    DecodedInstr result = {};
    ptr += pc;

    result.opcode = *(Opcode*)ptr;
    pc += sizeof(Opcode);

    switch (result.opcode) {
        case OPCODE_PUSH:
        case OPCODE_LOAD:
        case OPCODE_STORE:
        case OPCODE_JMP:
        case OPCODE_JMC:
        case OPCODE_CALL:
            result.args[0] = *(Data*)(ptr + sizeof(Opcode));
            pc += sizeof(Data);
            break;
        default:
            break;
    }

    return result;
}

}  // namespace zvm

