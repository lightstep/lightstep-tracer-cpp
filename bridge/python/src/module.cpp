#include <Python.h>

#include "lightstep/tracer.h"
#include "python_bridge_tracer/utility.h"
#include "python_bridge_tracer/python_string_wrapper.h"
#include "python_bridge_tracer/python_object_wrapper.h"
#include "python_bridge_tracer/module.h"
#include "tracer/lightstep_tracer_factory.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// ExtractScopeManager
//--------------------------------------------------------------------------------------------------
static bool ExtractScopeManager(
    PyObject* keywords,
    python_bridge_tracer::PythonObjectWrapper& result) noexcept {
  python_bridge_tracer::PythonObjectWrapper scope_manager =
      PyDict_GetItemString(keywords, "scope_manager");
  if (scope_manager == nullptr) {
    return true;
  }
  if (PyDict_DelItemString(keywords, "scope_manager") == -1) {
    return false;
  }
  result = std::move(scope_manager);
  return true;
}

//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
static PyObject* MakeTracer(PyObject* /*self*/, PyObject* /*args*/, PyObject* keywords) noexcept {
  python_bridge_tracer::PythonObjectWrapper scope_manager;
  if (!ExtractScopeManager(keywords, scope_manager)) {
    return nullptr;
  }
  python_bridge_tracer::PythonObjectWrapper dumps_function =
      python_bridge_tracer::getModuleAttribute("json", "dumps");
  if (dumps_function.error()) {
    return nullptr;
  }
  python_bridge_tracer::PythonStringWrapper json_config =
      PyObject_CallFunctionObjArgs(dumps_function, keywords, nullptr);
  if (json_config.error()) {
    return nullptr;
  }
  std::string error_message;
  LightStepTracerFactory tracer_factory;
  auto tracer_maybe = tracer_factory.MakeTracer(
      static_cast<opentracing::string_view>(json_config).data(), error_message);
  if (!tracer_maybe) {
    PyErr_Format(PyExc_RuntimeError, "failed to construct tracer: %s", error_message.c_str());
    return nullptr;
  }
  return python_bridge_tracer::makeTracer(std::move(*tracer_maybe), scope_manager);
}
} // namespace lightstep

//--------------------------------------------------------------------------------------------------
// ModuleMethods
//--------------------------------------------------------------------------------------------------
static PyMethodDef ModuleMethods[] = {
    {"Tracer", reinterpret_cast<PyCFunction>(lightstep::MakeTracer), METH_VARARGS | METH_KEYWORDS,
     PyDoc_STR("Construct the LightStep tracer")},
    {nullptr, nullptr}};

//--------------------------------------------------------------------------------------------------
// ModuleDefinition
//--------------------------------------------------------------------------------------------------
static PyModuleDef ModuleDefinition = {PyModuleDef_HEAD_INIT, "lightstep",
                                       PyDoc_STR("LightStep Python tracer"), -1,
                                       ModuleMethods};

//--------------------------------------------------------------------------------------------------
// PyInit_lightstep
//--------------------------------------------------------------------------------------------------
extern "C" {
PyMODINIT_FUNC PyInit_lightstep() noexcept {
  auto module = PyModule_Create(&ModuleDefinition);
  if (module == nullptr) {
    return nullptr;
  }
  if (!python_bridge_tracer::setupClasses(module)) {
    Py_DECREF(module);
    return nullptr;
  }
  return module;
}
} // extern "C"
