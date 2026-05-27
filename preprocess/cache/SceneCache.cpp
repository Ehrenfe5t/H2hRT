// 文件目标：
// - 实现模块2批次4的场景缓存能力。
//
// 主要功能：
// - 生成多维缓存元信息；
// - 读写 JSON meta 与二进制内容文件；
// - 显式判断 cache 命中/失效，避免静默使用过期缓存。

#include "SceneCache.h"

#include "../../core/query/SceneQuery.h"

#include <Windows.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

namespace rt {

namespace {

std::string BuildCacheBaseName(const AppConfig& config)
{
    return config.experiment.dataset_tag.empty() ? "scene_cache" : config.experiment.dataset_tag;
}

std::string ReadWholeFileOrEmpty(const std::string& filePath)
{
    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        return std::string();
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string ComputeSimpleHash(const std::string& text)
{
    unsigned long long hash = 1469598103934665603ull;
    for (unsigned char ch : text)
    {
        hash ^= static_cast<unsigned long long>(ch);
        hash *= 1099511628211ull;
    }

    std::ostringstream stream;
    stream << std::hex << hash;
    return stream.str();
}

std::string ComputeFileHash(const std::string& filePath)
{
    return ComputeSimpleHash(ReadWholeFileOrEmpty(filePath));
}

std::string GetLastWriteTimeString(const std::string& filePath)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(filePath.c_str(), GetFileExInfoStandard, &data))
    {
        return std::string();
    }

    ULARGE_INTEGER value;
    value.LowPart = data.ftLastWriteTime.dwLowDateTime;
    value.HighPart = data.ftLastWriteTime.dwHighDateTime;

    std::ostringstream stream;
    stream << value.QuadPart;
    return stream.str();
}

std::string BuildPreprocessConfigSignature(const AppConfig& config)
{
    std::ostringstream stream;
    stream << config.scene_preprocess.enable_wedge_build << "|"
           << config.scene_preprocess.enable_scene_cache << "|"
           << config.scene_import.coordinate_transform << "|"   // v7 H7: 坐标变换变更触发缓存失效
           << config.scene_preprocess.bvh_leaf_size << "|"
           << "v7";
    return stream.str();
}

void EnsureDirectory(const std::string& directoryPath)
{
    if (directoryPath.empty())
    {
        return;
    }

    std::string current;
    for (std::size_t i = 0; i < directoryPath.size(); ++i)
    {
        current.push_back(directoryPath[i]);
        if (directoryPath[i] == '/' || directoryPath[i] == '\\')
        {
            if (current.size() > 1U)
            {
                CreateDirectoryA(current.c_str(), nullptr);
            }
        }
    }

    CreateDirectoryA(directoryPath.c_str(), nullptr);
}

std::string BuildDirectoryFromPath(const std::string& filePath)
{
    const std::size_t pos = filePath.find_last_of("/\\");
    return (pos == std::string::npos) ? std::string() : filePath.substr(0, pos);
}

std::string CurrentTimestamp()
{
    SYSTEMTIME timeValue;
    GetLocalTime(&timeValue);

    char buffer[64] = {};
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%04u-%02u-%02u %02u:%02u:%02u",
        static_cast<unsigned>(timeValue.wYear),
        static_cast<unsigned>(timeValue.wMonth),
        static_cast<unsigned>(timeValue.wDay),
        static_cast<unsigned>(timeValue.wHour),
        static_cast<unsigned>(timeValue.wMinute),
        static_cast<unsigned>(timeValue.wSecond));
    return buffer;
}

std::string EncodeMetaToJson(const SceneCacheMeta& meta)
{
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"cache_format_version\": \"" << meta.cache_format_version << "\",\n";
    stream << "  \"generated_timestamp\": \"" << meta.generated_timestamp << "\",\n";
    stream << "  \"scene_source_file_path\": \"" << meta.scene_source_file_path << "\",\n";
    stream << "  \"scene_source_last_write_time\": \"" << meta.scene_source_last_write_time << "\",\n";
    stream << "  \"scene_source_hash\": \"" << meta.scene_source_hash << "\",\n";
    stream << "  \"material_rule_file_path\": \"" << meta.material_rule_file_path << "\",\n";
    stream << "  \"material_rule_last_write_time\": \"" << meta.material_rule_last_write_time << "\",\n";
    stream << "  \"material_rule_hash\": \"" << meta.material_rule_hash << "\",\n";
    stream << "  \"material_database_file_path\": \"" << meta.material_database_file_path << "\",\n";
    stream << "  \"material_database_last_write_time\": \"" << meta.material_database_last_write_time << "\",\n";
    stream << "  \"material_database_hash\": \"" << meta.material_database_hash << "\",\n";
    stream << "  \"preprocess_config_hash\": \"" << meta.preprocess_config_hash << "\",\n";
    stream << "  \"preprocess_algorithm_version\": \"" << meta.preprocess_algorithm_version << "\",\n";
    stream << "  \"vertex_count\": " << meta.vertex_count << ",\n";
    stream << "  \"face_count\": " << meta.face_count << ",\n";
    stream << "  \"edge_count\": " << meta.edge_count << ",\n";
    stream << "  \"wedge_count\": " << meta.wedge_count << ",\n";
    stream << "  \"contains_full_diagnostics\": " << (meta.contains_full_diagnostics ? "true" : "false") << ",\n";
    stream << "  \"contains_debug_auxiliary_data\": " << (meta.contains_debug_auxiliary_data ? "true" : "false") << "\n";
    stream << "}\n";
    return stream.str();
}

