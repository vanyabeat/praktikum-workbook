/// отсутствует защита от повторного подключения
#include "stdafx.h"
using std::string_literals::operator""s;	/// использование using в заголовочном файле и в глобальном пространстве имен засоряет это глобальное пространство имен

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(func) RunTestImpl((func), #func)

template<typename T, typename S>
std::ostream &operator<<(std::ostream &os, const std::pair<T, S> &v) {
    os << v.first << ": "
       << v.second;
    return os;
}

template<typename C>
void container_print(std::ostream &os, const C &cont) {
    size_t counter = 0;
    size_t size = cont.size();
    for (const auto &item : cont) {
        if (counter == (size - 1)) {
            os << item;
            continue;
        }
        os << item << ", ";
        ++counter;
    }
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &obj) {
    os << "[";
    container_print(os, obj);
    os << "]";
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const std::set<T> &obj) {
    os << "{";
    container_print(os, obj);
    os << "}";
    return os;
}

template<typename T, typename V>
std::ostream &operator<<(std::ostream &os, const std::map<T, V> &obj) {
    os << "{";
    container_print(os, obj);
    os << "}";
    return os;
}

template<typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const std::string &t_str, const std::string &u_str, const std::string &file,
                     const std::string &func, unsigned line, const std::string &hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

void AssertImpl(bool value, const std::string &expr_str, const std::string &file, const std::string &func, unsigned line,
                const std::string &hint) {
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

template<typename T>
void RunTestImpl(T func, const std::string &function) {
    func();
    std::cerr << function << " OK" << std::endl;
}
