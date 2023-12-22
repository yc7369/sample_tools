#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace util {

class StringAssist {
public:
    static std::string&
    replace(std::string& str, const std::string& old_val, const std::string& new_val) {
        for (std::string::size_type pos(0); pos != std::string::npos; pos += new_val.length()) {
            if ((pos = str.find(old_val, pos)) != std::string::npos)
                str.replace(pos, old_val.length(), new_val);
            else
                break;
        }
        return str;
    }

    static int
    split(const std::string& str, const std::string& sepr, std::vector<std::string>& elems) {
        elems.clear();
        if (str.size() == 0) {
            return 0;
        }
        size_t last = 0;
        size_t index = str.find_first_of(sepr, last);
        while (index != std::string::npos) {
            elems.push_back(str.substr(last, index - last));
            last = index + sepr.size();
            index = str.find_first_of(sepr, last);
        }
        if (index - last > 0) {
            elems.push_back(str.substr(last, index - last));
        }
        return 0;
    }

    static std::string& to_upper(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);
        return str;
    }
    static std::string& to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
        return str;
    }

    //
    // to hex
    static std::string to_hex(const std::string& str) {
        static const char hex_chars[] = "0123456789abcdef";
        std::string ret;
        for (std::string::const_iterator i = str.begin(); i != str.end(); ++i) {
            ret += hex_chars[((unsigned char)*i) >> 4];
            ret += hex_chars[((unsigned char)*i) & 0xf];
        }
        // c++ 14/17 copy elision
        return ret;
        // return std::move(ret);
    }

    // trim
    // ltrim, rtrim, trim
    static std::string& ltrim(std::string& str) {
        str.erase(
            str.begin(),
            std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return str;
    }
    static std::string& rtrim(std::string& str) {
        str.erase(
            std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace)))
                .base(),
            str.end());
        return str;
    }
    static std::string& trim(std::string& str) {
        return ltrim(rtrim(str));
    }
};

}  // namespace util