bool JsonContains(const std::string& text, const std::string& token)
{
    return text.find(token) != std::string::npos;
}

bool IsCacheMetaCompatible(const SceneCacheMeta& expected, const std::string& jsonText)
{
    return JsonContains(jsonText, "\"cache_format_version\": \"" + expected.cache_format_version + "\"") &&
           JsonContains(jsonText, "\"scene_source_hash\": \"" + expected.scene_source_hash + "\"") &&
           JsonContains(jsonText, "\"material_rule_hash\": \"" + expected.material_rule_hash + "\"") &&
           JsonContains(jsonText, "\"material_database_hash\": \"" + expected.material_database_hash + "\"") &&
           JsonContains(jsonText, "\"preprocess_config_hash\": \"" + expected.preprocess_config_hash + "\"") &&
           JsonContains(jsonText, "\"preprocess_algorithm_version\": \"" + expected.preprocess_algorithm_version + "\"");
}

void WriteString(std::ofstream& output, const std::string& value)
{
    const uint64_t size = static_cast<uint64_t>(value.size());
    output.write(reinterpret_cast<const char*>(&size), sizeof(size));
    if (size > 0U)
    {
        output.write(value.data(), static_cast<std::streamsize>(size));
    }
}

bool ReadString(std::ifstream& input, std::string& value)
{
    uint64_t size = 0U;
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (!input.good())
    {
        return false;
    }
    value.assign(size, '\0');
    if (size > 0U)
    {
        input.read(&value[0], static_cast<std::streamsize>(size));
    }
    return input.good();
}

template <typename T>
void WriteVector(std::ofstream& output, const std::vector<T>& values)
{
    const std::size_t size = values.size();
    output.write(reinterpret_cast<const char*>(&size), sizeof(size));
    if (size > 0U)
    {
        output.write(reinterpret_cast<const char*>(values.data()), static_cast<std::streamsize>(sizeof(T) * size));
    }
}

template <typename T>
bool ReadVector(std::ifstream& input, std::vector<T>& values)
{
    uint64_t size = 0U;
    input.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (!input.good())
    {
        return false;
    }
    values.resize(size);
    if (size > 0U)
    {
        input.read(reinterpret_cast<char*>(values.data()), static_cast<std::streamsize>(sizeof(T) * size));
    }
    return input.good();
}

void WriteSceneObjectRecord(std::ofstream& output, const SceneObjectRecord& record)
{
    output.write(reinterpret_cast<const char*>(&record.object_id), sizeof(record.object_id));
    WriteString(output, record.object_name);
    WriteVector(output, record.face_ids);
}

bool ReadSceneObjectRecord(std::ifstream& input, SceneObjectRecord& record)
{
    input.read(reinterpret_cast<char*>(&record.object_id), sizeof(record.object_id));
    if (!input.good())
    {
        return false;
    }
    return ReadString(input, record.object_name) && ReadVector(input, record.face_ids);
}

void WriteSceneMaterialBindingRecord(std::ofstream& output, const SceneMaterialBinding& binding)
{
    output.write(reinterpret_cast<const char*>(&binding.object_id), sizeof(binding.object_id));
    WriteString(output, binding.object_name);
    WriteString(output, binding.object_type);
    WriteString(output, binding.rule_match_mode);
    WriteString(output, binding.surface_material_name);
    WriteString(output, binding.front_material_name);
    WriteString(output, binding.back_material_name);
    WriteString(output, binding.normal_rule_tag);
    WriteString(output, binding.rule_name);
    WriteString(output, binding.recovery_quality_tag);
    WriteString(output, binding.unresolved_reason);
    output.write(reinterpret_cast<const char*>(&binding.used_default_front_material), sizeof(binding.used_default_front_material));
    output.write(reinterpret_cast<const char*>(&binding.used_default_back_material), sizeof(binding.used_default_back_material));
    WriteVector(output, binding.face_ids);
    const std::size_t flagCount = binding.face_dual_side_resolved_flags.size();
    output.write(reinterpret_cast<const char*>(&flagCount), sizeof(flagCount));
    for (bool flag : binding.face_dual_side_resolved_flags)
    {
        const char value = flag ? 1 : 0;
        output.write(&value, sizeof(value));
    }
}

