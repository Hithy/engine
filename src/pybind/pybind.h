#pragma once

#include <string>
#include "pyboxing.h"

template<typename T>
char GetPyBuildType() {
  return 'O';
}

template<typename... Res>
struct ParamList;

template<>
struct ParamList<> {};

template<typename T, typename... Res>
struct ParamList<T, Res...> : ParamList<Res...> {
  using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  PyObject* data;
};

template<int N>
struct ParamGet {
  template<typename T, typename... Res>
  static decltype(auto) get(ParamList<T, Res...>* param) {
    return ParamGet<N - 1>::get((ParamList<Res...>*)param);
  }

  template<typename T, typename... Res>
  static PyObject** get_ptr(ParamList<T, Res...>* param) {
    return ParamGet<N - 1>::get_ptr((ParamList<Res...>*)param);
  }
};

template<>
struct ParamGet<0> {
  template<typename T, typename... Res>
  static decltype(auto) get(ParamList<T, Res...>* param) {
    return detail::unboxing<T>(param->data);
  }

  template<typename T, typename... Res>
  static PyObject** get_ptr(ParamList<T, Res...>* param) {
    return &(param->data);
  }
};

template<int...>
struct seq {};

template<int N, int... S>
struct gen_seq : gen_seq<N - 1, N - 1, S...> {};

template<int... S>
struct gen_seq<0, S...> {
  typedef seq<S...> type;
};

template<typename... Res>
static void BuildParamTypes(char* addr, ParamList<Res...>* param);

template<>
void BuildParamTypes(char* addr, ParamList<>* param) {
}

template<typename T, typename... Res>
void BuildParamTypes(char* addr, ParamList<T, Res...>* param) {
  *addr = GetPyBuildType<T>();
  BuildParamTypes(addr + 1, (ParamList<Res...>*)param);
}

template <typename Res, typename ...Args, int... S>
auto wrap_func_inner(Res(*fn)(Args...), seq<S...>) {
  auto res = [=](PyObject* self, PyObject* args) -> PyObject* {
    ParamList<Args...> params;
    char param_build[sizeof...(S) + 1] = { 0 };
    BuildParamTypes(param_build, &params); // TODO: constexpr

    if (sizeof...(S) > 0) {
      if (!PyArg_ParseTuple(args, param_build, ParamGet<S>::get_ptr(&params)...)) {
        return NULL;
      }
    }

    decltype(auto) res = fn(ParamGet<S>::get(&params)...);

    return detail::boxing(res);
  };

  return res;
}

template <typename Res, typename ...Args>
auto wrap_func(Res(*fn)(Args...)) {
  typename gen_seq<sizeof...(Args)>::type param_len;

  auto func = wrap_func_inner(fn, param_len);

  return func;
}

template <typename ...Args, int... S>
auto wrap_func_inner(void (*fn)(Args...), seq<S...>) {
  auto res = [=](PyObject* self, PyObject* args) -> PyObject* {
    ParamList<Args...> params;
    char param_build[sizeof...(S) + 1] = { 0 };
    BuildParamTypes(param_build, &params); // TODO: constexpr

    if (sizeof...(S) > 0) {
      if (!PyArg_ParseTuple(args, param_build, ParamGet<S>::get_ptr(&params)...)) {
        return NULL;
      }
    }

    fn(ParamGet<S>::get(&params)...);

    Py_RETURN_NONE;
  };

  return res;
}

template <typename ...Args>
auto wrap_func(void (*fn)(Args...)) {
  typename gen_seq<sizeof...(Args)>::type param_len;

  auto func = wrap_func_inner(fn, param_len);

  return func;
}

template <typename Res, typename Cls, typename ...Args, int... S>
auto wrap_func_inner(Res(Cls::* fn)(Args...), seq<S...>) {
  auto res = [=](PyObject* self, PyObject* args) -> PyObject* {
    ParamList<Args...> params;
    char param_build[sizeof...(S) + 1] = { 0 };
    BuildParamTypes(param_build, &params); // TODO: constexpr

    if (sizeof...(S) > 0) {
      if (!PyArg_ParseTuple(args, param_build, ParamGet<S>::get_ptr(&params)...)) {
        return NULL;
      }
    }

    decltype(auto) c_obj = detail::unboxing<Cls*>(self);
    decltype(auto) res = (c_obj->*fn)(ParamGet<S>::get(&params)...);

    return detail::boxing(res);
  };

  return res;
}

template <typename Res, typename Cls, typename ...Args>
auto wrap_func(Res (Cls::*fn)(Args...)) {
  typename gen_seq<sizeof...(Args)>::type param_len;

  auto func = wrap_func_inner(fn, param_len);

  return func;
}

template <typename Cls, typename ...Args, int... S>
auto wrap_func_inner(void (Cls::* fn)(Args...), seq<S...>) {
  auto res = [=](PyObject* self, PyObject* args) -> PyObject* {
    ParamList<Args...> params;
    char param_build[sizeof...(S) + 1] = { 0 };
    BuildParamTypes(param_build, &params); // TODO: constexpr

    if (sizeof...(S) > 0) {
      if (!PyArg_ParseTuple(args, param_build, ParamGet<S>::get_ptr(&params)...)) {
        return NULL;
      }
    }

    decltype(auto) c_obj = detail::unboxing<Cls*>(self);
    (c_obj->*fn)(ParamGet<S>::get(&params)...);

    Py_RETURN_NONE;
  };

  return res;
}

template <typename Cls, typename ...Args>
auto wrap_func(void (Cls::* fn)(Args...)) {
  typename gen_seq<sizeof...(Args)>::type param_len;

  auto func = wrap_func_inner(fn, param_len);

  return func;
}

#define BIND_CLS_FUNC_DEFINE(cls, fn) \
static PyObject* PYBIND__##cls__##fn(PyObject* self, PyObject* args) { \
  static auto inner_func = wrap_func(&cls::fn); \
  return inner_func(self, args); \
}

#define BIND_CLS_FUNC_NAME(cls, fn) PYBIND__##cls__##fn

#define BIND_FUNC_DEFINE(fn) \
static PyObject* PYBIND_##fn(PyObject* self, PyObject* args) { \
  static auto inner_func = wrap_func(fn); \
  return inner_func(self, args); \
}

#define BIND_FUNC_NAME(fn) PYBIND_##fn
