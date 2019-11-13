#include <Python.h>

#include <exception>
#include <iostream>

#include "lightstep/tracer.h"
#include "python_bridge_tracer/module.h"
#include "python_bridge_tracer/python_object_wrapper.h"
#include "python_bridge_tracer/python_string_wrapper.h"
#include "python_bridge_tracer/utility.h"
#include "tracer/counting_metrics_observer.h"
#include "tracer/json_options.h"
#include "tracer/lightstep_tracer_factory.h"
#include "tracer/tracer_impl.h"

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
static PyObject* MakeTracer(PyObject* /*self*/, PyObject* /*args*/,
                            PyObject* keywords) noexcept {
  if (keywords == nullptr) {
    PyErr_Format(PyExc_RuntimeError, "no keyword arguments provided");
    return nullptr;
  }
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
  auto tracer_options_maybe = MakeTracerOptions(
      static_cast<opentracing::string_view>(json_config).data(), error_message);
  if (!tracer_options_maybe) {
    PyErr_Format(PyExc_RuntimeError, "failed to construct tracer: %s",
                 error_message.c_str());
    return nullptr;
  }
  auto& tracer_options = *tracer_options_maybe;
  tracer_options.metrics_observer.reset(new CountingMetricsObserver{});
  auto tracer = MakeLightStepTracer(std::move(tracer_options));
  if (tracer == nullptr) {
    PyErr_Format(PyExc_RuntimeError, "failed to construct tracer");
    return nullptr;
  }
  return python_bridge_tracer::makeTracer(std::move(tracer), scope_manager);
}

//--------------------------------------------------------------------------------------------------
// GetNumSpansSent
//--------------------------------------------------------------------------------------------------
static PyObject* GetNumSpansSent(PyObject* self, void* /*ignored*/) noexcept {
  auto& tracer = python_bridge_tracer::extractTracer(self);
  auto metrics_observer =
      static_cast<lightstep::TracerImpl&>(tracer).recorder().metrics_observer();
  if (metrics_observer == nullptr) {
    Py_RETURN_NONE;
  }
  return PyLong_FromLong(
      static_cast<const CountingMetricsObserver*>(metrics_observer)
          ->num_spans_sent);
}

//--------------------------------------------------------------------------------------------------
// GetNumSpansDropped
//--------------------------------------------------------------------------------------------------
static PyObject* GetNumSpansDropped(PyObject* self,
                                    void* /*ignored*/) noexcept {
  auto& tracer = python_bridge_tracer::extractTracer(self);
  auto metrics_observer =
      static_cast<lightstep::TracerImpl&>(tracer).recorder().metrics_observer();
  if (metrics_observer == nullptr) {
    Py_RETURN_NONE;
  }
  return PyLong_FromLong(
      static_cast<const CountingMetricsObserver*>(metrics_observer)
          ->num_spans_dropped);
}
}  // namespace lightstep

//--------------------------------------------------------------------------------------------------
// ModuleMethods
//--------------------------------------------------------------------------------------------------
static PyMethodDef ModuleMethods[] = {
    {"Tracer", reinterpret_cast<PyCFunction>(lightstep::MakeTracer),
     METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Construct the LightStep tracer")},
    {nullptr, nullptr}};

//--------------------------------------------------------------------------------------------------
// flush
//--------------------------------------------------------------------------------------------------
namespace python_bridge_tracer {
void flush(opentracing::Tracer& tracer,
           std::chrono::microseconds timeout) noexcept {
  auto& lightstep_tracer = static_cast<lightstep::LightStepTracer&>(tracer);
  if (timeout.count() > 0) {
    lightstep_tracer.FlushWithTimeout(timeout);
  } else {
    lightstep_tracer.Flush();
  }
}
}  // namespace python_bridge_tracer

//--------------------------------------------------------------------------------------------------
// PyInit_lightstep_streaming
//--------------------------------------------------------------------------------------------------
extern "C" {
PYTHON_BRIDGE_TRACER_DEFINE_MODULE(lightstep_streaming) {
  auto module = python_bridge_tracer::makeModule(
      "lightstep_streaming", "bridge to LightStep's C++ tracer", ModuleMethods);
  if (module == nullptr) {
    PYTHON_BRIDGE_TRACER_MODULE_RETURN(nullptr);
  }
  std::vector<PyGetSetDef> tracer_getsets = {
      {const_cast<char*>("num_spans_sent"),
       reinterpret_cast<getter>(lightstep::GetNumSpansSent), nullptr,
       const_cast<char*>(PyDoc_STR("Returns the number of spans sent"))},
      {const_cast<char*>("num_spans_dropped"),
       reinterpret_cast<getter>(lightstep::GetNumSpansDropped), nullptr,
       const_cast<char*>(PyDoc_STR("Returns the number of spans dropped"))}};
  if (!python_bridge_tracer::setupClasses(module, {}, tracer_getsets)) {
    std::cerr << "Failed to set up python classes\n";
    std::terminate();
  }
  PYTHON_BRIDGE_TRACER_MODULE_RETURN(module);
}
}  // extern "C"
