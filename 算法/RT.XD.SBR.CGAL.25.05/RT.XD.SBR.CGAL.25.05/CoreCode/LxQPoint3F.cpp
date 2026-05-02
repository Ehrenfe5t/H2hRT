#include"LxQPoint3F.h"



namespace Point3FStd {
    Point3F::Point3F() {
        this->x = 0.0F;
        this->y = 0.0F;
        this->z = 0.0F;
    }

    Point3F::Point3F(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Point3F::Point3F(const Point3F& p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }

    Point3F::~Point3F() {
    }

    bool Point3F::operator<(const Point3F& obj) const
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


    void CopyVectorPoint3F(const std::vector<Point3F>& obj, std::vector<Point3F>& res) {

        res.clear();
        res.resize(obj.size());

        for (int loop = 0; loop < obj.size(); ++loop) {
            res[loop] = obj[loop];
        }

    }

}

