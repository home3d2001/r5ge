#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Retrieves an OpenGL draw mode based on the specified primitive type
//============================================================================================================

inline void GetGLPrimitive (uint	primitive,
							uint	vertexCount,
							uint&	glPrimitive,
							uint&	triangleCount)
{
	switch (primitive)
	{
	case IGraphicsManager::Primitive::Triangle:
		glPrimitive = GL_TRIANGLES;
		triangleCount = vertexCount / 3;
		break;

	case IGraphicsManager::Primitive::TriangleStrip:
		glPrimitive = GL_TRIANGLE_STRIP;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::Quad:
		glPrimitive = GL_QUADS;
		triangleCount = vertexCount / 2;
		break;

	case IGraphicsManager::Primitive::QuadStrip:
		glPrimitive = GL_QUAD_STRIP;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::TriangleFan:
		glPrimitive = GL_TRIANGLE_FAN;
		triangleCount = vertexCount > 1 ? vertexCount - 2 : 0;
		break;

	case IGraphicsManager::Primitive::Line:
		glPrimitive = GL_LINES;
		triangleCount = vertexCount / 2;
		break;

	case IGraphicsManager::Primitive::LineStrip:
		glPrimitive = GL_LINE_STRIP;
		triangleCount = vertexCount > 0 ? vertexCount - 1 : 0;
		break;

	case IGraphicsManager::Primitive::Point:
		glPrimitive = GL_POINTS;
		triangleCount = vertexCount;
		break;

	default:
		glPrimitive = 0;
		triangleCount = 0;
		break;
	}
}

//============================================================================================================
// Helper conversion arrays
//============================================================================================================

uint ConvertCondition[] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS,
};

//------------------------------------------------------------------------------------------------------------

uint ConvertOperation[] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_DECR,
	GL_INVERT,
};

//============================================================================================================
// GLGraphics GLController constructor
//============================================================================================================

GLController::GLController() :
	mFog				(false),
	mDepthWrite			(false),
	mDepthTest			(false),
	mColorWrite			(true),
	mAlphaTest			(false),
	mStencilTest		(false),
	mWireframe			(false),
	mLighting			(Lighting::None),
	mBlending			(Blending::None),
	mCulling			(Culling::None),
	mAdt				(0.0f),
	mThickness			(1.0f),
	mNormalize			(false),
	mDepthOffset		(false),
	mFov				(0),
	mAf					(0),
	mSimpleMaterial		(false),
	mActiveLightCount	(0),
	mProjection			(Projection::Orthographic),
	mResetView			(false),
	mResetProj			(false),
	mMatGlow			(0.0f),
	mTarget				(0),
	mTechnique			(0),
	mMaterial			(0),
	mShader				(0),
	mSkybox				(0),
	mActiveTU			(0) {}

//============================================================================================================
// Changes the currently active texture unit
//============================================================================================================

inline uint GLController::_CountImageUnits()
{
	if ( mTu.IsEmpty() )
	{
		uint maxIU = glGetInteger(GL_MAX_TEXTURE_IMAGE_UNITS);
		ASSERT( maxIU > 0, "Could not retrieve the maximum number of texture units" );
		mTu.ExpandTo( maxIU );
	}
	return mTu.GetSize();
}

//============================================================================================================
// Finish all draw operations
//============================================================================================================

void GLController::Flush()
{
	glFlush();
}

//============================================================================================================
// Toggles the fog on/off
//============================================================================================================

void GLController::SetFog (bool val)
{
	if ( mFogRange.x == mFogRange.y ) val = false;

	if ( mFog != val )
	{
		mTechnique = 0;

		if (mFog = val)
		{
			glEnable(GL_FOG);
			glFogf(GL_FOG_START, mFogRange.x);
			glFogf(GL_FOG_END, mFogRange.y);
		}
		else
		{
			glFogf(GL_FOG_START, mClipRange.y);
			glFogf(GL_FOG_END, mClipRange.y);
			glDisable(GL_FOG);
		}
	}
}

//============================================================================================================
// Toggles writing to Z-buffer
//============================================================================================================

void GLController::SetDepthWrite (bool val)
{
	if (mDepthWrite != val)
	{
		if (val)
		{
			val = (mTarget == 0 || mTarget->HasDepth());
			if (mDepthWrite == val) return;
		}

		mTechnique = 0;
		glDepthMask( (mDepthWrite = val) ? 1 : 0 );
	}
}

//============================================================================================================
// Toggles depth test
//============================================================================================================

void GLController::SetDepthTest (bool val)
{
	if ( mDepthTest != val )
	{
		mTechnique = 0;
		if (mDepthTest = val) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	}
}

//============================================================================================================
// Toggles writing of color information
//============================================================================================================

