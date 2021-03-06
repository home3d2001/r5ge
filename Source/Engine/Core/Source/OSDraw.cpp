#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Initial validation of the draw script
//============================================================================================================

void OSDraw::OnInit()
{
	mCam = R5_CAST(Camera, mObject);

	if (mCam == 0)
	{
		ASSERT(false, "You've attached OSDraw to something other than a camera!");
		DestroySelf();
	}
	else
	{
		// Find the root
		mRoot = OSSceneRoot::FindRootOf(mObject);

		// For convenience
		mCore		= mObject->GetCore();
		mGraphics	= mObject->GetGraphics();

		// Set the root of the scene
		mScene.SetRoot(mRoot->GetOwner());
		mScene.Initialize(mGraphics);

		// Initialize the shadow class
		mShadow.Initialize(mGraphics);

		// Default fog and background color
		mFogRange.Set(0.5f, 1.0f);
		mBackground.Set(0.0f, 0.0f, 0.0f, 1.0f);
		
		// Add the draw callback to a high priority so that it's drawn before the default (1000) callbacks
		mCore->AddOnDraw( bind(&OSDraw::OnDraw, this), 10000 );

		// Save the depth texture
		mFinalDepth = mGraphics->GetTexture("[R5] Depth");
	}
}

//============================================================================================================
// Remove the draw callback from the core
//============================================================================================================

void OSDraw::OnDestroy()
{
	mCore->RemoveOnDraw( bind(&OSDraw::OnDraw, this) );
	mShadow.Release();
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void OSDraw::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Background Color", mBackground);
	root.AddChild("Fog Range", mFogRange);
	if (mGrid) root.AddChild("Grid", mGrid);
	mShadow.OnSerializeTo(root.AddChild("Shadowmap"));
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void OSDraw::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Background Color")
	{
		node.mValue >> mBackground;
	}
	else if (node.mTag == "Fog Range")
	{
		node.mValue >> mFogRange;
	}
	else if (node.mTag == "Grid")
	{
		node.mValue >> mGrid;
	}
	else if (node.mTag == "Shadowmap")
	{
		FOREACH(i, node.mChildren)
		{
			mShadow.OnSerializeFrom(node.mChildren[i]);
		}
	}
}