bool ReadSceneMaterialBindingRecord(std::ifstream& input, SceneMaterialBinding& binding)
{
    input.read(reinterpret_cast<char*>(&binding.object_id), sizeof(binding.object_id));
    if (!input.good())
    {
        return false;
    }
    if (!ReadString(input, binding.object_name) ||
        !ReadString(input, binding.object_type) ||
        !ReadString(input, binding.rule_match_mode) ||
        !ReadString(input, binding.surface_material_name) ||
        !ReadString(input, binding.front_material_name) ||
        !ReadString(input, binding.back_material_name) ||
        !ReadString(input, binding.normal_rule_tag) ||
        !ReadString(input, binding.rule_name) ||
        !ReadString(input, binding.recovery_quality_tag) ||
        !ReadString(input, binding.unresolved_reason) ||
        !ReadVector(input, binding.face_ids))
    {
        return false;
    }

    input.read(reinterpret_cast<char*>(&binding.used_default_front_material), sizeof(binding.used_default_front_material));
    input.read(reinterpret_cast<char*>(&binding.used_default_back_material), sizeof(binding.used_default_back_material));
    if (!input.good())
    {
        return false;
    }

    std::size_t flagCount = 0U;
    input.read(reinterpret_cast<char*>(&flagCount), sizeof(flagCount));
    if (!input.good())
    {
        return false;
    }
    binding.face_dual_side_resolved_flags.resize(flagCount);
    for (std::size_t i = 0; i < flagCount; ++i)
    {
        char value = 0;
        input.read(&value, sizeof(value));
        if (!input.good())
        {
            return false;
        }
        binding.face_dual_side_resolved_flags[i] = (value != 0);
    }
    return true;
}

void WriteFaceRecord(std::ofstream& output, const Face& face)
{
    output.write(reinterpret_cast<const char*>(&face.face_id), sizeof(face.face_id));
    output.write(reinterpret_cast<const char*>(&face.object_id), sizeof(face.object_id));
    output.write(reinterpret_cast<const char*>(&face.vertex_index0), sizeof(face.vertex_index0));
    output.write(reinterpret_cast<const char*>(&face.vertex_index1), sizeof(face.vertex_index1));
    output.write(reinterpret_cast<const char*>(&face.vertex_index2), sizeof(face.vertex_index2));
    output.write(reinterpret_cast<const char*>(&face.normal_index), sizeof(face.normal_index));
    output.write(reinterpret_cast<const char*>(&face.normal), sizeof(face.normal));
    output.write(reinterpret_cast<const char*>(&face.centroid), sizeof(face.centroid));
    output.write(reinterpret_cast<const char*>(&face.bounds), sizeof(face.bounds));
    output.write(reinterpret_cast<const char*>(&face.area), sizeof(face.area));
    output.write(reinterpret_cast<const char*>(&face.local_frame), sizeof(face.local_frame));
    WriteString(output, face.object_name);
    WriteString(output, face.object_type);
    WriteString(output, face.surface_material_name);
    WriteString(output, face.front_material_name);
    WriteString(output, face.back_material_name);
    WriteString(output, face.normal_rule_tag);
    output.write(reinterpret_cast<const char*>(&face.dual_side_material_resolved), sizeof(face.dual_side_material_resolved));
    output.write(reinterpret_cast<const char*>(&face.reflection_enabled), sizeof(face.reflection_enabled));
    output.write(reinterpret_cast<const char*>(&face.transmission_enabled), sizeof(face.transmission_enabled));
    output.write(reinterpret_cast<const char*>(&face.diffraction_candidate_enabled), sizeof(face.diffraction_candidate_enabled));
    output.write(reinterpret_cast<const char*>(&face.degenerate), sizeof(face.degenerate));
    output.write(reinterpret_cast<const char*>(&face.propagation_flags), sizeof(face.propagation_flags));
    output.write(reinterpret_cast<const char*>(&face.adjacent_edge_id0), sizeof(face.adjacent_edge_id0));
    output.write(reinterpret_cast<const char*>(&face.adjacent_edge_id1), sizeof(face.adjacent_edge_id1));
    output.write(reinterpret_cast<const char*>(&face.adjacent_edge_id2), sizeof(face.adjacent_edge_id2));
}

