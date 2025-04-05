#ifndef FOREXT_VALUE_H
#define FOREXT_VALUE_H

#include <forext/fxt_object.h>
#include <forext/fxt_stringobject.h>

#include <iostream>
#include <memory>

using uint256 = uint64_t; // Placeholder for uint256, replace with actual implementation if needed

class Value {
  union {
    bool boolean;
    uint256 number; // Replacing double with uint256
    std::shared_ptr<Object> object;
  };

public:
  enum class ValueType {
    VAL_BOOL,
    VAL_NUMBER,
    VAL_OBJECT,
    VAL_NULL,
    VAL_NONE
  } type = ValueType::VAL_NONE;

  Value();
  Value(bool boolean);
  Value(std::shared_ptr<Object> object);
  Value(ValueType type, uint256 number = 0);


  Value(const Value& value);
  Value& operator=(const Value& value);

  ~Value();

 

  // Accessors
  bool asBool() const;
  uint256 asNumber() const;
  std::shared_ptr<Object> asObject() const;
  std::shared_ptr<StringObject> asString() const;
  bool isBool() const;
  bool isNull();
  bool isNumber() const;
  bool isObject() const;
  bool isString() const;
  bool isFunction() const;
  bool isClosure()const;
  bool isUpvalue() const;
  bool isContract() const;
  bool isBoundMethod() const;
  bool isInstance() const;
  bool isFalsey();

  // Object type checks
  ObjectType getObjectType() const;
  std::string getFunctionName() const;
  std::string getClosureFunctionName() const;
  std::string getInstanceContractName() const;
  std::string getContractName() const;
  std::string getBoundMethodName() const;
  // Type checks
  ValueType getType() const;


  friend std::ostream& operator<<(std::ostream& out, const Value &value) {
    switch (value.type) {
      case ValueType::VAL_BOOL:
        value.asBool() ? out << "true" : out << "false";
        break;
      case ValueType::VAL_NULL: out << "null"; break;
      case ValueType::VAL_NUMBER: out << value.asNumber(); break;
      case ValueType::VAL_OBJECT:
        if (value.isString()) {
          out << *(value.asString());
        } else if (value.isFunction()) {
          out << value.getFunctionName();
        } else if (value.isClosure()) {
          out << value.getClosureFunctionName();
        } else if (value.isUpvalue()) {
          out << "upvalue";
        } else if (value.isInstance()) {
          out << value.getInstanceContractName();
        } else if (value.isContract()) {
          out << value.getContractName();
        } else if (value.isBoundMethod()) {
          out << "<fn " << value.getBoundMethodName() << ">";
        } else {
          out << "Unknown ObjectType";
        }
        break;
      case ValueType::VAL_NONE:
        out << "This Value currently has no type";
        break;
      default:
        out << "Unknown ValueType";
        break;
    }

    return out;
  }
};

#endif // FOREXT_VALUE_H