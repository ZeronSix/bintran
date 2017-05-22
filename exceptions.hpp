/*!
 exceptions.hpp - contains useful exception classes.
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

#ifndef ZVM_EXCEPTIONS_HPP_
#define ZVM_EXCEPTIONS_HPP_

#include <stdexcept>
#include <string>
#include "zvmarch.hpp"

namespace zvm {

enum ErrorCode {
    ERR_OK = 0,
    ERR_FILE_OPEN_FAILURE = 1,
    ERR_WRONG_CMD_LINE_ARGS = 2,
    ERR_FILE_DOESNT_EXIST = 3,
    ERR_FAILED_MEM_ALLOC = 4,
    ERR_SYNTAX_LABEL_REDEF = 5,
    ERR_SYNTAX_UNKNOWN_INSTR = 6,
    ERR_SYNTAX_WRONG_INSTR_ARGS = 7,
    ERR_SYNTAX_UNDEFINED_LABEL = 8,
    ERR_OUT_OF_BOUNDS = 9,
    ERR_STACK_UNDERFLOW = 10,
    ERR_UNDEFINED_OPCODE = 11
};

class IoException: public std::runtime_error {
public:
    IoException(const std::string& filename, ErrorCode errcode)
        : std::runtime_error("IO error"),
          errcode_{errcode} {
        switch (errcode_) {
            case ERR_FILE_OPEN_FAILURE:
                errmsg_ = "failed to open file \"" + filename + "\"";
                break;
            case ERR_FILE_DOESNT_EXIST:
                errmsg_ = "file \"" + filename + "\" doesn't exist";
                break;
            default:
                errmsg_ = "unknown Io exception";
                break;
        }
    }

    ErrorCode GetErrorCode() const {
        return errcode_;
    }

    virtual const char* what() const noexcept {
        return errmsg_.c_str();
    }
private:
    std::string filename_;
    ErrorCode errcode_;
    std::string errmsg_;
};

class AllocException: public std::runtime_error {
public:
    AllocException()
        : std::runtime_error(std::string("failed to allocate memory"))
    {}
};

class SyntaxError: public std::runtime_error {
public:
    SyntaxError(const std::string& filename, std::size_t line,
                ErrorCode errcode, const std::string& data)
        : std::runtime_error("syntax error"),
          filename_(filename),
          line_(line),
          errcode_(errcode),
          data_(data) {
        errmsg_ += "[" + filename_ + ":" + std::to_string(line_) + "]: ";
        switch (errcode_) {
            case ERR_SYNTAX_LABEL_REDEF:
                errmsg_ += "redefinition of label '" + data_ + "'";
                break;
            case ERR_SYNTAX_UNKNOWN_INSTR:
                errmsg_ += "unknown instruction: '" + data_ + "'";
                break;
            case ERR_SYNTAX_WRONG_INSTR_ARGS:
                errmsg_ += "wrong instruction args for '" + data_ + "'";
                break;
            case ERR_SYNTAX_UNDEFINED_LABEL:
                errmsg_ += "undefined label: '" + data_ + "'";
                break;
            default:
                errmsg_ += "unknown syntax error";
                break;
        };
    }

    ErrorCode GetErrorCode() const {
        return errcode_;
    }

    virtual const char* what() const noexcept {
        return errmsg_.c_str();
    }
private:
    std::string filename_;
    std::size_t line_;
    ErrorCode errcode_;
    std::string errmsg_;
    std::string data_;
};

class OutOfBoundsException: public std::runtime_error {
public:
    OutOfBoundsException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

class UndefinedOpcodeException: public std::runtime_error {
public:
    UndefinedOpcodeException(Opcode opcode)
        : std::runtime_error("undefined opcode") {
        const std::size_t MAX_STR_LEN = 256;
        errmsg_.reserve(MAX_STR_LEN);
        snprintf(&errmsg_[0], MAX_STR_LEN, "undefined opcode '0x%x'", opcode);
    }

    virtual const char* what() const noexcept {
        return errmsg_.c_str();
    }
private:
    std::string errmsg_;
};

class StackUnderflowException: public std::runtime_error {
public:
    StackUnderflowException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

class DivisionByZeroException: public std::runtime_error {
public:
    DivisionByZeroException(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

}  // namespace zvm

#endif /* ifndef ZVM_EXCEPTIONS_HPP_ */
