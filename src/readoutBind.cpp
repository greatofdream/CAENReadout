
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "readout.hxx"
#include "ReadoutType.h"
#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;

PYBIND11_MODULE(ReadoutPB, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: Readout pybind
        .. autosummary::
           :toctree: _generate
           add
           subtract
    )pbdoc";

    m.def("add", &add, R"pbdoc(
        Add two numbers
        Some other explanation about the add function.
    )pbdoc");

    m.def("subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers
        Some other explanation about the subtract function.
    )pbdoc");

    py::class_<ReadoutData>(m, "ReadoutData")
        .def(py::init<unsigned int, int, int, int>())
        .def("setTriggerMode", &ReadoutData::setTriggerMode)
        .def("setSampleCh", &ReadoutData::setSampleCh)
        .def("setPedestal", &ReadoutData::setPedestal)
        .def("readRunNo", &ReadoutData::readRunNo)
        .def("setDevice", &ReadoutData::setDevice)
        .def("SampleOne", &ReadoutData::SampleOne)
        .def_readwrite("readout", &ReadoutData::Readout);

    
#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}