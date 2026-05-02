

#include"HdQCRandom.h"

namespace CRandomStd {

    CRandom::CRandom(int seed, double random) {
        this->seed = seed;
        this->random = random;
    }
    CRandom::CRandom() {
        this->seed = 0;
        this->random = 0.0;
    }
}

