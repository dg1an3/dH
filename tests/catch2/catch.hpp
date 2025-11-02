// Catch2 v2.13.10 - Single Header Version
// This is a minimal stub - you should download the full catch.hpp from:
// https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp

#ifndef CATCH_HPP_INCLUDED
#define CATCH_HPP_INCLUDED

// Minimal Catch2 macros for testing
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE

#include <string>
#include <sstream>
#include <vector>
#include <exception>

namespace Catch {

class TestCase {
public:
    TestCase(const std::string& name) : m_name(name) {}
    std::string m_name;
};

struct SourceLineInfo {
    const char* file;
    int line;
};

class Section {
public:
    Section(const std::string& name) : m_name(name) {}
    ~Section() {}
    operator bool() const { return true; }
private:
    std::string m_name;
};

} // namespace Catch

// Test case macros
#define TEST_CASE(name, ...) \
    static void test_##name(); \
    static Catch::TestCase testCase_##name(#name); \
    void test_##name()

#define SECTION(name) \
    if (Catch::Section section = Catch::Section(name))

// Assertion macros
#define REQUIRE(expr) \
    if (!(expr)) { \
        throw std::runtime_error("REQUIRE failed: " #expr); \
    }

#define REQUIRE_FALSE(expr) \
    if (expr) { \
        throw std::runtime_error("REQUIRE_FALSE failed: " #expr); \
    }

#define REQUIRE_THROWS(expr) \
    try { expr; throw std::runtime_error("Expected exception not thrown"); } \
    catch (...) {}

#define CHECK(expr) REQUIRE(expr)
#define CHECK_FALSE(expr) REQUIRE_FALSE(expr)

// Floating point comparisons
#define REQUIRE_THAT(value, matcher) REQUIRE(matcher(value))

namespace Catch {
namespace Matchers {

class WithinAbsMatcher {
public:
    WithinAbsMatcher(double target, double margin)
        : m_target(target), m_margin(margin) {}

    bool operator()(double value) const {
        return std::abs(value - m_target) <= m_margin;
    }

private:
    double m_target;
    double m_margin;
};

inline WithinAbsMatcher WithinAbs(double target, double margin) {
    return WithinAbsMatcher(target, margin);
}

} // namespace Matchers
} // namespace Catch

#endif // CATCH_HPP_INCLUDED