bool ReadFaceRecord(std::ifstream& input, Face& face)
{
    input.read(reinterpret_cast<char*>(&face.face_id), sizeof(face.face_id));
    input.read(reinterpret_cast<char*>(&face.object_id), sizeof(face.object_id));
    input.read(reinterpret_cast<char*>(&face.vertex_index0), sizeof(face.vertex_index0));
    input.read(reinterpret_cast<char*>(&face.vertex_index1), sizeof(face.vertex_index1));
    input.read(reinterpret_cast<char*>(&face.vertex_index2), sizeof(face.vertex_index2));
    input.read(reinterpret_cast<char*>(&face.normal_index), sizeof(face.normal_index));
    input.read(reinterpret_cast<char*>(&face.normal), sizeof(face.normal));
    input.read(reinterpret_cast<char*>(&face.centroid), sizeof(face.centroid));
    input.read(reinterpret_cast<char*>(&face.bounds), sizeof(face.bounds));
    input.read(reinterpret_cast<char*>(&face.area), sizeof(face.area));
    input.read(reinterpret_cast<char*>(&face.local_frame), sizeof(face.local_frame));
    if (!input.good())
    {
        return false;
    }
    if (!ReadString(input, face.object_name) ||
        !ReadString(input, face.object_type) ||
        !ReadString(input, face.surface_material_name) ||
        !ReadString(input, face.front_material_name) ||
        !ReadString(input, face.back_material_name) ||
        !ReadString(input, face.normal_rule_tag))
    {
        return false;
    }
    input.read(reinterpret_cast<char*>(&face.dual_side_material_resolved), sizeof(face.dual_side_material_resolved));
    input.read(reinterpret_cast<char*>(&face.reflection_enabled), sizeof(face.reflection_enabled));
    input.read(reinterpret_cast<char*>(&face.transmission_enabled), sizeof(face.transmission_enabled));
    input.read(reinterpret_cast<char*>(&face.diffraction_candidate_enabled), sizeof(face.diffraction_candidate_enabled));
    input.read(reinterpret_cast<char*>(&face.degenerate), sizeof(face.degenerate));
    input.read(reinterpret_cast<char*>(&face.propagation_flags), sizeof(face.propagation_flags));
    input.read(reinterpret_cast<char*>(&face.adjacent_edge_id0), sizeof(face.adjacent_edge_id0));
    input.read(reinterpret_cast<char*>(&face.adjacent_edge_id1), sizeof(face.adjacent_edge_id1));
    input.read(reinterpret_cast<char*>(&face.adjacent_edge_id2), sizeof(face.adjacent_edge_id2));
    return input.good();
}

void WriteWedgeRecord(std::ofstream& output, const Wedge& wedge)
{
    output.write(reinterpret_cast<const char*>(&wedge.wedge_id), sizeof(wedge.wedge_id));
    output.write(reinterpret_cast<const char*>(&wedge.source_edge_id), sizeof(wedge.source_edge_id));
    output.write(reinterpret_cast<const char*>(&wedge.positive_face_id), sizeof(wedge.positive_face_id));
    output.write(reinterpret_cast<const char*>(&wedge.negative_face_id), sizeof(wedge.negative_face_id));
    output.write(reinterpret_cast<const char*>(&wedge.center_point), sizeof(wedge.center_point));
    output.write(reinterpret_cast<const char*>(&wedge.segment_start), sizeof(wedge.segment_start));
    output.write(reinterpret_cast<const char*>(&wedge.segment_end), sizeof(wedge.segment_end));
    output.write(reinterpret_cast<const char*>(&wedge.direction), sizeof(wedge.direction));
    output.write(reinterpret_cast<const char*>(&wedge.length), sizeof(wedge.length));
    output.write(reinterpret_cast<const char*>(&wedge.wedge_angle_deg), sizeof(wedge.wedge_angle_deg));
    output.write(reinterpret_cast<const char*>(&wedge.dihedral_angle_deg), sizeof(wedge.dihedral_angle_deg));
    WriteString(output, wedge.positive_material_name);
    WriteString(output, wedge.negative_material_name);
    output.write(reinterpret_cast<const char*>(&wedge.diffractable), sizeof(wedge.diffractable));
    output.write(reinterpret_cast<const char*>(&wedge.from_non_manifold_source), sizeof(wedge.from_non_manifold_source));
    output.write(reinterpret_cast<const char*>(&wedge.wedge_flags), sizeof(wedge.wedge_flags));
    output.write(reinterpret_cast<const char*>(&wedge.bounds), sizeof(wedge.bounds));
}

