#ifndef FOREXT_OBJECT_H
#define FOREXT_OBJECT_H

#include <string>

enum class ObjectType {
  OBJECT_BOUND_METHOD,
  OBJECT_CONTRACT,
  OBJECT_CLOSURE,
  OBJECT_FUNCTION,
  OBJECT_INSTANCE,
  OBJECT_STRING,
  OBJECT_UPVALUE
};

class Object {
protected:
  ObjectType type;

public:
  Object(ObjectType type);

  ObjectType getType();

  friend std::ostream& operator<<(std::ostream& out, const Object &object);
};

#endif // FOREXT_OBJECT_H
