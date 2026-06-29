#pragma once

#include "../core/antenna/AntennaPattern.h"
#include "../core/antenna/AntennaModel.h"
#include "../core/common/config/AppConfig.h"
#include "../core/common/math/MathConstants.h"
#include "../core/em/ComputeMEG.h"
#include "../core/em/BuildChannelStatistics.h"
#include "../core/em/BuildAPS.h"
#include "../core/em/BuildXPRStatistics.h"
#include "../core/em/FieldAccumulator.h"
#include "../core/em/FinalizeAtReceiver.h"
#include "../core/em/InitializeTxField.h"
#include "../core/em/ApplyReflectionInteraction.h"
#include "../core/em/ApplyDiffractionInteraction.h"
#include "../core/em/FresnelInterface.h"
#include "../core/em/BuildBroadbandCFR.h"
#include "../core/search/SbrPathRefiner.h"
#include "../core/search/SbrDiffractionTracer.h"
#include "../preprocess/build/EdgeBuilder.h"
#include "../preprocess/build/WedgeBuilder.h"

#include <cmath>
#include <iostream>
#include <string>

namespace rt {

inline int RunV11Validation()
{
    int passed = 0;
    int failed = 0;
    auto check = [&](const std::string& name, bool ok) {
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
        ok ? ++passed : ++failed;
    };

    {
        AppConfig config;
        config.scene_preprocess.enable_wedge_build = true;
        Scene scene;
        scene.vertices = {MakeVec3(0.0, 0.0, 0.0), MakeVec3(1.0, 0.0, 0.0),
                          MakeVec3(1.0, 1.0, 0.0), MakeVec3(0.0, 1.0, 0.0)};
        Face a; a.face_id = 0; a.vertex_index0 = 0; a.vertex_index1 = 1; a.vertex_index2 = 2;
        a.normal = MakeVec3(0.0, 0.0, 1.0); a.diffraction_candidate_enabled = true;
        Face b; b.face_id = 1; b.vertex_index0 = 0; b.vertex_index1 = 2; b.vertex_index2 = 3;
        // Deliberately mimic a bad smooth/shading normal on a coplanar triangle.
        b.normal = Normalize(MakeVec3(0.12, 0.0, 0.99)); b.diffraction_candidate_enabled = true;
        scene.faces = {a, b};
        BuildSceneEdges(config, scene);
        BuildSceneWedges(config, scene);
        bool sharedEdgeCoplanar = false;
        for (const Edge& edge : scene.edges)
            if (edge.face_id0 >= 0 && edge.face_id1 >= 0) sharedEdgeCoplanar = edge.is_coplanar;
        check("coplanar mesh seam cannot become a UTD wedge",
              sharedEdgeCoplanar && scene.wedges.empty());
    }

    {
        Scene scene;
        Face xFace; xFace.normal = MakeVec3(1.0, 0.0, 0.0);
        Face yFace; yFace.normal = MakeVec3(0.0, 1.0, 0.0);
        scene.faces = {xFace, yFace};
        Wedge wedge; wedge.wedge_angle_deg = 270.0; wedge.positive_face_id = 0;
        wedge.negative_face_id = 1; wedge.center_point = MakeVec3(0.0, 0.0, 0.0);
        check("UTD exterior-side test follows outward normals",
              SbrPointOnLargeWedgeSide(scene, wedge, MakeVec3(1.0, 1.0, 0.0)) &&
              !SbrPointOnLargeWedgeSide(scene, wedge, MakeVec3(-1.0, -1.0, 0.0)));
    }

    {
        AppConfig config;
        config.scene_preprocess.enable_wedge_build = true;
        config.scene_preprocess.convex_wedges_only = true;
        Scene cube;
        cube.vertices = {
            MakeVec3(-1.0, -1.0, -1.0), MakeVec3( 1.0, -1.0, -1.0),
            MakeVec3( 1.0,  1.0, -1.0), MakeVec3(-1.0,  1.0, -1.0),
            MakeVec3(-1.0, -1.0,  1.0), MakeVec3( 1.0, -1.0,  1.0),
            MakeVec3( 1.0,  1.0,  1.0), MakeVec3(-1.0,  1.0,  1.0)};
        auto addFace = [&](int a, int b, int c) {
            Face face;
            face.face_id = static_cast<int>(cube.faces.size());
            face.vertex_index0 = a; face.vertex_index1 = b; face.vertex_index2 = c;
            face.normal = Normalize(Cross(Subtract(cube.vertices[b], cube.vertices[a]),
                                          Subtract(cube.vertices[c], cube.vertices[a])));
            face.diffraction_candidate_enabled = true;
            cube.faces.push_back(face);
        };
        addFace(0, 2, 1); addFace(0, 3, 2); // -Z
        addFace(4, 5, 6); addFace(4, 6, 7); // +Z
        addFace(0, 1, 5); addFace(0, 5, 4); // -Y
        addFace(3, 7, 6); addFace(3, 6, 2); // +Y
        addFace(0, 4, 7); addFace(0, 7, 3); // -X
        addFace(1, 2, 6); addFace(1, 6, 5); // +X
        BuildSceneEdges(config, cube);
        BuildSceneWedges(config, cube);
        bool allConvex = cube.wedges.size() == 12;
        bool interiorRejected = allConvex;
        for (const Wedge& wedge : cube.wedges) {
            allConvex = allConvex && wedge.convexity == WedgeConvexity::Convex &&
                        std::fabs(wedge.wedge_angle_deg - 270.0) < 1.0e-12;
            interiorRejected = interiorRejected &&
                !SbrPointOnLargeWedgeSide(cube, wedge, MakeVec3(0.0, 0.0, 0.0));
        }
        check("closed cube exposes exactly twelve convex UTD wedges", allConvex);
        check("cube-interior endpoint rejects every exterior UTD wedge", interiorRejected);
    }

    {
        AppConfig config;
        config.scene_preprocess.enable_wedge_build = true;
        config.scene_preprocess.convex_wedges_only = true;
        Scene concave;
        concave.vertices = {MakeVec3(0.0, 0.0, 0.0), MakeVec3(0.0, 0.0, 1.0),
                            MakeVec3(0.0, -1.0, 0.0), MakeVec3(-1.0, 0.0, 0.0)};
        Face a; a.face_id = 0; a.vertex_index0 = 0; a.vertex_index1 = 1; a.vertex_index2 = 2;
        a.normal = MakeVec3(-1.0, 0.0, 0.0); a.diffraction_candidate_enabled = true;
        Face b; b.face_id = 1; b.vertex_index0 = 1; b.vertex_index1 = 0; b.vertex_index2 = 3;
        b.normal = MakeVec3(0.0, -1.0, 0.0); b.diffraction_candidate_enabled = true;
        concave.faces = {a, b};
        BuildSceneEdges(config, concave);
        WedgeConvexity classification = WedgeConvexity::Unknown;
        for (const Edge& edge : concave.edges) {
            if (edge.face_id0 >= 0 && edge.face_id1 >= 0)
                classification = ClassifySharedEdgeConvexity(concave, edge);
        }
        BuildSceneWedges(config, concave);
        check("solid-concave shared edge is classified and filtered",
              classification == WedgeConvexity::Concave && concave.wedges.empty());
    }

    {
        AppConfig config;
        config.scene_preprocess.enable_wedge_build = true;
        Scene wedge60;
        const double c = std::sqrt(3.0) * 0.5;
        wedge60.vertices = {MakeVec3(0.0, 0.0, 0.0), MakeVec3(0.0, 0.0, 1.0),
                            MakeVec3(c, -0.5, 0.0), MakeVec3(c, 0.5, 0.0)};
        Face lower; lower.face_id = 0; lower.vertex_index0 = 1;
        lower.vertex_index1 = 0; lower.vertex_index2 = 2;
        lower.normal = Normalize(Cross(Subtract(wedge60.vertices[0], wedge60.vertices[1]),
                                       Subtract(wedge60.vertices[2], wedge60.vertices[1])));
        lower.surface_material_name = "Concrete";
        lower.diffraction_candidate_enabled = true;
        Face upper; upper.face_id = 1; upper.vertex_index0 = 0;
        upper.vertex_index1 = 1; upper.vertex_index2 = 3;
        upper.normal = Normalize(Cross(Subtract(wedge60.vertices[1], wedge60.vertices[0]),
                                       Subtract(wedge60.vertices[3], wedge60.vertices[0])));
        upper.surface_material_name = "Metal";
        upper.diffraction_candidate_enabled = true;
        wedge60.faces = {lower, upper};
        BuildSceneEdges(config, wedge60);
        BuildSceneWedges(config, wedge60);
        check("60-degree solid wedge maps to 300-degree UTD exterior opening",
              wedge60.wedges.size() == 1 &&
              std::fabs(wedge60.wedges[0].wedge_angle_deg - 300.0) < 1.0e-12);
        check("wedge preserves both exposed surface materials",
              wedge60.wedges.size() == 1 &&
              !wedge60.wedges[0].positive_material_name.empty() &&
              !wedge60.wedges[0].negative_material_name.empty() &&
              wedge60.wedges[0].positive_material_name != wedge60.wedges[0].negative_material_name);
    }

    {
        EMPathResultSet paths;
        EMPathResult a; a.valid = true; a.amplitude_real = 0.3; a.amplitude_imag = 0.4;
        a.power_linear = 0.25; a.delay_s = 7.0e-9;
        paths.results = {a};
        FrequencySweepConfig sweep; sweep.enabled = true; sweep.center_hz = 3.0e9;
        sweep.bandwidth_hz = 2.0e8; sweep.point_count = 3; sweep.spacing = "linear";
        ChannelObservationConfig observation; observation.export_observed_cir_ifft = false;
        const auto broadband = BuildBroadbandCFR_FixedGain(paths, sweep, observation);
        check("fixed-gain CFR preserves center-frequency complex path", broadband.valid &&
              std::fabs(broadband.cfr[1].H_real - 0.3) < 1.0e-15 &&
              std::fabs(broadband.cfr[1].H_imag - 0.4) < 1.0e-15);
    }

    {
        check("UTD spherical spreading compensation",
              std::fabs(UtdSphericalSpreadingCompensation(10.0, 10.0) - std::sqrt(0.2)) < 1.0e-15 &&
              UtdSphericalSpreadingCompensation(0.0, 10.0) == 0.0);
        check("UTD exterior wedge index", std::fabs(UtdWedgeIndexFromExteriorAngle(270.0) - 1.5) < 1.0e-15);
        const Complex f1 = EvaluateUtdTransition(1.0);
        const Complex f10 = EvaluateUtdTransition(10.0);
        check("UTD transition function matches Fresnel reference values",
              std::fabs(f1.re - 0.8095255) < 1.0e-6 &&
              std::fabs(f1.im - 0.2321994) < 1.0e-6 &&
              std::fabs(f10.re - 0.9930411) < 1.0e-6 &&
              std::fabs(f10.im - 0.0483515) < 1.0e-6);
    }

    {
        MaterialDatabase db;
        const bool dbOk = db.LoadFromCsv("test/materials/L1_materials.csv");
        const double frequency = 3.5e9;
        const Vec3 edge = MakeVec3(0.0, 0.0, 1.0);
        const Vec3 kIn = Normalize(MakeVec3(-1.0, -1.0, 0.0));
        const Vec3 kOut = Normalize(MakeVec3(-1.0, 1.0, 0.0));

        Scene baseScene;
        Face face0; face0.face_id = 0; face0.normal = MakeVec3(0.0, 1.0, 0.0);
        Face faceN; faceN.face_id = 1; faceN.normal = MakeVec3(-1.0, 0.0, 0.0);
        baseScene.faces = {face0, faceN};
        Wedge wedge; wedge.wedge_id = 0; wedge.positive_face_id = 0; wedge.zero_face_id = 0;
        wedge.negative_face_id = 1; wedge.direction = edge; wedge.wedge_angle_deg = 270.0;
        baseScene.wedges = {wedge};

        GeometricPath path;
        path.valid = true;
        PathNode tx; tx.valid = true; tx.interaction_type = InteractionType::Tx;
        tx.point = Scale(kIn, -5.0);
        PathNode diffraction; diffraction.valid = true; diffraction.interaction_type = InteractionType::Diffraction;
        diffraction.point = MakeVec3(0.0, 0.0, 0.0); diffraction.wedge_id = 0;
        diffraction.incident_direction = kIn; diffraction.direction = kOut;
        diffraction.segment_length_from_previous = 5.0; diffraction.diffraction_diag.s2 = 5.0;
        PathNode rx; rx.valid = true; rx.interaction_type = InteractionType::Rx;
        rx.point = Scale(kOut, 5.0); rx.segment_length_from_previous = 5.0;
        path.nodes = {tx, diffraction, rx};

        auto solve = [&](const std::string& material0, const std::string& materialN,
                         const MaterialDatabase* materialDb) {
            Scene scene = baseScene;
            scene.wedges[0].positive_material_name = material0;
            scene.wedges[0].negative_material_name = materialN;
            FieldAccumulator field;
            field.valid = true; field.vector_field_valid = true;
            field.frequency_hz = frequency; field.wavelength_m = kC0 / frequency;
            const Vec3 incidentPolarization = Normalize(Cross(kIn, edge));
            field.electric_field_world = ScaleComplexVec3(incidentPolarization, Complex(1.0, 0.0));
            EMSolverInput input; input.scene = &scene; input.path = &path; input.material_db = materialDb;
            if (!ApplyDiffractionInteraction(field, path.nodes[1], input)) field.valid = false;
            return field;
        };

        const FieldAccumulator metal = solve("Metal", "Metal", &db);
        const FieldAccumulator concrete = solve("Concrete", "Concrete", &db);
        const FieldAccumulator asymmetric = solve("Concrete", "Metal", &db);
        const FieldAccumulator unresolved = solve("MissingMaterial", "Concrete", &db);

        check("finite-conductivity UTD resolves both wedge materials",
              dbOk && metal.last_diffraction.valid &&
              metal.last_diffraction.face0_material_resolved &&
              metal.last_diffraction.facen_material_resolved &&
              metal.last_diffraction.model == "finite_conductivity_utd");
        check("high-conductivity wedge approaches PEC UTD limit",
              (metal.last_diffraction.face0_reflection_te - Complex(-1.0, 0.0)).Norm() < 5.0e-3 &&
              (metal.last_diffraction.face0_reflection_tm - Complex(1.0, 0.0)).Norm() < 5.0e-3 &&
              (metal.last_diffraction.facen_reflection_te - Complex(-1.0, 0.0)).Norm() < 5.0e-3 &&
              (metal.last_diffraction.facen_reflection_tm - Complex(1.0, 0.0)).Norm() < 5.0e-3);
        check("dielectric wedge changes coherent UTD field",
              concrete.last_diffraction.valid &&
              Norm(Subtract(concrete.electric_field_world, metal.electric_field_world)) > 1.0e-6);
        check("two wedge-face materials independently affect UTD Jones matrix",
              asymmetric.last_diffraction.valid &&
              Norm(Subtract(asymmetric.electric_field_world, concrete.electric_field_world)) > 1.0e-7);
        check("unresolved wedge material rejects EM path instead of assuming PEC",
              !unresolved.valid && !unresolved.last_diffraction.valid);
        check("diffracted Jones field remains transverse",
              ComplexDot(concrete.electric_field_world, kOut).Norm() < 1.0e-12);
    }

    {
        MaterialProps vacuum; vacuum.epsilon_r = 1.0; vacuum.sigma = 0.0;
        MaterialProps dielectric; dielectric.epsilon_r = 5.24; dielectric.sigma = 0.0;
        const double cosI = 0.8;
        const auto f = EvaluateFresnelInterface(vacuum, dielectric, cosI, 2.4e9);
        const double fluxScale = std::sqrt((f.n2 * f.cos_t).re / (f.n1.re * cosI));
        const Complex ts = f.transmission_te * fluxScale;
        const Complex tp = f.transmission_tm * fluxScale;
        check("lossless Fresnel TE/TM conserve interface power",
              std::fabs(f.reflection_te.NormSq() + ts.NormSq() - 1.0) < 1.0e-12 &&
              std::fabs(f.reflection_tm.NormSq() + tp.NormSq() - 1.0) < 1.0e-12);

        MaterialDatabase db;
        const bool dbOk = db.LoadFromCsv("test/materials/L1_materials.csv");
        Scene scene;
        Face face; face.face_id = 0; face.normal = MakeVec3(-1.0, 0.0, 0.0);
        face.front_material_name = "Vacuum"; face.back_material_name = "Concrete";
        scene.faces = {face};
        const Vec3 kIn = Normalize(MakeVec3(0.8, 0.6, 0.0));
        const Vec3 kOut = Normalize(Reflect(kIn, face.normal));
        const Vec3 te = Normalize(Cross(kIn, face.normal));
        const Vec3 tmIn = Normalize(Cross(te, kIn));
        PathNode node; node.valid = true; node.face_id = 0; node.surface_normal = face.normal;
        node.incident_direction = kIn; node.direction = kOut;
        FieldAccumulator field; field.valid = true; field.vector_field_valid = true;
        field.frequency_hz = 2.4e9;
        field.electric_field_world = ScaleComplexVec3(tmIn, Complex(1.0, 0.0));
        EMSolverInput input; input.scene = &scene; input.material_db = &db;
        const bool reflected = ApplyReflectionInteraction(field, node, input);
        check("reflected TM field uses outgoing transverse basis", dbOk && reflected &&
              ComplexDot(field.electric_field_world, kOut).Norm() < 1.0e-12);
    }

    {
        Scene scene;
        scene.vertices = {
            MakeVec3(0.0, -2.0, -1.0), MakeVec3(0.0, 2.0, -1.0),
            MakeVec3(0.0, 2.0, 1.0), MakeVec3(0.0, -2.0, 1.0)
        };
        Face f0; f0.face_id = 0; f0.object_id = 7; f0.vertex_index0 = 0;
        f0.vertex_index1 = 1; f0.vertex_index2 = 2; f0.normal = MakeVec3(-1.0, 0.0, 0.0);
        f0.centroid = MakeVec3(0.0, 2.0 / 3.0, -1.0 / 3.0);
        Face f1 = f0; f1.face_id = 1; f1.vertex_index0 = 0; f1.vertex_index1 = 2;
        f1.vertex_index2 = 3; f1.centroid = MakeVec3(0.0, -2.0 / 3.0, 1.0 / 3.0);
        scene.faces = {f0, f1};
        Edge shared; shared.face_id0 = 0; shared.face_id1 = 1;
        shared.vertex_index0 = 0; shared.vertex_index1 = 2;
        scene.edges = {shared};

        PathNode tx; tx.interaction_type = InteractionType::Tx;
        tx.point = MakeVec3(-1.0, -1.0, 0.0); tx.valid = true;
        PathNode reflection; reflection.interaction_type = InteractionType::Reflection;
        reflection.point = MakeVec3(0.0, 0.25, 0.0); reflection.face_id = 0;
        reflection.object_id = 7; reflection.surface_normal = f0.normal; reflection.valid = true;
        PathNode rx; rx.interaction_type = InteractionType::Rx;
        rx.point = MakeVec3(-1.0, 1.0, 0.0); rx.valid = true;
        GeometricPath candidate; candidate.valid = true; candidate.nodes = {tx, reflection, rx};
        const auto patches = BuildSbrSurfacePatchIds(scene);
        std::string reason;
        const bool refined = RefineSbrPathGeometry(candidate, scene, nullptr, nullptr, 2.4e9, patches, &reason);
        check("coplanar triangles form one physical patch", patches.size() == 2 && patches[0] == patches[1]);
        check("image refinement recovers exact specular point", refined && candidate.geometry_refined &&
              Length(Subtract(candidate.nodes[1].point, MakeVec3(0.0, 0.0, 0.0))) < 1.0e-10 &&
              candidate.max_reflection_residual < 1.0e-10);
    }

    {
        AntennaPattern pattern;
        pattern.Ntheta = 2; pattern.Nphi = 3; pattern.loaded = true;
        pattern.thetaDeg = {0.0, 180.0};
        pattern.phiDeg = {10.0, 120.0, 250.0};
        pattern.gainDBi = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0};
        const double at359 = pattern.QueryGainDBi(0.0, 359.0);
        const double atMinus1 = pattern.QueryGainDBi(0.0, -1.0);
        check("phi periodic interpolation", std::fabs(at359 - atMinus1) < 1e-12);
    }

