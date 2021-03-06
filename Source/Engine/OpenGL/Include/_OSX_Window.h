#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// OpenGL Window creation
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _MACOS

class GLWindow : public SysWindow
{
private:

	void*				mGraphicsThread;
	NSOpenGLContext*	mContext;

public:

	GLWindow (uint msaa = 0) : mGraphicsThread(0), mContext(0) {}

protected:

	virtual bool _CreateContext();
	virtual void _UpdateContext();
	virtual void _ReleaseContext();
	virtual void BeginFrame();
	virtual void EndFrame();
};

#endif