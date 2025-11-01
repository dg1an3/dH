// Copyright (C) 2nd Messenger Systems
// pybind11 bindings for RtModel

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

// RtModel includes
#include "Prescription.h"
#include "Plan.h"
#include "Series.h"
#include "Structure.h"
#include "VectorN.h"
#include "ConjGradOptimizer.h"

namespace py = pybind11;
using namespace dH;

// Helper class to wrap Prescription for Python optimization
class PrescriptionWrapper {
public:
    PrescriptionWrapper(Prescription* presc) : m_presc(presc) {}

    // Evaluate objective function (for scipy.optimize)
    // Returns tuple: (value, gradient)
    std::tuple<double, py::array_t<double>> evaluate(py::array_t<double> x) {
        py::buffer_info buf = x.request();

        if (buf.ndim != 1)
            throw std::runtime_error("Input must be 1-dimensional");

        int n = buf.shape[0];
        double* x_ptr = static_cast<double*>(buf.ptr);

        // Convert numpy array to CVectorN
        CVectorN<> vInput(n);
        for (int i = 0; i < n; i++) {
            vInput[i] = x_ptr[i];
        }

        // Evaluate with gradient
        CVectorN<> vGrad;
        double value = (*m_presc)(vInput, &vGrad);

        // Convert gradient to numpy array
        auto grad_np = py::array_t<double>(n);
        py::buffer_info grad_buf = grad_np.request();
        double* grad_ptr = static_cast<double*>(grad_buf.ptr);

        for (int i = 0; i < n; i++) {
            grad_ptr[i] = vGrad[i];
        }

        return std::make_tuple(value, grad_np);
    }

    // Just evaluate value (no gradient)
    double evaluate_value(py::array_t<double> x) {
        py::buffer_info buf = x.request();
        int n = buf.shape[0];
        double* x_ptr = static_cast<double*>(buf.ptr);

        CVectorN<> vInput(n);
        for (int i = 0; i < n; i++) {
            vInput[i] = x_ptr[i];
        }

        return (*m_presc)(vInput, nullptr);
    }

    // Get dimension of the problem
    int get_dimension() const {
        return m_presc->get_number_of_unknowns();
    }

    // Access to underlying prescription
    Prescription* get_prescription() { return m_presc; }

private:
    Prescription* m_presc;
};

// Helper to convert CVectorN to numpy array
py::array_t<double> vector_to_numpy(const CVectorN<>& vec) {
    auto result = py::array_t<double>(vec.GetDim());
    py::buffer_info buf = result.request();
    double* ptr = static_cast<double*>(buf.ptr);

    for (int i = 0; i < vec.GetDim(); i++) {
        ptr[i] = vec[i];
    }

    return result;
}

// Helper to convert numpy array to CVectorN
CVectorN<> numpy_to_vector(py::array_t<double> arr) {
    py::buffer_info buf = arr.request();
    if (buf.ndim != 1)
        throw std::runtime_error("Input must be 1-dimensional");

    int n = buf.shape[0];
    double* ptr = static_cast<double*>(buf.ptr);

    CVectorN<> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = ptr[i];
    }

    return result;
}

PYBIND11_MODULE(rtmodel_core, m) {
    m.doc() = "RtModel Python bindings for variational Bayes optimization";

    // Expose CVectorN
    py::class_<CVectorN<>>(m, "VectorN")
        .def(py::init<int>())
        .def("__len__", &CVectorN<>::GetDim)
        .def("__getitem__", [](const CVectorN<>& v, int i) {
            if (i < 0 || i >= v.GetDim())
                throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](CVectorN<>& v, int i, double val) {
            if (i < 0 || i >= v.GetDim())
                throw py::index_error();
            v[i] = val;
        })
        .def("to_numpy", &vector_to_numpy)
        .def_static("from_numpy", &numpy_to_vector);

    // Expose Prescription (base objective function)
    py::class_<Prescription, DynamicCovarianceCostFunction>(m, "Prescription")
        .def("get_number_of_unknowns", &Prescription::get_number_of_unknowns)
        .def("set_gbin_var", &Prescription::SetGBinVar,
             py::arg("var_min"), py::arg("var_max"),
             "Set adaptive variance parameters");

    // Expose PrescriptionWrapper for easy Python optimization
    py::class_<PrescriptionWrapper>(m, "PrescriptionWrapper")
        .def(py::init<Prescription*>())
        .def("evaluate", &PrescriptionWrapper::evaluate,
             "Evaluate objective function and gradient")
        .def("evaluate_value", &PrescriptionWrapper::evaluate_value,
             "Evaluate objective function value only")
        .def("get_dimension", &PrescriptionWrapper::get_dimension)
        .def("get_prescription", &PrescriptionWrapper::get_prescription,
             py::return_value_policy::reference);

    // Expose DynamicCovarianceOptimizer for comparison
    py::class_<DynamicCovarianceOptimizer>(m, "ConjGradOptimizer")
        .def(py::init<DynamicCovarianceCostFunction*>())
        .def("set_adaptive_variance", &DynamicCovarianceOptimizer::SetAdaptiveVariance)
        .def("set_compute_free_energy", &DynamicCovarianceOptimizer::SetComputeFreeEnergy,
             "Enable explicit free energy calculation")
        .def("minimize", [](DynamicCovarianceOptimizer& opt, py::array_t<double> x0) {
            CVectorN<> vInit = numpy_to_vector(x0);
            vnl_vector<REAL> vInitVnl(vInit.GetDim());
            for (int i = 0; i < vInit.GetDim(); i++) {
                vInitVnl[i] = vInit[i];
            }

            opt.minimize(vInitVnl);

            return vector_to_numpy(opt.GetFinalParameter());
        })
        .def("get_final_value", &DynamicCovarianceOptimizer::GetFinalValue)
        .def("get_entropy", &DynamicCovarianceOptimizer::GetEntropy,
             "Get computed entropy (if free energy calculation enabled)")
        .def("get_free_energy", &DynamicCovarianceOptimizer::GetFreeEnergy,
             "Get computed free energy (if enabled)");

    // Helper functions
    m.def("vector_to_numpy", &vector_to_numpy, "Convert CVectorN to numpy array");
    m.def("numpy_to_vector", &numpy_to_vector, "Convert numpy array to CVectorN");
}
