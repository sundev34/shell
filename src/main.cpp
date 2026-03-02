#include <iostream>
#include <string>

enum class REPLReturnCode {
  Success = 0,
  Failure = 1,
  Exit = 2,
};


// read, evaluate, print, loop
REPLReturnCode repl() {
  std::cout << "$ ";

  std::string input;
  if (!std::getline(std::cin, input))
    return REPLReturnCode::Failure;

  if (input == "exit")
    return REPLReturnCode::Exit;
  
  std::cout << input << ": command not found" << "\n";
  return REPLReturnCode::Success;
}

int main() {
  // configure std::cout global instance to flush after every std::cout /
  // std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    REPLReturnCode returnCode = repl();
    if (returnCode == REPLReturnCode::Failure) {
      return 1;
    } else if (returnCode == REPLReturnCode::Exit) {
      return 0;
    }
  }
}
