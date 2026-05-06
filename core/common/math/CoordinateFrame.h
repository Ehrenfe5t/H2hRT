#pragma once
#include "Vec3.h"
#include "MathConstants.h"

namespace rt {

struct CoordFrame { Vec3 ex, ey, ez; bool valid = false; };

inline CoordFrame BuildEdgeFixedFrame(const Vec3& edgeDir, const Vec3& incDir) {
    CoordFrame f;
    f.ez = Normalize(edgeDir);
    if (Length(f.ez) <= 0.0) return f;
    double proj = Dot(incDir, f.ez);
    Vec3 perp = MakeVec3(incDir.x - proj*f.ez.x, incDir.y - proj*f.ez.y, incDir.z - proj*f.ez.z);
    double plen = Length(perp);
    if (plen <= 0.0) return f;
    f.ex = Scale(perp, 1.0/plen);
    f.ey = Normalize(Cross(f.ez, f.ex));
    f.valid = Length(f.ey) > 0.0;
    return f;
}

inline Vec3 SphericalToCartesian(double thetaRad, double phiRad) {
    double st = std::sin(thetaRad);
    return MakeVec3(st*std::cos(phiRad), st*std::sin(phiRad), std::cos(thetaRad));
}

inline void CartesianToSpherical(const Vec3& dir, double& thetaRad, double& phiRad) {
    Vec3 d = Normalize(dir);
    thetaRad = std::acos(d.z);
    phiRad = std::atan2(d.y, d.x);
    if (phiRad < 0.0) phiRad += 6.28318530717958647693;
}

} // namespace rt