    {
        AntennaPattern pattern;
        const bool gainOk = pattern.LoadCsv("configs/antennas/patch_gain.csv");
        const bool polOk = pattern.LoadPolarizationCsv("configs/antennas/patch_jones.csv");
        double a, b, c, d;
        pattern.QueryPolarization(30.0, 90.0, a, b, c, d);
        const double norm = std::sqrt(a*a + b*b + c*c + d*d);
        check("six-column Jones CSV", gainOk && polOk && pattern.polarization_loaded);
        check("interpolated Jones normalization", std::fabs(norm - 1.0) < 1e-12);
    }

    {
        AntennaPattern pattern;
        const bool ok = pattern.LoadPolarizationCsv("test/materials/dipole_polarization.csv");
        check("seven-column Jones CSV", ok && pattern.loaded && pattern.polarization_loaded);
    }

    {
        const Vec3 forward = MakeVec3(1.0, 0.0, 0.0);
        const Vec3 right = MakeVec3(0.0, -1.0, 0.0);
        const Vec3 up = MakeVec3(0.0, 0.0, 1.0);
        Vec3 co0, cross0, co90, cross90;
        AntennaLudwig3BasisToWorld(forward, right, up, 0.0, 0.0, co0, cross0);
        AntennaLudwig3BasisToWorld(forward, right, up, 0.0, kPi/2.0, co90, cross90);
        check("Ludwig-3 boresight co basis", Length(Subtract(co0, up)) < 1e-12 && Length(Subtract(co90, up)) < 1e-12);
        check("Ludwig-3 boresight cross basis", Length(Subtract(cross0, right)) < 1e-12 && Length(Subtract(cross90, right)) < 1e-12);
    }

