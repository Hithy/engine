#include "pyobject.h"

void pybind__dealloc__(PyObject* self) {
  auto* cobj = ((PyBindObject*)self)->obj;
  if (cobj) {
    cobj->SetPyObj(nullptr);
    cobj->DecRef();
  }

  PyTypeObject* tp = Py_TYPE(self);
  tp->tp_free(self);
}