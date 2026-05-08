#include "DiffractionExpander.h"
#include "StateSignatureBuilder.h"
#include "../common/math/Vec3.h"
#include <algorithm>
#include <cmath>

namespace rt {

namespace {

constexpr double kGoldenRatio = 1.618033988749895;
constexpr int kMaxGssIter = 32;
constexpr double kGssTol = 1e-6;
constexpr double kKellerTol = 1e-4;
constexpr double kEdgeMargin = 1e-4;

/// <summary>
/// Evaluates the total Fermat path length Tx-diffraction-Rx for a diffraction
/// point parameterised by t along the edge from e1 in direction edgeVec.
/// </summary>
double FermatPathLength(double t, const Point3& tx, const Point3& rx, const Point3& e1, const Vec3& edgeVec)
{
    const double px = e1.x + t * edgeVec.x;
    const double py = e1.y + t * edgeVec.y;
    const double pz = e1.z + t * edgeVec.z;
    const double dx1 = px - tx.x, dy1 = py - tx.y, dz1 = pz - tx.z;
    const double dx2 = px - rx.x, dy2 = py - rx.y, dz2 = pz - rx.z;
    return std::sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1) + std::sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
}

/// <summary>
/// Minimises the Fermat path length along the parameterised edge [0,1] using the
/// Golden Section Search algorithm to locate the optimum diffraction point.
/// </summary>
double GoldenSectionSearch(const Point3& tx, const Point3& rx, const Point3& e1, const Vec3& edgeVec)
{
    double a = 0.0, b = 1.0;
    double c = b - (b - a) / kGoldenRatio;
    double d = a + (b - a) / kGoldenRatio;
    double fc = FermatPathLength(c, tx, rx, e1, edgeVec);
    double fd = FermatPathLength(d, tx, rx, e1, edgeVec);
    for (int i = 0; i < kMaxGssIter; ++i)
    {
        if (fc < fd) { b = d; d = c; fd = fc; c = b - (b - a) / kGoldenRatio; fc = FermatPathLength(c, tx, rx, e1, edgeVec); }
        else         { a = c; c = d; fc = fd; d = a + (b - a) / kGoldenRatio; fd = FermatPathLength(d, tx, rx, e1, edgeVec); }
        if ((b - a) < kGssTol) break;
    }
    return (a + b) * 0.5;
}

/// <summary>
/// UTD Keller-cone verification: checks that the direction cosines of the
/// incident and diffracted rays with respect to the edge direction are equal
/// within tolerance, ensuring the diffracted ray lies on the Keller cone.
/// </summary>
bool VerifyKellerCone(const Point3& tx, const Point3& rx, const Point3& dp, const Vec3& ed)
{
    const double dtx = dp.x - tx.x, dty = dp.y - tx.y, dtz = dp.z - tx.z;
    const double drx = rx.x - dp.x, dry = rx.y - dp.y, drz = rx.z - dp.z;
    const double ltx = std::sqrt(dtx*dtx + dty*dty + dtz*dtz);
    const double lrx = std::sqrt(drx*drx + dry*dry + drz*drz);
    if (ltx <= 0.0 || lrx <= 0.0) return false;
    const double cbTx = std::fabs((dtx*ed.x + dty*ed.y + dtz*ed.z)) / ltx;
    const double cbRx = std::fabs((drx*ed.x + dry*ed.y + drz*ed.z)) / lrx;
    return std::fabs(cbTx - cbRx) <= kKellerTol;
}

/// <summary>
/// Looks up the two endpoint positions of a scene edge by its id; returns false
/// if the id is out of range so the caller can reject the candidate.
/// </summary>
bool LookupEdgeEndpoints(const Scene& scene, int edgeId, Point3& e1, Point3& e2)
{
    if (edgeId < 0 || edgeId >= static_cast<int>(scene.edges.size())) return false;
    e1 = scene.edges[edgeId].start;
    e2 = scene.edges[edgeId].end;
    return true;
}

/// <summary>
/// Simple diffraction-candidate scoring metric: sum of the segment lengths from
/// the state's current point to the diffraction point and from there to Rx.
/// </summary>
double ScoreD(const PathState& st, const Point3& dp, const Point3& rx)
{
    double a = Length(Subtract(dp, st.current_point));
    double b = Length(Subtract(rx, dp));
    return a + b;
}

} // namespace

