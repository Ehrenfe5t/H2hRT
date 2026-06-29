#include "V11UserConfigAdapter.h"

#include "../../../tools/SDK/support/tinygltf/json.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

using nlohmann::json;

namespace rt {

namespace {

bool HasKey(const json& obj, const char* key)
{
    return obj.is_object() && obj.find(key) != obj.end();
}

template <typename T>
void ReadField(const json& obj, const char* key, T& target)
{
    if (!HasKey(obj, key)) return;
    try { obj.at(key).get_to(target); } catch (...) {}
}

std::string ToStringPath(const std::filesystem::path& path)
{
    return path.lexically_normal().generic_string();
}

std::string ResolvePath(const std::filesystem::path& configDir, const std::string& input)
{
    if (input.empty()) return input;
    std::filesystem::path p(input);
    if (p.is_absolute()) return ToStringPath(p);
    return ToStringPath(configDir / p);
}

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool ReadVec3Array(const json& obj, const char* key, double& x, double& y, double& z)
{
    if (!HasKey(obj, key) || !obj.at(key).is_array() || obj.at(key).size() < 3) return false;
    try {
        x = obj.at(key).at(0).get<double>();
        y = obj.at(key).at(1).get<double>();
        z = obj.at(key).at(2).get<double>();
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> ReadStringList(const json& obj, const char* key)
{
    std::vector<std::string> values;
    if (!HasKey(obj, key) || !obj.at(key).is_array()) return values;
    for (const auto& item : obj.at(key)) {
        if (item.is_string()) values.push_back(item.get<std::string>());
    }
    return values;
}

std::string InferAntennaSourceType(const std::string& patternFile, const std::string& polarizationFile)
{
    return (!patternFile.empty() || !polarizationFile.empty()) ? "Pattern" : "Ideal";
}

bool LoadJsonFile(const std::string& path, json& root, std::string& error)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        error = "Unable to open JSON file: " + path;
        return false;
    }
    try {
        in >> root;
    } catch (const std::exception& e) {
        error = std::string("JSON parse error in ") + path + ": " + e.what();
        return false;
    }
    return true;
}

bool ReadTxList(const json& antennaRoot,
                const std::filesystem::path& antennaDir,
                std::vector<TxTarget>& txList,
                std::string& error)
{
    if (!HasKey(antennaRoot, "tx_list") || !antennaRoot.at("tx_list").is_array()) {
        error = "antenna.json requires tx_list array.";
        return false;
    }

    for (const auto& item : antennaRoot.at("tx_list")) {
        if (!item.is_object()) continue;
        TxTarget tx;
        ReadField(item, "id", tx.id);
        ReadVec3Array(item, "position_m", tx.x, tx.y, tx.z);
        ReadField(item, "power_dBm", tx.power_dBm);
        ReadField(item, "pattern_file", tx.tx_pattern_file);
        ReadField(item, "polarization_file", tx.tx_polarization_file);
        ReadVec3Array(item, "forward", tx.tx_forward_x, tx.tx_forward_y, tx.tx_forward_z);
        ReadVec3Array(item, "up", tx.tx_up_x, tx.tx_up_y, tx.tx_up_z);
        tx.tx_pattern_file = ResolvePath(antennaDir, tx.tx_pattern_file);
        tx.tx_polarization_file = ResolvePath(antennaDir, tx.tx_polarization_file);
        tx.tx_source_type = InferAntennaSourceType(tx.tx_pattern_file, tx.tx_polarization_file);
        if (!tx.id.empty()) txList.push_back(tx);
    }

    if (txList.empty()) {
        error = "antenna.json tx_list does not contain any valid transmitter.";
        return false;
    }
    return true;
}

bool ReadRxList(const json& antennaRoot,
                const std::filesystem::path& antennaDir,
                std::vector<RxTarget>& rxList,
                std::string& error)
{
    if (!HasKey(antennaRoot, "rx_list") || !antennaRoot.at("rx_list").is_array()) {
        error = "antenna.json requires rx_list array.";
        return false;
    }

    for (const auto& item : antennaRoot.at("rx_list")) {
        if (!item.is_object()) continue;
        RxTarget rx;
        ReadField(item, "id", rx.id);
        ReadVec3Array(item, "position_m", rx.x, rx.y, rx.z);
        ReadField(item, "pattern_file", rx.rx_pattern_file);
        ReadField(item, "polarization_file", rx.rx_polarization_file);
        ReadVec3Array(item, "forward", rx.rx_forward_x, rx.rx_forward_y, rx.rx_forward_z);
        ReadVec3Array(item, "up", rx.rx_up_x, rx.rx_up_y, rx.rx_up_z);
        rx.rx_pattern_file = ResolvePath(antennaDir, rx.rx_pattern_file);
        rx.rx_polarization_file = ResolvePath(antennaDir, rx.rx_polarization_file);
        rx.rx_source_type = InferAntennaSourceType(rx.rx_pattern_file, rx.rx_polarization_file);
        if (!rx.id.empty()) rxList.push_back(rx);
    }

    if (rxList.empty()) {
        error = "antenna.json rx_list does not contain any valid receiver.";
        return false;
    }
    return true;
}

std::vector<std::string> AllTxIds(const std::vector<TxTarget>& txList)
{
    std::vector<std::string> ids;
    for (const TxTarget& tx : txList) ids.push_back(tx.id);
    return ids;
}

std::vector<std::string> AllRxIds(const std::vector<RxTarget>& rxList)
{
    std::vector<std::string> ids;
    for (const RxTarget& rx : rxList) ids.push_back(rx.id);
    return ids;
}

} // namespace

V11UserConfigDecodeResult DecodeV11UserConfigFromJsonFile(const std::string& filePath)
{
    V11UserConfigDecodeResult result;

    json root;
    std::string error;
    if (!LoadJsonFile(filePath, root, error)) {
        result.error_message = error;
        return result;
    }

    result.recognized = HasKey(root, "scene") || HasKey(root, "p2p") || HasKey(root, "antenna_file");
    if (!result.recognized) return result;

    const std::filesystem::path configDir = std::filesystem::absolute(filePath).parent_path();

    AppConfig config;
    config.v11_user_config_enabled = true;
    config.app_runtime.mode = "debug";
    config.app_runtime.log_level = "INFO";
    config.validation.run_module1_self_check = false;
    config.pipeline.enable_stage0_precompute = false;
    config.pipeline.enable_stage4_precise_em = true;
    config.output.export_paths = true;
    config.output.export_cir = true;
    config.output.export_pdp = true;
    config.output.export_aps = true;
    config.output.export_config_snapshot = true;
    config.em_solver.solver_mode = "Precise";
    config.frequency_sweep.enabled = false;
    config.sbr.enabled = false;
    config.sbr.trace_profile = "P2PCompare";
    config.sbr.store_paths = true;
    config.sbr.deterministic_interaction_split = true;
    config.sbr.disable_no_new_hit_early_stop = true;
    config.sbr.max_paths_per_ray = 0;
    config.sbr.max_paths_per_rx = 0;
    config.sbr.enable_dynamic_rx_radius = true;
    config.sbr.ray_tube_angle_rad = 0.0;
    config.sbr.ray_tube_radius_scale = 1.0;
    config.sbr.ray_tube_min_radius_m = 0.5;
    config.sbr.ray_tube_max_radius_m = 3.0;
    config.sbr.enable_path_dedup = true;
    config.sbr.enable_path_similarity_pruning = false;
    config.sbr.path_top_n_per_rx = 0;

    ReadField(root, "run_id", config.app_runtime.run_id);
    if (config.app_runtime.run_id.empty()) {
        result.error_message = "v11 config requires top-level run_id.";
        return result;
    }
    config.experiment.experiment_tag = config.app_runtime.run_id;
    config.experiment.dataset_tag = config.app_runtime.run_id;

    if (!HasKey(root, "scene") || !root.at("scene").is_object()) {
        result.error_message = "v11 config requires scene object.";
        return result;
    }
    const json& scene = root.at("scene");
    ReadField(scene, "obj_file", config.scene_import.source_file);
    ReadField(scene, "material_binding_file", config.scene_import.scene_material_map_file);
    ReadField(scene, "material_database_file", config.material.material_database_file);
    config.scene_import.source_file = ResolvePath(configDir, config.scene_import.source_file);
    config.scene_import.scene_material_map_file = ResolvePath(configDir, config.scene_import.scene_material_map_file);
    config.material.material_database_file = ResolvePath(configDir, config.material.material_database_file);
    config.scene_import.source_format = "obj";
    config.material.missing_material_policy = "strict";
    config.scene_preprocess.enable_scene_cache = false;
    config.scene_preprocess.convex_wedges_only = true;
    config.scene_preprocess.bvh_leaf_size = 16;

    const std::string ext = Lower(std::filesystem::path(config.scene_import.source_file).extension().string());
    if (ext != ".obj") {
        result.error_message = "v11 config currently supports only OBJ scene files.";
        return result;
    }

    std::string antennaFile;
    ReadField(root, "antenna_file", antennaFile);
    if (antennaFile.empty()) {
        result.error_message = "v11 config requires antenna_file.";
        return result;
    }
    antennaFile = ResolvePath(configDir, antennaFile);
    config.v11_antenna_file = antennaFile;

    json antennaRoot;
    if (!LoadJsonFile(antennaFile, antennaRoot, error)) {
        result.error_message = error;
        return result;
    }
    const std::filesystem::path antennaDir = std::filesystem::absolute(antennaFile).parent_path();

    std::vector<TxTarget> txList;
    std::vector<RxTarget> rxList;
    if (!ReadTxList(antennaRoot, antennaDir, txList, result.error_message)) return result;
    if (!ReadRxList(antennaRoot, antennaDir, rxList, result.error_message)) return result;

    if (!HasKey(root, "simulation") || !root.at("simulation").is_object()) {
        result.error_message = "v11 config requires simulation object.";
        return result;
    }
    const json& sim = root.at("simulation");
    bool enableP2P = true;
    bool enableCoverage = false;
    ReadField(sim, "enable_p2p", enableP2P);
    ReadField(sim, "enable_coverage", enableCoverage);
    if (!enableP2P) {
        result.error_message = "v11 config currently requires simulation.enable_p2p=true.";
        return result;
    }
    if (enableCoverage) {
        result.error_message = "coverage interface is reserved but not enabled in current v11 CPU P2P chain.";
        return result;
    }

    if (!HasKey(root, "p2p") || !root.at("p2p").is_object()) {
        result.error_message = "v11 config requires p2p object.";
        return result;
    }
    const json& p2p = root.at("p2p");
    std::vector<std::string> txIds = ReadStringList(p2p, "tx_ids");
    std::vector<std::string> rxIds = ReadStringList(p2p, "rx_ids");
    if (txIds.empty()) txIds = AllTxIds(txList);
    if (rxIds.empty()) rxIds = AllRxIds(rxList);

    ReadField(p2p, "ray_count", config.sbr.ray_count);
    ReadField(p2p, "max_depth", config.sbr.max_ray_depth);
    ReadField(p2p, "max_reflection", config.sbr.max_reflection_count);
    ReadField(p2p, "max_transmission", config.sbr.max_transmission_count);
    ReadField(p2p, "max_diffraction", config.sbr.max_diffraction_count);
    ReadField(p2p, "power_threshold_dB", config.sbr.ray_power_threshold_dB);
    ReadField(p2p, "rx_sphere_radius_m", config.sbr.rx_sphere_radius_m);
    ReadField(p2p, "enable_dynamic_rx_radius", config.sbr.enable_dynamic_rx_radius);
    ReadField(p2p, "ray_tube_angle_rad", config.sbr.ray_tube_angle_rad);
    ReadField(p2p, "ray_tube_radius_scale", config.sbr.ray_tube_radius_scale);
    ReadField(p2p, "ray_tube_min_radius_m", config.sbr.ray_tube_min_radius_m);
    ReadField(p2p, "ray_tube_max_radius_m", config.sbr.ray_tube_max_radius_m);
    ReadField(p2p, "enable_wedge_tube_coupling", config.sbr.enable_wedge_tube_coupling);
    ReadField(p2p, "wedge_tube_radius_scale", config.sbr.wedge_tube_radius_scale);
    ReadField(p2p, "diffraction_rays_per_event", config.sbr.diffraction_rays_per_event);
    ReadField(p2p, "enable_path_dedup", config.sbr.enable_path_dedup);
    ReadField(p2p, "enable_path_similarity_pruning", config.sbr.enable_path_similarity_pruning);
    ReadField(p2p, "path_similarity_length_tol_m", config.sbr.path_similarity_length_tol_m);
    ReadField(p2p, "enable_path_residual_filter", config.sbr.enable_path_residual_filter);
    ReadField(p2p, "path_geometry_residual_tol", config.sbr.path_geometry_residual_tol);
    ReadField(p2p, "reflection_residual_tol_m", config.sbr.reflection_residual_tol_m);
    ReadField(p2p, "snell_residual_tol", config.sbr.snell_residual_tol);
    ReadField(p2p, "keller_residual_tol", config.sbr.keller_residual_tol);
    config.scene_preprocess.enable_wedge_build = config.sbr.max_diffraction_count > 0;

    config.path_search.max_path_depth = config.sbr.max_ray_depth;
    config.path_search.max_reflection_count = config.sbr.max_reflection_count;
    config.path_search.max_transmission_count = config.sbr.max_transmission_count;
    config.path_search.max_diffraction_count = config.sbr.max_diffraction_count;

    if (!HasKey(root, "em") || !root.at("em").is_object()) {
        result.error_message = "v11 config requires em object.";
        return result;
    }
    const json& em = root.at("em");
    if (!HasKey(em, "frequency_list_hz") || !em.at("frequency_list_hz").is_array() || em.at("frequency_list_hz").empty()) {
        result.error_message = "em.frequency_list_hz must contain at least one frequency.";
        return result;
    }
    try {
        config.em_solver.frequency_hz = em.at("frequency_list_hz").at(0).get<double>();
    } catch (...) {
        result.error_message = "em.frequency_list_hz[0] must be numeric.";
        return result;
    }
    ReadField(em, "aps_theta_bins", config.em_solver.aps_theta_bins);
    ReadField(em, "aps_phi_bins", config.em_solver.aps_phi_bins);
    ReadField(em, "compute_meg", config.em_solver.compute_meg);

    std::map<std::string, TxTarget> txById;
    std::map<std::string, RxTarget> rxById;
    for (const TxTarget& tx : txList) txById[tx.id] = tx;
    for (const RxTarget& rx : rxList) rxById[rx.id] = rx;

    for (const std::string& txId : txIds) {
        auto txIt = txById.find(txId);
        if (txIt == txById.end()) {
            result.error_message = "p2p.tx_ids references unknown Tx id: " + txId;
            return result;
        }
        for (const std::string& rxId : rxIds) {
            auto rxIt = rxById.find(rxId);
            if (rxIt == rxById.end()) {
                result.error_message = "p2p.rx_ids references unknown Rx id: " + rxId;
                return result;
            }
            P2PLinkTask task;
            task.tx = txIt->second;
            task.rx = rxIt->second;
            task.id = task.tx.id + "-" + task.rx.id;
            config.v11_p2p_tasks.push_back(task);
        }
    }

    if (config.v11_p2p_tasks.empty()) {
        result.error_message = "v11 config produced no P2P Tx-Rx task.";
        return result;
    }

    const P2PLinkTask& first = config.v11_p2p_tasks.front();
    config.path_search.tx_x = first.tx.x;
    config.path_search.tx_y = first.tx.y;
    config.path_search.tx_z = first.tx.z;
    config.path_search.rx_x = first.rx.x;
    config.path_search.rx_y = first.rx.y;
    config.path_search.rx_z = first.rx.z;
    config.path_search.rx_list.clear();
    for (const P2PLinkTask& task : config.v11_p2p_tasks) {
        config.path_search.rx_list.push_back(task.rx);
    }

    result.config = std::move(config);
    result.succeeded = true;
    return result;
}

} // namespace rt
