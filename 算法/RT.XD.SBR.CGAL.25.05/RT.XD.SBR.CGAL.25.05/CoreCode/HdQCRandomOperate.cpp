
#include"HdQCRandomOperate.h"
#include<Windows.h>
#include<Math.h>
#include<time.h>


namespace CRandomStd {


    CRandom CreateCRandom(int z)
        // 16807 way to create random numbers
        // z is the seed number, num is the total random number to create
    {
        //z(n+1)=(a*z(n)+b) mod m
        //describe m=a*q+r to avoid that the number is large than the computer can bear
        const int m = int(pow(2, 31) - 1);
        const int a = 16807;
        const int q = 127773;
        const int r = 2836;

        int temp = a * (z % q) - r * (z / q);

        if (temp < 0)
        {
            temp = m + temp;
        }
        //z is the seed number
        z = temp;
        double t = z * 1.0 / m;

        CRandom cr;
        cr.random = t;
        cr.seed = z;
        return cr;
    }


    int getRandInt(int m, int n, int seek) {
        // 产生 m <= num < n的随机数
        // 随机数种子
        srand((unsigned)time(NULL) * seek);
        // 模除加加法计算 [m, n) 的随机整数
        int randint = rand() % (n - m) + m;
        return randint;
    }

    double Random(double a, double b)		//生成a与b之间的均匀随机数,区间[a,b]
    {
        Sleep(5);
        SYSTEMTIME sys;
        GetLocalTime(&sys);
        int seed = sys.wMilliseconds;
        srand(seed);
        double t;
        if (b >= a)
            t = ((double)rand() / RAND_MAX * (b - a)) + a;
        else
            t = ((double)rand() / RAND_MAX * (a - b)) + b;
        return t;
    }

    int RandInt(int low, int high)		/* 在整数 low 和 high 之间产生一个随机整数，区间为闭区间 */
    {
        int tmp;
        if (low == high)
            return low;
        if (low > high)
        {
            tmp = high;
            high = low;
            low = tmp;
        }
        tmp = (int)(Random(0, 1) * (high - low + 1)) + low;
        if (tmp > high)
            tmp = high;
        return tmp;
    }

}