#include "lib.h"

#include "fs.h"
#include "lib/backend.h"
#include "lib/opt.h"
#include "lib/parser.h"
#include "lib/print_ir.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"

#include <memory>

namespace {
class InputFileError : public Error<InputFileError> {
private:
  std::string message;

public:
  InputFileError(sc::parser::ParserError &&__err) noexcept {
    using namespace std::string_literals;
    message = "invalid input file\n"s.append(__err.what());
  }

  InputFileError(fs::FilesystemError &&__err) noexcept {
    using namespace std::string_literals;
    message = "invalid input file\n"s.append(__err.what());
  }

  const char *what() const noexcept { return message.c_str(); }
};

class OutputFileError : public Error<OutputFileError> {
private:
  std::string message;

public:
  OutputFileError(const std::string_view __message) {
    using namespace std::string_literals;
    message = "invalid output file\n"s;
    message.append(__message);
  }
  const char *what() const noexcept { return message.c_str(); }
};

Result<std::string, InputFileError>
readFile(const std::string_view __filename) {
  auto read_result = fs::readFile(__filename);
  return read_result.mapErr<InputFileError>(
      [](auto &&err) { return InputFileError(std::move(err)); });
}

Result<size_t, OutputFileError> writeFile(const std::string_view __filename,
                                          const std::string_view __content) {
  auto write_result = fs::writeFile(__filename, __content);
  return write_result.mapErr<OutputFileError>(
      [](auto &&err) { return OutputFileError(err.what()); });
}
} // namespace

namespace sc {
template <typename E>
SWPPCompilerError::SWPPCompilerError(Error<E> &&__err) noexcept {
  using namespace std::string_literals;
  message = "swpp-compiler crashed: "s.append(__err.what());
}

Result<size_t, SWPPCompilerError>
compile(const std::string_view __input_filename,
        const std::string_view __output_filename,
        const bool __verbose_printing = false) noexcept {
  auto read_result =
      readFile(__input_filename).mapErr<SWPPCompilerError>([](auto &&err) {
        return SWPPCompilerError(std::move(err));
      });

  llvm::LLVMContext context;

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;

  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  if (__verbose_printing) {
    print_ir::setVerbose();
  }

  auto parse_result = read_result.andThen<std::unique_ptr<llvm::Module>>(
      [&context, __input_filename](std::string &&code) {
        auto res = parser::parseIR(code, __input_filename, context);
        return res.mapErr<SWPPCompilerError>([](auto &&err) {
          return SWPPCompilerError(InputFileError(std::move(err)));
        });
      });

  auto optimize_result = parse_result.andThen<std::unique_ptr<llvm::Module>>(
      [&MAM](std::unique_ptr<llvm::Module> &&M) {
        auto res = sc::opt::optimizeIR(std::move(M), MAM);
        return res.mapErr<SWPPCompilerError>(
            [](auto &&err) { return SWPPCompilerError(std::move(err)); });
      });

  auto emit_result = optimize_result.andThen<std::string>(
      [&MAM](std::unique_ptr<llvm::Module> &&M) {
        auto res = sc::backend::emitAssembly(std::move(M), MAM);
        return res.mapErr<SWPPCompilerError>(
            [](auto &&err) { return SWPPCompilerError(std::move(err)); });
      });

  auto write_result =
      emit_result.andThen<size_t>([__output_filename](std::string &&assembly) {
        auto res = writeFile(__output_filename, assembly);
        return res.mapErr<SWPPCompilerError>(
            [](auto &&err) { return SWPPCompilerError(std::move(err)); });
      });

  auto compile_result =
      write_result.map<size_t>([](const auto written_bytes) { return 0; });

  return compile_result;
}
} // namespace sc
