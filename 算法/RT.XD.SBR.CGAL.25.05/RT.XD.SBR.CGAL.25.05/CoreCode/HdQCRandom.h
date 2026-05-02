#pragma once



namespace CRandomStd {

    /// <summary>
    /// https://www.cnblogs.com/cofludy/p/5894270.html
    /// https://blog.csdn.net/xuejinglingai/article/details/113267713
    /// </summary>
    class CRandom
    {
    public:
        int seed;
        double random;
        CRandom(int x, double y);
        CRandom();

    };

}