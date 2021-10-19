#include "pyobject.h"

void pybind__dealloc__(PyObject* self) {
  auto* cobj = ((PyBindObject*)self)->obj;
  if (cobj) {
    cobj->SetPyObj(nullptr);
    cobj->DecRef();
  }
  // ::printf("free obj py: %p, c: %p\n", self, cobj);
  PyTypeObject* tp = Py_TYPE(self);
  tp->tp_free(self);
}

PyObject* pybind__new__(PyTypeObject* subtype, PyObject* args, PyObject* kwds) {
  auto new_obj = subtype->tp_alloc(subtype, 0);
  // ::printf("new obj py: %p\n", new_obj);

  /*T* obj = new T();
  ((PyBindObject*)new_obj)->obj = (BindObject*)obj;
  obj->SetPyObj((PyBindObject*)new_obj);
  std::cout << "new py obj: " << new_obj << std::endl;*/
  return new_obj;
}