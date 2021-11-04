#include "kdTree.h"

kdTree::kdTree()
{
}

//THIS K D tree implimentation is taken from geeksforgeeks with some minor alterations
//original source code available at : https://www.geeksforgeeks.org/k-dimensional-tree/

//k = 3 as point is in 3d space
const int k = 3;

// A method to create a node of K D tree 
struct PmNode* newNode(Ray photon)
{
	struct PmNode* temp = new PmNode;

	temp->photon = photon;

	temp->left = temp->right = NULL;
	return temp;
}

// Inserts a new node and returns root of modified tree 
// The parameter depth is used to decide axis of comparison 
PmNode * kdTree::insertRec(PmNode *node, Ray photon, int depth) {

    if (node == nullptr) {
        return newNode(photon);
    }

	// Calculate current dimension (cd) of comparison 
    int cd = depth % k;
	//repeat for x,y,z dimensions
    if (cd == 0) {
		//if photon x pos is greater or equal to curr node, insert right
        if (photon.position.x >= node -> photon.position.x) {
            node -> right = insertRec(node -> right, photon, depth + 1);
        }
		//if x pos is less, insert left
        else {
			node->left = insertRec(node->left, photon, depth + 1);
        }
    }
    if (cd == 1) {
		//if photon y pos is greater or equal to curr node, insert right
        if (photon.position.y >= node -> photon.position.y) {
			node->right = insertRec(node->right, photon, ++depth);
        }
		//if y pos is less, insert left
        else {
			node->left = insertRec(node->left, photon, depth + 1);
        }
    }
    if (cd == 2) {
		//if photon z pos is greater or equal to curr node, insert right
        if (photon.position.y >= node -> photon.position.y) {
            node -> right = insertRec(node -> right, photon, depth + 1);
        }
		//if z pos is less, insert left
        else {
            node -> left = insertRec(node -> left, photon, depth + 1);
        }
    }

	return node;
}

void kdTree::findNearestRecord(Vertex hitPosition, PmNode* node, int depth) {
	Vector nearestPosition;
    int cd = depth % k;
	depth++;
	//if node has no children then return
    if (node-> right == nullptr && node-> left == nullptr) {
        return;
    }
	//if hit position is closer to new node than the furthest away node in the array, then replace
    if (hitPosition.distanceFrom(closestNodes[furthestPos].photon.position) > hitPosition.distanceFrom(node->photon.position)) {
        closestNodes[furthestPos] = *node;
		//assign the position of the furthest away photon
		float furthestDist = 0;
		for (int i = 0; i < n; i++) {
		    if (hitPosition.distanceFrom(closestNodes[i].photon.position) > furthestDist) {
				furthestDist = hitPosition.distanceFrom(closestNodes[i].photon.position);
		        furthestPos = i;
		    }
		}
    }
	//x depth
    if (cd == 0) {
        nearestPosition = Vector(node-> photon.position.x, hitPosition.y, hitPosition.z);
		//if node x is greater or equal to hit position, search left if possible
		if (node->photon.position.x >= hitPosition.x) {
			if (node->left != nullptr) {
				findNearestRecord(hitPosition, node->left, depth);
			}
			//search right if possible and useful
			if (node->right != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->right, depth);
			}
		}
		//search right if possible
		else {
			if (node->right != nullptr) {
				findNearestRecord(hitPosition, node->right, depth);
			}
			//search left if possible and useful
			if (node->left != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->left, depth);
			}
		}
    }
	//y depth
    if (cd == 1) {
        nearestPosition = Vector(hitPosition.x, node-> photon.position.y, hitPosition.z);
		//if node y is greater or equal to hit position, search left if possible
		if (node->photon.position.y >= hitPosition.y) {
			if (node->left != nullptr) {
				findNearestRecord(hitPosition, node->left, depth);
			}
			if (node->right != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->right, depth);
			}
		}
		else {
			if (node->right != nullptr) {
				findNearestRecord(hitPosition, node->right, depth);
			}
			if (node->left != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->left, depth);
			}
		}
    }
	//z depth
    if (cd == 2) {
        nearestPosition = Vector(hitPosition.x, hitPosition.y, node-> photon.position.z);
		//if node z is greater or equal to hit position, search left if possible
		if (node->photon.position.z >= hitPosition.z) {
			if (node->left != nullptr) {
				findNearestRecord(hitPosition, node->left, depth);
			}
			if (node->right != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->right, depth);
			}
		}
		else {
			if (node->right != nullptr) {
				findNearestRecord(hitPosition, node->right, depth);
			}
			if (node->left != nullptr && nearestPosition.distanceFrom(Vector(hitPosition.x, hitPosition.y, hitPosition.z)) < hitPosition.distanceFrom(closestNodes[furthestPos].photon.position)) {
				findNearestRecord(hitPosition, node->left, depth);
			}
		}
    }
    return;
}

bool kdTree::getNearestPhotons(Vertex hitPosition, Ray photons[], int n) {
    if (photonArrayEmpty) {
		photonArrayEmpty = false;
		closestNodes = new PmNode[n];
    } 

	if (root == nullptr || photons == nullptr) {
		return false;
	}
	else {
		furthestPos = 0;
		this->n = n;
		//assigns each item in array to root before they are assigned properly
		for (int i = 0; i < n; i++) {
			closestNodes[i] = *root;
		}
		findNearestRecord(hitPosition, root, 0);
		//add newly computed closest photons
		for (int i = 0; i < n; i++) {
			photons[i] = closestNodes[i].photon;
		}
		return true;
	}
}

void kdTree::insert(Ray photon) {
	if (!emptyTree) {
		insertRec(root, photon, 0);
	}
	else {
		//assign to root if the tree is empty
		root = newNode(photon);;
		emptyTree = false;
	}
}
