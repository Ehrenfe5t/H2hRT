#include "SbrEngine.h"
#include "../common/math/Vec3.h"
#include "../common/math/MathConstants.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace rt {

namespace {

// Fibonacci sphere: N uniformly distributed directions on unit sphere
/// <summary>
/// Generates N approximately uniformly distributed unit-sphere directions
/// using the Fibonacci (golden-angle) lattice for quasi-Monte Carlo ray launching.
/// </summary>
std::vector<Vec3> GenerateFibonacciRays(int N)
{
    std::vector<Vec3> rays;
    rays.reserve(N);
    const double phi = 3.14159265358979323846 * (3.0 - std::sqrt(5.0)); // golden angle
    for (int i = 0; i < N; ++i)
    {
        double y = 1.0 - (i / double(N - 1)) * 2.0; // [-1, 1]
        double radius = std::sqrt(1.0 - y * y);
        double theta = phi * i;
        rays.push_back(MakeVec3(std::cos(theta) * radius, y, std::sin(theta) * radius));
    }
    return rays;
}

// Point-to-segment minimum distance squared
/// <summary>
/// Returns the squared shortest Euclidean distance from point p to the line
/// segment ab, using clamped barycentric projection.
/// </summary>
double PointToSegmentDistSq(const Point3& p, const Point3& a, const Point3& b)
{
    double abx = b.x - a.x, aby = b.y - a.y, abz = b.z - a.z;
    double apx = p.x - a.x, apy = p.y - a.y, apz = p.z - a.z;
    double ab2 = abx*abx + aby*aby + abz*abz;
    if (ab2 <= 0.0) return apx*apx + apy*apy + apz*apz;
    double t = (apx*abx + apy*aby + apz*abz) / ab2;
    if (t <= 0.0) return apx*apx + apy*apy + apz*apz;
    if (t >= 1.0) {
        double dx = p.x - b.x, dy = p.y - b.y, dz = p.z - b.z;
        return dx*dx + dy*dy + dz*dz;
    }
    double cx = a.x + t*abx, cy = a.y + t*aby, cz = a.z + t*abz;
    double dx = p.x - cx, dy = p.y - cy, dz = p.z - cz;
    return dx*dx + dy*dy + dz*dz;
}

// Check if ray segment passes through Rx reception sphere
/// <summary>
/// Tests whether the ray segment [segStart, segEnd] passes within the reception
/// sphere of radius sphereRadius centred at rxPos (SBR reception-sphere heuristic).
/// </summary>
bool HitsRxSphere(const Point3& segStart, const Point3& segEnd,
                  const Point3& rxPos, double sphereRadius)
{
    double d2 = PointToSegmentDistSq(rxPos, segStart, segEnd);
    return d2 <= sphereRadius * sphereRadius;
}

// Simple LOS check between two points using SceneQuery
/// <summary>
/// Simple line-of-sight visibility test between two points via the scene
/// acceleration structure.
/// </summary>
bool IsLosClear(const Point3& a, const Point3& b, const SceneQuery& sq)
{
    VisibilityQueryContext vc;
    return sq.IsVisible(a, b, vc);
}

} // namespace

