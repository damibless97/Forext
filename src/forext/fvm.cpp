#include <forext/fxt_boundmethodobject.h>
#include <forext/fxt_bytecode.h>
#include <forext/fxt_closureobject.h>
#include <forext/fxt_constants.h>
#include <forext/fxt_instanceobject.h>
#include <forext/fxt_value.h>
#include <forext/fvm.h>


#include <iostream>
#include <memory>
#include <stdarg.h>

FVM::~FVM() {
  // Destructor implementation
}

//#define DEBUG_TRACE_EXECUTION
FVM::FVM() : initString{ std::make_unique<StringObject>("init") }
{
}

InterpreterResult FVM::run() {
  CallFrame* frame = &callFrames[callFrameCount - 1];

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    std::cout << "          ";
    stack.print();
    std::cout << '\n';
    frame->closure->getFunction()->getBytecode()->disassembleInstruction(frame->functionProgramCounter);
#endif // DEBUG_TRACE_EXECUTION
    uint8_t instruction;
    switch (instruction = readByte(frame)) {
      case static_cast<unsigned int>(Bytecode::OpCode::FOP_CONSTANT): {
        Value constant = readConstant(frame);
        stack.push(constant);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_NULL):
        stack.push(Value{ Value::ValueType::VAL_NULL }); break;
      case static_cast<unsigned int>(Bytecode::OpCode::FOP_TRUE):
        stack.push(Value{ true }); break;
      case static_cast<unsigned int>(Bytecode::OpCode::FOP_FALSE):
        stack.push(Value{ false }); break;
      case static_cast<unsigned int>(Bytecode::OpCode::FOP_POP):
        stack.pop(); break;
      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GET_LOCAL): {
        uint8_t slot = readByte(frame);
        stack.push(frame->slots[slot]);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SET_LOCAL): {
        uint8_t slot = readByte(frame);
        frame->slots[slot] = stack.peek(0);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GET_GLOBAL): {
        auto name = readString(frame);
        Value value;
        std::unordered_map<std::string, Value>::iterator it = globals.find(name->getChars());
        if (it == globals.end()) {
          runtimeError("Undefined variable '%s'.", name->getChars().c_str());
          return InterpreterResult::RUNTIME_ERROR;
        }

        value = it->second;
        stack.push(value);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_DEFINE_GLOBAL): {
        auto name = readString(frame);
        globals[name->getChars()] = stack.peek(0);
        stack.pop();
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SET_GLOBAL): {
        auto name = readString(frame);
        std::unordered_map<std::string, Value>::iterator it = globals.find(name->getChars());
        if (it == globals.end()) {
          // Implicit variable declaration not allowed
          runtimeError("Undefined variable '%s'.", name->getChars().c_str());
          return InterpreterResult::RUNTIME_ERROR;
        } else {
          it->second = stack.peek(0);
        }

        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GET_UPVALUE): {
        uint8_t slot = readByte(frame);
        stack.push(*frame->closure->getUpvalue(slot)->getLocation());
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SET_UPVALUE): {
        uint8_t slot = readByte(frame);
        frame->closure->getUpvalue(slot)->setLocationValue(stack.peek(0));
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GET_PROPERTY): {
        if (!stack.peek(0).isInstance()) {
          runtimeError("Only instances have properties.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        auto instance = std::static_pointer_cast<InstanceObject>(stack.peek(0).asObject());
        auto name = readString(frame);
        Value value;
        if (instance->hasField(name->getChars())) {
          value = instance->getField(name->getChars());
          stack.pop();
          stack.push(value);
          break;
        }

        if (!bindMethod(instance->getContract(), name.get())) {
          return InterpreterResult::RUNTIME_ERROR;
        }

        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SET_PROPERTY): {
        if (!stack.peek(1).isInstance()) {
          runtimeError("Only instances have fields.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        auto instance = std::static_pointer_cast<InstanceObject>(stack.peek(1).asObject());
        instance->setField(readString(frame)->getChars(), stack.peek(0));
        Value value = stack.pop();
        stack.pop();
        stack.push(value);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GET_SUPER): {
        auto name = readString(frame);
        auto supercontract = std::static_pointer_cast<ContractObject>(stack.pop().asObject());
        if (!bindMethod(supercontract.get(), name.get())) {
          return InterpreterResult::RUNTIME_ERROR;
        }

        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_EQUAL): {
        Value b = stack.pop();
        Value a = stack.pop();
        stack.push(Value{ valuesEqual(a, b) });
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_GREATER):
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
          runtimeError("Operands must be numbers.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        greaterThan();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_LESS):
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
          runtimeError("Operands must be numbers.");
          return InterpreterResult::RUNTIME_ERROR;
        }
        lessThan();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_ADD):
        if (stack.peek(0).isString() && stack.peek(1).isString()) {
          auto b = stack.pop().asString();
          auto a = stack.pop().asString();
          auto result{ std::make_shared<StringObject>((*a).getChars() + (*b).getChars()) };
          stack.push(Value{ result });
        } else if (stack.peek(0).isNumber() && stack.peek(1).isNumber()) {
          add();
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SUBTRACT):
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
          runtimeError("Operands must be numbers.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        subtract();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_DIVIDE):
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
          runtimeError("Operands must be numbers.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        divide();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_MULTIPLY):
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
          runtimeError("Operands must be numbers.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        multiply();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_NOT):
        stack.push(Value{ stack.pop().isFalsey() });
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_NEGATE): {
        if (!stack.peek(0).isNumber()) {
          runtimeError("Operand must be a number.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        double negatedValue = -(stack.pop().asNumber());
        stack.push(Value{ Value::ValueType::VAL_NUMBER, negatedValue });
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_PRINT): {
        std::cout << stack.pop() << '\n';
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_JUMP): {
        uint16_t offset = readShort(frame);
        frame->functionProgramCounter += offset;
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_JUMP_IF_FALSE): {
        uint16_t offset = readShort(frame);
        if (stack.peek(0).isFalsey()) frame->functionProgramCounter += offset;
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_LOOP): {
        uint16_t offset = readShort(frame);
        frame->functionProgramCounter -= offset;
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_CALL): {
        int argCount = static_cast<int>(readByte(frame));
        if (!callValue(stack.peek(argCount), argCount)) {
          return InterpreterResult::RUNTIME_ERROR;
        }

        frame = &(callFrames[callFrameCount - 1]);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_INVOKE): {
        auto method = readString(frame);
        int argCount = static_cast<int>(readByte(frame));
        if (!invoke(method.get(), argCount)) {
          return InterpreterResult::RUNTIME_ERROR;
        }

        frame = &(callFrames[callFrameCount - 1]);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_SUPER_INVOKE): {
        auto method = readString(frame);
        int argCount = readByte(frame);
        auto supercontract = std::static_pointer_cast<ContractObject>(stack.pop().asObject());
        if (!invokeFromContract(supercontract.get(), method.get(), argCount)) {
          return InterpreterResult::RUNTIME_ERROR;
        }

        frame = &(callFrames[callFrameCount - 1]);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_CLOSURE): {
        auto function = std::static_pointer_cast<FunctionObject>(readConstant(frame).asObject());
        auto closure{ std::make_shared<ClosureObject>(function) };
        stack.push(Value{ closure });
        for (int i = 0; i < closure->getFunction()->getUpvalueCount(); i++) {
          uint8_t isLocal = readByte(frame);
          uint8_t index = readByte(frame);
          if (isLocal) {
            closure->setUpvalue(i, captureUpvalue(frame->slots + index));
          } else {
            closure->setUpvalue(i, frame->closure->getUpvalue(index));
          }
        }

        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_CLOSE_UPVALUE):
        closeUpvalues(stack.getTop() - 1);
        stack.pop();
        break;

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_RETURN): {
        Value result = stack.pop();
        closeUpvalues(frame->slots);
        callFrameCount--;
        if (callFrameCount == 0) {
          stack.pop();
          return InterpreterResult::OK;
        }

        stack.setTop(frame->slots);
        stack.push(result);
        frame = &(callFrames[callFrameCount - 1]);
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_CONTRACT): {
        auto contractObject{ std::make_shared<ContractObject>(readString(frame)) };
        stack.push(Value{ contractObject });
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_INHERIT): {
        Value supercontract = stack.peek(1);
        if (!supercontract.isContract()) {
          runtimeError("supercontract must be a class.");
          return InterpreterResult::RUNTIME_ERROR;
        }

        auto subclassPointer = std::static_pointer_cast<ContractObject>(stack.peek(0).asObject());
        auto supercontractPointer = std::static_pointer_cast<ContractObject>(supercontract.asObject());
        auto supercontractMethods = supercontractPointer->getMethods();
        subclassPointer->getMethods()->insert(supercontractMethods->begin(), supercontractMethods->end());
        stack.pop();
        break;
      }

      case static_cast<unsigned int>(Bytecode::OpCode::FOP_METHOD): {
        defineMethod(readString(frame).get());
        break;
      }
    }
  }
}

uint8_t FVM::readByte(CallFrame* frame) {
  return frame->closure->getFunction()->getBytecode()->getBytecode().at(frame->functionProgramCounter++);
}

uint16_t FVM::readShort(CallFrame* frame) {
  frame->functionProgramCounter += 2;
  uint8_t firstPart = frame->closure->getFunction()->getBytecode()->getBytecode().at(frame->functionProgramCounter-2) << 8;
  uint8_t secondPart = frame->closure->getFunction()->getBytecode()->getBytecode().at(frame->functionProgramCounter-1);
  return firstPart | secondPart;
}

Value FVM::readConstant(CallFrame* frame) {
  return frame->closure->getFunction()->getBytecode()->getConstants().at(readByte(frame));
}

std::shared_ptr<StringObject> FVM::readString(CallFrame* frame) {
  return readConstant(frame).asString();
}

void FVM::add() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ Value::ValueType::VAL_NUMBER, a + b });
}

void FVM::subtract() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ Value::ValueType::VAL_NUMBER, a - b });
}

