#ifndef FOREXT_H
#define FOREXT_H

#include <forext/fxt_compiler.h>
#include <forext/fxt_scanner.h>
#include <forext/fvm.h>


#include <string>
#include <unordered_map>
#include <vector>

class Forext {
private:
  Scanner scanner;
  Compiler compiler;
  FVM vm;
  std::unordered_map<std::string, Value> strings;

  InterpreterResult interpret(const std::string &source);

public:
  Forext();

  void repl();
  void runFile(const char *path);
};

#endif // FOREXT_H

