#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Noise testing application
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Noise/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	Core*		mCore;
	Scene		mScene;
	Camera*		mCam;
	UI*			mUI;
	Noise		mNoise;
	bool		mRegenerate;

public:

	TestApp();
	~TestApp();

	void InitUI();
	void Run();
	void OnDraw();
	uint DrawScene();
	void Regenerate();

	bool Generate (UIArea* area, const Vector2i& pos, byte key, bool isDown);
	bool UpdateTooltips (UIArea* area);
};
};