    {
        AppConfig config;
        config.em_solver.frequency_hz = 2.4e9;
        config.sbr.tx_power_dBm = 0.0;
        GeometricPath path;
        path.valid = true;
        path.sampling_weight = 0.25;
        PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point = MakeVec3(0.0, 0.0, 0.0); tx.valid = true;
        PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point = MakeVec3(1.0, 0.0, 0.0); rx.valid = true;
        path.nodes = {tx, rx};
        EMSolverInput input; input.config = &config; input.path = &path; input.tx_power_dBm = 0.0;
        FieldAccumulator field;
        const bool ok = InitializeTxField(input, field);
        check("SBR support does not scale deterministic Tx power", ok &&
              std::fabs(field.power_linear - 0.001) < 1e-15 &&
              std::fabs(field.tx_power_w - 0.001) < 1e-15);
    }

    {
        APSResult aps;
        aps.has_2d_grid = true; aps.n_theta = 2; aps.n_phi = 2;
        aps.theta_min_deg = 0.0; aps.theta_step_deg = 90.0;
        aps.phi_min_deg = 0.0; aps.phi_step_deg = 180.0;
        aps.power_grid_linear.assign(4, 1.0);
        aps.incident_power_grid_linear.assign(4, 1.0);

        AntennaModel antenna;
        antenna.forward = MakeVec3(1.0, 0.0, 0.0);
        antenna.right = MakeVec3(0.0, -1.0, 0.0);
        antenna.up = MakeVec3(0.0, 0.0, 1.0);
        antenna.pattern.loaded = true;
        antenna.pattern.Ntheta = 2; antenna.pattern.Nphi = 2;
        antenna.pattern.thetaDeg = {0.0, 180.0};
        antenna.pattern.phiDeg = {0.0, 180.0};
        antenna.pattern.gainDBi.assign(4, 3.0);
        const auto meg = ComputeMEG(aps, &antenna);
        check("constant 3 dBi MEG", std::fabs(meg.second - 3.0) < 1e-12);
    }

