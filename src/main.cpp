#include "args.h"
#include "lib.h"
#include "result.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  auto argparse_result = scargs::parse(argc, argv);
  if (argparse_result.isErr()) {
    const auto err = argparse_result.inspect();
    std::cerr << err.what() << "\n";
    return 1;
  }

  auto args = argparse_result.unwrap();
  auto compile_result = sc::compile(args.input_filename, args.output_filename,
                                    args.verbose_printing);
  auto ret = compile_result.unwrapOrElse([](auto &&e) {
    std::cerr << e.what() << "\n";
    return 1;
  });
  return ret;
}