void FVM::divide() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ Value::ValueType::VAL_NUMBER, a / b });
}

void FVM::multiply() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ Value::ValueType::VAL_NUMBER, a * b });
}

void FVM::greaterThan() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ a > b });
}

void FVM::lessThan() {
  double b = stack.pop().asNumber();
  double a = stack.pop().asNumber();
  stack.push(Value{ a < b });
}

void FVM::runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = callFrameCount - 1; i >= 0; i--) {
    CallFrame* frame = &(callFrames[i]);
    FunctionObject* function = frame->closure->getFunction();
    int instruction = frame->functionProgramCounter - 1;
    std::cerr << "[line " << function->getBytecode()->getLine(instruction) << "] in ";
    if (function->getName() == nullptr) {
      std::cerr << "script\n";
    } else {
      std::cerr << function->getName()->getChars() << '\n';
    }
  }

  stack.reset();
  callFrameCount = 0;
  openUpvalues = nullptr;
}

void FVM::defineMethod(StringObject* name) {
  Value method = stack.peek(0);
  auto contract = std::static_pointer_cast<ContractObject>(stack.peek(1).asObject());
  contract->setMethod(name->getChars(), method);
  stack.pop();
}

bool FVM::bindMethod(ContractObject* contract, StringObject* name) {
  Value method;
  if (!contract->findMethod(name->getChars())) {
    runtimeError("Undefined property '%s'.", name->getChars().c_str());
    return false;
  }

  method = contract->getMethod(name->getChars());
  auto closure = std::static_pointer_cast<ClosureObject>(method.asObject());
  auto bound{ std::make_shared<BoundMethodObject>(stack.peek(0), closure) };
  stack.pop();
  stack.push(Value{ bound });
  return true;
}