/// <summary>
/// Expands the current path state by one diffraction interaction. For each
/// candidate wedge, performs Golden Section Search for the Fermat-optimal
/// diffraction point, UTD Keller-cone verification, visibility checks, and
/// builds the successor PathState.
/// </summary>
ExpanderResult ExpandDiffraction(const PathSearchContext& context, const PathState& state)
{
    ExpanderResult result;
    if (!state.allow_diffraction || state.remaining_diffractions <= 0)
    {
        result.failure_reasons.push_back(GeometryValidityReason::OutOfBudget);
        return result;
    }

    WedgeQueryContext wqc;
    wqc.ignored_wedge_id = state.last_hit_wedge_id;
    wqc.recent_face_id = state.last_hit_face_id;
    wqc.recent_wedge_id = state.last_hit_wedge_id;
    const std::vector<WedgeCandidate>& cands = context.scene_query->QueryCandidateWedges(state.current_point, wqc);

    struct DCand { PathState st; double sc; };
    std::vector<DCand> accepted;

    for (const WedgeCandidate& cand : cands)
    {
        GeometryValidityResult cv = IsValidDiffractionCandidate(cand);
        if (!cv.valid) { result.failure_reasons.push_back(cv.reason); continue; }

        // B3: Look up actual edge endpoints
        Point3 e1, e2;
        if (!LookupEdgeEndpoints(*context.scene, cand.source_edge_id, e1, e2))
        {
            result.failure_reasons.push_back(GeometryValidityReason::DegenerateWedge);
            continue;
        }
        Vec3 edgeVec = Subtract(e2, e1);
        if (Length(edgeVec) <= 0.0)
        {
            result.failure_reasons.push_back(GeometryValidityReason::DegenerateWedge);
            continue;
        }
        Vec3 edgeDir = Normalize(edgeVec);

        // B3: Fermat principle - golden section search for exact diffraction point
        double tBest = GoldenSectionSearch(state.current_point, context.rx_point, e1, edgeVec);
        tBest = Clamp(tBest, 0.0, 1.0);

        Point3 dp = MakeVec3(
            e1.x + tBest * edgeVec.x,
            e1.y + tBest * edgeVec.y,
            e1.z + tBest * edgeVec.z);

        // B3: Keller cone verification
        if (!VerifyKellerCone(state.current_point, context.rx_point, dp, edgeDir))
        {
            result.failure_reasons.push_back(GeometryValidityReason::CandidateRejectedByControl);
            continue;
        }

        // Visibility using exact diffraction point
        VisibilityQueryContext vc;
        if (!context.scene_query->IsVisible(state.current_point, dp, vc) ||
            !context.scene_query->IsVisible(dp, context.rx_point, vc))
        {
            result.failure_reasons.push_back(GeometryValidityReason::VisibilityBlocked);
            continue;
        }

        PathState ns = state;
        ns.current_point = dp;
        ns.current_direction = SafeNormalize(Subtract(context.rx_point, dp));
        ns.last_interaction_type = InteractionType::Diffraction;
        ns.last_hit_wedge_id = cand.wedge_id;
        double seg = Length(Subtract(dp, state.current_point));
        ns.accumulated_length += seg;
        ns.path_depth += 1;
        ns.interaction_count += 1;
        ns.remaining_total_expansions -= 1;
        ns.remaining_diffractions -= 1;
        ns.has_diffraction = true;
        ns.mixed_path_enabled = false;
        ns.clipped_by_control_rules = false;

        if (state.last_interaction_type != InteractionType::None &&
            state.last_interaction_type != InteractionType::Tx &&
            state.last_interaction_type != InteractionType::Diffraction)
        { ns.mechanism_switch_count += 1; }
        ns.consecutive_same_interaction_count = (state.last_interaction_type == InteractionType::Diffraction)
            ? (state.consecutive_same_interaction_count + 1) : 0;

        PathNode nd;
        nd.interaction_type = InteractionType::Diffraction;
        nd.wedge_id = cand.wedge_id;
        nd.point = dp;
        nd.direction = ns.current_direction;
        nd.incident_direction = Normalize(Subtract(dp, state.current_point)); // C3-A: incoming direction for UTD phi'
        nd.segment_length_from_previous = seg;
        nd.valid = true;
        ns.traversed_nodes.push_back(nd);
        ns.state_signature = BuildStateSignature(ns, *context.config);
        ns.valid = true;

        GeometryValidityResult ev = IsValidExpandedState(context, ns);
        if (!ev.valid) { result.failure_reasons.push_back(ev.reason); continue; }

        DCand dc; dc.st = ns; dc.sc = ScoreD(state, dp, context.rx_point);
        accepted.push_back(dc);
    }

    std::sort(accepted.begin(), accepted.end(),
        [](const DCand& a, const DCand& b) { return a.sc < b.sc; });
    const std::size_t keep = std::min<std::size_t>(accepted.size(), 8U);
    for (std::size_t i = 0; i < keep; ++i)
        result.next_states.push_back(accepted[i].st);

    if (result.next_states.empty() && result.failure_reasons.empty())
        result.failure_reasons.push_back(GeometryValidityReason::NoCandidate);
    return result;
}

} // namespace rt
