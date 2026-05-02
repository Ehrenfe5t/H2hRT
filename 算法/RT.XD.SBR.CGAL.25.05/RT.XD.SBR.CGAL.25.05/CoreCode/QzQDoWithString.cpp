
#define _CRT_SECURE_NO_WARNINGS

#include"QzQDoWithString.h"




namespace DoWithStringStd {


    /// <summary>
    /// 去掉字符串前后的空格
    /// </summary>
    /// <param name="s"></param>
    /// <returns></returns>
    std::string& Trim(std::string& s)
    {
        if (s.empty())
        {
            return s;
        }

        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }
    /// <summary>
    /// 分割字符串
    /// </summary>
    /// <param name="str"></param>
    /// <param name="pattern"></param>
    /// <returns></returns>
    std::vector<std::string> SplitWithStl(const std::string& str, const std::string& pattern)
    {
        std::vector<std::string> resVec;

        if ("" == str)
        {
            return resVec;
        }
        //方便截取最后一段数据
        std::string strs = str + pattern;

        size_t pos = strs.find(pattern);
        size_t size = strs.size();

        while (pos != std::string::npos)
        {
            std::string x = strs.substr(0, pos);
            resVec.push_back(x);
            strs = strs.substr(pos + 1, size);
            pos = strs.find(pattern);
        }

        return resVec;
    }

    std::vector<char*> SplitCharWithStl(const char* str, const char split[]) {
        std::vector<char*> res_split;//存储分割后的字符串
        char image_name[100];
        strcpy(image_name, str);
        char* res = strtok(image_name, split);//image_name必须为char[]
        while (res != NULL)
        {
            res_split.push_back(res);
            res = strtok(NULL, split);
        }
        return res_split;
    }

    static bool compare_pred(unsigned char a, unsigned char b) {
        return std::tolower(a) == std::tolower(b);
    }
    /// <summary>
    /// 判断s1的结尾是不是s2
    /// </summary>
    /// <param name="s1"></param>
    /// <param name="s2"></param>
    /// <returns>当s2长度为0时返回false,当s1结尾不是s2返回false,反之返回true</returns>
    bool StringEndWith(const std::string& s1, const std::string& s2) {
        if (s1.size() == 0 || s2.size() == 0) {
            return false;
        }
        if (s1.size() < s2.size()) {
            return false;
        }
        std::string tstr = s1.substr(s1.size() - s2.size());

        if (tstr.size() == s2.size()) {
            return std::equal(s2.begin(), s2.end(), tstr.begin(), compare_pred);
        }
        else {
            return false;
        }
    }

    /// <summary>
    /// 字符串s1是不是以s2开头的
    /// </summary>
    /// <param name="s1"></param>
    /// <param name="s2"></param>
    /// <returns></returns>
    bool StringStartWith(const std::string& s1, const std::string& s2) {
        if (s1.size() == 0 || s2.size() == 0) {
            return false;
        }
        if (s1.size() < s2.size()) {
            return false;
        }
        std::string tstr = s1.substr(0, s2.size());

        if (tstr.size() == s2.size()) {
            return std::equal(s2.begin(), s2.end(), tstr.begin(), compare_pred);
        }
        else {
            return false;
        }
    }


}

