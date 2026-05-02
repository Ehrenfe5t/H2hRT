


#include"HdQBvhBall3D.h"

#include<stack>
#include<iostream>

namespace BvhBall3DStd {


	int BvhBall3D::GetLevel() const
	{
		return nodeInfo.GetLevel();
	}

	BvhBall3D::BvhBall3D()
	{
	}

	BvhBall3D::BvhBall3D(int level, const BoundingBox3DStd::BoundingBox3D& box3D)
	{
		nodeInfo.SetLevel(level);
		nodeInfo.ReBuildByBox3D(box3D);
	}

	void BvhBall3D::GetStack(BvhBall3D* p, std::stack<BvhBall3D*>&stack) {
		if (p == NULL) {
			return;
		}
		stack.push(p);
		GetStack(p->leftNode_safe, stack);
		p->leftNode_safe = NULL;
		GetStack(p->rightNode_safe, stack);
		p->rightNode_safe = NULL;
	}

	BvhBall3D::~BvhBall3D()
	{
		std::stack<BvhBall3D*> nodesToDelete;
		GetStack(this, nodesToDelete);

		while (!nodesToDelete.empty()) {
			BvhBall3D* currentNode = nodesToDelete.top();
			nodesToDelete.pop();
			delete currentNode;
		}
	}

	

	bool BvhBall3D::UpdateSafeIdSetInfo()
	{
		std::stack<BvhBall3D*> nodesToDelete;
		nodesToDelete.push(this);

		while (!nodesToDelete.empty()) {
			BvhBall3D* currentNode = nodesToDelete.top();
			nodesToDelete.pop();

			if (currentNode->leftNode_safe != nullptr) {
				//需要释放内存
				nodesToDelete.push(currentNode->leftNode_safe);
			}

			if (currentNode->rightNode_safe != nullptr) {
				//需要释放内存
				nodesToDelete.push(currentNode->rightNode_safe);
			}
			if (!currentNode->cornerIdSet_safe.UpdateBySet(currentNode->cornerIdSet)) {
				return false;
			}
			if (!currentNode->triangleIdSet_safe.UpdateBySet(currentNode->triangleIdSet)) {
				return false;
			}
		}
		return true;
	}

	bool BvhBall3D::IsTriangle3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3)
	{
		return this->nodeInfo.ball3D.IsTriangle3DInBoxPlus(p1, p2, p3);
	}


	void BvhBall3D::AddTriangleId(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3, int id)
	{
		if (nullptr == this->leftNode_safe) {
			if(nullptr == this->rightNode_safe){
				triangleIdSet.insert(id);
			}
			else {
				if (this->rightNode_safe->IsTriangle3DInBoxPlus(p1, p2, p3)) {
					return this->rightNode_safe->AddTriangleId(p1, p2, p3, id);
				}
				else {
					triangleIdSet.insert(id);
				}
			}
		}
		else {
			if (nullptr == this->rightNode_safe) {
				if (this->leftNode_safe->IsTriangle3DInBoxPlus(p1, p2, p3)) {
					return this->leftNode_safe->AddTriangleId(p1, p2, p3, id);
				}
				else {
					triangleIdSet.insert(id);
				}
			}
			else {

				if (this->leftNode_safe->IsTriangle3DInBoxPlus(p1, p2, p3)) {
					return this->leftNode_safe->AddTriangleId(p1, p2, p3, id);
				}
				else {
					if (this->rightNode_safe->IsTriangle3DInBoxPlus(p1, p2, p3)) {
						return this->rightNode_safe->AddTriangleId(p1, p2, p3, id);
					}
					else {
						triangleIdSet.insert(id);
					}
				}
			}
		}
	}


	bool BvhBall3D::IsLineSegment3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
	{
		return this->nodeInfo.ball3D.IsLineSegment3DInBoxPlus(p1, p2);
	}

	void BvhBall3D::AddCornerId(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, int id)
	{
		if (nullptr == this->leftNode_safe) {
			if (nullptr == this->rightNode_safe) {
				cornerIdSet.insert(id);
			}
			else {
				if (this->rightNode_safe->IsLineSegment3DInBoxPlus(p1, p2)) {
					return this->rightNode_safe->AddCornerId(p1, p2, id);
				}
				else {
					cornerIdSet.insert(id);
				}
			}
		}
		else {
			if (nullptr == this->rightNode_safe) {
				if (this->leftNode_safe->IsLineSegment3DInBoxPlus(p1, p2)) {
					return this->leftNode_safe->AddCornerId(p1, p2, id);
				}
				else {
					cornerIdSet.insert(id);
				}
			}
			else {

				if (this->leftNode_safe->IsLineSegment3DInBoxPlus(p1, p2)) {
					return this->leftNode_safe->AddCornerId(p1, p2, id);
				}
				else {
					if (this->rightNode_safe->IsLineSegment3DInBoxPlus(p1, p2)) {
						return this->rightNode_safe->AddCornerId(p1, p2, id);
					}
					else {
						cornerIdSet.insert(id);
					}
				}
			}
		}
	}

	bool BvhBall3D::DelNullLastNodeLeft() {
		if (leftNode_safe != nullptr) {
			if (leftNode_safe->DelNullLastNode()) {
				delete leftNode_safe;
				leftNode_safe = nullptr;
				return true;
			}
		}
		return false;
	}

	bool BvhBall3D::DelNullLastNode()
	{
		if (leftNode_safe == nullptr && rightNode_safe == nullptr && triangleIdSet.size()<1) {
			return true;
		}
		return false;
	}


	bool BvhBall3D::DelNullLastNodeRight() {
		if (rightNode_safe != nullptr) {
			if (rightNode_safe->DelNullLastNode()) {
				delete rightNode_safe;
				rightNode_safe = nullptr;
				return true;
			}
		}
		return false;
	}

	bool BvhBall3D::DelOneNullNode()
	{
		if (!DelNullLastNode()) {
			std::stack<BvhBall3D*> nodesToDelete;
			nodesToDelete.push(this);

			while (!nodesToDelete.empty()) {
				BvhBall3D* currentNode = nodesToDelete.top();
				nodesToDelete.pop();

				if (currentNode->leftNode_safe != nullptr) {
					//需要释放内存
					nodesToDelete.push(currentNode->leftNode_safe);
				}

				if (currentNode->rightNode_safe != nullptr) {
					//需要释放内存
					nodesToDelete.push(currentNode->rightNode_safe);
				}
				if (currentNode->DelNullLastNodeLeft()) {
					return true;
				}
				if (currentNode->DelNullLastNodeRight()) {
					return true;
				}
			}
		}
		
		return false;
	}
	void BvhBall3D::DelNullNode()
	{
		while (DelOneNullNode()) {

		}
	}
}