bool FVM::invoke(StringObject* name, int argCount) {
  Value receiver = stack.peek(argCount);
  if (!receiver.isInstance()) {
    runtimeError("Only instances have methods.");
    return false;
  }

  auto instance = std::static_pointer_cast<InstanceObject>(receiver.asObject());
  Value value;
  if (instance->hasField(name->getChars())) {
    value = instance->getField(name->getChars());
    Value* valueToSet = stack.getTop() - argCount - 1;
    *valueToSet = value;
    return callValue(value, argCount);
  }

  return invokeFromContract(instance->getContract(), name, argCount);
}

bool FVM::invokeFromContract(ContractObject* contract, StringObject* name, int argCount) {
  Value method;
  if (contract->findMethod(name->getChars())) {
    method = contract->getMethod(name->getChars());
  } else {
    runtimeError("Undefined property '%s'.", name->getChars().c_str());
    return false;
  }

  auto closure = std::static_pointer_cast<ClosureObject>(method.asObject());
  return call(closure.get(), argCount);
}

bool FVM::valuesEqual(const Value &a, const Value &b) {
  if (a.getType() != b.getType()) return false;

  switch (a.getType()) {
    case Value::ValueType::VAL_BOOL:   return a.asBool() == b.asBool();
    case Value::ValueType::VAL_NULL:   return true;
    case Value::ValueType::VAL_NUMBER: return a.asNumber() == b.asNumber();
    case Value::ValueType::VAL_OBJECT: {
      return a.asObject() == b.asObject();
    }
    default:
      return false;
  }
}

