#pragma once
#include <cstdint>

namespace ECS {

enum EntityType {
  EntityType_Unknown = 0,
  EntityType_Base,
};

class IEntity {
public:
  virtual EntityType GetType() const = 0;
  virtual ~IEntity(){};
  virtual uint64_t GetID() = 0;
};

} // namespace ECS
