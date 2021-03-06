#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep track of the active buffer
//============================================================================================================

uint g_activeBuffer = 0;

//============================================================================================================
// Function that checks whether the FBO is marked as ready
//============================================================================================================

const char* CheckFBO()
{
	// Check the frame buffer's status
	uint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (result != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		switch (result)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			return "Not all framebuffer attachments are in a complete state";

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			return "No attachments were associated with the frame buffer";

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			return "Not all attachments have identical width and height";

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			return "The requested combination of attachments is not supported on your hardware";

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			return "Missing a call to 'glDrawBuffers'";

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			return "Format mismatch";

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			return "Incomplete read buffer. Did you attach a depth with no color? (Mac issue)";
		
		default:
			return "Unknown CheckFBO() error";
		}
	}
	return 0;
}

//============================================================================================================
// Delayed callback executed by the GLGraphics manager (buffers should only be released on the graphics thread)
//============================================================================================================

void DeleteFBO (IGraphicsManager* graphics, void* ptr)
{
	uint fbo = (uint)(ulong)ptr;

	if (g_activeBuffer == fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_activeBuffer = 0);
	}
	glDeleteFramebuffers(1, &fbo);
}

//============================================================================================================
// FBO constructor
//============================================================================================================

GLFBO::GLFBO(IGraphics* graphics) :
	mGraphics	(graphics),
	mFbo		(0),
	mDepthTex	(0),
	mStencilTex	(0),
	mDummyTex	(0),
	mMSAA		(0),
	mUsesSkybox	(true),
	mIsDirty	(true)
{
	ASSERT( g_caps.mMaxFBOAttachments != 0, "Frame Buffer Objects are not supported?" );
	mAttachments.ExpandTo(g_caps.mMaxFBOAttachments);
}

//============================================================================================================
// Releases the resources associated with the FBO
//============================================================================================================

void GLFBO::_InternalRelease (bool delayExecution)
{
	mAttachments.Clear();

	if (mDummyTex != 0)
	{
		mGraphics->DeleteTexture(mDummyTex);
		mDummyTex = 0;
	}
	
	if (mFbo != 0)
	{
		if (delayExecution)
		{
			mGraphics->ExecuteBeforeNextFrame(DeleteFBO, (void*)mFbo);
		}
		else
		{
			// Optimized version for when we are inside the graphics thread
			DeleteFBO(mGraphics, (void*)mFbo);
		}
		mFbo = 0;
	}
}

//============================================================================================================
// Returns the maximum number of color attachments
//============================================================================================================

uint GLFBO::GetMaxColorAttachments() const
{
	return g_caps.mMaxFBOAttachments;
}

//============================================================================================================
// Whether stencil buffer attachments are supported -- only via the GL_EXT_packed_depth_stencil
//============================================================================================================

bool GLFBO::SupportsStencilAttachments() const
{
	return g_caps.mDepthStencil;
}

//============================================================================================================
// Changes the buffer's size
//============================================================================================================