bool ReadWedgeRecord(std::ifstream& input, Wedge& wedge)
{
    input.read(reinterpret_cast<char*>(&wedge.wedge_id), sizeof(wedge.wedge_id));
    input.read(reinterpret_cast<char*>(&wedge.source_edge_id), sizeof(wedge.source_edge_id));
    input.read(reinterpret_cast<char*>(&wedge.positive_face_id), sizeof(wedge.positive_face_id));
    input.read(reinterpret_cast<char*>(&wedge.negative_face_id), sizeof(wedge.negative_face_id));
    input.read(reinterpret_cast<char*>(&wedge.center_point), sizeof(wedge.center_point));
    input.read(reinterpret_cast<char*>(&wedge.segment_start), sizeof(wedge.segment_start));
    input.read(reinterpret_cast<char*>(&wedge.segment_end), sizeof(wedge.segment_end));
    input.read(reinterpret_cast<char*>(&wedge.direction), sizeof(wedge.direction));
    input.read(reinterpret_cast<char*>(&wedge.length), sizeof(wedge.length));
    input.read(reinterpret_cast<char*>(&wedge.wedge_angle_deg), sizeof(wedge.wedge_angle_deg));
    input.read(reinterpret_cast<char*>(&wedge.dihedral_angle_deg), sizeof(wedge.dihedral_angle_deg));
    if (!input.good())
    {
        return false;
    }
    if (!ReadString(input, wedge.positive_material_name) || !ReadString(input, wedge.negative_material_name))
    {
        return false;
    }
    input.read(reinterpret_cast<char*>(&wedge.diffractable), sizeof(wedge.diffractable));
    input.read(reinterpret_cast<char*>(&wedge.from_non_manifold_source), sizeof(wedge.from_non_manifold_source));
    input.read(reinterpret_cast<char*>(&wedge.wedge_flags), sizeof(wedge.wedge_flags));
    input.read(reinterpret_cast<char*>(&wedge.bounds), sizeof(wedge.bounds));
    return input.good();
}

void WriteScene(std::ofstream& output, const Scene& scene)
{
    WriteString(output, scene.meta.source_file_path);
    WriteString(output, scene.meta.source_format);
    output.write(reinterpret_cast<const char*>(&scene.meta.object_count), sizeof(scene.meta.object_count));
    output.write(reinterpret_cast<const char*>(&scene.meta.vertex_count), sizeof(scene.meta.vertex_count));
    output.write(reinterpret_cast<const char*>(&scene.meta.normal_count), sizeof(scene.meta.normal_count));
    output.write(reinterpret_cast<const char*>(&scene.meta.face_count), sizeof(scene.meta.face_count));
    WriteVector(output, scene.vertices);
    WriteVector(output, scene.normals);

    std::size_t objectCount = scene.objects.size();
    output.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));
    for (const SceneObjectRecord& object : scene.objects)
    {
        WriteSceneObjectRecord(output, object);
    }

    std::size_t faceCount = scene.faces.size();
    output.write(reinterpret_cast<const char*>(&faceCount), sizeof(faceCount));
    for (const Face& face : scene.faces)
    {
        WriteFaceRecord(output, face);
    }

    std::size_t bindingCount = scene.material_bindings.size();
    output.write(reinterpret_cast<const char*>(&bindingCount), sizeof(bindingCount));
    for (const SceneMaterialBinding& binding : scene.material_bindings)
    {
        WriteSceneMaterialBindingRecord(output, binding);
    }

    WriteVector(output, scene.edges);
    std::size_t wedgeCount = scene.wedges.size();
    output.write(reinterpret_cast<const char*>(&wedgeCount), sizeof(wedgeCount));
    for (const Wedge& wedge : scene.wedges)
    {
        WriteWedgeRecord(output, wedge);
    }
    WriteVector(output, scene.diagnostics.degenerate_faces);
    WriteVector(output, scene.diagnostics.non_manifold_edges);
    WriteVector(output, scene.diagnostics.duplicated_faces);
    WriteVector(output, scene.diagnostics.flipped_normal_faces);
    output.write(reinterpret_cast<const char*>(&scene.diagnostics.passed), sizeof(scene.diagnostics.passed));
    std::size_t missingObjectCount = scene.diagnostics.objects_missing_material_mapping.size();
    output.write(reinterpret_cast<const char*>(&missingObjectCount), sizeof(missingObjectCount));
    for (const std::string& item : scene.diagnostics.objects_missing_material_mapping)
    {
        WriteString(output, item);
    }
    WriteVector(output, scene.diagnostics.faces_missing_dual_side_material);

    output.write(reinterpret_cast<const char*>(&scene.acceleration.face_acceleration.scene_bounds), sizeof(scene.acceleration.face_acceleration.scene_bounds));
    WriteVector(output, scene.acceleration.face_acceleration.face_bvh.nodes);
    WriteVector(output, scene.acceleration.face_acceleration.face_bvh.primitive_face_ids);
    output.write(reinterpret_cast<const char*>(&scene.acceleration.face_acceleration.face_bvh.valid), sizeof(scene.acceleration.face_acceleration.face_bvh.valid));
    WriteVector(output, scene.acceleration.face_acceleration.face_query_records);
    output.write(reinterpret_cast<const char*>(&scene.acceleration.face_acceleration.valid), sizeof(scene.acceleration.face_acceleration.valid));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.face_acceleration.bvh_node_count), sizeof(scene.acceleration.face_acceleration.bvh_node_count));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.face_acceleration.leaf_node_count), sizeof(scene.acceleration.face_acceleration.leaf_node_count));
    WriteVector(output, scene.acceleration.wedge_acceleration.wedge_query_records);
    output.write(reinterpret_cast<const char*>(&scene.acceleration.wedge_acceleration.valid), sizeof(scene.acceleration.wedge_acceleration.valid));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.wedge_acceleration.wedge_count), sizeof(scene.acceleration.wedge_acceleration.wedge_count));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.wedge_acceleration.diffractable_wedge_count), sizeof(scene.acceleration.wedge_acceleration.diffractable_wedge_count));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.build_succeeded), sizeof(scene.acceleration.diagnostics.build_succeeded));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.brute_force_validation_ran), sizeof(scene.acceleration.diagnostics.brute_force_validation_ran));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.brute_force_validation_passed), sizeof(scene.acceleration.diagnostics.brute_force_validation_passed));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.face_bvh_build_time_ms), sizeof(scene.acceleration.diagnostics.face_bvh_build_time_ms));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.wedge_acceleration_build_time_ms), sizeof(scene.acceleration.diagnostics.wedge_acceleration_build_time_ms));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.face_bvh_max_depth), sizeof(scene.acceleration.diagnostics.face_bvh_max_depth));
    output.write(reinterpret_cast<const char*>(&scene.acceleration.diagnostics.face_bvh_average_leaf_faces), sizeof(scene.acceleration.diagnostics.face_bvh_average_leaf_faces));
}

