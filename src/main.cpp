#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace REPL {

struct ReadSuccessCode {
  std::vector<std::string> args;
};

struct ReadFailureCode {
};

using ReadReturnCode = std::variant<ReadSuccessCode, ReadFailureCode>;

ReadReturnCode read() {
  while (true) {
    std::cout << "$ ";

    std::string input;
    if (!std::getline(std::cin, input)) {
      return ReadFailureCode{};
    }

    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string token;
    while (iss >> token) {
      args.push_back(token);
    }

    if (!args.empty()) {
      return ReadSuccessCode{std::move(args)};
    }
  }
}

struct UnknownCommandT {
  std::string reason;
};

struct ExitCommandT {
  static constexpr std::string_view name = "exit";
};

struct EchoCommandT {
  static constexpr std::string_view name = "echo";
  std::string output;
};

struct TypeCommandT {
  static constexpr std::string_view name = "type";
  std::string output;
};

using EvaluateReturnT =
    std::variant<UnknownCommandT, ExitCommandT, EchoCommandT, TypeCommandT>;

constexpr std::array BUILTIN_NAMES = {
    ExitCommandT::name,
    EchoCommandT::name,
    TypeCommandT::name,
};

EvaluateReturnT evaluate(const std::vector<std::string> &args) {
  const std::string &cmd = args[0];

  if (cmd == ExitCommandT::name) {
    return ExitCommandT{};
  }

  if (cmd == EchoCommandT::name) {
    std::string output;
    for (size_t i = 1; i < args.size(); ++i) {
      if (i > 1)
        output += ' ';
      output += args[i];
    }
    return EchoCommandT{std::move(output)};
  }

  if (cmd == TypeCommandT::name) {
    if (args.size() < 2)
      return TypeCommandT{"type: missing argument"};
    const std::string &target = args[1];
    bool isBuiltin = std::ranges::contains(BUILTIN_NAMES, target);
    if (isBuiltin)
      return TypeCommandT{target + " is a shell builtin"};
    return TypeCommandT{target + ": not found"};
  }

  return UnknownCommandT{cmd + ": command not found"};
}

void print(const EvaluateReturnT &result) {
  std::visit(
      [](auto &&state) {
        using T = std::decay_t<decltype(state)>;
        if constexpr (std::is_same_v<T, UnknownCommandT>) {
          std::cout << state.reason << '\n';
        } else if constexpr (std::is_same_v<T, EchoCommandT>) {
          std::cout << state.output << '\n';
        } else if constexpr (std::is_same_v<T, TypeCommandT>) {
          std::cout << state.output << '\n';
        }
      },
      result);
}

} // namespace REPL

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    auto readResult = REPL::read();

    if (std::holds_alternative<REPL::ReadFailureCode>(readResult))
      break;

    auto &args = std::get<REPL::ReadSuccessCode>(readResult).args;
    auto evalResult = REPL::evaluate(args);

    if (std::holds_alternative<REPL::ExitCommandT>(evalResult))
      break;

    REPL::print(evalResult);
  }

  return 0;
}
