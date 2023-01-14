#include <string>
#include <algorithm>

using namespace std;

class StringUtils
{
    public:
    // trim from start (in place)
    void ltrim(string &s) {
        s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    void rtrim(string &s) {
        s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(string &s) {
        ltrim(s);
        rtrim(s);
    }
};