bool ReadScene(std::ifstream& input, Scene& scene)
{
    if (!ReadString(input, scene.meta.source_file_path) || !ReadString(input, scene.meta.source_format))
    {
        return false;
    }
    input.read(reinterpret_cast<char*>(&scene.meta.object_count), sizeof(scene.meta.object_count));
    input.read(reinterpret_cast<char*>(&scene.meta.vertex_count), sizeof(scene.meta.vertex_count));
    input.read(reinterpret_cast<char*>(&scene.meta.normal_count), sizeof(scene.meta.normal_count));
    input.read(reinterpret_cast<char*>(&scene.meta.face_count), sizeof(scene.meta.face_count));
    if (!input.good() || !ReadVector(input, scene.vertices) || !ReadVector(input, scene.normals))
    {
        return false;
    }

    std::size_t objectCount = 0U;
    input.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount));
    if (!input.good()) return false;
    scene.objects.resize(objectCount);
    for (std::size_t i = 0; i < objectCount; ++i)
    {
        if (!ReadSceneObjectRecord(input, scene.objects[i])) return false;
    }

    std::size_t faceCount = 0U;
    input.read(reinterpret_cast<char*>(&faceCount), sizeof(faceCount));
    if (!input.good()) return false;
    scene.faces.resize(faceCount);
    for (std::size_t i = 0; i < faceCount; ++i)
    {
        if (!ReadFaceRecord(input, scene.faces[i])) return false;
    }

    std::size_t bindingCount = 0U;
    input.read(reinterpret_cast<char*>(&bindingCount), sizeof(bindingCount));
    if (!input.good()) return false;
    scene.material_bindings.resize(bindingCount);
    for (std::size_t i = 0; i < bindingCount; ++i)
    {
        if (!ReadSceneMaterialBindingRecord(input, scene.material_bindings[i])) return false;
    }

    if (!ReadVector(input, scene.edges))
    {
        return false;
    }

    std::size_t wedgeCount = 0U;
    input.read(reinterpret_cast<char*>(&wedgeCount), sizeof(wedgeCount));
    if (!input.good())
    {
        return false;
    }
    scene.wedges.resize(wedgeCount);
    for (std::size_t i = 0; i < wedgeCount; ++i)
    {
        if (!ReadWedgeRecord(input, scene.wedges[i]))
        {
            return false;
        }
    }

    if (
        !ReadVector(input, scene.diagnostics.degenerate_faces) ||
        !ReadVector(input, scene.diagnostics.non_manifold_edges) ||
        !ReadVector(input, scene.diagnostics.duplicated_faces) ||
        !ReadVector(input, scene.diagnostics.flipped_normal_faces))
    {
        return false;
    }

    input.read(reinterpret_cast<char*>(&scene.diagnostics.passed), sizeof(scene.diagnostics.passed));
    if (!input.good()) return false;
    std::size_t missingObjectCount = 0U;
    input.read(reinterpret_cast<char*>(&missingObjectCount), sizeof(missingObjectCount));
    if (!input.good()) return false;
    scene.diagnostics.objects_missing_material_mapping.resize(missingObjectCount);
    for (std::size_t i = 0; i < missingObjectCount; ++i)
    {
        if (!ReadString(input, scene.diagnostics.objects_missing_material_mapping[i])) return false;
    }
    if (!ReadVector(input, scene.diagnostics.faces_missing_dual_side_material)) return false;

    input.read(reinterpret_cast<char*>(&scene.acceleration.face_acceleration.scene_bounds), sizeof(scene.acceleration.face_acceleration.scene_bounds));
    if (!input.good() ||
        !ReadVector(input, scene.acceleration.face_acceleration.face_bvh.nodes) ||
        !ReadVector(input, scene.acceleration.face_acceleration.face_bvh.primitive_face_ids))
    {
        return false;
    }
    input.read(reinterpret_cast<char*>(&scene.acceleration.face_acceleration.face_bvh.valid), sizeof(scene.acceleration.face_acceleration.face_bvh.valid));
    if (!input.good() || !ReadVector(input, scene.acceleration.face_acceleration.face_query_records)) return false;
    input.read(reinterpret_cast<char*>(&scene.acceleration.face_acceleration.valid), sizeof(scene.acceleration.face_acceleration.valid));
    input.read(reinterpret_cast<char*>(&scene.acceleration.face_acceleration.bvh_node_count), sizeof(scene.acceleration.face_acceleration.bvh_node_count));
    input.read(reinterpret_cast<char*>(&scene.acceleration.face_acceleration.leaf_node_count), sizeof(scene.acceleration.face_acceleration.leaf_node_count));
    if (!input.good() || !ReadVector(input, scene.acceleration.wedge_acceleration.wedge_query_records)) return false;
    input.read(reinterpret_cast<char*>(&scene.acceleration.wedge_acceleration.valid), sizeof(scene.acceleration.wedge_acceleration.valid));
    input.read(reinterpret_cast<char*>(&scene.acceleration.wedge_acceleration.wedge_count), sizeof(scene.acceleration.wedge_acceleration.wedge_count));
    input.read(reinterpret_cast<char*>(&scene.acceleration.wedge_acceleration.diffractable_wedge_count), sizeof(scene.acceleration.wedge_acceleration.diffractable_wedge_count));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.build_succeeded), sizeof(scene.acceleration.diagnostics.build_succeeded));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.brute_force_validation_ran), sizeof(scene.acceleration.diagnostics.brute_force_validation_ran));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.brute_force_validation_passed), sizeof(scene.acceleration.diagnostics.brute_force_validation_passed));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.face_bvh_build_time_ms), sizeof(scene.acceleration.diagnostics.face_bvh_build_time_ms));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.wedge_acceleration_build_time_ms), sizeof(scene.acceleration.diagnostics.wedge_acceleration_build_time_ms));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.face_bvh_max_depth), sizeof(scene.acceleration.diagnostics.face_bvh_max_depth));
    input.read(reinterpret_cast<char*>(&scene.acceleration.diagnostics.face_bvh_average_leaf_faces), sizeof(scene.acceleration.diagnostics.face_bvh_average_leaf_faces));
    return input.good();
}

} // namespace

