#include <forext/fxt_bytecode.h>
#include <forext/fxt_functionobject.h>

#include <iostream>
#include <memory>

void Bytecode::appendByte(uint8_t byte, int line) {
  bytecode.push_back(byte);
  if (!lines.empty() && lines.at(lines.size()-1).line == line) return;
  lines.push_back({ line, static_cast<int>(bytecode.size()-1) });
}

void Bytecode::disassemble(const std::string &name) {
  std::cout << "== " << name << " ==\n";
  for (int offset = 0; offset < static_cast<int>(bytecode.size()); ) {
    offset = disassembleInstruction(offset);
  }
}

int Bytecode::disassembleInstruction(int offset) {
  printf("%04d ", offset);
  int line = getLine(offset);
  if (offset > 0 && line == getLine(offset-1)) {
    std::cout << "   | ";
  } else {
    printf("%4d ", line);
  }

  uint8_t instruction = bytecode.at(offset);
  switch (instruction) {
    case static_cast<unsigned int>(OpCode::FOP_CONSTANT):
      return disassembleConstantInstruction("FOP_CONSTANT", offset);
    case static_cast<unsigned int>(OpCode::FOP_NULL):
      return disassembleSimpleInstruction("FOP_NULL", offset);
    case static_cast<unsigned int>(OpCode::FOP_TRUE):
      return disassembleSimpleInstruction("FOP_TRUE", offset);
    case static_cast<unsigned int>(OpCode::FOP_FALSE):
      return disassembleSimpleInstruction("FOP_FALSE", offset);
    case static_cast<unsigned int>(OpCode::FOP_POP):
      return disassembleSimpleInstruction("FOP_POP", offset);
    case static_cast<unsigned int>(OpCode::FOP_GET_LOCAL):
      return disassembleByteInstruction("FOP_GET_LOCAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_SET_LOCAL):
      return disassembleByteInstruction("FOP_SET_LOCAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_GET_GLOBAL):
      return disassembleConstantInstruction("FOP_GET_GLOBAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_DEFINE_GLOBAL):
      return disassembleConstantInstruction("FOP_DEFINE_GLOBAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_SET_GLOBAL):
      return disassembleConstantInstruction("FOP_SET_GLOBAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_GET_UPVALUE):
      return disassembleByteInstruction("FOP_GET_UPVALUE", offset);
    case static_cast<unsigned int>(OpCode::FOP_SET_UPVALUE):
      return disassembleByteInstruction("FOP_SET_UPVALUE", offset);
    case static_cast<unsigned int>(OpCode::FOP_GET_PROPERTY):
      return disassembleConstantInstruction("FOP_GET_PROPERTY", offset);
    case static_cast<unsigned int>(OpCode::FOP_SET_PROPERTY):
      return disassembleConstantInstruction("FOP_SET_PROPERTY", offset);
    case static_cast<unsigned int>(OpCode::FOP_GET_SUPER):
      return disassembleConstantInstruction("FOP_GET_SUPER", offset);
    case static_cast<unsigned int>(OpCode::FOP_EQUAL):
      return disassembleSimpleInstruction("FOP_EQUAL", offset);
    case static_cast<unsigned int>(OpCode::FOP_GREATER):
      return disassembleSimpleInstruction("FOP_GREATER", offset);
    case static_cast<unsigned int>(OpCode::FOP_LESS):
      return disassembleSimpleInstruction("FOP_LESS", offset);
    case static_cast<unsigned int>(OpCode::FOP_ADD):
      return disassembleSimpleInstruction("FOP_ADD", offset);
    case static_cast<unsigned int>(OpCode::FOP_SUBTRACT):
      return disassembleSimpleInstruction("FOP_SUBTRACT", offset);
    case static_cast<unsigned int>(OpCode::FOP_MULTIPLY):
      return disassembleSimpleInstruction("FOP_MULTIPLY", offset);
    case static_cast<unsigned int>(OpCode::FOP_DIVIDE):
      return disassembleSimpleInstruction("FOP_DIVIDE", offset);
    case static_cast<unsigned int>(OpCode::FOP_NOT):
      return disassembleSimpleInstruction("FOP_NOT", offset);
    case static_cast<unsigned int>(OpCode::FOP_NEGATE):
      return disassembleSimpleInstruction("FOP_NEGATE", offset);
    case static_cast<unsigned int>(OpCode::FOP_PRINT):
      return disassembleSimpleInstruction("FOP_PRINT", offset);
    case static_cast<unsigned int>(OpCode::FOP_JUMP):
      return disassembleJumpInstruction("FOP_JUMP", 1, offset);
    case static_cast<unsigned int>(OpCode::FOP_JUMP_IF_FALSE):
      return disassembleJumpInstruction("FOP_JUMP_IF_FALSE", 1, offset);
    case static_cast<unsigned int>(OpCode::FOP_LOOP):
      return disassembleJumpInstruction("FOP_LOOP", -1, offset);
    case static_cast<unsigned int>(OpCode::FOP_CALL):
      return disassembleByteInstruction("FOP_CALL", offset);
    case static_cast<unsigned int>(OpCode::FOP_INVOKE):
      return disassembleInvokeInstruction("FOP_INVOKE", offset);
    case static_cast<unsigned int>(OpCode::FOP_SUPER_INVOKE):
      return disassembleInvokeInstruction("FOP_SUPER_INVOKE", offset);
    case static_cast<unsigned int>(OpCode::FOP_CLOSURE): {
      offset++;
      uint8_t constant = bytecode.at(offset++);
      printf("%-16s %4d ", "FOP_CLOSURE", constant);
      std::cout << "<fn " << constants.at(constant) << ">\n";
      auto function = std::static_pointer_cast<FunctionObject>(constants.at(constant).asObject());
      auto upvalueCount = function->getUpvalueCount();
      for (int j = 0; j < upvalueCount; j++) {
        int isLocal = bytecode.at(offset++);
        int index = bytecode.at(offset++);
        printf("%04d      |                     %s %d\n",
               offset - 2,
               isLocal ? "local" : "upvalue",
               index);
      }

      return offset;
    }
    case static_cast<unsigned int>(OpCode::FOP_CLOSE_UPVALUE):
      return disassembleSimpleInstruction("FOP_CLOSE_UPVALUE", offset);
    case static_cast<unsigned int>(OpCode::FOP_RETURN):
      return disassembleSimpleInstruction("FOP_RETURN", offset);
    case static_cast<unsigned int>(OpCode::FOP_CONTRACT):
      return disassembleConstantInstruction("FOP_CLASS", offset);
    case static_cast<unsigned int>(OpCode::FOP_INHERIT):
      return disassembleSimpleInstruction("FOP_INHERIT", offset);
    case static_cast<unsigned int>(OpCode::FOP_METHOD):
      return disassembleConstantInstruction("FOP_METHOD", offset);
    default:
      std::cout << "Unknown opcode " << +instruction << '\n';
      return offset + 1;
  }
}

int Bytecode::disassembleSimpleInstruction(const std::string& name, int offset) {
  std::cout << name << '\n';
  return offset + 1;
}

int Bytecode::addConstant(const Value &value) {
  constants.push_back(value);
  return static_cast<int>(constants.size() - 1);
}

int Bytecode::disassembleConstantInstruction(const std::string& name, int offset) {
  uint8_t constant = bytecode.at(offset + 1);
  printf("%-16s %4d '", name.c_str(), constant);
  std::cout << constants.at(constant) << "'\n";
  return offset + 2;
}

int Bytecode::disassembleByteInstruction(const std::string& name, int offset) {
  uint8_t slot = bytecode.at(offset + 1);
  printf("%-16s %4d\n", name.c_str(), slot);
  return offset + 2;
}

int Bytecode::disassembleInvokeInstruction(const std::string& name, int offset) {
  uint8_t constant = bytecode.at(offset + 1);
  uint8_t argCount = bytecode.at(offset + 2);
  printf("%-16s (%d args) %4d '", name.c_str(), argCount, constant);
  std::cout << constants.at(constant) << "'\n";
  return offset + 3;
}

int Bytecode::disassembleJumpInstruction(const std::string& name, int sign, int offset) {
  uint16_t jump = static_cast<uint16_t>(bytecode.at(offset + 1) << 8);
  jump |= bytecode.at(offset + 2);
  printf("%-16s %4d -> %d\n", name.c_str(), offset, offset + 3 + (sign * jump));
  return offset + 3;
}

int Bytecode::getLine(int offset) {
  if (offset < 0) return -1;
  int start = 0;
  int end = static_cast<int>(lines.size() - 1);
  for (;;) {
    int mid = (start + end) / 2;
    LineStart lineStart = lines.at(mid);
    if (offset < lineStart.offset) {
      end = mid - 1;
    } else if (mid == static_cast<int>(lines.size() - 1) ||
               offset < lines.at(mid + 1).offset) {

      return lineStart.line;
    } else {
      start = mid + 1;
    }
  }
}

void Bytecode::setBytecodeValue(int offset, uint8_t byte) {
  bytecode.at(offset) = byte;
}

int Bytecode::getBytecodeCount() {
  return static_cast<int>(bytecode.size());
}

std::vector<uint8_t> Bytecode::getBytecode() {
  return bytecode;
}

std::vector<Value> Bytecode::getConstants() {
  return constants;
}
