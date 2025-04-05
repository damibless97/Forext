#ifndef FOREXT_BOUND_METHOD_OBJECT_H
#define FOREXT_BOUND_METHOD_OBJECT_H

#include <forext/fxt_object.h>
#include <forext/fxt_value.h>
#include <forext/fxt_closureobject.h>

#include <memory>

class BoundMethodObject : public Object {
  Value receiver;
  std::shared_ptr<ClosureObject> method;

public:
  BoundMethodObject(const Value &receiver,
                    std::shared_ptr<ClosureObject> method);

  std::shared_ptr<ClosureObject> getMethod();
  Value getReceiver();
};

#endif // FOREXT_BOUND_METHOD_OBJECT_H
