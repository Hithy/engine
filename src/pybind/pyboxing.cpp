#include "pyboxing.h"

namespace detail {
  template<> PyObject* base_box_struct<int>::boxing(int obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<unsigned int>::boxing(unsigned int obj) { return PyLong_FromUnsignedLong(obj); }
  template<> PyObject* base_box_struct<long>::boxing(long obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<unsigned long>::boxing(unsigned long obj) { return PyLong_FromUnsignedLong(obj); }
  template<> PyObject* base_box_struct<long long>::boxing(long long obj) { return PyLong_FromLongLong(obj); }
  template<> PyObject* base_box_struct<unsigned long long>::boxing(unsigned long long obj) { return PyLong_FromUnsignedLongLong(obj); }
  template<> PyObject* base_box_struct<short>::boxing(short obj) { return PyLong_FromLong(obj); }
  template<> PyObject* base_box_struct<double>::boxing(double obj) { return PyFloat_FromDouble(obj); }
  template<> PyObject* base_box_struct<float>::boxing(float obj) { return PyFloat_FromDouble(static_cast<double>(obj)); }

  template<> int base_box_struct<int>::unboxing(PyObject* obj) { return static_cast<int>(PyLong_AsLong(obj)); }
  template<> unsigned int base_box_struct<unsigned int>::unboxing(PyObject* obj) { return static_cast<unsigned int>(PyLong_AsUnsignedLong(obj)); }
  template<> long base_box_struct<long>::unboxing(PyObject* obj) { return PyLong_AsLong(obj); }
  template<> unsigned long base_box_struct<unsigned long>::unboxing(PyObject* obj) { return PyLong_AsUnsignedLong(obj); }
  template<> long long base_box_struct<long long>::unboxing(PyObject* obj) { return PyLong_AsLongLong(obj); }
  template<> unsigned long long base_box_struct<unsigned long long>::unboxing(PyObject* obj) { return PyLong_AsUnsignedLongLong(obj); }
  template<> short base_box_struct<short>::unboxing(PyObject* obj) { return static_cast<short>(PyLong_AsLong(obj)); }
  template<> double base_box_struct<double>::unboxing(PyObject* obj) { return PyFloat_AsDouble(obj); }
  template<> float base_box_struct<float>::unboxing(PyObject* obj) { return static_cast<float>(PyFloat_AsDouble(obj)); }
}