/// <summary>
/// 获取场景缓存元信息文件路径。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>缓存 meta 文件路径。</returns>
std::string BuildSceneCacheMetaFilePath(const AppConfig& config)
{
    return config.app_runtime.cache_directory + "/" + BuildCacheBaseName(config) + ".scene_cache_meta.json";
}

/// <summary>
/// 获取场景缓存内容文件路径。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>缓存 content 文件路径。</returns>
std::string BuildSceneCacheContentFilePath(const AppConfig& config)
{
    return config.app_runtime.cache_directory + "/" + BuildCacheBaseName(config) + ".scene_cache_content.bin";
}

/// <summary>
/// 构建当前场景的缓存元信息。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待摘要的场景对象。</param>
/// <returns>当前场景对应的缓存元信息。</returns>
SceneCacheMeta BuildSceneCacheMeta(const AppConfig& config, const Scene& scene)
{
    SceneCacheMeta meta;
    meta.cache_format_version = std::string("1.0");
    meta.generated_timestamp = CurrentTimestamp();
    meta.scene_source_file_path = config.scene_import.source_file;
    meta.scene_source_last_write_time = GetLastWriteTimeString(config.scene_import.source_file);
    meta.scene_source_hash = ComputeFileHash(config.scene_import.source_file);
    meta.material_rule_file_path = config.scene_import.scene_material_map_file;
    meta.material_rule_last_write_time = GetLastWriteTimeString(config.scene_import.scene_material_map_file);
    meta.material_rule_hash = ComputeFileHash(config.scene_import.scene_material_map_file);
    meta.material_database_file_path = config.material.material_database_file;
    meta.material_database_last_write_time = GetLastWriteTimeString(config.material.material_database_file);
    meta.material_database_hash = ComputeFileHash(config.material.material_database_file);
    meta.preprocess_config_hash = ComputeSimpleHash(BuildPreprocessConfigSignature(config));
    meta.preprocess_algorithm_version = std::string("v6");
    meta.vertex_count = static_cast<int>(scene.vertices.size());
    meta.face_count = static_cast<int>(scene.faces.size());
    meta.edge_count = static_cast<int>(scene.edges.size());
    meta.wedge_count = static_cast<int>(scene.wedges.size());
    meta.contains_full_diagnostics = true;
    meta.contains_debug_auxiliary_data = true;
    meta.replay_support_ready = meta.face_count > 0;
    meta.cache_status_reason = "built_from_current_scene";
    return meta;
}

