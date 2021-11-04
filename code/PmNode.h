#ifndef PMNODE_H
#define PMNODE_H
#include "Ray.h"

class PmNode
{
    public:
		Ray photon;
		PmNode(Ray);
		PmNode();
		PmNode*left = nullptr;
		PmNode*right = nullptr;
};

#endif