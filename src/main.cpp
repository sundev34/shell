#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <cstdlib>
#include <filesystem>

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

class ICommand {
public:
  virtual ~ICommand() = default;
  virtual std::string_view name() const = 0;
  virtual bool isBuiltin() const = 0;
  // Returns false to signal the shell should exit.
  virtual bool execute(std::span<const std::string> args) = 0;
};

class CommandRegistry {
public:
  void registerCommand(std::unique_ptr<ICommand> cmd) {
    commands_[std::string(cmd->name())] = std::move(cmd);
  }

  ICommand *find(std::string_view name) const {
    auto it = commands_.find(std::string(name));
    return it != commands_.end() ? it->second.get() : nullptr;
  }

  bool isBuiltin(std::string_view name) const {
    auto *cmd = find(name);
    return cmd && cmd->isBuiltin();
  }

private:
  std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
};

class ExitCommand : public ICommand {
public:
  std::string_view name() const override { return "exit"; }
  bool isBuiltin() const override { return true; }
  bool execute(std::span<const std::string>) override { return false; }
};

class EchoCommand : public ICommand {
public:
  std::string_view name() const override { return "echo"; }
  bool isBuiltin() const override { return true; }
  bool execute(std::span<const std::string> args) override {
    for (size_t i = 1; i < args.size(); ++i) {
      if (i > 1)
        std::cout << ' ';
      std::cout << args[i];
    }
    std::cout << '\n';
    return true;
  }
};

const char* getenvVar(const char* name) {
  const char* envVarValue = getenv(name);
  if (envVarValue != nullptr) {
    return envVarValue;
  }

  return nullptr;
}

class TypeCommand : public ICommand {
public:
  explicit TypeCommand(const CommandRegistry &registry) : registry_(registry) {}

  std::string_view name() const override { return "type"; }
  bool isBuiltin() const override { return true; }

  bool execute(std::span<const std::string> args) override {
    if (args.size() < 2) {
      std::cout << "type: missing argument\n";
      return true;
    }
    const std::string &target = args[1];
    if (registry_.isBuiltin(target)) {
      std::cout << target << " is a shell builtin\n";
      return true;
    }

    if (const char *pathEnv = getenvVar("PATH")) {
      std::istringstream ss(pathEnv);
      std::string dir;
      while (std::getline(ss, dir, ':')) {
        auto fullPath = std::filesystem::path(dir) / target;
        if (std::filesystem::is_regular_file(fullPath) &&
            (std::filesystem::status(fullPath).permissions() &
             std::filesystem::perms::owner_exec) != std::filesystem::perms::none) {
          std::cout << target << " is " << fullPath.string() << "\n";
          return true;
        }
      }
    }


    std::cout << target << ": not found\n";
    return true;
    
  }

private:
  const CommandRegistry &registry_;
};



} // namespace REPL

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  REPL::CommandRegistry registry;
  registry.registerCommand(std::make_unique<REPL::ExitCommand>());
  registry.registerCommand(std::make_unique<REPL::EchoCommand>());
  registry.registerCommand(std::make_unique<REPL::TypeCommand>(registry));

  while (true) {
    auto readResult = REPL::read();

    if (std::holds_alternative<REPL::ReadFailureCode>(readResult))
      break;

    auto &args = std::get<REPL::ReadSuccessCode>(readResult).args;

    if (auto *cmd = registry.find(args[0])) {
      if (!cmd->execute(args))
        break;
    } else {
      std::cout << args[0] << ": command not found\n";
    }
  }

  return 0;
}
