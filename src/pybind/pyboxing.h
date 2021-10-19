#pragma once

#include <Python.h>
#include <type_traits>
#include <vector>

#include "pyobject.h"

namespace detail {
template <typename T> PyObject *boxing(T &&obj);

template <typename T> decltype(auto) unboxing(PyObject *obj);

template <typename> struct get_vector_type {};

template <typename T, typename A> struct get_vector_type<std::vector<T, A>> {
  using type = T;
};

template <typename U> struct base_box_struct {
  static PyObject *boxing(U obj);
  static U unboxing(PyObject *obj);
};

template <typename U> struct vector_box_struct {
  static PyObject *boxing(const U &obj) {
    auto res = PyList_New(obj.size());
    for (int i = 0; i < obj.size(); i++) {
      auto item = detail::boxing(obj[i]);
      PyList_SetItem(res, i, item);
    }
    return res;
  }

  static U unboxing(PyObject *obj) {
    int list_size = PyList_GET_SIZE(obj);
    U res(list_size);

    for (int i = 0; i < list_size; i++) {
      auto item = PyList_GetItem(obj, i);
      res[i] = detail::unboxing<typename get_vector_type<U>::type>(item);
    }

    return res;
  }
};

template <typename U> struct obj_box_struct {
  static PyObject *boxing(U &obj) { return PyCXXObject<U>::boxing(&obj); }

  static U &unboxing(PyObject *obj) { return PyCXXObject<U>::unboxing(obj); }
};

template <typename U> struct ptr_box_struct {
  using type_non_ptr = std::remove_pointer_t<U>;

  static PyObject *boxing(U obj) {
    static_assert(
        std::is_same_v<type_non_ptr, std::remove_pointer_t<type_non_ptr>>);
    return detail::boxing<type_non_ptr &>(*obj);
  }
  static U unboxing(PyObject *obj) {
    static_assert(
        std::is_same_v<type_non_ptr, std::remove_pointer_t<type_non_ptr>>);
    return &detail::unboxing<type_non_ptr &>(obj);
  }
};

template <typename> struct is_std_vector : std::false_type {};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type {};

template <typename T> PyObject *boxing(T &&obj) {
  typedef std::remove_reference_t<T> type_base;
  typedef std::remove_const_t<type_base> type_nc;
  typedef std::remove_pointer_t<type_nc> type_ncp;
  typedef std::remove_reference_t<type_nc> type_ncr;
  typedef std::remove_reference_t<type_ncp> type_ncrp;

  typedef std::conditional_t<
      // check pointer
      std::is_same_v<type_nc, type_ncp>,
      std::conditional_t<
          // check bind obj
          std::is_base_of_v<BindObject, type_ncrp>,
          obj_box_struct<type_ncrp>,
          std::conditional_t<
              // check vector
              is_std_vector<type_ncrp>::value, vector_box_struct<type_ncrp>,
              base_box_struct<type_ncrp>>>,
      ptr_box_struct<type_nc>>
      box_type;

  return box_type::boxing(std::forward<T>(obj));
}

template <typename T> decltype(auto) unboxing(PyObject *obj) {
  typedef std::remove_reference_t<T> type_base;
  typedef std::remove_const_t<type_base> type_nc;
  typedef std::remove_pointer_t<type_nc> type_ncp;
  typedef std::remove_reference_t<type_nc> type_ncr;
  typedef std::remove_reference_t<type_ncp> type_ncrp;

  typedef std::conditional_t<
      // check pointer
      std::is_same_v<type_nc, type_ncp>,
      std::conditional_t<
          // check bind obj
          std::is_base_of_v<BindObject, type_ncrp>,
          obj_box_struct<type_ncrp>,
          std::conditional_t<
              // check vector
              is_std_vector<type_ncrp>::value, vector_box_struct<type_ncrp>,
              base_box_struct<type_ncrp>>>,
      ptr_box_struct<type_nc>>
      box_type;

  return box_type::unboxing(obj);
}

template<> PyObject* base_box_struct<const char *>::boxing(const char* obj);
template<> const char* base_box_struct<const char*>::unboxing(PyObject* obj);

template <> inline PyObject* boxing<const char*>(const char*&& obj) {
  return base_box_struct<const char*>::boxing(obj);
}

template <> inline decltype(auto) unboxing<const char*>(PyObject* obj) {
  return base_box_struct<const char*>::unboxing(obj);
}
} // namespace detail
