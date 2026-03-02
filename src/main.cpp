#include <iostream>
#include <string>

void repl() {
  std::cout << "$ ";
  std::string input;
  std::getline(std::cin, input);

  std::cout << input << ": command not found";
  std::cout << std::endl;
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true) {
  repl();
  }

}