    {
        EMPathResultSet paths;
        EMPathResult rejected;
        rejected.valid = true;
        rejected.power_linear = 0.0;
        rejected.incident_power_linear = 1.0;
        rejected.aoa_theta_deg = 90.0;
        rejected.aoa_phi_deg = 45.0;
        paths.results = {rejected};
        AppConfig config;
        config.em_solver.aps_theta_bins = 18;
        config.em_solver.aps_phi_bins = 36;
        const auto aps = BuildAPS(paths, config);
        double incidentSum = 0.0, observedSum = 0.0;
        for (double value : aps.incident_power_grid_linear) incidentSum += value;
        for (double value : aps.power_grid_linear) observedSum += value;
        check("incident APS retains Rx-rejected paths", std::fabs(incidentSum - 1.0) < 1e-15 &&
              observedSum == 0.0);
    }

    {
        EMPathResultSet paths;
        EMPathResult los;
        los.valid = true; los.is_los = true; los.power_linear = 0.75; los.delay_s = 1.0e-9;
        EMPathResult nlos;
        nlos.valid = true; nlos.is_los = false; nlos.power_linear = 0.25; nlos.delay_s = 3.0e-9;
        paths.results = {los, nlos};
        const auto stats = BuildChannelStatistics(paths);
        check("LOS/NLOS statistics partition", std::fabs(stats.los_power_linear - 0.75) < 1e-15 &&
              std::fabs(stats.nlos_power_linear - 0.25) < 1e-15 &&
              std::fabs(stats.k_factor_dB - 10.0 * std::log10(3.0)) < 1e-12);
        check("power-weighted RMS delay spread", std::fabs(stats.power_weighted_mean_delay_s - 1.5e-9) < 1e-20 &&
              std::fabs(stats.rms_delay_spread_s - std::sqrt(0.75) * 1e-9) < 1e-20 &&
              stats.effective_path_count == 2);
    }