void GLController::SetColorWrite (bool val)
{
	if ( mColorWrite != val )
	{
		if (val)
		{
			val = (mTarget == 0 || mTarget->HasColor());
			if (mColorWrite == val) return;
		}

		mTechnique = 0;
		if (mColorWrite = val) glColorMask(true, true, true, true);
		else glColorMask(false, false, false, false);
	}
}

//============================================================================================================
// Toggles alpha testing
//============================================================================================================

void GLController::SetAlphaTest (bool val)
{
	if ( mAlphaTest != val )
	{
		mTechnique = 0;
		if (mAlphaTest = val) glEnable(GL_ALPHA_TEST);
		else glDisable(GL_ALPHA_TEST);
	}
}

//============================================================================================================
// Toggles stencil test
//============================================================================================================

void GLController::SetStencilTest (bool val)
{
	if ( mStencilTest != val )
	{
		if (mStencilTest = val) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);
	}
}

//============================================================================================================
// Whether the geometry is rendered in wireframe
//============================================================================================================

void GLController::SetWireframe (bool  val)
{
	if (mWireframe != val)
	{
		mTechnique = 0;
		if (mWireframe = val)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

//============================================================================================================
// Toggles lighting on/off
//============================================================================================================

void GLController::SetLighting (uint val)
{
	if (mLighting != val)
	{
		mTechnique = 0;

		if (val == Lighting::None)
		{
			// Turn off lighting
			glDisable(GL_LIGHTING);
		}
		else
		{
			if (val == Lighting::OneSided)
			{
				// Enable normal one-sided lighting
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
				glDisable(GL_VERTEX_PROGRAM_TWO_SIDE);
			}
			else
			{
				// Enable two-sided lighting
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
				glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
			}

			if (mLighting == Lighting::None)
			{
				// If lighting was off, turn it on
				glEnable(GL_LIGHTING);
			}
		}

		// Update the local state
		mLighting = val;
	}
}

//============================================================================================================
// Toggles blending on/off
//============================================================================================================

void GLController::SetBlending (uint val)
{
	if (mBlending != val)
	{
		mTechnique = 0;
		mBlending = val;

		if (mBlending == Blending::None)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			if		(mBlending == Blending::Normal)		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			else if (mBlending == Blending::Add)		glBlendFunc(GL_ONE, GL_ONE);
			else if (mBlending == Blending::Subtract)	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
			else if (mBlending == Blending::Modulate)	glBlendFunc(GL_DST_COLOR, GL_ZERO);

			glEnable(GL_BLEND);
		}
	}
}

//============================================================================================================
// Changes the culling -- front-faced, back-faced, or no culling
//============================================================================================================

void GLController::SetCulling (uint val)
{
	if (mCulling != val)
	{
		mTechnique = 0;
		mCulling = val;

		if (mCulling == Culling::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glCullFace( (mCulling == Culling::Front) ? GL_FRONT : GL_BACK );
			glEnable(GL_CULL_FACE);
		}
	}
}

//============================================================================================================
// Alpha testing will discard fragments with alpha less than the specified value
//============================================================================================================

void GLController::SetADT (float val)
{
	if (mAlphaTest && Float::IsNotEqual(mAdt, val))
	{
		mMaterial = (const IMaterial*)(-1);
		glAlphaFunc(GL_GREATER, mAdt = val);
	}
}

//============================================================================================================
// Sets the point/line thickness
//============================================================================================================

void GLController::SetThickness (float val)
{
	val = Float::Clamp(val, 1.0f, 10.0f);

	if (mThickness != val)
	{
		mThickness = val;

		glPointSize(val);
		glLineWidth(val);

		if (val == 1.0f)
		{
			glDisable(GL_POINT_SMOOTH);
			glDisable(GL_LINE_SMOOTH);
		}
		else
		{
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_LINE_SMOOTH);
		}
	}
}

//============================================================================================================
// Whether to automatically normalize normals
//============================================================================================================

void GLController::SetNormalize (bool val)
{
	if ( mNormalize != val )
	{
		if ((mNormalize = val) && (mShader == 0)) glEnable(GL_RESCALE_NORMAL);
		else glDisable(GL_RESCALE_NORMAL);
	}
}

//============================================================================================================
// Whether to offset depth testing of rendered geometry
//============================================================================================================

void GLController::SetDepthOffset (bool val)
{
	if ( mDepthOffset != val )
	{
		if (mDepthOffset = val)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, -1);
		}
		else glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

//============================================================================================================
// Changes the viewport dimensions
//============================================================================================================

void GLController::SetViewport(const Vector2i& size)
{
	if (mSize != size)
	{
		mSize = size;
		glViewport(0, 0, mSize.x, mSize.y);
		mProj.mRecalculate = true;
		_ProjHasChanged();
	}
}

//============================================================================================================
// Sets the fog starting and ending distance
//============================================================================================================

