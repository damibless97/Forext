//> A Virtual Machine fvm-h
#ifndef FOREXT_VM_H
#define FOREXT_VM_H

#include <forext/fxt_contractobject.h>
#include <forext/fxt_closureobject.h>
#include <forext/fxt_constants.h>
#include <forext/fxt_stack.h>
#include <forext/fxt_value.h>

#include <memory>
#include <unordered_map>

enum class InterpreterResult {
  OK,
  COMPILATION_ERROR,
  RUNTIME_ERROR
};


struct CallFrame {
  ClosureObject* closure;
  int functionProgramCounter;
  Value* slots;
};



class FVM {

  CallFrame callFrames[constants::callFramesMax];
  int callFrameCount{ 0 };
  Stack stack;
  std::unordered_map<std::string, Value> globals;
  std::shared_ptr<UpvalueObject> openUpvalues;
  std::unique_ptr<StringObject> initString;


  uint8_t readByte(CallFrame* frame);
  uint16_t readShort(CallFrame* frame);
  Value readConstant(CallFrame* frame);
  std::shared_ptr<StringObject> readString(CallFrame* frame);
  void add();
  void subtract();
  void divide();
  void multiply();
  void greaterThan();
  void lessThan();
  void runtimeError(const char* format, ...);
  bool valuesEqual(const Value &a, const Value &b);
  bool callValue(const Value &callee, int argCount);
  std::shared_ptr<UpvalueObject> captureUpvalue(Value* local);
  void closeUpvalues(Value* last);
  void defineMethod(StringObject* name);
  bool bindMethod(ContractObject* klass, StringObject* name);
  bool invoke(StringObject* name, int argCount);
  bool invokeFromContract(ContractObject* klass, StringObject* name, int argCount);
  StringObject* copyString(const std::string &name);


  public:
      FVM();
      ~FVM();

      InterpreterResult run();
      bool call(ClosureObject* closure, int argCount);
      void pushOntoStack(const Value &value);
      Value* getStackTop();
};

#endif // FOREXT_VM_H