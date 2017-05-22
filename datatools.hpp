/*!
 datatools.hpp - contains useful functions for working with raw byte arrays.
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
#ifndef ZVM_DATATOOLS_HPP_
#define ZVM_DATATOOLS_HPP_

#include "zvmarch.hpp"

namespace zvm {

typedef unsigned char Byte;

template<class T>
void EmitAndShiftBuf(Byte*& buf, T data) {
    *(T*)buf = data;
    buf += sizeof(T);
}

template<class T>
void EmitAt(Byte*& buf, T data, std::size_t at) {
    *(T*)(buf + at) = data;
}

DecodedInstr FetchInstr(const Byte* ptr, Register& pc);

}  // namespace zvm

#endif /* ifndef ZVM_DATATOOLS_HPP_ */