void GLController::SetFogRange (const Vector2f& range)
{
	mFogRange = range;
	glFogf(GL_FOG_START, range.x);
	glFogf(GL_FOG_END, range.y);
	//glFogi(GL_FOG_MODE, GL_LINEAR);
}

//============================================================================================================
// Changes the clear and the fog color in one call
//============================================================================================================

void GLController::SetBackgroundColor(const Color4f& color)
{
	mBackground = color;
	glFogfv(GL_FOG_COLOR, mBackground);
}

//============================================================================================================
// Whether current active color defines the material colors
//============================================================================================================

void GLController::SetSimpleMaterial (bool val)
{
	if ( mSimpleMaterial != val )
	{
		if (mSimpleMaterial = val) glEnable(GL_COLOR_MATERIAL);
		else glDisable(GL_COLOR_MATERIAL);
	}
}

//============================================================================================================
// Retrieves the model (world) matrix
//============================================================================================================

const Matrix43& GLController::GetModelMatrix()
{
	if (!mModel.mOverwritten && mModel.mRecalculate)
	{
		mModel.mRecalculate = false;
		mModel.mMat.SetToIdentity();
	}
	return mModel.mMat;
}

//============================================================================================================
// Retrieves the view matrix
//============================================================================================================

const Matrix43& GLController::GetViewMatrix()
{
	if (mView.mRecalculate)
	{
		mView.mRecalculate = false;
		mView.mMat.SetToView(mEye, mDir, mUp);
	}
	return mView.mMat;
}

//============================================================================================================
// Retrieves the projection matrix
//============================================================================================================

const Matrix44& GLController::GetProjectionMatrix()
{
	if (mProj.mRecalculate)
	{
		mProj.mRecalculate = false;
		Vector2i size ( (mTarget != 0) ? mTarget->GetSize() : mSize );
		mProj.mMat.SetToProjection( mFov, (float)size.x / size.y, mClipRange.x, mClipRange.y );
	}
	return mProj.mMat;
}

//============================================================================================================
// Retrieves the Model * View matrix
//============================================================================================================

const Matrix43& GLController::GetModelViewMatrix()
{
	// If the ModelView matrix has been overwritten, use it as-is
	if (mMV.mOverwritten) return mMV.mMat;

	// If the model matrix hasn't been set, ModelView matrix is the view matrix
	if (!mModel.mOverwritten) return GetViewMatrix();

	// Recalculate the ModelView matrix if need be
	if (mMV.mRecalculate)
	{
		mMV.mRecalculate = false;
		mMV.mMat = GetModelMatrix() * GetViewMatrix();
	}
	return mMV.mMat;
}

//============================================================================================================
// Retrieves the View * Projection matrix used for frustum culling
//============================================================================================================

const Matrix44& GLController::GetModelViewProjMatrix()
{
	if (mMVP.mRecalculate)
	{
		mMVP.mRecalculate = false;
		mMVP.mMat = GetModelViewMatrix() * GetProjectionMatrix();
	}
	return mMVP.mMat;
}

//============================================================================================================
// Retrieves the inverse view matrix used in converting view space to world space
//============================================================================================================

const Matrix43& GLController::GetInverseModelViewMatrix()
{
	if (mIMV.mRecalculate)
	{
		mIMV.mRecalculate = false;
		mIMV.mMat = GetModelViewMatrix();
		mIMV.mMat.Invert();
	}
	return mIMV.mMat;
}

//============================================================================================================
// Retrieves the inverse projection matrix used in converting screen space to view space
//============================================================================================================

const Matrix44& GLController::GetInverseProjMatrix()
{
	if (mIP.mRecalculate)
	{
		mIP.mRecalculate = false;
		mIP.mMat = GetProjectionMatrix();
		mIP.mMat.Invert();
	}
	return mIP.mMat;
}

//============================================================================================================
// Retrieves (and updates if necessary) the inverse ModelView * Projection matrix used in converting 2D to 3D
//============================================================================================================

const Matrix44& GLController::GetInverseMVPMatrix()
{
	if (mIMVP.mRecalculate)
	{
		mIMVP.mRecalculate = false;
		mIMVP.mMat = GetModelViewProjMatrix();
		mIMVP.mMat.Invert();
	}
	return mIMVP.mMat;
}

//============================================================================================================
// Sets the model matrix to the specified value
//============================================================================================================

void GLController::SetModelMatrix (const Matrix43& mat)
{
	mModel.mMat			= mat;
	mModel.mRecalculate	= false;
	mModel.mOverwritten	= true;
	mMV.mOverwritten	= false;
	_ViewHasChanged();
}

//============================================================================================================
// Resets the model matrix to identity
//============================================================================================================

void GLController::ResetModelMatrix()
{
	if (mModel.mOverwritten)
	{
		mModel.mOverwritten	= false;
		mModel.mRecalculate = true;
		_ViewHasChanged();
	}
}