/// <summary>
/// Main SBR (Shooting-and-Bouncing-Rays) entry point. Launches N Fibonacci rays
/// from Tx, iteratively reflects at each hit, and records Rx reception-sphere
/// intersections as geometric paths.
/// </summary>
SbrCoverageResult SbrEngine::Run(const SbrContext& context) const
{
    SbrCoverageResult result;
    if (!context.config || !context.scene || !context.scene_query)
    {
        result.trace_lines.push_back("SbrEngine: incomplete context");
        return result;
    }

    const auto& cfg = context.config->sbr;
    const int N = cfg.ray_count;
    const int maxDepth = cfg.max_ray_depth;
    const double pwrThreshold = cfg.ray_power_threshold_linear;
    const double freqHz = context.config->em_solver.frequency_hz;

    // Reception sphere radius factor
    const double alpha = std::sqrt(4.0 * kPi / N);
    const double sphereFactor = cfg.rx_sphere_radius_factor * alpha / std::sqrt(3.0);

    // Init Rx records
    result.rx_records.resize(context.rx_grid.size());
    for (size_t i = 0; i < context.rx_grid.size(); ++i) {
        result.rx_records[i].rx_position = context.rx_grid[i];
        result.rx_records[i].rx_index = static_cast<int>(i);
    }

    // Generate rays
    std::vector<Vec3> rayDirs = GenerateFibonacciRays(N);
    result.total_rays = N;

    std::ostringstream oss;
    oss << "SbrEngine: launching " << N << " rays, maxDepth=" << maxDepth
        << ", rxCount=" << context.rx_grid.size();
    result.trace_lines.push_back(oss.str());

    // For each ray, trace forward
    for (int ri = 0; ri < N; ++ri)
    {
        Point3 curPt = context.tx_point;
        Vec3 curDir = rayDirs[ri];
        double curPwr = 1.0;
        int depth = 0;

        while (depth <= maxDepth && curPwr > pwrThreshold)
        {
            // Cast ray into scene
            Ray ray;
            ray.origin = curPt;
            ray.direction = curDir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance = context.config->numeric_tolerance.self_hit_ignore_distance;

            FaceHit hit = context.scene_query->QueryClosestFaceHit(ray, fqc);

            // Determine segment end point
            Point3 segEnd;
            double segLength;
            if (hit.hit)
            {
                segEnd = hit.position;
                segLength = hit.distance;
            }
            else
            {
                // Ray escapes to infinity; use a long segment for Rx check
                segEnd = Add(curPt, Scale(curDir, 1e6));
                segLength = 1e6;
            }

            // Check Rx reception: does this segment pass near any Rx?
            double totalDist = Length(Subtract(curPt, context.tx_point));
            double sphereR = sphereFactor * (totalDist + segLength * 0.5);

            for (size_t rxi = 0; rxi < context.rx_grid.size(); ++rxi)
            {
                if (HitsRxSphere(curPt, segEnd, context.rx_grid[rxi], sphereR))
                {
                    // Record a simple LOS-like path for this Rx
                    GeometricPath path;
                    path.path_id = static_cast<int>(result.rx_records[rxi].paths.size());
                    path.total_length = totalDist + Length(Subtract(segEnd, context.rx_grid[rxi]));
                    path.is_los = (depth == 0);
                    path.valid = true;

                    PathNode txNode;
                    txNode.interaction_type = InteractionType::Tx;
                    txNode.point = context.tx_point;
                    txNode.valid = true;
                    path.nodes.push_back(txNode);

                    PathNode rxNode;
                    rxNode.interaction_type = InteractionType::Rx;
                    rxNode.point = context.rx_grid[rxi];
                    rxNode.valid = true;
                    path.nodes.push_back(rxNode);

                    result.rx_records[rxi].paths.push_back(path);
                    result.rx_records[rxi].ray_hit_count++;
                    result.rx_records[rxi].total_power_linear += curPwr;
                }
            }

            // If no hit, ray escapes
            if (!hit.hit) break;

            // Propagate: reflection
            const Face& face = context.scene->faces[hit.face_id];
            bool propagated = false;

            if (face.reflection_enabled && depth < maxDepth)
            {
                // Reflect
                double d = Dot(curDir, hit.normal);
                curDir = MakeVec3(
                    curDir.x - 2.0 * d * hit.normal.x,
                    curDir.y - 2.0 * d * hit.normal.y,
                    curDir.z - 2.0 * d * hit.normal.z);
                curPt = hit.position;
                curPwr *= 0.5; // placeholder reflection loss (replaced in B5)
                depth++;
                propagated = true;
            }

            if (!propagated) break;
        }
    }

    // Finalize
    for (auto& rec : result.rx_records)
    {
        if (rec.ray_hit_count > 0)
        {
            rec.total_power_dBm = 10.0 * std::log10(rec.total_power_linear * 1000.0);
            result.active_rx_count++;
        }
    }

    result.succeeded = true;
    oss.str(""); oss << "SbrEngine: complete, activeRx=" << result.active_rx_count
                      << "/" << context.rx_grid.size();
    result.trace_lines.push_back(oss.str());
    return result;
}

} // namespace rt
