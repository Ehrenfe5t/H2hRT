
#include"HdQBvhBall3DParameter.h"

namespace BvhBall3DParameterStd {

	BvhBall3DParameter::BvhBall3DParameter()
	{
		this->maxLevel = 1;
	}


	BvhBall3DParameter::BvhBall3DParameter(int maxLevel)
	{
		this->maxLevel = maxLevel;
	}

	BvhBall3DParameter::~BvhBall3DParameter()
	{
	}

}