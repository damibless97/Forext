#ifndef FOREXT_STACK_H
#define FOREXT_STACK_H


#include <forext/fxt_constants.h>
#include <forext/fxt_value.h>

#include <array>

class Stack {
  static const int stackMax{ 256 };
  static const int max{ constants::callFramesMax * stackMax };
  std::array<Value, max> arr;
  Value* top;

public:
  Stack();

  void reset();
  void push(const Value &value);
  Value pop();
  Value peek(int distance);
  Value at(int slot);
  void set(int slot, const Value &value);
  void print();
  int getSize();

  Value* getTop();
  void setTop(Value *value);
};

#endif // FOREXT_STACK_H
