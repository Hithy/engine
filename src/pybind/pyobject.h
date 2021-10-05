#pragma once

#include <Python.h>
#include <iostream>

class BindObject;
struct PyBindObject;

struct PyBindObject {
  PyObject_HEAD
    BindObject* obj;
};

class BindObject {
public:
  BindObject() : _ref_count(1), _pyobj(nullptr) {}
  virtual ~BindObject() = default;

  BindObject(const BindObject&) = delete;
  BindObject& operator=(const BindObject&) = delete;

  void AddRef() { ++_ref_count; }
  void DecRef() {
    if (--_ref_count == 0) {
      delete this;
    }
  }
  void SetRef(int val) { _ref_count = val; }

  PyBindObject* GetPyObj() { return _pyobj; }
  void SetPyObj(PyBindObject* obj) { _pyobj = obj; }

private:
  PyBindObject* _pyobj;
  int _ref_count;
};

template<typename T>
class PyCXXObject : public BindObject {
public:
  static PyObject* boxing(PyCXXObject<T>& cobj) {
    auto pyobj = cobj.GetPyObj();
    if (pyobj) {
      Py_INCREF(pyobj);
      return (PyObject *)pyobj;
    }

    pyobj = PyObject_New(PyBindObject, T::GetPyType());
    pyobj->obj = &cobj;
    cobj.SetPyObj(pyobj);
    cobj.AddRef();
    return (PyObject*)pyobj;
  }

  static T& unboxing(PyObject* pyobj) {
    return *((T*)((PyBindObject*)pyobj)->obj);
  }
};

void pybind__dealloc__(PyObject* self);

template<typename T>
static PyObject* pybind__new__(PyTypeObject* subtype, PyObject* args, PyObject* kwds) {
  auto new_obj = subtype->tp_alloc(subtype, 0);
  T* obj = new T();
  ((PyBindObject*)new_obj)->obj = (BindObject*)obj;
  obj->SetPyObj((PyBindObject*)new_obj);
  std::cout << "new py obj: " << new_obj << std::endl;
  return new_obj;
}

#define DECLEAR_PYCXX_OBJECT_TYPE(cls) static PyTypeObject *GetPyType()

#define DEFINE_PYCXX_OBJECT_TYPE_BASE(cls, name, methods)                      \
  PyTypeObject *cls::GetPyType() {                                             \
    static PyTypeObject *new_type = nullptr;                                   \
    if (new_type) {                                                            \
      return new_type;                                                         \
    }                                                                          \
    new_type = new PyTypeObject{PyVarObject_HEAD_INIT(NULL, 0) name,           \
                                sizeof(PyBindObject)};                         \
    new_type->tp_dealloc = pybind__dealloc__;                                  \
    new_type->tp_new = pybind__new__<cls>;                                     \
    new_type->tp_methods = methods;                                            \
    new_type->tp_flags |= Py_TPFLAGS_BASETYPE;                                 \
    return new_type;                                                           \
  }

#define DEFINE_PYCXX_OBJECT_TYPE(base, cls, name, methods)                     \
  PyTypeObject *cls::GetPyType() {                                             \
    static PyTypeObject *new_type = nullptr;                                   \
    if (new_type) {                                                            \
      return new_type;                                                         \
    }                                                                          \
    new_type = new PyTypeObject{PyVarObject_HEAD_INIT(NULL, 0) name,           \
                                sizeof(PyBindObject)};                         \
    new_type->tp_dealloc = pybind__dealloc__;                                  \
    new_type->tp_new = pybind__new__<cls>;                                     \
    new_type->tp_methods = methods;                                            \
    new_type->tp_flags |= Py_TPFLAGS_BASETYPE;                                 \
    new_type->tp_base = base::GetPyType();                                     \
    return new_type;                                                           \
  }