/// <summary>
/// 尝试从缓存加载场景内容。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>结构化缓存加载结果。</returns>
SceneCacheLoadResult TryLoadSceneCache(const AppConfig& config)
{
    SceneCacheLoadResult result;
    if (!config.scene_preprocess.enable_scene_cache)
    {
        result.content.meta.cache_status_reason = "cache_disabled_by_config";
        return result;
    }

    const std::string metaPath = BuildSceneCacheMetaFilePath(config);
    const std::string contentPath = BuildSceneCacheContentFilePath(config);
    const std::string metaText = ReadWholeFileOrEmpty(metaPath);
    if (metaText.empty())
    {
        result.content.meta.cache_status_reason = "meta_file_missing_or_empty";
        return result;
    }

    const SceneCacheMeta expectedMeta = BuildSceneCacheMeta(config, Scene{});
    if (!IsCacheMetaCompatible(expectedMeta, metaText))
    {
        result.content.meta.cache_status_reason = "meta_incompatible_with_current_inputs";
        return result;
    }

    std::ifstream input(contentPath.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::FileNotFound,
            "Module2",
            "Scene cache meta matched but content file is missing.",
            contentPath,
            "Rebuild scene cache and check cache directory integrity.",
            true));
        return result;
    }

    Scene scene;
    if (!ReadScene(input, scene))
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::ValidationFailed,
            "Module2",
            "Failed to restore full scene cache content.",
            contentPath,
            "Rebuild cache because binary content appears corrupted or incompatible.",
            true));
        return result;
    }

    result.content.meta = BuildSceneCacheMeta(config, scene);
    result.content.meta.generated_timestamp = expectedMeta.generated_timestamp;
    result.content.meta.cache_format_version = expectedMeta.cache_format_version;
    result.content.meta.replay_support_ready = true;
    result.content.meta.cache_status_reason = "cache_hit_replayed";
    result.content.scene = scene;
    result.cache_hit = true;
    result.succeeded = true;
    return result;
}

/// <summary>
/// 将场景内容写入缓存。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待缓存的场景对象。</param>
/// <returns>结构化缓存写出结果。</returns>
SceneCacheWriteResult WriteSceneCache(const AppConfig& config, const Scene& scene)
{
    SceneCacheWriteResult result;
    if (!config.scene_preprocess.enable_scene_cache)
    {
        result.succeeded = true;
        return result;
    }

    result.meta = BuildSceneCacheMeta(config, scene);
    result.meta.cache_status_reason = "cache_written_for_replay";
    const std::string metaPath = BuildSceneCacheMetaFilePath(config);
    const std::string contentPath = BuildSceneCacheContentFilePath(config);
    EnsureDirectory(BuildDirectoryFromPath(metaPath));
    EnsureDirectory(BuildDirectoryFromPath(contentPath));

    std::ofstream metaOutput(metaPath.c_str(), std::ios::out | std::ios::trunc);
    if (!metaOutput.is_open())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::InternalError,
            "Module2",
            "Failed to open scene cache meta file for writing.",
            metaPath,
            "Check cache directory permissions.",
            true));
        return result;
    }
    metaOutput << EncodeMetaToJson(result.meta);
    metaOutput.flush();

    std::ofstream contentOutput(contentPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!contentOutput.is_open())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::InternalError,
            "Module2",
            "Failed to open scene cache content file for writing.",
            contentPath,
            "Check cache directory permissions.",
            true));
        return result;
    }

    WriteScene(contentOutput, scene);
    contentOutput.flush();

    if (!contentOutput.good())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::InternalError,
            "Module2",
            "Failed while writing scene cache content file.",
            contentPath,
            "Check disk availability and rebuild cache.",
            true));
        return result;
    }

    result.succeeded = true;
    return result;
}

} // namespace rt