bool FVM::callValue(const Value &callee, int argCount) {
  if (callee.isObject()) {
    switch (callee.getObjectType()) {
      case ObjectType::OBJECT_BOUND_METHOD: {
        auto bound = std::static_pointer_cast<BoundMethodObject>(callee.asObject());
        Value* valueToSet = stack.getTop() - argCount - 1;
        *valueToSet = bound->getReceiver();
        return call(bound->getMethod().get(), argCount);
      }

      case ObjectType::OBJECT_CONTRACT: {
        auto contract = std::static_pointer_cast<ContractObject>(callee.asObject());
        auto instance{ std::make_shared<InstanceObject>(contract) };
        Value value = Value{ instance };
        Value* valueToSet = stack.getTop() - argCount - 1;
        *valueToSet = value;
        Value initializer;
        if (contract->findMethod(initString->getChars())) {
          initializer = contract->getMethod(initString->getChars());
          auto initializerClosure = std::static_pointer_cast<ClosureObject>(initializer.asObject());
          return call(initializerClosure.get(), argCount);
        } else if (argCount != 0) {
          runtimeError("Expected 0 arguments but got %d.", argCount);
          return false;
        }

        return true;
      }

      case ObjectType::OBJECT_CLOSURE: {
        auto calleeClosure = std::static_pointer_cast<ClosureObject>(callee.asObject());
        return call(calleeClosure.get(), argCount);
      }

      default:
        // Non-callable object type.
        break;
    }
  }

  runtimeError("Can only call functions and classes.");
  return false;
}

bool FVM::call(ClosureObject* closure, int argCount) {
  if (argCount != closure->getFunction()->getArity()) {
    runtimeError("Expected %d arguments but got %d.",
        closure->getFunction()->getArity(), argCount);
    return false;
  }

  if (callFrameCount == constants::callFramesMax) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &(callFrames[callFrameCount++]);
  frame->closure = closure;
  frame->functionProgramCounter = 0;
  frame->slots = stack.getTop() - argCount - 1;
  return true;
}

std::shared_ptr<UpvalueObject> FVM::captureUpvalue(Value* local) {
  std::shared_ptr<UpvalueObject> previousUpvalue;
  auto upvalue = openUpvalues;
  while (upvalue != nullptr && upvalue->getLocation() > local) {
    previousUpvalue = upvalue;
    upvalue = upvalue->getNext();
  }

  if (upvalue != nullptr && upvalue->getLocation() == local) return upvalue;
  auto createdUpvalue{ std::make_shared<UpvalueObject>( local ) };
  createdUpvalue->setNext(upvalue);
  if (previousUpvalue == nullptr) {
    openUpvalues = createdUpvalue;
  } else {
    previousUpvalue->setNext(createdUpvalue);
  }

  return createdUpvalue;
}

void FVM::closeUpvalues(Value* last) {
  while (openUpvalues != nullptr &&
         openUpvalues->getLocation() >= last) {

    auto upvalue = openUpvalues;
    upvalue->setClosed(*(upvalue->getLocation()));
    upvalue->setLocation(upvalue->getClosed());
    openUpvalues = upvalue->getNext();
  }
}

void FVM::pushOntoStack(const Value &value) {
  stack.push(value);
}