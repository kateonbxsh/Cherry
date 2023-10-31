#include "type.h"
#include <utility>

Type::Type(std::string typeName, std::any defaultValue) {
    name = std::move(typeName);
}


void Type::setConstructor(const Value& function) {

}


