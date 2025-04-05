#ifndef FOREXT_CLOSURE_OBJECT_H
#define FOREXT_CLOSURE_OBJECT_H

#include <forext/fxt_functionobject.h>
#include <forext/fxt_object.h>
#include <forext/fxt_upvalueobject.h>

#include <memory>
#include <vector>

class ClosureObject : public Object {
  std::shared_ptr<FunctionObject> function;
  std::vector<std::shared_ptr<UpvalueObject>> upvalues;

public:
  ClosureObject(std::shared_ptr<FunctionObject> function);

  void setUpvalue(int index, std::shared_ptr<UpvalueObject> upvalue);
  std::shared_ptr<UpvalueObject> getUpvalue(int index);

  FunctionObject* getFunction();
};

#endif // FOREXT_CLOSURE_OBJECT_H
