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

  virtual PyTypeObject* GetRealType() = 0;

private:
  PyBindObject* _pyobj;
  int _ref_count;
};

template<typename T>
class PyCXXObject {
public:
  static PyObject* boxing(BindObject* cobj) {
    auto pyobj = cobj->GetPyObj();
    if (pyobj) {
      Py_INCREF(pyobj);
      return (PyObject *)pyobj;
    }

    pyobj = PyObject_New(PyBindObject, cobj->GetRealType());
    pyobj->obj = cobj;
    cobj->SetPyObj(pyobj);
    cobj->AddRef();
    return (PyObject*)pyobj;
  }

  static T& unboxing(PyObject* pyobj) {
    return *((T*)((PyBindObject*)pyobj)->obj);
  }
};

void pybind__dealloc__(PyObject* self);
PyObject* pybind__new__(PyTypeObject* subtype, PyObject* args, PyObject* kwds);

#define DECLEAR_PYCXX_OBJECT_TYPE(cls) \
  static PyTypeObject *GetPyType(); \
  virtual PyTypeObject *GetRealType() { \
    return GetPyType(); \
  }

#define DEFINE_PYCXX_OBJECT_TYPE_BASE(cls, name, methods, init_params)                      \
  PyTypeObject *cls::GetPyType() {                                             \
    static PyTypeObject *new_type = nullptr;                                   \
    if (new_type) {                                                            \
      return new_type;                                                         \
    }                                                                          \
    new_type = new PyTypeObject{PyVarObject_HEAD_INIT(NULL, 0) name,           \
                                sizeof(PyBindObject)};                         \
    new_type->tp_dealloc = pybind__dealloc__;                                  \
    new_type->tp_init = GenPyInitFunc<cls>(init_params);                                  \
    new_type->tp_new = pybind__new__;                                     \
    new_type->tp_methods = methods;                                            \
    new_type->tp_flags |= Py_TPFLAGS_BASETYPE;                                 \
    return new_type;                                                           \
  }

#define DEFINE_PYCXX_OBJECT_TYPE(base, cls, name, methods, init_params)                     \
  PyTypeObject *cls::GetPyType() {                                             \
    static PyTypeObject *new_type = nullptr;                                   \
    if (new_type) {                                                            \
      return new_type;                                                         \
    }                                                                          \
    new_type = new PyTypeObject{PyVarObject_HEAD_INIT(NULL, 0) name,           \
                                sizeof(PyBindObject)};                         \
    new_type->tp_dealloc = pybind__dealloc__;                                  \
    new_type->tp_init = GenPyInitFunc<cls>(init_params);                                  \
    new_type->tp_new = pybind__new__;                                     \
    new_type->tp_methods = methods;                                            \
    new_type->tp_flags |= Py_TPFLAGS_BASETYPE;                                 \
    new_type->tp_base = base::GetPyType();                                     \
    return new_type;                                                           \
  }