bool GLFBO::SetSize (const Vector2i& size)
{
	if ( mSize != size )
	{
		if ( g_caps.mMaxFBOAttachments == 0 )
		{
			ASSERT(false, "Your videocard does not support Frame Buffer Objects");
			return false;
		}

		if ( size.x < 8 || size.y < 8 )
		{
			ASSERT(false, "The requested size for FBO::SetSize() is too small");
			return false;
		}
		
		if ( size.x > (short)g_caps.mMaxTextureSize ||
			 size.y > (short)g_caps.mMaxTextureSize )
		{
			ASSERT(false, "The requested size for FBO::SetSize() exceeds your hardware's capabilities");
			return false;
		}

		mLock.Lock();
		{
			mSize = size;
			mIsDirty = true;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Attaches a texture to the specified color index
//============================================================================================================

bool GLFBO::AttachColorTexture (uint bufferIndex, ITexture* tex, uint format)
{
	if ( bufferIndex < mAttachments.GetSize() )
	{
		if ( tex != mAttachments[bufferIndex].mTex )
		{
			mLock.Lock();
			{
				TextureEntry& entry (mAttachments[bufferIndex]);
				entry.mTex		= tex;
				entry.mFormat	= format;
				mIsDirty		= true;

				// If this is a valid format (and it really should be...)
				if (format != ITexture::Format::Invalid)
				{
					bool revert = false;

					// If mixed attachments are not supported
					if (!g_caps.mMixedAttachments)
					{
						// Check to see if we have formats that don't match
						for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
						{
							TextureEntry& current (mAttachments[i]);

							if (current.mFormat != format &&
								current.mFormat != ITexture::Format::Invalid)
							{
								revert = true;
								break;
							}
						}
					}

					// If the alpha texture attachments are not supported, we want to revert to RGBA
					if (!revert && !g_caps.mAlphaAttachments && format == ITexture::Format::Alpha)
					{
						revert = true;
					}

					// Revert to RGBA if either mixed format or alpha attachments are not supported
					if (revert)
					{
						for (uint i = 0; i < g_caps.mMaxFBOAttachments; ++i)
						{
							TextureEntry& current (mAttachments[i]);
							if (current.mFormat != ITexture::Format::Invalid)
								current.mFormat  = ITexture::Format::RGBA;
						}
					}
				}
			}
			mLock.Unlock();
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Attaches a depth texture
//============================================================================================================

bool GLFBO::AttachDepthTexture (ITexture* tex)
{
	if (mDepthTex != tex)
	{
		mLock.Lock();
		{
			mIsDirty = true;
			mDepthTex = tex;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Attaches a stencil texture
//============================================================================================================

bool GLFBO::AttachStencilTexture (ITexture* tex)
{
	if (mStencilTex != tex)
	{
		// FBOs only support combined depth+stencil textures. Stand-alone stencil buffers are not supported.
		if ( (!g_caps.mDepthStencil && tex != 0) || (mDepthTex != 0 && mDepthTex != tex) )
			return false;

		// Depth must be attached in order for this to work
		if (mDepthTex == 0) AttachDepthTexture(tex);

		mLock.Lock();
		{
			mIsDirty = true;
			mStencilTex = tex;
		}
		mLock.Unlock();
	}
	return true;
}

//============================================================================================================
// Determines if we have a valid color attachment
//============================================================================================================

bool GLFBO::HasColor() const
{
	FOREACH(i, mAttachments)
	{
		const ITexture* tex ( mAttachments[i].mTex );
		if (tex != 0) return true;
	}
	return false;
}

//============================================================================================================
// Activates (or deactivates) the buffer
//============================================================================================================

void GLFBO::Activate() const
{
	// If the buffer isn't valid, don't bother
	if (mSize == 0) return;
	if (mFbo  == 0)
	{
		glGenFramebuffers(1, &mFbo);
		ASSERT( mFbo != 0, "Failed to create a FBO!" );
	}

	// If this isn't the currently active buffer, bind it
	if (g_activeBuffer != mFbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_activeBuffer = mFbo);
		CHECK_GL_ERROR;
	}

#ifdef _DEBUG
	FOREACH(i, mAttachments)
	{
		ITexture* tex ( mAttachments[i].mTex );

		if (tex != 0 && tex->GetFiltering() > ITexture::Filter::Linear)
		{
			// If this assert gets triggered, then you've changed the filtering of one or more textures
			// attached to this render target to be mip-mapped or anisotropic-filtered (which also generates
			// a mip-map). This is a problem, as the render target will only render into the top-level layer
			// of a mip-mapped texture, ignoring others. 99% of the time you don't want textures attached
			// to a render target to be mip-mapped. In the rare cases that you do, change the texture's
			// filtering to 'nearest' or 'linear' prior to activating the render target, then change it back.

			ASSERT(false, "Probable error: render target's textures can't be mip-mapped!");
		}
	}
#endif

	if (mIsDirty)
	{
		// NOTE: Depth with no color texture causes an issue with some drivers, such as NVidia drivers
		// for Mac OSX. The FBO simply fails to complete. ATI cards seem to support depth-only attachments,
		// but alpha testing doesn't work, which coincidently follows OpenGL's specs:
		// http://www.opengl.org/sdk/docs/man/xhtml/glAlphaFunc.xml
		// Solution? Create a dummy color texture.
		
		if (!HasColor() && (mDepthTex != 0) && !mGraphics->GetDeviceInfo().mDepthAttachments)
		{
			if (mDummyTex == 0)
			{
				mDummyTex = mGraphics->CreateRenderTexture("Dummy Color Texture");
				mDummyTex->SetFiltering(ITexture::Filter::Nearest);
			}
			((GLFBO*)this)->AttachColorTexture(0, mDummyTex, g_caps.mAlphaAttachments ?
				ITexture::Format::Alpha : ITexture::Format::RGBA);
		}

#ifdef GL_ARB_texture_multisample
		// Attachments are always either 2D textures or multi-sampled 2D textures
		bool useMSAA = (mMSAA && g_caps.mMSAA);
		uint type = (useMSAA) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
#else
		bool useMSAA = false;
		uint type = GL_TEXTURE_2D;
#endif

		mLock.Lock();
		{
			mIsDirty = false;

			// Clear mask is passed to glClear()
			uint clearMask = 0;

			mBuffers.Clear();

			// Run through all attachments and prepare their textures
			FOREACH(i, mAttachments)
			{
				const TextureEntry& ent = mAttachments[i];

				if (ent.mTex != 0)
				{
					mBuffers.Expand() = GL_COLOR_ATTACHMENT0_EXT + i;

					uint format = ent.mFormat;

					// Auto-adjust unsupported RGB30A2 textures to be RGBA
					if (!g_caps.mMixedAttachments)
					{
						if (ITexture::Format::RGB30A2 == format &&
							ITexture::Format::RGBA == ent.mTex->GetFormat())
						{
							format = ITexture::Format::RGBA;
						}
					}

					ASSERT(ent.mTex->GetFormat() == ITexture::Format::Invalid || ent.mTex->GetFormat() == format,
						"Changing color attachment's format type... is this intentional?");

					// If the target size has changed, the texture needs to be resized as well
					if (mSize != ent.mTex->GetSize())
					{
						// We will need to clear the render target to ensure that the new texture is clean
						clearMask |= GL_COLOR_BUFFER_BIT;

						if (useMSAA)
						{
							ent.mTex->SetFiltering(ITexture::Filter::Nearest);
						}
						else if (ent.mTex->GetFiltering() > ITexture::Filter::Linear)
						{
							ent.mTex->SetFiltering(ITexture::Filter::Linear);
						}
						ent.mTex->SetWrapMode(ITexture::WrapMode::ClampToEdge);
						
						// Set the size of the texture to match the render target's size
						ent.mTex->Reserve(mSize.x, mSize.y, 1, format, mMSAA);
					}
				}

				// If either current or previous attachments were valid we need to update the FBO.
				// Apparently only updating attachments that change generates a "no attachments" error.
				if (ent.mTex || ent.mActive)
				{
					ent.mActive = ent.mTex;
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT + i, type,
						ent.mTex ? ent.mTex->Activate() : 0, 0);
					CHECK_GL_ERROR;
				}
			}

			// Determine the appropriate attachment formats
			uint depthFormat   = ITexture::Format::Invalid;
			uint stencilFormat = ITexture::Format::Invalid;

			if (mDepthTex != 0)
			{
				if (g_caps.mDepthStencil && mDepthTex == mStencilTex)
				{
					depthFormat	  = ITexture::Format::DepthStencil;
					stencilFormat = ITexture::Format::DepthStencil;
				}
				else
				{
					depthFormat = ITexture::Format::Depth;
				}
			}

			// Attach the depth buffer
			if (depthFormat == ITexture::Format::Invalid)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_EXT, type, 0, 0);
			}
			else
			{
				ASSERT(	mDepthTex->GetFormat() == ITexture::Format::Invalid ||
						mDepthTex->GetFormat() == depthFormat,
						"Changing depth attachment's format type... is this intentional?");
				
				if (mDepthTex->GetSize() != mSize || mDepthTex->GetFormat() != depthFormat)
				{
					clearMask |= GL_DEPTH_BUFFER_BIT;

					mDepthTex->SetFiltering(ITexture::Filter::Nearest);
					mDepthTex->SetWrapMode(useMSAA ? ITexture::WrapMode::ClampToEdge : ITexture::WrapMode::ClampToOne);
					mDepthTex->Reserve(mSize.x, mSize.y, 1, depthFormat, mMSAA);
				}

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_EXT, type, mDepthTex->Activate(), 0);
				CHECK_GL_ERROR;
			}

			// Attach the stencil buffer
			if (stencilFormat == ITexture::Format::Invalid)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT_EXT, type, 0, 0);
			}
			else
			{
				ASSERT(	mStencilTex->GetFormat() == ITexture::Format::Invalid ||
						mStencilTex->GetFormat() == stencilFormat,
						"Changing stencil attachment's format type... is this intentional?");

				if (mStencilTex->GetSize() != mSize || mStencilTex->GetFormat() != stencilFormat)
				{
					clearMask |= GL_STENCIL_BUFFER_BIT;
					mStencilTex->SetFiltering(ITexture::Filter::Nearest);
					mStencilTex->SetWrapMode(useMSAA ? ITexture::WrapMode::ClampToEdge : ITexture::WrapMode::ClampToOne);
					mStencilTex->Reserve(mSize.x, mSize.y, 1, stencilFormat, mMSAA);
				}

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT_EXT, type, mStencilTex->Activate(), 0);
				CHECK_GL_ERROR;
			}

			// If there were attachments to work with, activate them all
			if (mBuffers.IsValid())
			{
				// Specify which buffers will be drawn to
				glDrawBuffers(mBuffers.GetSize(), mBuffers);
				CHECK_GL_ERROR;
			}
			else
			{
				// Color information should be discarded
				glDrawBuffer(GL_NONE);
				CHECK_GL_ERROR;
			}
			
			const char* msg = CheckFBO();

			if (msg == 0)
			{
				// Since textures were reset, everything should be cleared -- just in case.
				// Not doing so causes some very odd artifacts -- took me a day to track this down.
				if (clearMask != 0)
				{
					if ((clearMask & GL_COLOR_BUFFER_BIT) != 0) glClearColor(0, 0, 0, 0);
					glClear(clearMask);
					const Color4f& c = mGraphics->GetBackgroundColor();
					glClearColor(c.r, c.g, c.b, c.a);
					CHECK_GL_ERROR;
				}
#ifdef _DEBUG
				System::Log("[FBO]     Attachments were relinked for FBO #%u", mFbo);
				System::Log("          - Color:   %u/%u", mBuffers.GetSize(), g_caps.mMaxFBOAttachments);
				System::Log("          - Depth:   %u/1", (depthFormat != ITexture::Format::Invalid) ? 1 : 0);
				System::Log("          - Stencil: %u/1", (stencilFormat != ITexture::Format::Invalid) ? 1 : 0);
#endif
			}
			else
			{
				String error;
				error << "Error: ";
				error << msg;
				error << "\n\n";
				error << "Attachments:\n";

				if (mDepthTex != 0)
				{
					error << "Depth: ";
					error << ITexture::FormatToString(mDepthTex->GetFormat());
					error << " - ";
					error << ITexture::FilterToString(mDepthTex->GetFiltering());
					error << " - ";
					error << ITexture::WrapModeToString(mDepthTex->GetWrapMode());
					error << "\n";
				}
				
				FOREACH(i, mAttachments)
				{
					ITexture* tex = mAttachments[i].mTex;
					
					if (tex != 0)
					{
						error << "Color ";
						error << i;
						error << ": ";
						error << ITexture::FormatToString(tex->GetFormat());
						error << " - ";
						error << ITexture::FilterToString(tex->GetFiltering());
						error << " - ";
						error << ITexture::WrapModeToString(tex->GetWrapMode());
						error << "\n";
					}
				}

				ASSERT(false, error.GetBuffer());
			}

		}
		mLock.Unlock();
	}
}

//============================================================================================================
// Deactivates the current frame buffer
//============================================================================================================

void GLFBO::Deactivate() const
{
	if (g_activeBuffer != 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_activeBuffer = 0);
	}
}

//============================================================================================================
// Copy the render target's content into the destination buffer
//============================================================================================================

bool GLFBO::CopyTo (const IRenderTarget* destination, bool color, bool depth, bool stencil) const
{
	if (destination != 0)
	{
		GLFBO* dest = (GLFBO*)destination;

		if (dest->mIsDirty) destination->Activate();

		if (mFbo != 0 && dest->mFbo != 0)
		{
			const Vector2i& ds = destination->GetSize();

			uint mask = 0;

			if (color && HasColor())		mask |= GL_COLOR_BUFFER_BIT;
			if (depth && HasDepth())		mask |= GL_DEPTH_BUFFER_BIT;
			if (stencil && HasStencil())	mask |= GL_STENCIL_BUFFER_BIT;

			if (mask)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->mFbo);
				glBlitFramebuffer(0, 0, mSize.x, mSize.y, 0, 0, ds.x, ds.y, mask, GL_NEAREST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				g_activeBuffer = 0;
				CHECK_GL_ERROR;
				return true;
			}
		}
	}
	return false;
}