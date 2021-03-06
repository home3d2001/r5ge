#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Higher level of renderer API interaction, handles resource management for graphical resources
// Author: Michael Lyashenko
//============================================================================================================

class GLGraphics : public GLController
{
private:

	struct DelegateEntry
	{
		DelayedDelegate callback;
		void* param;
	};

	typedef Array<DelegateEntry> Delegates;

protected:

	Thread::IDType		mThread;
	IVBO*				mSkyboxVBO;
	IVBO*				mSkyboxIBO;
	uint				mQuery;

	// Resources
	PointerArray<GLVBO>	mVbos;
	PointerArray<GLFBO>	mFbos;
	Delegates			mDelegates;
	Techniques			mTechs;
	Materials			mMaterials;
	Textures			mTextures;
	Textures			mTempTex;
	Shaders				mShaders;
	Shaders				mSpecial;	// Array of special shaders that's not exposed outside this library
	Fonts				mFonts;

public:

	GLGraphics();
	virtual ~GLGraphics() { Release(); }

	// ID of the thread the graphics class was initialized in
	Thread::IDType GetThreadID() const { return mThread; }

	// Returns whether the specified point would be visible if rendered
	virtual bool IsPointVisible (const Vector3f& v);

	// Reads the buffer's color at the specified pixel
	virtual Color4f ReadColor (const Vector2i& pos);

	// Converts screen coordinates to world coordinates and vice versa
	virtual Vector3f  ConvertTo3D (const Vector2i& pos, bool unproject = true);
	virtual Vector2i  ConvertTo2D (const Vector3f& pos);

	// Returns the distance between 'v' and the depth-determined point
	// of intersection on the ray from the camera's eyepoint to 'v'
	//virtual float GetDistanceToDepthAt (const Vector3f& v);

	// Initialize/release the graphics manager
	virtual bool Init (uint version = 200);
	virtual void Release();

	// Adds a delayed callback function that should be executed on the next frame (at BeginFrame)
	virtual void ExecuteBeforeNextFrame(const DelayedDelegate& callback, void* param);

	// Clear the screen or the off-screen target, rendering the skybox if necessary (pre-render)
	virtual void Clear (bool color = true, bool depth = true, bool stencil = true);

	// Pre/post-render
	virtual void BeginFrame();
	virtual void EndFrame();

	// Draws a pre-defined drawable object such as a skybox or a full-screen quad
	virtual uint Draw (uint drawable);

	// Direct access to managed resource arrays
	virtual Techniques&	GetAllTechniques()	{ return mTechs;		}
	virtual Materials&	GetAllMaterials()	{ return mMaterials;	}
	virtual Textures&	GetAllTextures()	{ return mTextures;		}
	virtual Shaders&	GetAllShaders()		{ return mShaders;		}
	virtual Fonts&		GetAllFonts()		{ return mFonts;		}

	// Managed unnamed resources
	virtual IVBO*			CreateVBO();
	virtual ITexture*		CreateRenderTexture(const char* name = 0);
	virtual IRenderTarget*	CreateRenderTarget();

	// Resource removal
	virtual void DeleteVBO			(const IVBO*			ptr);
	virtual void DeleteTexture		(const ITexture*		ptr);
	virtual void DeleteRenderTarget	(const IRenderTarget*	ptr);

	// Managed named resources
	virtual ITechnique*		GetTechnique	(const String& name, bool createIfMissing = true);
	virtual IMaterial*		GetMaterial		(const String& name, bool createIfMissing = true);
	virtual ITexture*		GetTexture		(const String& name, bool createIfMissing = true);
	virtual IShader*		GetShader		(const String& name, bool createIfMissing = true);
	virtual IFont*			GetFont			(const String& name, bool createIfMissing = true);

	// Serialization
	virtual bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);
	virtual bool SerializeTo (TreeNode& root) const;
};