#pragma once


#include"HdQCRandom.h"



namespace CRandomStd {


    CRandom CreateCRandom(int z);


    int getRandInt(int m, int n, int seek);

    double Random(double a, double b);

    int RandInt(int low, int high);

}