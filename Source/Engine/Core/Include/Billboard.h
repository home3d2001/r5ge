#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Simple screen-aligned billboard
// Author: Michael Lyashenko
//============================================================================================================

class Billboard : public Object
{
protected:

	Color4ub			mColor;		// Color tint
	const ITexture*		mTex;		// Texture used to draw the Billboard
	const ITechnique*	mTech;		// Technique used by the billboard

	// Objects should never be created manually. Use the AddObject<> template instead.
	Billboard() : mColor(0xFFFFFFFF), mTex(0), mTech(0) { mCalcAbsBounds = false; }

public:

	R5_DECLARE_INHERITED_CLASS(Billboard, Object, Object);

	const Color4ub&		GetColor()		const	{ return mColor;	}
	const ITexture*		GetTexture()	const	{ return mTex;		}
	const ITechnique*	GetTechnique()	const	{ return mTech;		}

	void SetColor		(const Color4ub& color)	{ mColor = color;	}
	void SetTexture		(const ITexture* tex)	{ mTex = tex;		}
	void SetTechnique	(const ITechnique* tech){ mTech = tech;		}

protected:

	virtual void OnUpdate();
	virtual bool OnFill (FillParams& params);
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);

	// Draws the actual billboard reusing the common buffers
	void DrawBillboard();

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};