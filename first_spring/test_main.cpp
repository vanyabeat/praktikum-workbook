#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

using namespace std;
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& obj)
{
    if (obj.empty()){
        return os;
    }
    else{
        os << "[";
        for (auto i = 0; i < obj.size(); ++i){
            if (i == obj.size() - 1){
                os << obj.at(i);
                continue;
            }
            os << obj.at(i) << ", ";
        }
        os << "]";
    }
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& obj)
{
    if (obj.empty()){
        return os;
    }
    else{
        os << "{";
        auto counter = 0;
        auto size = obj.size();
        for (auto it = obj.begin(); it != obj.end(); ++it){
            if (counter == (size - 1)){
                os << *it;
                continue;
            }
            os << *it << ", ";
            ++counter;
        }
        os << "}";
    }
    return os;
}

template <typename T, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<T, V>& obj)
{
    if (obj.empty()){
        return os;
    }
    else{
        os << "{";
        auto counter = 0;
        auto size = obj.size();
        for (auto it = obj.begin(); it != obj.end(); ++it){
            if (counter == (size - 1)){
                os << it->first << ": " << it->second;
                continue;
            }
            os << it->first << ": " << it->second << ", ";
            ++counter;
        }
        os << "}";
    }
    return os;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template<typename T>
void RunTestImpl(T func, const std::string &function) {
    func();
    std::cerr << function << " OK";
}

#define RUN_TEST(func) RunTestImpl((func),  #func)

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

using namespace std;

class Synonyms {
public:
    void Add(const string& first_word, const string& second_word) {
        synonyms_[first_word].insert(second_word);
        synonyms_[second_word].insert(first_word);
    }

    size_t GetSynonymCount(const string& word) const {
        if (synonyms_.count(word) != 0) {
            return synonyms_.at(word).size();
        }
        return 0;
    }

    bool AreSynonyms(const string& first_word, const string& second_word) const {
        if (synonyms_.count(first_word) != 0) {
            return synonyms_.at(first_word).count(second_word) != 0;
        }
        return false;
    }

private:
    map<string, set<string>> synonyms_;
};

void TestAddingSynonymsIncreasesTheirCount() {
    Synonyms synonyms;
    assert(synonyms.GetSynonymCount("music"s) == 0);
    assert(synonyms.GetSynonymCount("melody"s) == 0);

    synonyms.Add("music"s, "melody"s);
    assert(synonyms.GetSynonymCount("music"s) == 1);
    assert(synonyms.GetSynonymCount("melody"s) == 1);

    synonyms.Add("music"s, "tune"s);
    assert(synonyms.GetSynonymCount("music"s) == 2);
    assert(synonyms.GetSynonymCount("tune"s) == 1);
    assert(synonyms.GetSynonymCount("melody"s) == 1);
}

void TestAreSynonyms() {
    Synonyms synonyms;
    synonyms.Add("winner"s, "champion"s);
    synonyms.Add("good"s, "nice"s);

    assert(synonyms.AreSynonyms("winner"s, "champion"s));
    assert(synonyms.AreSynonyms("champion"s, "winner"s));

    assert(!synonyms.AreSynonyms("good"s, "champion"s));
    assert(synonyms.AreSynonyms("good"s, "nice"s));
}

void TestSynonyms() {
    TestAddingSynonymsIncreasesTheirCount();
    TestAreSynonyms();
}

int main() {
    TestSynonyms();




    {
        Synonyms synonyms;
        synonyms.Add("syn1", "syn2");
        ASSERT(synonyms.AreSynonyms("syn1", "syn2"));
        ASSERT(!synonyms.AreSynonyms("syn1", "syn3"));
    }
    {
        Synonyms synonyms;
        synonyms.Add("syn1", "syn2");
        ASSERT_EQUAL(synonyms.GetSynonymCount("syn1"), 2);

    }

    return 0;
}
