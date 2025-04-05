#include <forext/forext.h>

#include <iostream>

int main(int argc, char *argv[]) {
  Forext forext;

  if (argc == 1) {
    forext.repl();
  } else if (argc == 2) {
    forext.runFile(argv[1]);
  } else {
    std::cerr << "Expected usage: forext [path]\n";
    exit(64);
  }

  return 0;
}
