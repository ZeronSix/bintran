/*!
 Generates some tests for binary translator.
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
#include "zvmarch.hpp"
#include <cstdio>

void testPush() {
    FILE* f = fopen("push.zbin", "wb");
    Opcode opcode = OPCODE_PUSH;
    int data = 666;
    fwrite(&opcode, sizeof(opcode), 1, f);
    fwrite(&data, sizeof(data), 1, f);
    fclose(f);
}

int main() {
    testPush();
}

