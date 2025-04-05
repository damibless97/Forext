#ifndef INSTANCE_OBJECT_H
#define INSTANCE_OBJECT_H

#include <forext/fxt_object.h>
#include <forext/fxt_value.h>
#include <forext/fxt_contractobject.h>

#include <memory>
#include <unordered_map>

class InstanceObject : public Object {
  std::shared_ptr<ContractObject> contract;
  std::unordered_map<std::string, Value> fields;

public:
  InstanceObject(std::shared_ptr<ContractObject> contract);

  bool hasField(const std::string &name);

  Value getField(const std::string &name);
  void setField(const std::string &name, const Value &value);
  ContractObject* getContract();
};

#endif // INSTANCE_OBJECT_H
