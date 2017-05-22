/*!
 zvmarch.hpp - contains common data for vm, asm, translator and etc.
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

#ifndef ZVM_ZVMARCH_HPP_
#define ZVM_ZVMARCH_HPP_

#include <cstdint>
#include <cstddef>

namespace zvm {
/*!
 * Type of all registers.
 */
typedef std::uint32_t Register;
/*!
 * Virtual machine data type.
 */
typedef std::int32_t Data;
/*!
 * Byte.
 */
typedef std::uint8_t Byte;
/*!
 * Maximum size of program memory.
 */
const std::size_t PROGRAM_MEMORY_SIZE = 1024;
/*!
 * Maximum size of data memory.
 */
const std::size_t DATA_MEMORY_SIZE = 2048;
/*!
 * Instruction size (in bytes).
 */
const std::size_t MAX_INSTRUCTION_SIZE = 5;
/*!
 * Command argument count.
 */
const std::size_t MAX_ARG_COUNT = 1;
/*!
 * OpCode enum
 */
enum Opcode: std::int8_t {
    OPCODE_UD = -0x1, // undefined opcode
    OPCODE_HALT = 0x0,
    OPCODE_PUSH = 0x1,
    OPCODE_POP = 0x2,
    OPCODE_ADD = 0x3,
    OPCODE_LOAD = 0x4,
    OPCODE_STORE = 0x5,
    OPCODE_INPUT = 0x6,
    OPCODE_OUTPUT = 0x7,
    OPCODE_JMP = 0x9,
    OPCODE_JMC = 0xA,
    OPCODE_SUB = 0xB,
    OPCODE_MUL = 0xC,
    OPCODE_DIV = 0xD,
    OPCODE_GZ = 0xE,
    OPCODE_BZ = 0xF,
    OPCODE_GEZ = 0x10,
    OPCODE_BEZ = 0x11,
    OPCODE_CALL = 0x12,
    OPCODE_RET = 0x13,
    OPCODE_PUSHBP = 0x14,
    OPCODE_POPBP = 0x15,
    OPCODE_EQZ = 0x16,
    OPCODE_NEQZ = 0x17,
};

struct DecodedInstr {
    Opcode opcode;
    Data args[MAX_ARG_COUNT];
};

}  // namespace zvm

#endif /* ifndef ZVM_ZVMARCH_HPP_ */
