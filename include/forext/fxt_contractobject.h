#ifndef CONTRACT_OBJECT_H
#define CONTRACT_OBJECT_H

#include <forext/fxt_object.h>
#include <forext/fxt_stringobject.h>
#include <forext/fxt_value.h>

#include <memory>
#include <unordered_map>

class ContractObject : public Object {
  std::shared_ptr<StringObject> name;
  std::unordered_map<std::string, Value> methods;

public:
  ContractObject(std::shared_ptr<StringObject> name);

  void setMethod(const std::string &name, const Value &method);
  Value getMethod(const std::string &name);
  bool findMethod(const std::string &name);

  std::shared_ptr<StringObject> getName();
  std::unordered_map<std::string, Value>* getMethods();
};

#endif // CONTRACT_OBJECT_H
