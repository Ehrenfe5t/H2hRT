// 文件目标：
// - 实现模块2材质映射规则加载器。
//
// 主要功能：
// - 读取 `scene_material_map.json`；
// - 解析默认介质与对象级规则；
// - 为批次2双侧材质恢复提供统一规则入口。

#include "MaterialRuleLoader.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace rt {

namespace {

bool WildcardMatch(const std::string& pattern, const std::string& text)
{
    if (pattern.empty())
    {
        return false;
    }

    const std::size_t starPos = pattern.find('*');
    if (starPos == std::string::npos)
    {
        return pattern == text;
    }

    const std::string prefix = pattern.substr(0, starPos);
    const std::string suffix = pattern.substr(starPos + 1U);
    if (text.size() < prefix.size() + suffix.size())
    {
        return false;
    }
    if (!prefix.empty() && text.compare(0, prefix.size(), prefix) != 0)
    {
        return false;
    }
    if (!suffix.empty() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) != 0)
    {
        return false;
    }
    return true;
}

int GetOrCreateSemanticId(std::map<std::string, int>& dictionary, const std::string& name)
{
    if (name.empty())
    {
        return -1;
    }

    const auto found = dictionary.find(name);
    if (found != dictionary.end())
    {
        return found->second;
    }

    const int id = static_cast<int>(dictionary.size());
    dictionary[name] = id;
    return id;
}

bool ReadWholeFile(const std::string& filePath, std::string& content)
{
    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    content = buffer.str();
    return true;
}

bool ExtractStringValue(const std::string& text, const std::string& key, std::string& value)
{
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = text.find(token);
    if (keyPos == std::string::npos)
    {
        return false;
    }

    const std::size_t colonPos = text.find(':', keyPos + token.size());
    if (colonPos == std::string::npos)
    {
        return false;
    }

    std::size_t valuePos = colonPos + 1U;
    while (valuePos < text.size() && std::isspace(static_cast<unsigned char>(text[valuePos])) != 0)
    {
        ++valuePos;
    }

    if (valuePos >= text.size() || text[valuePos] != '"')
    {
        return false;
    }

    ++valuePos;
    std::size_t endPos = valuePos;
    while (endPos < text.size())
    {
        if (text[endPos] == '"' && text[endPos - 1U] != '\\')
        {
            value = text.substr(valuePos, endPos - valuePos);
            return true;
        }
        ++endPos;
    }

    return false;
}

bool ExtractBoolValue(const std::string& text, const std::string& key, bool& value)
{
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = text.find(token);
    if (keyPos == std::string::npos)
    {
        return false;
    }

    const std::size_t colonPos = text.find(':', keyPos + token.size());
    if (colonPos == std::string::npos)
    {
        return false;
    }

    std::size_t valuePos = colonPos + 1U;
    while (valuePos < text.size() && std::isspace(static_cast<unsigned char>(text[valuePos])) != 0)
    {
        ++valuePos;
    }

    if (text.compare(valuePos, 4U, "true") == 0)
    {
        value = true;
        return true;
    }

    if (text.compare(valuePos, 5U, "false") == 0)
    {
        value = false;
        return true;
    }

    return false;
}

bool ExtractRuleObjects(const std::string& text, std::vector<std::string>& objectTexts)
{
    const std::string token = "\"objects\"";
    const std::size_t keyPos = text.find(token);
    if (keyPos == std::string::npos)
    {
        return false;
    }

    const std::size_t arrayStart = text.find('[', keyPos + token.size());
    if (arrayStart == std::string::npos)
    {
        return false;
    }

    int braceDepth = 0;
    bool inString = false;
    std::size_t objectStart = std::string::npos;
    for (std::size_t i = arrayStart + 1U; i < text.size(); ++i)
    {
        const char ch = text[i];
        const bool escaped = (i > 0U && text[i - 1U] == '\\');
        if (ch == '"' && !escaped)
        {
            inString = !inString;
        }

        if (inString)
        {
            continue;
        }

        if (ch == '{')
        {
            if (braceDepth == 0)
            {
                objectStart = i;
            }
            ++braceDepth;
        }
        else if (ch == '}')
        {
            --braceDepth;
            if (braceDepth == 0 && objectStart != std::string::npos)
            {
                objectTexts.push_back(text.substr(objectStart, i - objectStart + 1U));
                objectStart = std::string::npos;
            }
        }
        else if (ch == ']' && braceDepth == 0)
        {
            return true;
        }
    }

    return !objectTexts.empty();
}

} // namespace

