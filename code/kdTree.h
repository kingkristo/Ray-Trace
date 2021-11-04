#include "PmNode.h"

class kdTree
{
	PmNode* root = nullptr;
	bool emptyTree = true;
	PmNode* closestNodes;
	int furthestPos; 
	bool photonArrayEmpty = true;
	int n = 0;
    public:
		PmNode* nodes;
        kdTree();
		PmNode* insertRec(PmNode* node, Ray photon, int depth);
        void insert(Ray);
		void findNearestRecord(Vertex hitPosition, PmNode* node, int depth);
        bool getNearestPhotons(Vertex, Ray[], int);
};