    {
        EMPathResultSet paths;
        EMPathResult pureCo, finite, pureCross;
        pureCo.valid = true; pureCo.power_linear = 1.0;
        pureCo.co_pol_power_linear = 1.0; pureCo.xpr_dB = 300.0;
        finite.valid = true; finite.power_linear = 2.0;
        finite.co_pol_power_linear = 10.0; finite.cross_pol_power_linear = 1.0; finite.xpr_dB = 10.0;
        pureCross.valid = true; pureCross.power_linear = 1.0;
        pureCross.cross_pol_power_linear = 1.0; pureCross.xpr_dB = -300.0;
        paths.results = {pureCo, finite, pureCross};
        const auto stats = BuildXPRStatistics(paths);
        check("XPR censoring and aggregate", stats.valid_path_count == 3 &&
              stats.pure_co_path_count == 1 && stats.pure_cross_path_count == 1 &&
              std::fabs(stats.median_dB - 10.0) < 1e-12 &&
              std::fabs(stats.aggregate_xpr_dB - 10.0 * std::log10(5.5)) < 1e-12);
    }

    {
        FieldAccumulator field;
        field.valid = true; field.vector_field_valid = true;
        field.total_length_m = 1.0; field.delay_s = 1.0 / kC0;
        field.wavelength_m = 0.1;
        field.electric_field_world = ComplexVec3(Complex(0.0, 0.0), Complex(1.0, 0.0), Complex(0.0, 0.0));
        GeometricPath path;
        path.valid = true;
        PathNode tx; tx.valid = true; tx.interaction_type = InteractionType::Tx; tx.point = MakeVec3(0.0, 0.0, 0.0);
        PathNode rx; rx.valid = true; rx.interaction_type = InteractionType::Rx; rx.point = MakeVec3(1.0, 0.0, 0.0);
        path.nodes = {tx, rx};
        AntennaModel antenna;
        antenna.forward = MakeVec3(-1.0, 0.0, 0.0);
        antenna.right = MakeVec3(0.0, 0.0, -1.0);
        antenna.up = MakeVec3(0.0, 1.0, 0.0);
        antenna.polarization_vector = MakeVec3(0.0, 1.0, 0.0);
        EMSolverInput input; input.path = &path; input.rx_antenna = &antenna;
        const auto matched = FinalizeAtReceiver(field, path, input);
        antenna.polarization_vector = MakeVec3(0.0, 0.0, 1.0);
        const auto orthogonal = FinalizeAtReceiver(field, path, input);
        const double expected = std::pow(0.1 / (4.0 * kPi), 2.0);
        check("Rx polarization matched/orthogonal limits", matched.valid && orthogonal.valid &&
              std::fabs(matched.power_linear - expected) < 1e-15 && orthogonal.power_linear < 1e-30);
    }

    std::cout << "V11 validation: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 2;
}

} // namespace rt