MaterialRuleLoadResult LoadSceneMaterialRules(const std::string& filePath)
{
    MaterialRuleLoadResult result;
    std::map<std::string, int> semanticIdDictionary;

    std::string content;
    if (!ReadWholeFile(filePath, content))
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::FileNotFound,
            "Module2",
            "Unable to open scene material rule file.",
            filePath,
            "Check the scene_material_map.json path.",
            true));
        return result;
    }

    ExtractStringValue(content, "default_medium", result.rule_set.default_medium);
    result.rule_set.default_medium_id = GetOrCreateSemanticId(semanticIdDictionary, result.rule_set.default_medium);

    std::vector<std::string> objectTexts;
    if (!ExtractRuleObjects(content, objectTexts))
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::JsonParseError,
            "Module2",
            "No valid objects rule array found in scene material rule file.",
            filePath,
            "Check the objects field in scene_material_map.json.",
            true));
        return result;
    }

    for (const std::string& objectText : objectTexts)
    {
        SceneMaterialRule rule;
        ExtractStringValue(objectText, "rule_name", rule.rule_name);
        ExtractStringValue(objectText, "object_name", rule.object_name);
        ExtractStringValue(objectText, "name_pattern", rule.object_name_pattern);
        ExtractStringValue(objectText, "object_type", rule.object_type);
        ExtractStringValue(objectText, "surface_material_name", rule.surface_material_name);
        ExtractStringValue(objectText, "front_material_name", rule.front_material_name);
        ExtractStringValue(objectText, "back_material_name", rule.back_material_name);
        ExtractStringValue(objectText, "normal_rule", rule.normal_rule);
        ExtractBoolValue(objectText, "reflection_enabled", rule.reflection_enabled);
        ExtractBoolValue(objectText, "transmission_enabled", rule.transmission_enabled);
        ExtractBoolValue(objectText, "diffraction_candidate_enabled", rule.diffraction_candidate_enabled);

        if (rule.object_name.empty() && rule.object_name_pattern.empty())
        {
            result.errors.push_back(RtError::Create(
                ErrorCode::JsonParseError,
                "Module2",
                "Scene material rule is missing both object_name and name_pattern.",
                filePath,
                rule.rule_name.empty() ? "unnamed_rule" : rule.rule_name,
                false));
            continue;
        }

        if (rule.surface_material_name.empty())
        {
            result.errors.push_back(RtError::Create(
                ErrorCode::JsonParseError,
                "Module2",
                "Scene material rule is missing surface_material_name.",
                filePath,
                rule.rule_name.empty() ? (rule.object_name.empty() ? rule.object_name_pattern : rule.object_name) : rule.rule_name,
                false));
            continue;
        }

        bool duplicatedExactRule = false;
        for (const SceneMaterialRule& existingRule : result.rule_set.rules)
        {
            if (!rule.object_name.empty() && rule.object_name == existingRule.object_name)
            {
                duplicatedExactRule = true;
                break;
            }
        }
        if (duplicatedExactRule)
        {
            result.errors.push_back(RtError::Create(
                ErrorCode::JsonParseError,
                "Module2",
                "Duplicate scene material rule object_name detected.",
                filePath,
                rule.object_name,
                false));
            continue;
        }

        if (rule.front_material_name.empty())
        {
            rule.front_material_name = result.rule_set.default_medium;
        }
        if (rule.back_material_name.empty())
        {
            rule.back_material_name = rule.surface_material_name;
        }

        rule.surface_material_id = GetOrCreateSemanticId(semanticIdDictionary, rule.surface_material_name);
        rule.front_material_id = GetOrCreateSemanticId(semanticIdDictionary, rule.front_material_name);
        rule.back_material_id = GetOrCreateSemanticId(semanticIdDictionary, rule.back_material_name);
        rule.front_medium_id = rule.front_material_id;
        rule.back_medium_id = rule.back_material_id;
        result.rule_set.rules.push_back(rule);
    }

    result.succeeded = result.rule_set.rules.size() > 0;
    return result;
}

} // namespace rt
