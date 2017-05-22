/*!
 zasm.cpp - assembler for ZVM architecture.
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

#include <cstdio>
#include <string>
#include <cstring>
#include <cctype>
#include <map>
#include <experimental/filesystem>
#include "exceptions.hpp"
#include "zvmarch.hpp"
#include "datatools.hpp"

namespace fs = std::experimental::filesystem;

namespace zvm {

static const char COMMENT_SYMBOL = ';';
static const char LABEL_SUFFIX = ':';

class Asm {
public:
    Asm();
    ~Asm();
    void LoadSource(const std::string& filename);
    void Assemble();
    void WriteBinaryToFile(const std::string& filename);
private:
    std::string source_filename_;
    std::size_t source_size_;
    char* source_;
    Byte* output_buf_;
    std::size_t output_buf_size_;
};

Asm::Asm(): source_size_(0),
            source_(nullptr),
            output_buf_(nullptr),
            output_buf_size_(0) {}

Asm::~Asm() {
    delete[] source_;
    delete[] output_buf_;
}

void Asm::LoadSource(const std::string& filename) {
    if (!fs::exists(filename))
        throw IoException(filename, ERR_FILE_DOESNT_EXIST);

    if (source_)
        delete[] source_;
    source_filename_ = filename;

    std::size_t filesize = fs::file_size(filename);

    std::FILE* f = std::fopen(filename.c_str(), "r");
    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    source_ = new char[filesize + 1]();
    if (!source_)
        throw AllocException();

    std::fread(source_, sizeof(*source_), filesize, f);

    std::fclose(f);

    source_size_ = filesize; // may cause extra bytes on Windows
}

inline void SkipSpaces(char*& current) {
    while (std::isspace(*current))
        current++;
}

inline void SkipComment(char*& current) {
    SkipSpaces(current);
    while (*current == COMMENT_SYMBOL) {
        current = strchrnul(current, '\n');
        SkipSpaces(current);
    }
}

inline std::size_t GetLineNum(const char* current) {
    // TODO: return line number
    return 0;
}

#define COMPARE_INSTR(instr) if (strcasecmp(curword, #instr) == 0) { \
    opcode = OPCODE_ ## instr; \
}

void Asm::Assemble() {
    const std::size_t MAX_WORD_SIZE = 256;

    char curword[MAX_WORD_SIZE + 1] = {};
    char* current = source_;

    output_buf_ = new Byte[MAX_INSTRUCTION_SIZE * source_size_]();
    if (!output_buf_)
        throw AllocException();
    Byte* cur_outptr = output_buf_;

    std::map<std::string, std::size_t> labels;
    std::multimap<std::size_t, std::string> label_patches;

    while (true) {
        int wordlen = 0;

        SkipSpaces(current);
        SkipComment(current);
        std::sscanf(current, "%s%n", curword, &wordlen);

        if (wordlen == 0) {
            break;
        }
        current += wordlen;

        if (curword[wordlen - 1] == LABEL_SUFFIX) {
            curword[wordlen - 1] = '\0';
            if (labels.find(curword) != labels.end()) {
                throw SyntaxError(source_filename_,
                                  GetLineNum(current),
                                  ERR_SYNTAX_LABEL_REDEF,
                                  curword);
            }

            labels[curword] = cur_outptr - output_buf_;
            continue;
        }

        Opcode opcode = OPCODE_UD;

        // see the COMPARE_INSTR macro
        COMPARE_INSTR(PUSH)
        else COMPARE_INSTR(HALT)
        else COMPARE_INSTR(POP)
        else COMPARE_INSTR(ADD)
        else COMPARE_INSTR(LOAD)
        else COMPARE_INSTR(STORE)
        else COMPARE_INSTR(INPUT)
        else COMPARE_INSTR(OUTPUT)
        else COMPARE_INSTR(JMP)
        else COMPARE_INSTR(JMC)
        else COMPARE_INSTR(SUB)
        else COMPARE_INSTR(MUL)
        else COMPARE_INSTR(DIV)
        else COMPARE_INSTR(GZ)
        else COMPARE_INSTR(BZ)
        else COMPARE_INSTR(GEZ)
        else COMPARE_INSTR(EQZ)
        else COMPARE_INSTR(NEQZ)
        else COMPARE_INSTR(BEZ)
        else COMPARE_INSTR(CALL)
        else COMPARE_INSTR(RET)
        else COMPARE_INSTR(PUSHBP)
        else COMPARE_INSTR(POPBP)
        else {
            throw SyntaxError(source_filename_,
                              GetLineNum(current),
                              ERR_SYNTAX_UNKNOWN_INSTR,
                              curword);
        }

        EmitAndShiftBuf(cur_outptr, opcode);

        // argument reader
        SkipSpaces(current);
        SkipComment(current);

        Data arg = 0;
        switch (opcode) {
            // 1 arg instructions
            case OPCODE_PUSH:
            case OPCODE_LOAD:
            case OPCODE_STORE:
                if (std::sscanf(current, "%d%n", &arg, &wordlen) == 0)
                    throw SyntaxError(source_filename_,
                                      GetLineNum(current),
                                      ERR_SYNTAX_WRONG_INSTR_ARGS,
                                      "opcode " + std::to_string(opcode));
                current += wordlen;
                EmitAndShiftBuf(cur_outptr, arg);
                break;
            case OPCODE_JMP:
            case OPCODE_JMC:
            case OPCODE_CALL:
                std::sscanf(current, "%s%n", curword, &wordlen);
                if (wordlen == 0)
                    throw SyntaxError(source_filename_,
                                      GetLineNum(current),
                                      ERR_SYNTAX_WRONG_INSTR_ARGS,
                                      "opcode " + std::to_string(opcode));
                current += wordlen;
                arg = 0;
                label_patches.insert(std::make_pair((int)(cur_outptr - output_buf_),
                                                    std::string(curword)));
                EmitAndShiftBuf(cur_outptr, 0);
                break;
            default:
                break;
        }

    }

    output_buf_size_ = cur_outptr - output_buf_;
    for (auto it = label_patches.begin(); it != label_patches.end(); ++it) {
        if (labels.find(it->second) == labels.end())
            throw SyntaxError(source_filename_,
                              GetLineNum(current),
                              ERR_SYNTAX_UNDEFINED_LABEL,
                              it->second);
        else
            EmitAt(output_buf_, (int)labels[it->second], it->first);
    }
}
#undef COMPARE_INSTR

void Asm::WriteBinaryToFile(const std::string& filename) {
    FILE* f = std::fopen(filename.c_str(), "wb");

    if (!f)
        throw IoException(filename, ERR_FILE_OPEN_FAILURE);

    fwrite(output_buf_, sizeof(*output_buf_), output_buf_size_, f);
    fclose(f);
}

}  // namespace zvm

inline void DisplayUsage() {
    std::printf("Usage: zasm SOURCE OUTPUT\n");
}

int main(int argc, char* argv[]) {
    using namespace zvm;

    if (argc != 3) {
        DisplayUsage();
        return ERR_WRONG_CMD_LINE_ARGS;
    }

    try {
        Asm zasm;
        zasm.LoadSource(argv[1]);
        zasm.Assemble();
        zasm.WriteBinaryToFile(argv[2]);
    } catch (const IoException& ioerr) {
        std::fprintf(stderr, "IO error: %s\n", ioerr.what());
        return ioerr.GetErrorCode();
    } catch (const AllocException& allocerr) {
        std::fprintf(stderr, "Allocation error: %s\n", allocerr.what());
        return ERR_FAILED_MEM_ALLOC;
    } catch (const SyntaxError& synterr) {
        std::fprintf(stderr, "Syntax error: %s\n", synterr.what());
        return synterr.GetErrorCode();
    }

    return ERR_OK;
}
