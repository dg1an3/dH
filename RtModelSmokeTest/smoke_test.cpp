// Smoke test for the hand-rolled 1-D linear convolution that replaced
// ippsConv_64f in RtModel/Histogram.cpp::ConvGauss and
// RtModel/HistogramGradient.cpp::Conv_dGauss.
//
// The loop body below is a verbatim copy from those files. We feed it
// known inputs and verify mathematical properties of the result:
//   * output dimension = src + kernel - 1
//   * sum-preservation: sum(out) = sum(in) * sum(k)
//   * delta-input recovers the kernel at the correct offset
//   * symmetry: constant input + symmetric kernel ⇒ symmetric output
//   * boundary taps match by inspection
//
// Build: cl /EHsc smoke_test.cpp
// Run:   smoke_test.exe   (returns 0 on success)

#include <cmath>
#include <cstdio>
#include <vector>

namespace {

int g_failures = 0;

// Verbatim copy of the loop body inserted into Histogram.cpp::ConvGauss
// (and the equivalent in HistogramGradient.cpp::Conv_dGauss).
void conv1d_linear(const std::vector<double>& buffer_in,
                   const std::vector<double>& kernel_in,
                   std::vector<double>& buffer_out)
{
    const int srcLen = (int)buffer_in.size();
    const int kerLen = (int)kernel_in.size();
    const int dstLen = srcLen + kerLen - 1;
    buffer_out.assign(dstLen, 0.0);
    for (int n = 0; n < dstLen; ++n)
    {
        double acc = 0.0;
        const int kMin = (n - srcLen + 1 > 0) ? n - srcLen + 1 : 0;
        const int kMax = (n < kerLen - 1) ? n : kerLen - 1;
        for (int k = kMin; k <= kMax; ++k)
            acc += kernel_in[k] * buffer_in[n - k];
        buffer_out[n] = acc;
    }
}

void check_close(const char* what, double got, double expected, double tol = 1e-9)
{
    const double diff = std::fabs(got - expected);
    const bool ok = diff <= tol;
    std::printf("  [%s] %-32s got=%.9g expected=%.9g diff=%.3g\n",
                ok ? "PASS" : "FAIL", what, got, expected, diff);
    if (!ok) ++g_failures;
}

void check_eq_int(const char* what, int got, int expected)
{
    const bool ok = got == expected;
    std::printf("  [%s] %-32s got=%d expected=%d\n",
                ok ? "PASS" : "FAIL", what, got, expected);
    if (!ok) ++g_failures;
}

} // namespace

int main()
{
    std::printf("RtModel ConvGauss smoke test\n");
    std::printf("============================\n\n");

    // ----------------------------------------------------------------
    std::printf("[1] uniform input, symmetric kernel\n");
    {
        std::vector<double> input(10, 1.0);
        std::vector<double> kernel = {0.25, 0.5, 0.25};
        std::vector<double> output;
        conv1d_linear(input, kernel, output);

        check_eq_int("output dim (10+3-1=12)", (int)output.size(), 12);

        double inSum = 0, outSum = 0, kSum = 0;
        for (double x : input)  inSum  += x;
        for (double x : kernel) kSum   += x;
        for (double x : output) outSum += x;
        check_close("sum(out) == sum(in)*sum(k)", outSum, inSum * kSum);

        check_close("output[0] (left edge)",  output[0],  0.25);
        check_close("output[11] (right edge)", output[11], 0.25);
        check_close("output[5] (interior)",   output[5],  1.0);

        for (int i = 0; i < (int)output.size() / 2; ++i)
        {
            char tag[64];
            std::sprintf(tag, "symmetric out[%d]==out[%d]",
                         i, (int)output.size() - 1 - i);
            check_close(tag, output[i], output[output.size() - 1 - i], 1e-12);
        }
    }

    // ----------------------------------------------------------------
    std::printf("\n[2] delta input recovers kernel\n");
    {
        std::vector<double> input(8, 0.0);
        input[4] = 1.0;
        std::vector<double> kernel = {0.25, 0.5, 0.25};
        std::vector<double> output;
        conv1d_linear(input, kernel, output);

        check_eq_int("output dim (8+3-1=10)", (int)output.size(), 10);
        check_close("out[3] (before)",     output[3], 0.0);
        check_close("out[4] = kernel[0]",  output[4], kernel[0]);
        check_close("out[5] = kernel[1]",  output[5], kernel[1]);
        check_close("out[6] = kernel[2]",  output[6], kernel[2]);
        check_close("out[7] (after)",      output[7], 0.0);
    }

    // ----------------------------------------------------------------
    std::printf("\n[3] non-trivial signal x asymmetric kernel\n");
    {
        // Verify against hand computation. input=[1,2,3], kernel=[1,1,1]:
        // out = [1, 3, 6, 5, 3]
        std::vector<double> input  = {1.0, 2.0, 3.0};
        std::vector<double> kernel = {1.0, 1.0, 1.0};
        std::vector<double> output;
        conv1d_linear(input, kernel, output);

        check_eq_int("output dim (3+3-1=5)", (int)output.size(), 5);
        check_close("out[0] = 1", output[0], 1.0);
        check_close("out[1] = 3", output[1], 3.0);
        check_close("out[2] = 6", output[2], 6.0);
        check_close("out[3] = 5", output[3], 5.0);
        check_close("out[4] = 3", output[4], 3.0);
    }

    // ----------------------------------------------------------------
    std::printf("\n[4] asymmetric kernel — verify orientation, not flipped\n");
    {
        // input=[1,0,0,0,0], kernel=[1,2,3]
        // Linear convolution (commutative in math; this loop computes
        // out[n] = Σ_k kernel[k] * input[n-k]):
        // out = [1, 2, 3, 0, 0, 0, 0]
        std::vector<double> input  = {1.0, 0.0, 0.0, 0.0, 0.0};
        std::vector<double> kernel = {1.0, 2.0, 3.0};
        std::vector<double> output;
        conv1d_linear(input, kernel, output);

        check_eq_int("output dim (5+3-1=7)", (int)output.size(), 7);
        check_close("out[0] = kernel[0]", output[0], 1.0);
        check_close("out[1] = kernel[1]", output[1], 2.0);
        check_close("out[2] = kernel[2]", output[2], 3.0);
        check_close("out[3] = 0",         output[3], 0.0);
    }

    // ----------------------------------------------------------------
    std::printf("\n============================\n");
    if (g_failures == 0)
    {
        std::printf("ALL CHECKS PASSED\n");
        return 0;
    }
    std::printf("FAILED: %d check(s)\n", g_failures);
    return 1;
}
