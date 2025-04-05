#ifndef FOREXT_FUNCTION_OBJECT_H
#define FOREXT_FUNCTION_OBJECT_H

#include <forext/fxt_bytecode.h>
#include <forext/fxt_object.h>
#include <forext/fxt_stringobject.h>

#include <memory>

class FunctionObject : public Object {
  int arity{ 0 };
  int upvalueCount{ 0 };
  Bytecode bytecode;
  std::shared_ptr<StringObject> name;

public:
  enum class FunctionType {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
  };

  FunctionObject();

  void incrementArity();
  void incrementUpvalueCount();

  StringObject* getName();
  Bytecode* getBytecode();
  int getArity();
  int getUpvalueCount();
  void setName(std::shared_ptr<StringObject> name);

  friend std::ostream& operator<<(std::ostream& out, const FunctionObject &object) {
    return out << (object.name)->getChars();
  }
};


#endif // FOREXT_FUNCTION_OBJECT_H
