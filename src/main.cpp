#include <iostream>
#include <string>

// read, evaluate, print, loop
bool repl() {
  std::cout << "$ ";

  std::string input;
  if (!std::getline(std::cin, input))
    return false;

  std::cout << input << ": command not found" << "\n";
  return true;
}

int main() {
  // configure std::cout global instance to flush after every std::cout /
  // std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    repl();
  }
}
