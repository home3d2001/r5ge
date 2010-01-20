#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Class managing an array of drawable objects
//============================================================================================================

class DrawGroup
{
	struct Entry
	{
		Object*		mObject;	// Pointer to the object that will be rendered
		float		mDistance;	// Squared distance to the camera, used to sort objects

		// Comparison operator for sorting
		bool operator < (const Entry& obj) const { return (mDistance < obj.mDistance); }
	};

	Array<Entry> mEntries;

public:

	void Add (Object* object, float distance)
	{
		Entry& ent		= mEntries.Expand();
		ent.mObject		= object;
		ent.mDistance	= distance;
	}

	void Sort()  { mEntries.Sort();  }
	void Clear() { mEntries.Clear(); }
	uint Draw(const ITechnique* tech, bool insideOut);
};