#include "pyboxing.h"

#include "glm/glm.hpp"
#include <glm/gtx/quaternion.hpp>
#include <string>

namespace detail {
  template<> PyObject* base_box_struct<bool>::boxing(bool obj) 
  {
    if (obj) {
      Py_RETURN_TRUE;
    }
    else {
      Py_RETURN_FALSE;
    }
  }
  template<> PyObject* base_box_struct<int>::boxing(int obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<unsigned int>::boxing(unsigned int obj) { return PyLong_FromUnsignedLong(obj); }
  template<> PyObject* base_box_struct<long>::boxing(long obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<unsigned long>::boxing(unsigned long obj) { return PyLong_FromUnsignedLong(obj); }
  template<> PyObject* base_box_struct<long long>::boxing(long long obj) { return PyLong_FromLongLong(obj); }
  template<> PyObject* base_box_struct<unsigned long long>::boxing(unsigned long long obj) { return PyLong_FromUnsignedLongLong(obj); }
  template<> PyObject* base_box_struct<short>::boxing(short obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<double>::boxing(double obj) { return PyFloat_FromDouble(obj); }
  template<> PyObject* base_box_struct<float>::boxing(float obj) { return PyFloat_FromDouble(static_cast<double>(obj)); }

  template<> bool base_box_struct<bool>::unboxing(PyObject* obj) { return PyObject_IsTrue(obj); }
  template<> int base_box_struct<int>::unboxing(PyObject* obj) { return static_cast<int>(PyLong_AsLong(obj)); }
  template<> unsigned int base_box_struct<unsigned int>::unboxing(PyObject* obj) { return static_cast<unsigned int>(PyLong_AsUnsignedLong(obj)); }
  template<> long base_box_struct<long>::unboxing(PyObject* obj) { return PyLong_AsLong(obj); }
  template<> unsigned long base_box_struct<unsigned long>::unboxing(PyObject* obj) { return PyLong_AsUnsignedLong(obj); }
  template<> long long base_box_struct<long long>::unboxing(PyObject* obj) { return PyLong_AsLongLong(obj); }
  template<> unsigned long long base_box_struct<unsigned long long>::unboxing(PyObject* obj) { return PyLong_AsUnsignedLongLong(obj); }
  template<> short base_box_struct<short>::unboxing(PyObject* obj) { return static_cast<short>(PyLong_AsLong(obj)); }
  template<> double base_box_struct<double>::unboxing(PyObject* obj) { return PyFloat_AsDouble(obj); }
  template<> float base_box_struct<float>::unboxing(PyObject* obj) { return static_cast<float>(PyFloat_AsDouble(obj)); }

  template<> PyObject* base_box_struct<glm::vec3>::boxing(glm::vec3 obj) 
  { 
    auto res = PyList_New(3);
    PyList_SetItem(res, 0, detail::boxing(obj.x));
    PyList_SetItem(res, 1, detail::boxing(obj.y));
    PyList_SetItem(res, 2, detail::boxing(obj.z));
    return res;
  }
  template<> glm::vec3 base_box_struct<glm::vec3>::unboxing(PyObject* obj)
  { 
    glm::vec3 res;
    res.x = detail::unboxing<float>(PyList_GetItem(obj, 0));
    res.y = detail::unboxing<float>(PyList_GetItem(obj, 1));
    res.z = detail::unboxing<float>(PyList_GetItem(obj, 2));
    return res;
  }

  template<> PyObject* base_box_struct<glm::quat>::boxing(glm::quat obj)
  {
    auto res = PyList_New(4);
    PyList_SetItem(res, 0, detail::boxing(obj.x));
    PyList_SetItem(res, 1, detail::boxing(obj.y));
    PyList_SetItem(res, 2, detail::boxing(obj.z));
    PyList_SetItem(res, 3, detail::boxing(obj.w));
    return res;
  }
  template<> glm::quat base_box_struct<glm::quat>::unboxing(PyObject* obj)
  {
    glm::quat res;
    res.x = detail::unboxing<float>(PyList_GetItem(obj, 0));
    res.y = detail::unboxing<float>(PyList_GetItem(obj, 1));
    res.z = detail::unboxing<float>(PyList_GetItem(obj, 2));
    res.w = detail::unboxing<float>(PyList_GetItem(obj, 3));
    return res;
  }

  template<> PyObject* base_box_struct<std::string>::boxing(std::string obj) 
  { 
    return _PyUnicode_FromASCII(obj.c_str(), obj.size());
  }
  template<> std::string base_box_struct<std::string>::unboxing(PyObject* obj) {
    return _PyUnicode_AsString(obj);
  }

  template<> PyObject* base_box_struct<const char *>::boxing(const char* obj)
  {
    return _PyUnicode_FromASCII(obj, strlen(obj));
  }
  template<> const char* base_box_struct<const char*>::unboxing(PyObject* obj) {
    return _PyUnicode_AsString(obj);
  }

}