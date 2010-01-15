#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Simple struct that combines relative and absolute coordinates
//============================================================================================================

struct UIAnchor
{
	float mRelative;
	float mAbsolute;

	UIAnchor()									: mRelative(0.0f), mAbsolute(0.0f) {}
	UIAnchor(float absolute)					: mRelative(0.0f), mAbsolute(absolute) {}
	UIAnchor(float relative, float absolute)	: mRelative(relative), mAbsolute(absolute) {}
	void Set(float relative, float absolute)	{ mRelative = relative; mAbsolute = absolute; }
};