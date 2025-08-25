#include "vm.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
void repl() {
  std::string line;
  while (true) {
    std::cout << "> ";
    if (!std::getline(std::cin, line)) {
      std::cout << std::endl;
      break;
    }
    VM vm;
    vm.interpret(line);
  }
}

std::string readFile(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) {
    std::cerr << "Could not open file \"" << path.string() << "\"" << std::endl;
    std::exit(74);
  }

  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "Could not open file \"" << path.string() << "\"" << std::endl;
    std::exit(74);
  }

  // Get file size
  file.seekg(0, std::ios::end);
  const auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  // Read file
  std::string content(fileSize, '\0');
  if (!file.read(content.data(), fileSize)) {
    std::cerr << "Could not read file \"" << path.string() << "\"" << std::endl;
    std::exit(74);
  }

  return content;
}

void runFile(const std::filesystem::path &path) {
  std::string source = readFile(path);
  VM vm;
  InterpretResult result = vm.interpret(source);

  if (result == InterpretResult::InterpretCompileError) {
    std::exit(65);
  }
  if (result == InterpretResult::InterpretRuntimeError) {
    std::exit(70);
  }
}
} // namespace

int main(int argc, char **argv) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::cout << "Usage: cpplox [path]" << std::endl;
    std::exit(64);
  }
  return 0;
}
