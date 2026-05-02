#pragma once



namespace Point2DStd {
    
    class Point2D
    {
    public:
        /// <summary>
        /// 点的x坐标
        /// </summary>
        double x;
        /// <summary>
        /// 点的y坐标
        /// </summary>
        double y;
        Point2D();
        Point2D(double x, double y);
        ~Point2D();

    private:

    };


}