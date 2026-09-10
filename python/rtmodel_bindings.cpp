// Copyright (C) 2nd Messenger Systems
// pybind11 bindings for RtModel
//
// Windows-only. RtModel is built against MFC (Dynamic), Intel IPP, ITK and
// VNL, so this extension can only be compiled with MSVC. See BUILD_NATIVE.md.
//
// Include ORDER matters: MFC's <afx.h> refuses to compile if <windows.h> was
// already pulled in (Python.h includes it). So the MFC headers MUST come
// before <pybind11/pybind11.h> (which includes Python.h). The RtModel public
// headers (Prescription.h, Plan.h, VectorN.h ...) reference CString, CArray
// and CTypedPtrMap without including MFC themselves -- they rely on the
// translation unit having already included these, exactly as stdafx.h does.

// --- MFC first (mirrors RtModel/stdafx.h) -----------------------------------
#include <afx.h>
#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>
#include <atlcoll.h>

// --- then pybind11 / Python -------------------------------------------------
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

// --- the rest of what RtModel/stdafx.h provides to every RtModel TU ----------
// The public headers use DeclareMember / DECLARE_ATTRIBUTE_PTR* (UtilMacros.h),
// CMatrixNxM (MatrixNxM.h) and the VolumeReal typedefs (ItkUtils.h) without
// including them, exactly as they rely on the MFC headers above. These come
// AFTER pybind11: the utility headers define short, unprefixed macros that
// break pybind11's own templates if they are visible first.
#include <math.h>
#include <UtilMacros.h>
#include <MatrixNxM.h>
#include <itkVector.h>
#include <ItkUtils.h>

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

// A DynamicCovarianceCostFunction whose evaluation is a Python callable, so
// the unmodified C++ DynamicCovarianceOptimizer (Brent line search, PR
// direction update, adaptive-variance bookkeeping) can be run on objectives
// defined in Python -- e.g. the pybrimstone Prescription used by
// python/experiments/sigma_calibration.py, which lets that experiment be
// repeated against the C++ optimizer instead of the Python port.
//
// The callable receives (x: np.ndarray, need_grad: bool) and returns
// (value, grad_or_None). The optimizer calls it with need_grad=True at the
// start of minimize and after each line search, and with need_grad=False
// from inside the Brent line minimizer.
class PyCostFunction : public DynamicCovarianceCostFunction {
public:
    PyCostFunction(int n, py::function fn) : m_fn(std::move(fn)) {
        set_number_of_unknowns(n);
    }

    REAL operator()(const CVectorN<>& vInput, CVectorN<>* pGrad = NULL) const override {
        py::gil_scoped_acquire gil;
        py::object res = m_fn(vector_to_numpy(vInput), pGrad != NULL);
        py::tuple t = res.cast<py::tuple>();
        const double f = t[0].cast<double>();
        if (pGrad != NULL) {
            py::array_t<double, py::array::c_style | py::array::forcecast> g =
                t[1].cast<py::array_t<double, py::array::c_style | py::array::forcecast>>();
            py::buffer_info b = g.request();
            if (b.ndim != 1 || b.shape[0] != vInput.GetDim())
                throw std::runtime_error("PyCostFunction: gradient must be 1-D with len(x) entries");
            const double* p = static_cast<const double*>(b.ptr);
            if (pGrad->GetDim() != vInput.GetDim())
                pGrad->SetDim(vInput.GetDim());
            for (int i = 0; i < vInput.GetDim(); i++)
                (*pGrad)[i] = p[i];
        }
        return (REAL) f;
    }

    // the adaptive-variance vector the optimizer hands the cost function
    // (SetAdaptiveVariance), or None before minimize() has initialized it
    py::object adaptive_variance() const {
        if (m_pAV == NULL || m_pAV->GetDim() == 0)
            return py::none();
        return vector_to_numpy(*m_pAV);
    }

private:
    py::function m_fn;
};

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

    // Register the abstract base BEFORE any derived class names it. pybind11
    // requires every declared base to be a registered type, and registering it
    // also teaches pybind11 the Prescription -> DynamicCovarianceCostFunction
    // relationship, so a Prescription can be passed where the optimizer
    // constructor expects a DynamicCovarianceCostFunction*. No constructor is
    // exposed because the class is abstract (pure-virtual operator()).
    py::class_<DynamicCovarianceCostFunction>(m, "DynamicCovarianceCostFunction");

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

    // Python-defined objective driven by the C++ optimizer
    py::class_<PyCostFunction, DynamicCovarianceCostFunction>(m, "PyCostFunction")
        .def(py::init<int, py::function>(), py::arg("n"), py::arg("fn"),
             "fn(x: ndarray, need_grad: bool) -> (value, grad or None)")
        .def("adaptive_variance", &PyCostFunction::adaptive_variance,
             "Adaptive-variance vector the optimizer shares with this cost function");

    // Expose DynamicCovarianceOptimizer for comparison
    py::class_<DynamicCovarianceOptimizer>(m, "ConjGradOptimizer")
        .def(py::init<DynamicCovarianceCostFunction*>(), py::keep_alive<1, 2>())
        .def("set_adaptive_variance", &DynamicCovarianceOptimizer::SetAdaptiveVariance,
             py::arg("calc_var"), py::arg("var_min"), py::arg("var_max"))
        .def("set_x_tolerance", &DynamicCovarianceOptimizer::set_x_tolerance,
             "Relative-change convergence tolerance: 2|dF| <= xtol (|F_old| + |F_new| + ZEPS)")
        .def("set_line_optimizer_tolerance", &DynamicCovarianceOptimizer::SetLineOptimizerTolerance,
             "x-tolerance handed to the Brent line minimizer (not defaulted by the C++ ctor)")
        .def("get_num_iterations", &DynamicCovarianceOptimizer::get_num_iterations)
        .def("get_final_parameter", [](const DynamicCovarianceOptimizer& opt) {
            return vector_to_numpy(opt.GetFinalParameter());
        })
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
             "Get computed free energy (if enabled)")
        .def("get_adaptive_variance",
             [](const DynamicCovarianceOptimizer& opt) {
                 return vector_to_numpy(opt.GetAdaptiveVariance());
             },
             "Per-parameter adaptive variance (sigma_weights). Required by "
             "the hierarchical-Bayes outer loop; see HIERARCHICAL_BAYES_DESIGN.md.");

    // Helper functions
    m.def("vector_to_numpy", &vector_to_numpy, "Convert CVectorN to numpy array");
    m.def("numpy_to_vector", &numpy_to_vector, "Convert numpy array to CVectorN");
}
