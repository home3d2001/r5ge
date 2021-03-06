#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Vector3f spline
// Author: Michael Lyashenko
//============================================================================================================

class SplineV
{
public:

	struct CtrlPoint
	{
		float		mTime;
		Vector3f	mVal;
		Vector3f	mTan;

		bool operator < (const CtrlPoint& ctrl) const { return mTime < ctrl.mTime; }
	};

private:

	Array<CtrlPoint> mCp;
	bool mIsSmooth;
	bool mSeamless;

	// For caching purposes we'll remember the last sampling values
	mutable float mLastSample;
	mutable uint mLastIndex;

public:

	SplineV() : mIsSmooth(false), mSeamless(false), mLastSample(0.0f), mLastIndex(0) {}

	void Release() { mCp.Release(); }

	float	StartTime() const	{ return mCp.IsEmpty()  ? 0 : mCp.Front().mTime;  }
	float	EndTime()	const	{ return mCp.IsEmpty()  ? 0 : mCp.Back().mTime;   }
	float	Length()	const	{ return mCp.GetSize() < 2 ? 0 : mCp.Back().mTime - mCp.Front().mTime; }
	bool	IsValid()	const	{ return mCp.IsValid(); }
	bool	IsEmpty()	const	{ return mCp.IsEmpty(); }
	uint	GetSize()	const	{ return mCp.GetSize(); }
	void	Clear()				{ mCp.Clear(); mIsSmooth = false; mLastIndex = 0; mLastSample = 0.0f; }

	const	CtrlPoint& operator [](uint index) const	{ return mCp[index]; }
			CtrlPoint& operator [](uint index)			{ return mCp[index]; }

	// Retrieves the first / last values
	Vector3f GetFirst() const { return mCp.IsEmpty() ? Vector3f() : mCp[0].mVal; }
	Vector3f GetLast()  const { return mCp.IsEmpty() ? Vector3f() : mCp[mCp.GetSize() - 1].mVal; }

	// Seamless splines don't have ease in/ease out (for looping animations)
	void SetSeamless (bool val) { if (mSeamless != val) { mSeamless = val; mIsSmooth = false; } }

	// Adds an element to the spline
	void AddKey (float time, const Vector3f& pos);

	// Calculates all tangents and rotational control points
	void Smoothen();

	// Sample the spline at the given time. Smoothness 0 = no interpolation, 1 = linear, 2 = spline
	Vector3f Sample (float time, byte smoothness = 2) const;
};