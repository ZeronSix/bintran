#include "bintran.hpp"
#include "exceptions.hpp"
#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

inline void DisplayUsage() {
    std::printf("Usage: bintran PROGRAM\n");
}

static const char* FILE_EXTENSION = ".x86";

int main(int argc, char* argv[]) {
    using namespace zvm;

    if (argc != 2) {
        DisplayUsage();
        return ERR_WRONG_CMD_LINE_ARGS;
    }

    std::string filename = argv[1];
    std::string x86_filename = argv[1] + std::string(FILE_EXTENSION);

    try {
        BinTran bt;
        if (fs::exists(filename) && fs::exists(x86_filename) &&
            fs::last_write_time(filename) <= fs::last_write_time(x86_filename)) {
            bt.LoadX86CodeFromFile(x86_filename);
        } else {
            bt.LoadBinary(filename);
            bt.Translate();
        }
        bt.Execute();
        bt.SaveX86CodeToFile(x86_filename);
    } catch (const IoException& ioerr) {
        std::fprintf(stderr, "IO error: %s\n", ioerr.what());
        return ioerr.GetErrorCode();
    } catch (const AllocException& allocerr) {
        std::fprintf(stderr, "Allocation error: %s\n", allocerr.what());
        return ERR_FAILED_MEM_ALLOC;
    } catch (const UndefinedOpcodeException& opcerr) {
        std::fprintf(stderr, "Runtime error: %s\n", opcerr.what());
        return ERR_OUT_OF_BOUNDS;
    }

    return ERR_OK;
}