//============================================================================================================
// Changes the view matrix to the specified value
//============================================================================================================

void GLController::SetModelViewMatrix (const Matrix43& mat)
{
	// ModelView now has a valid value and it doesn't need to be recalculated
	mMV.mMat			= mat;
	mMV.mOverwritten	= true;
	mMV.mRecalculate	= false;

	// The view has changed
	mResetView			= true;
	mMVP.mRecalculate	= true;
	mIMV.mRecalculate	= true;
	mIMVP.mRecalculate	= true;
}

//============================================================================================================
// Resets the view matrix to camera's POV
//============================================================================================================

void GLController::ResetModelViewMatrix()
{
	if (mMV.mOverwritten)
	{
		mMV.mOverwritten = false;
		mMV.mRecalculate = true;
		_ViewHasChanged();
	}

	// Ensure that the model matrix is reset to identity
	ResetModelMatrix();
}

//============================================================================================================
// Changes the ModelView matrix
//============================================================================================================

void GLController::SetCameraOrientation (const Vector3f& eye, const Vector3f& dir, const Vector3f& up)
{
	if (mEye != eye	|| mDir != dir || mUp != up)
	{
		mEye = eye;
		mDir = dir;
		mUp	 = up;

		// View matrix needs to be recalculated
		mView.mRecalculate = true;
		_ViewHasChanged();
	}

	// Ensure that the model matrix is reset to identity
	ResetModelMatrix();
}

//============================================================================================================
// Changes the far clipping plane
//============================================================================================================

void GLController::SetCameraRange (const Vector3f& range)
{
	// Ignore stupid values
	if (range.y > 1.0f)
	{
		// Minimum allowed precision is 10000:1
		Vector2f copy (range);
		float minNear = copy.y * 0.0001f;
		if (copy.x < minNear) copy.x = minNear;

		if ( mClipRange != copy )
		{
			mClipRange = copy;
			mProj.mRecalculate = true;
			_ProjHasChanged();
		}
	}

	float deg = Float::Clamp(range.z, 1.0f, 180.0f);

	if ( Float::IsNotZero(mFov - deg) )
	{
		mFov = deg;
		mProj.mRecalculate = true;
		_ProjHasChanged();
	}
}

//============================================================================================================
// Sets the active camera properties
//============================================================================================================

void GLController::SetActiveRenderTarget (const IRenderTarget* tar)
{
	// If the target is changing...
	if (mTarget != tar)
	{
		if (tar)
		{
			// If the target has been specified, we'll be rendering to an off-screen buffer
			Vector2i activeSize ( mTarget ? mTarget->GetSize() : mSize );
			const Vector2i& size ( tar->GetSize() );

			// Activate it
			tar->Activate();

			// If the size is different from what's currently active, adjust the viewport
			if ( activeSize != size )
			{
				glViewport(0, 0, size.x, size.y);
				mProj.mRecalculate = true;
				_ProjHasChanged();
			}
		}
		// Should be rendering to screen, but a target is currently active
		else if (mTarget)
		{
			// Deactivate the off-screen target
			mTarget->Deactivate();

			// If the size is changing, adjust the viewport
			if ( mSize != mTarget->GetSize() )
			{
				glViewport(0, 0, mSize.x, mSize.y);
				mProj.mRecalculate = true;
				_ProjHasChanged();
			}
		}
		CHECK_GL_ERROR;

		// Update the target
		mTarget = tar;
	}
}

//============================================================================================================
// Activates the specified render group
//============================================================================================================

void GLController::SetActiveTechnique (const ITechnique* ptr, bool insideOut)
{
	if (ptr != 0)
	{
		byte culling = ptr->GetCulling();

		if (insideOut)
		{
			// If inside-out mode was requested, flip culling
			if		(culling == Culling::Front)	culling = Culling::Back;
			else if (culling == Culling::Back)	culling = Culling::Front;
		}

		if (mTechnique == ptr)
		{
			// Same group as last time, but culling might be inverted
			if (mCulling != culling)
			{
				SetCulling(culling);
			}

		}
		// If the technique is changing
		else if (mTechnique != ptr)
		{
			// Activate all the new states
			SetFog			( ptr->GetFog()			);
			SetDepthWrite	( ptr->GetDepthWrite()	);
			SetDepthTest	( ptr->GetDepthTest()	);
			SetColorWrite	( ptr->GetColorWrite()	);
			SetAlphaTest	( ptr->GetAlphaTest()	);
			SetWireframe	( ptr->GetWireframe()	);
			SetLighting		( ptr->GetLighting()	);
			SetBlending		( ptr->GetBlending()	);
			SetCulling		( culling				);

			// Invalidate any active material
			mMaterial = (const IMaterial*)(-1);
			++mStats.mTechSwitches;
		}
	}
	mTechnique = ptr;
}

