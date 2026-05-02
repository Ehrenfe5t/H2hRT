

#include"../Input.h"

#include <random>

namespace SbrRayGeneratedByTransmittingAntennaStd {

    /// <summary>
    /// 基于Marsaglia方法生成均匀的发射球
    /// </summary>
    /// <param name="txPoints"></param>
    void GetUniformUnitVectorByMarsagliaMethod(int num, std::vector<Point3DStd::Point3D>& vecPoints) {
        int z1 = 20;
        int z2 = 112;
        CRandomStd::CRandom sita(z1, 0.0);
        CRandomStd::CRandom pesi(z2, 0.0);
        for (; vecPoints.size() < num;)
        {
            sita = CRandomStd::CreateCRandom(pesi.seed);
            pesi = CRandomStd::CreateCRandom(sita.seed);
            double u = 2 * sita.random - 1.0;
            double v = 2 * pesi.random - 1.0;
            double r2 = pow(u, 2) + pow(v, 2);
            if (r2 < 1)
            {
                double x = 2 * u * sqrt(1 - r2);
                double y = 2 * v * sqrt(1 - r2);
                double z = 1 - 2 * r2;
                vecPoints.push_back(Point3DStd::Point3D(x, y, z));
            }
        }
    }

    void GetUniformUnitVectorByMarsagliaMethod2(size_t num, std::vector<Ray3DStd::Ray3D>& txRays) {
        int z1 = 20;
        int z2 = 112;
        CRandomStd::CRandom sita(z1, 0.0);
        CRandomStd::CRandom pesi(z2, 0.0);
        size_t count = 0;
        for (; count < num;)
        {
            sita = CRandomStd::CreateCRandom(pesi.seed);
            pesi = CRandomStd::CreateCRandom(sita.seed);
            double u = 2 * sita.random - 1.0;
            double v = 2 * pesi.random - 1.0;
            double r2 = pow(u, 2) + pow(v, 2);
            if (r2 < 1)
            {
                txRays[count].vec.x = 2 * u * sqrt(1 - r2);
                txRays[count].vec.y = 2 * v * sqrt(1 - r2);
                txRays[count].vec.z = 1 - 2 * r2;
                count++;
            }
        }
    }

    /// <summary>
    /// 基于Marsaglia方法生成均匀的发射射线
    /// </summary>
    /// <param name="o"></param>
    /// <param name="num"></param>
    /// <param name="txRays"></param>
    void InitSBRLaunchRayMarsaglia(const Point3DStd::Point3D& o, int num, std::vector<Ray3DStd::Ray3D>& txRays) {
        std::vector<Point3DStd::Point3D> vecPoints;
        GetUniformUnitVectorByMarsagliaMethod(num, vecPoints);
        for (int i = 0; i < vecPoints.size(); i++) {
            txRays.push_back(Ray3DStd::Ray3D(o, vecPoints[i]));
        }
    }


    void InitSBRLaunchRayMarsaglia2(const Point3DStd::Point3D& o, size_t num, std::vector<Ray3DStd::Ray3D>& txRays) {
        GetUniformUnitVectorByMarsagliaMethod2(num, txRays);
        for (int i = 0; i < num; ++i) {
            Geometry3DOperateStd::AssignmentPoint3DPoint3D(txRays[i].o, o);
        }
    }

    void InitSBRLaunchRayMarsaglia3(const Point3DStd::Point3D& o, size_t num, std::vector<Ray3DStd::Ray3D>& txRays) {

        double phi = GlobalConstantStd::Pi * (3.0 - sqrt(5.0));
        double tx = 0.0;
        double ty = 0.0;
        double tz = 0.0;

        for (int i = 0; i < num; ++i) {
            double y = 1.0 - (i / (num - 1.0)) * 2.0;
            double radius = sqrt(1.0 - y * y);
            double theta = phi * i;
            double x = cos(theta) * radius + tx;
            double z = sin(theta) * radius + tz;
            y += ty;

            Geometry3DOperateStd::AssignmentPoint3DPoint3D(txRays[i].o, o);

            txRays[i].vec.x = x;
            txRays[i].vec.y = y;
            txRays[i].vec.z = z;

        }
    }


    void InitSBRLaunchRayMarsaglia4(const Point3DStd::Point3D& o, size_t num, Ray3DStd::Ray3D* txRays) {

        double phi = GlobalConstantStd::Pi * (3.0 - sqrt(5.0));
        double tx = 0.0;
        double ty = 0.0;
        double tz = 0.0;

        for (int i = 0; i < num; ++i) {
            double y = 1.0 - (i / (num - 1.0)) * 2.0;
            double radius = sqrt(1.0 - y * y);
            double theta = phi * i;
            double x = cos(theta) * radius + tx;
            double z = sin(theta) * radius + tz;
            y += ty;

            Geometry3DOperateStd::AssignmentPoint3DPoint3D(txRays[i].o, o);

            txRays[i].vec.x = x;
            txRays[i].vec.y = y;
            txRays[i].vec.z = z;

        }
    }

    void InitSBRLaunchRayMarsagliaVec(size_t num, std::vector<Point3DStd::Point3D>& txRayVec) {

        txRayVec.resize(num);

        int z1 = 20;
        int z2 = 112;
        CRandomStd::CRandom sita(z1, 0.0);
        CRandomStd::CRandom pesi(z2, 0.0);
        size_t count = 0;
        for (; count < num;)
        {
            sita = CRandomStd::CreateCRandom(pesi.seed);
            pesi = CRandomStd::CreateCRandom(sita.seed);
            double u = 2 * sita.random - 1.0;
            double v = 2 * pesi.random - 1.0;
            double r2 = pow(u, 2) + pow(v, 2);
            if (r2 < 1)
            {
                txRayVec[count].x = 2 * u * sqrt(1 - r2);
                txRayVec[count].y = 2 * v * sqrt(1 - r2);
                txRayVec[count].z = 1 - 2 * r2;
                count++;
            }
        }
    }


    void InitSBRLaunchRayVec(size_t num, std::vector<Point3DStd::Point3D>& txRayVec) {

        txRayVec.resize(num);
        
        double phi = GlobalConstantStd::Pi * (3.0 - sqrt(5.0));
        double tx = 0.0;
        double ty = 0.0;
        double tz = 0.0;
        
        for (int i = 0; i < num; ++i) {
            double y = 1.0 - (i / (num - 1.0)) * 2.0;
            double radius = sqrt(1.0 - y * y);
            double theta = phi * i;
            double x = cos(theta) * radius + tx;
            double z = sin(theta) * radius + tz;
            y += ty;
        
            txRayVec[i].x = x;
            txRayVec[i].y = y;
            txRayVec[i].z = z;
        
        }

        //auto cd_num = num / 1000;
        //
        //double dt_phi = 2 * GlobalConstantStd::Pi / cd_num;
        //
        //for (int i = 0; i < cd_num; ++i) {
        //    double phi_i = i * dt_phi;
        //    double x = cos(phi_i);
        //    double y = sin(phi_i);
        //    double z = 0;
        //    txRayVec.emplace_back(Point3DStd::Point3D(x, y, z));
        //}



        // 创建随机数生成器和引擎
        //std::mt19937 g(9973);

        // 使用std::shuffle来打乱vector
        //std::shuffle(txRayVec.begin(), txRayVec.end(), g);

    }


}
