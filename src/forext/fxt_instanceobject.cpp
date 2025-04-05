#include <forext/fxt_instanceobject.h>

#include <utility>

InstanceObject::InstanceObject(std::shared_ptr<ContractObject> contract) :
  Object{ ObjectType::OBJECT_INSTANCE }, contract(std::move(contract))
{
}

bool InstanceObject::hasField(const std::string &name) {
  std::unordered_map<std::string, Value>::iterator it = fields.find(name);
  return it != fields.end();
}

Value InstanceObject::getField(const std::string &name) {
  return fields.at(name);
}

void InstanceObject::setField(const std::string &name, const Value &value) {
  fields[name] = value;
}

ContractObject* InstanceObject::getContract() {
  return contract.get();
}
