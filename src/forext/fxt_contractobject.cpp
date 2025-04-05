#include <forext/fxt_contractobject.h>

ContractObject::ContractObject(std::shared_ptr<StringObject> name) :
  Object{ ObjectType::OBJECT_CONTRACT }, name(std::move(name))
{
}

Value ContractObject::getMethod(const std::string &name) {
  return methods.at(name);
}

bool ContractObject::findMethod(const std::string &name) {
  std::unordered_map<std::string, Value>::iterator it = methods.find(name);
  return it != methods.end();
}

void ContractObject::setMethod(const std::string &name, const Value &method) {
  methods[name] = method;
}

std::shared_ptr<StringObject> ContractObject::getName() {
  return name;
}

std::unordered_map<std::string, Value>* ContractObject::getMethods() {
  return &methods;
}
