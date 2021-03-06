#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Base camera class
// Author: Michael Lyashenko
//============================================================================================================

class Camera : public Object
{
protected:

	Vector3f mRelativeRange;	// X = Near, Y = Far, Z = FOV
	Vector3f mAbsoluteRange;

	// Objects should never be created manually. Use the AddObject<> template instead.
	Camera() : mRelativeRange(0.3f, 100.0f, 90.0f) {}

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS(Camera, Object, Object);

	const Vector3f& GetRelativeRange() const { return mRelativeRange; }
	const Vector3f& GetAbsoluteRange() const { return mAbsoluteRange; }

	void SetRelativeRange (const Vector3f& range) { mRelativeRange = range; }
	void SetAbsoluteRange (const Vector3f& range);

protected:

	// Custom overrideable function called after the node's absolute coordinates have been calculated
	virtual void OnUpdate();

public:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};