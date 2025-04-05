#ifndef FOREXT_BYTECODE_H
#define FOREXT_BYTECODE_H

#include <forext/fxt_value.h>

#include <map>
#include <string>
#include <vector>

struct LineStart {
  int line;
  int offset;
};

class Bytecode {
  std::vector<uint8_t> bytecode;
  std::vector<Value> constants;
  std::vector<LineStart> lines;

  int disassembleSimpleInstruction(const std::string& name, int offset);
  int disassembleConstantInstruction(const std::string& name, int offset);
  int disassembleByteInstruction(const std::string& name, int offset);
  int disassembleJumpInstruction(const std::string& name, int sign, int offset);
  int disassembleInvokeInstruction(const std::string& name, int offset);

public:
  enum class OpCode {
    FOP_CONSTANT,
    FOP_NULL,
    FOP_TRUE,
    FOP_FALSE,
    FOP_POP,
    FOP_GET_LOCAL,
    FOP_SET_LOCAL,
    FOP_GET_GLOBAL,
    FOP_DEFINE_GLOBAL,
    FOP_SET_GLOBAL,
    FOP_GET_UPVALUE,
    FOP_SET_UPVALUE,
    FOP_GET_PROPERTY,
    FOP_SET_PROPERTY,
    FOP_GET_SUPER,
    FOP_EQUAL,
    FOP_GREATER,
    FOP_LESS,
    FOP_ADD,
    FOP_SUBTRACT,
    FOP_MULTIPLY,
    FOP_DIVIDE,
    FOP_NOT,
    FOP_NEGATE,
    FOP_PRINT,
    FOP_JUMP,
    FOP_JUMP_IF_FALSE,
    FOP_LOOP,
    FOP_CALL,
    FOP_INVOKE,
    FOP_SUPER_INVOKE,
    FOP_CLOSURE,
    FOP_CLOSE_UPVALUE,
    FOP_RETURN,
    FOP_CONTRACT,
    FOP_INHERIT,
    FOP_METHOD,
    FOP_BLOCKCHAIN_READ,
    FOP_BLOCKCHAIN_WRITE
  };

  void appendByte(uint8_t byte, int line);
  void disassemble(const std::string &name);
  int disassembleInstruction(int offset);
  int addConstant(const Value &value);
  int getLine(int offset);
  int getBytecodeCount();
  void setBytecodeValue(int offset, uint8_t byte);

  std::vector<uint8_t> getBytecode();
  std::vector<Value> getConstants();
};

#endif // FOREXT_BYTECODE_H
