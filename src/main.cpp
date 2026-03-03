#include <iostream>
#include <ranges>
#include <string>
#include <variant>
#include <vector>
#include <sstream> // Added for std::istringstream

namespace REPL {
struct ReadContinueCode {
  std::vector<std::string> args;
};

struct ReadStopCode {
  std::string reason;
};

struct ReadExitCode {
  std::string reason;
};

using ReadReturnCode =
    std::variant<ReadContinueCode, ReadStopCode, ReadExitCode>;

ReadReturnCode read() {
  std::cout << "$ ";

  std::string input;

  // TODO: handle EOF (Ctrl+D), input errors, and other edge cases
  if (!std::getline(std::cin, input)) {
    ReadStopCode stopCode{"input error"};
    return stopCode;
  }

  if (input == "exit") {
    ReadExitCode exitCode{"user requested exit"};
    return exitCode;
  }
  std::istringstream iss(input);
  std::vector<std::string> args;
  std::string token;

  while (iss >> token) {   // automatically splits on whitespace
    args.push_back(token);
  }

  if (args.empty()) {
    return ReadStopCode{"empty input"};  // or handle however you prefer
  }

  return ReadContinueCode{args};
}


// use structs to enforce type safety and make it easier to add more commands in the future
struct UnkownCommandT {
  std::string reason;
};

struct EchoReturnT {
  std::string output;
};

using EvaluateReturnT = std::variant<UnkownCommandT, EchoReturnT>;
EvaluateReturnT evaluate(const std::vector<std::string>& args) {
  // assert args is not empty
  std::string input = args[0];

  if (input == "echo") {
    std::string output;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) {
            output += " ";
        }
        output += args[i];
    }
    return EchoReturnT{output};
  }

  std::string reason = input + ": command not found";
  return UnkownCommandT{reason};
}

void print(const EvaluateReturnT& result) {
  std::visit(
    [](auto &&state) {
      using T = std::decay_t<decltype(state)>;

      if constexpr (std::is_same_v<T, REPL::UnkownCommandT>) {
        std::cout << state.reason << "\n";
      } else if constexpr (std::is_same_v<T, REPL::EchoReturnT>) {
        std::cout << state.output << "\n";
      }
    }, result);
}

} // namespace REPL

int main() {
  // configure std::cout global instance to flush after every std::cout /
  // std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    // +++++++++++++
    // READ
    // +++++++++++++
    auto readResult = REPL::read();

    std::vector<std::string> args;
    bool exit = false;
    std::string exitReason;
    std::visit(
        [&args, &exit, &exitReason](auto &&state) {
          using T = std::decay_t<decltype(state)>;

          if constexpr (std::is_same_v<T, REPL::ReadContinueCode>) {
            args = state.args;
          } else if constexpr (std::is_same_v<T, REPL::ReadStopCode>) {
            // TODO: print error message to stderr
            exit = true;
            exitReason = state.reason;
            
          } else if constexpr (std::is_same_v<T, REPL::ReadExitCode>) {
            exit = true;
            exitReason = state.reason;
          }
        },
        readResult);

    if (exit) {
      // TODO: print exit reason to stderr
      break;
    }

    // +++++++++++++
    // EVALUATE
    // +++++++++++++
    auto evalResult = REPL::evaluate(args);

    // +++++++++++++
    // PRINT
    // +++++++++++++
    print(evalResult);
  }
}
