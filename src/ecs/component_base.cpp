#include "component_base.h"

#include "pybind/pybind.h"

namespace ECS {


DEFINE_PYCXX_OBJECT_TYPE_BASE(Component, "Component", nullptr, py_init_params<>())

}; // namespace ECS