//============================================================================================================
// Activates the specified material
//============================================================================================================

bool GLController::SetActiveMaterial (const IMaterial* ptr)
{
	if (mMaterial != ptr)
	{
		static uint maxIU = _CountImageUnits();

		// If the material is invisible under the current technique, consider options to be invalid
		const IMaterial::DrawMethod* ren = ( (ptr != 0 && mTechnique != 0) ?
			ptr->GetVisibleMethod(mTechnique) : 0 );

		if (ren == 0)
		{
			for (uint i = maxIU; i > 0; )
				SetActiveTexture(--i, 0);

			SetADT(0.003921568627451f);
			SetActiveShader(0);
			SetActiveColor( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
			CHECK_GL_ERROR;
			return false;
		}
		else
		{
			const Color& diff = ptr->GetDiffuse();
			const Color& spec = ptr->GetSpecular();
			const float  glow = ptr->GetGlow();

			// Shininess should be clamped between 1 and 128
			int shininess = Float::RoundToInt(spec.GetColor4f().a * 128.0f);
			if		(shininess < 1)		shininess = 1;
			else if (shininess > 128)	shininess = 128;

			if (mSimpleMaterial)
			{
				// Disable simple material
				SetSimpleMaterial(false);

				// Update the saved material properties
				mMatDiff = diff;
				mMatSpec = spec;
				mMatGlow = glow;

				Color4f emis (diff.GetColor4f() * glow);
				emis.a = glow;

				// Set all material properties
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
				glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
				glMateriali (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
			}
			else
			{
				if (mMatDiff != diff)
				{
					// Diffuse color has changed -- it affects emissive color as well
					mMatDiff = diff;
					mMatGlow = glow;

					Color4f emis (diff.GetColor4f() * glow);
					emis.a = glow;

					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff);
					glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
				}
				else if ( Float::IsNotEqual(mMatGlow, glow) )
				{
					// Only glow has changed -- affects the emission color
					mMatGlow = glow;

					Color4f emis (diff.GetColor4f() * glow);
					emis.a = glow;

					glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
				}

				if (mMatSpec != spec)
				{
					// Specular color has changed -- specular color also sets shininess
					mMatSpec = spec;
					glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
					glMateriali (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
				}
			}

			// Materials have their own ADT (alpha discard threshold)
			SetADT( ptr->GetADT() * diff.GetColor4f().a );
			CHECK_GL_ERROR;

			// Retrieve all textures used by the material
			const IMaterial::Textures& textures (ren->GetAllTextures());
			uint lastTex = textures.GetSize();
			if (lastTex > maxIU) lastTex = maxIU;

			// Activate all textures
			textures.Lock();
			{
				for (uint i = maxIU; i > 0; )
				{
					const ITexture* tex = (--i < lastTex) ? textures[i] : 0;
					SetActiveTexture(i, tex);
				}
			}
			textures.Unlock();

			// Activate the shader
			SetActiveShader( ren->GetShader() );
		}

		// Remember the currently active material
		mMaterial = ptr;
	}
	return true;
}

//============================================================================================================
// Activates this texture on Texture Unit 0, and disables all other texture units
//============================================================================================================

bool GLController::SetActiveMaterial (const ITexture* ptr)
{
	static uint count = _CountImageUnits();

	for (uint i = count; i > 1; )
		SetActiveTexture(--i, 0);

	SetActiveTexture(0, ptr);
	mMaterial = (const IMaterial*)(-1);
	return (ptr != 0 && ptr->IsValid());
}

//============================================================================================================
// Changes the currently active shader
//============================================================================================================

uint GLController::SetActiveShader (const IShader* ptr, bool forceUpdateUniforms)
{
	IShader::ActivationResult result;

	// Which shader is currently active is kept inside the Shader.cpp file,
	// so we don't check for inequality here.
	if (ptr != 0)
	{
		mMaterial = (const IMaterial*)(-1);
		mShader = ptr;
		result = mShader->Activate(mActiveLightCount, forceUpdateUniforms);
		CHECK_GL_ERROR;
		if (!result.mReused) ++mStats.mShaderSwitches;
	}
	else if (mShader != 0)
	{
		mShader->Deactivate();
		mShader = 0;
		mMaterial = (const IMaterial*)(-1);
	}
	return result.mCount;
}

//============================================================================================================
// Changes the active color
//============================================================================================================

void GLController::SetActiveColor (const Color& color)
{
	SetSimpleMaterial(true);

	if (mMatDiff != color)
	{
		mMaterial = (const IMaterial*)(-1);
		mMatDiff = color;
		glColor4fv(color.GetColor4f());
	}
}

//============================================================================================================
// Changes the camera projection and updates the matrices
//============================================================================================================

void GLController::SetActiveProjection (uint projection)
{
	if (mProjection != projection)
	{
		mProjection = projection;

		if ( projection == Projection::Orthographic )
		{
			SetActiveVertexAttribute(Attribute::Normal, 0, 0, 0, 0, 0);
		}

		mResetProj = true;
		mResetView = true;
	}
}

//============================================================================================================
// Activates the vertex buffer object
//============================================================================================================

void GLController::SetActiveVBO (const IVBO* ptr, uint type)
{
	if (ptr != 0)
	{
		GLVBO::Activate( ptr->GetID(), ptr->GetType() );
	}
	else
	{
		GLVBO::Activate(0, type);
	}
}

//============================================================================================================
// Sets the active texture on the specified texture unit
//============================================================================================================

const uint convertTextureTypeToGL [5] = { 0, GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

void GLController::SetActiveTexture ( uint textureUnit, const ITexture* tex )
{
	static uint maxIU = _CountImageUnits();

	if (textureUnit < maxIU)
	{
		TextureUnit& tu ( mTu[textureUnit] );
		mMaterial = (const IMaterial*)(-1);

		uint glType (0);

		if (tex != 0 && tex->IsValid())
		{
			uint type = tex->GetType();
			ASSERT(type < 5, "Invalid texture type passed");
			glType = convertTextureTypeToGL[type];
		}

		if ( tu.mType != glType )
		{
			if (_SetActiveTextureUnit(textureUnit))
			{
				if ( tu.mType != 0 )
				{
					// If the type is changing, the old type needs to be disabled first (going from CUBE to 2D for example)
					glBindTexture( tu.mType, 0 );
					glDisable( tu.mType );
					tu.mType = 0;
					tu.mTex = 0;
				}

				if ( glType != 0 )
				{
					// If there's a valid new texture type, enable the texture unit and bind the texture
					if (tu.mTex != tex)
					{
						tu.mTex = tex;
						tu.mType = glType;
						glEnable( glType );
						glBindTexture( tu.mType, tex->GetTextureID() );
						CHECK_GL_ERROR;
						++mStats.mTexSwitches;
					}
				}
				else if ( textureUnit < 8 )
				{
					BufferEntry& buffer = mBuffers[ Attribute::TexCoord0 + textureUnit ];

					if (buffer.mEnabled)
					{
						SetActiveVertexAttribute( Attribute::TexCoord0 + textureUnit, 0, 0, 0, 0, 0 );
					}
				}
			}
		}
		else if ( glType )
		{
			if (tu.mTex != tex && _SetActiveTextureUnit(textureUnit))
			{
				// Only the texture ID changed
				tu.mTex = tex;
				glBindTexture( tu.mType, tex->GetTextureID() );
				CHECK_GL_ERROR;
				++mStats.mTexSwitches;
			}
		}
	}
}

//============================================================================================================
// Activates and/or changes light properties for the specified light
//============================================================================================================

void GLController::SetActiveLight (uint index, const Light* ptr)
{
	CHECK_GL_ERROR;

	if (mLu.IsEmpty())
	{
		ASSERT( g_caps.mMaxLights > 0, "Could not retrieve the maximum number of lights" );
		mLu.ExpandTo( g_caps.mMaxLights );
		for (uint i = 0; i < mLu.GetSize(); ++i)
			mLu[i] = false;
	}

	if (index < mLu.GetSize())
	{
		bool& active (mLu[index]);
		index += GL_LIGHT0;

		if (ptr == 0)
		{
			if (active)
			{
				--mActiveLightCount;
				active = false;
				glDisable(index);
			}
		}
		else
		{
			// Activate the matrices as they will affect the lights
			_ActivateMatrices();

			// Activate the light
			if (!active)
			{
				++mActiveLightCount;
				active = true;
				glEnable(index);
			}

			if (ptr->mType == Light::Type::Directional)
			{
				Vector3f dir (ptr->mDir);

				// Automatically adjust the light position to always be in view space
				if (mProjection == Projection::Orthographic)
					dir %= GetModelViewMatrix();

				// Set the light's position
				glLightfv(index, GL_POSITION, Quaternion(-dir.x, -dir.y, -dir.z, 0.0f));
			}
			else
			{
				Vector3f pos (ptr->mPos);

				// Automatically adjust the light position to always be in view space
				if (mProjection == Projection::Orthographic)
					pos *= GetModelViewMatrix();

				// Set the light's position
				glLightfv(index, GL_POSITION, Quaternion(pos.x, pos.y, pos.z, 1.0f));

				// Point lights are marked by having a cutoff of 180
				if (ptr->mType == Light::Type::Point)
				{
					glLightf(index, GL_SPOT_CUTOFF, 180.0f);
				}
				//else
				//{
				//	glLightfv(index, GL_SPOT_DIRECTION, ptr->mDir);
				//	glLightf(index, GL_SPOT_EXPONENT, Float::Clamp(ptr->mSpot.x, 0.0f, 128.0f));
				//	glLightf(index, GL_SPOT_CUTOFF, Float::Clamp(ptr->mSpot.y, 0.0f, 90.0f));
				//}

				// Shaders perform completely different operations with the attenuation parameters
				// As such, point lights are not supported using the fixed-function pipeline.
				glLightf(index, GL_CONSTANT_ATTENUATION,	ptr->mAtten.x);
				glLightf(index, GL_LINEAR_ATTENUATION,		ptr->mAtten.y);
				glLightf(index, GL_QUADRATIC_ATTENUATION,	ptr->mAtten.z);
			}

			// Common light parameters
			glLightfv(index, GL_AMBIENT,  ptr->mAmbient);
			glLightfv(index, GL_DIFFUSE,  ptr->mDiffuse);
			glLightfv(index, GL_SPECULAR, ptr->mSpecular);

			++mStats.mLightSwitches;
		}
	}
	CHECK_GL_ERROR;
}

//============================================================================================================
// Sets the active depth buffer comparison function
//============================================================================================================

void GLController::SetActiveDepthFunction (uint condition)
{
#ifdef _DEBUG
	if (condition < 8) glDepthFunc(ConvertCondition[condition]);
	else ASSERT(false, "Invalid condition -- use the IGraphics::Condition values");
	CHECK_GL_ERROR;
#else
	glDepthFunc(ConvertCondition[condition]);
#endif
}

//============================================================================================================
// Set the active stencil buffer function
//============================================================================================================

void GLController::SetActiveStencilFunction (uint condition, uint val, uint mask)
{
#ifdef _DEBUG
	if (condition < 8) glStencilFunc(ConvertCondition[condition], val, mask);
	else ASSERT(false, "Invalid condition -- use the IGraphics::Condition values");
	CHECK_GL_ERROR;
#else
	glStencilFunc(ConvertCondition[condition], val, mask);
#endif
}

//============================================================================================================
// Set the active stencil buffer operation
//============================================================================================================

void GLController::SetActiveStencilOperation (uint testFail, uint depthFail, uint pass)
{
	if (testFail < 6 && depthFail < 6 && pass < 6)
	{
		glStencilOp(ConvertOperation[testFail], ConvertOperation[depthFail], ConvertOperation[pass]);
	}
#ifdef _DEBUG
	else ASSERT(false, "Invalid argument -- use the IGraphics::Operation values");
	CHECK_GL_ERROR;
#endif
}

//============================================================================================================
// Sets the currently active vertex buffer
//============================================================================================================
// Short note about this function: Attribute arrays can be used everywhere instead of Client State calls
// if the shaders are used. If they are not however, position must be specified via glVertexPointer.
// Also, attribute array calls require OpenGL 2.0, so for the sake of compatibility client state calls
// are used instead of attribute arrays whenever possible.
//============================================================================================================

void GLController::SetActiveVertexAttribute(
	uint		attribute,
	const IVBO*	vbo,
	const void*	ptr,
	uint		dataType,
	uint		elements,
	uint		stride )
{
	ASSERT(attribute < 16, "Invalid attribute");
	BufferEntry& buffer = mBuffers[attribute];

	bool changed = false;

	if ( ptr != 0 || (vbo != 0 && vbo->IsValid()) )
	{
		if ( !buffer.mEnabled )
		{
			switch (attribute)
			{
				case Attribute::Position:	glEnableClientState(GL_VERTEX_ARRAY);			break;
				case Attribute::Normal:		glEnableClientState(GL_NORMAL_ARRAY);			break;
				case Attribute::Color:		glEnableClientState(GL_COLOR_ARRAY);			break;
				case Attribute::TexCoord0:	glEnableClientState(GL_TEXTURE_COORD_ARRAY);	break;
				default:
				{
					if (glEnableVertexAttribArray != 0 && glVertexAttribPointer != 0)
					{
						glEnableVertexAttribArray(attribute);
						break;
					}
					return;
				}
			};

			changed = true;
		}
		else
		{
			changed = (buffer.mVbo != vbo) || (buffer.mPtr != ptr);
		}

		if (changed)
		{
			buffer.mEnabled = true;
			SetActiveVBO( buffer.mVbo = vbo, IVBO::Type::Vertex );
			++mStats.mBufferBinds;

			switch (attribute)
			{
				case Attribute::Position:	glVertexPointer			(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::Normal:		glNormalPointer			(						dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::Color:		glColorPointer			(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				case Attribute::TexCoord0:	glTexCoordPointer		(			 elements,	dataType,    stride, buffer.mPtr = ptr);	break;
				default:					glVertexAttribPointer	(attribute,  elements,	dataType, 0, stride, buffer.mPtr = ptr);	break;
			};
		}
		CHECK_GL_ERROR;
	}
	else if ( buffer.mEnabled )
	{
		buffer.mEnabled = false;

		switch (attribute)
		{
			case Attribute::Position:	glDisableClientState(GL_VERTEX_ARRAY);			break;
			case Attribute::Normal:		glDisableClientState(GL_NORMAL_ARRAY);			break;
			case Attribute::Color:		glDisableClientState(GL_COLOR_ARRAY);			break;
			case Attribute::TexCoord0:	glDisableClientState(GL_TEXTURE_COORD_ARRAY);	break;
			default:					glDisableVertexAttribArray(attribute);			break;
		};
		CHECK_GL_ERROR;
	}
}

//============================================================================================================
// Draw the active buffers using an index buffer
//============================================================================================================

uint GLController::DrawVertices(uint primitive, uint vertexCount)
{
	uint glPrimitive(0), triangleCount(0);
	GetGLPrimitive(primitive, vertexCount, glPrimitive, triangleCount);

	if (triangleCount > 0)
	{
		// Activate the matrices
		_ActivateMatrices();

		// Draw the arrays
		glDrawArrays( glPrimitive, 0, vertexCount );
		mStats.mTriangles += triangleCount;
		++mStats.mDrawCalls;
	}
	return triangleCount;
}

//============================================================================================================
// Draw using an index array
//============================================================================================================

uint GLController::_DrawIndices(const IVBO* vbo, const ushort* ptr, uint primitive, uint indexCount)
{
	uint glPrimitive(0), triangleCount(0);
	GetGLPrimitive(primitive, indexCount, glPrimitive, triangleCount);

	if (triangleCount > 0)
	{
		// Activate the VBO, if any
		SetActiveVBO( vbo, IVBO::Type::Index );

		// Activate the matrices
		_ActivateMatrices();

		// Draw the indices
		glDrawElements( glPrimitive, indexCount, GL_UNSIGNED_SHORT, ptr );
		mStats.mTriangles += triangleCount;
		++mStats.mDrawCalls;
	}
	return triangleCount;
}

//============================================================================================================
// Activate all appropriate matrices
//============================================================================================================

void GLController::_ActivateMatrices()
{
	if (mResetView || mResetProj)
	{
		if (mProjection == Projection::Orthographic)
		{
			Vector2i activeSize ( mTarget ? mTarget->GetSize() : mSize );
			Matrix43 mat( activeSize.x, activeSize.y );

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mat.mF);
			++mStats.mMatSwitches;
		}
		else
		{
			bool reset (false);

			if (mResetProj)
			{
				reset = true;
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(GetProjectionMatrix().mF);
				++mStats.mMatSwitches;
			}

			if (mResetView)
			{
				// If the active shader supports pseudo-instancing, take advantage of that
				if (mShader != 0 && mShader->GetFlag(IShader::Flag::Instanced))
				{
					const Matrix43& mat = GetModelMatrix();

					glMultiTexCoord4fv(GL_TEXTURE2, mat.mColumn0);
					glMultiTexCoord4fv(GL_TEXTURE3, mat.mColumn1);
					glMultiTexCoord4fv(GL_TEXTURE4, mat.mColumn2);
					glMultiTexCoord4fv(GL_TEXTURE5, mat.mColumn3);
				}
				else
				{
					glMatrixMode(GL_MODELVIEW);
					glLoadMatrixf(GetModelViewMatrix().mF);
					++mStats.mMatSwitches;
					reset = false;
				}
			}
			
			if (reset)
			{
				// Always end with ModelView
				glMatrixMode(GL_MODELVIEW);
			}
		}
		mResetView = false;
		mResetProj = false;
	}
}

//============================================================================================================
// Changes the currently active texture unit
//============================================================================================================

bool GLController::_SetActiveTextureUnit( uint textureUnit )
{
	static uint maxTU = glGetInteger(GL_MAX_TEXTURE_UNITS);
	static uint maxIU = _CountImageUnits();

	if ( textureUnit < maxIU )
	{
		textureUnit += GL_TEXTURE0;

		if ( mActiveTU != textureUnit )
		{
			glActiveTexture(mActiveTU = textureUnit);
			if (mActiveTU < maxTU) glActiveClientTexture(mActiveTU);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Marks all view-affected matrices as needing to be recalculated
//============================================================================================================

void GLController::_ViewHasChanged()
{
	mResetView			= true;
	mMV.mRecalculate	= true;
	mMVP.mRecalculate	= true;
	mIMV.mRecalculate	= true;
	mIMVP.mRecalculate	= true;
}

//============================================================================================================
// Marks all projection-affected matrices as needing to be recalculated
//============================================================================================================

void GLController::_ProjHasChanged()
{
	mResetProj			= true;
	mIP.mRecalculate	= true;
	mMVP.mRecalculate	= true;
	mIMVP.mRecalculate	= true;
}