
#pragma once

#include<string>
#include<vector>


namespace DoWithStringStd {


    /// <summary>
    /// 去掉字符串前后的空格
    /// </summary>
    /// <param name="s"></param>
    /// <returns></returns>
    std::string& Trim(std::string& s);

    /// <summary>
    /// 分割字符串
    /// </summary>
    /// <param name="str"></param>
    /// <param name="pattern"></param>
    /// <returns></returns>
    std::vector<std::string> SplitWithStl(const std::string& str, const std::string& pattern);

    std::vector<char*> SplitCharWithStl(const char* str, const char split[]);

    static bool compare_pred(unsigned char a, unsigned char b);

    /// <summary>
    /// 判断s1的结尾是不是s2
    /// </summary>
    /// <param name="s1"></param>
    /// <param name="s2"></param>
    /// <returns>当s2长度为0时返回false,当s1结尾不是s2返回false,反之返回true</returns>
    bool StringEndWith(const std::string& s1, const std::string& s2);

    /// <summary>
    /// 字符串s1是不是以s2开头的
    /// </summary>
    /// <param name="s1"></param>
    /// <param name="s2"></param>
    /// <returns></returns>
    bool StringStartWith(const std::string& s1, const std::string& s2);


}