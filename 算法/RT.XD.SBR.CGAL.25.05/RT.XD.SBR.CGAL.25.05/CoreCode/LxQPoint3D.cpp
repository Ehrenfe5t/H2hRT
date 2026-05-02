#include"LxQPoint3D.h"



namespace Point3DStd {
    Point3D::Point3D() {
        this->x = 0.0;
        this->y = 0.0;
        this->z = 0.0;
    }

    Point3D::Point3D(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Point3D::Point3D(const Point3D& p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }

    Point3D::~Point3D() {
    }

    const double& Point3D::operator[](int axis) const {
        assert(axis >= 0 && axis < 3 && "Axis index out of range [0,2]");
        return (&x)[axis];
    }

    bool Point3D::operator<(const Point3D& obj) const
    {
        if (obj.x > x) {
            return true;
        }
        else if (obj.x == x) {
            if (obj.y > y) {
                return true;
            }
            else if (obj.y == y) {
                if (obj.z > z) {
                    return true;
                }
            }
        }
        return false;
    }


    // 加法运算符重载
    Point3D Point3D::operator+(const Point3D& p) const {
        return Point3D(x + p.x, y + p.y, z + p.z);
    }

    // 减法运算符重载
    Point3D Point3D::operator-(const Point3D& p) const {
        return Point3D(x - p.x, y - p.y, z - p.z);
    }

    // 标量乘法运算符重载（向量 * 标量）
    Point3D Point3D::operator*(double s) const {
        return Point3D(x * s, y * s, z * s);
    }

    // 标量除法运算符重载
    Point3D Point3D::operator/(double s) const {
        return Point3D(x / s, y / s, z / s);
    }


    double Point3D::distance(const Point3D& p) const {
        double dx = x - p.x;
        double dy = y - p.y;
        double dz = z - p.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    void CopyVectorPoint3D(const std::vector<Point3D>& obj, std::vector<Point3D>& res) {

        res.clear();
        res.resize(obj.size());

        for (int loop = 0; loop < obj.size(); ++loop) {
            res[loop] = obj[loop];
        }

    }


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, Point3DStd::Point3D& p) {
        j.at("x").get_to(p.x);
        j.at("y").get_to(p.y);
        j.at("z").get_to(p.z);
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const Point3DStd::Point3D& p) {
        j = nlohmann::json{ {"x",p.x}, {"y",p.y}, {"z",p.z} };
    }
}

