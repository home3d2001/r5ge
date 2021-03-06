#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Object point light
// Author: Michael Lyashenko
//============================================================================================================

class PointLight : public LightSource
{
protected:

	Color3f		mAmbient;		// Ambient color
	Color3f		mDiffuse;		// Diffuse color
	float		mBrightness;	// Light's brighness
	float		mRange;			// Range of the light
	float		mPower;			// Attenuation power
	IShader*	mShader0;
	IShader*	mShader1;

	PointLight();

	// Updates appropriate fields in 'mParams'
	void _UpdateColors();
	void _UpdateAtten();

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS(PointLight, Object, Object);

	const Color3f&	GetAmbient()	const { return mAmbient;	}
	const Color3f&	GetDiffuse()	const { return mDiffuse;	}
	float			GetBrightness() const { return mBrightness;	}
	float			GetRange()		const { return mRange;		}
	float			GetPower()		const { return mPower;		}

	void SetAmbient		(const Color3f& c)	{ mAmbient	= c; _UpdateColors(); }
	void SetDiffuse		(const Color3f& c)	{ mDiffuse	= c; _UpdateColors(); }
	void SetBrightness	(float val);
	void SetRange		(float val);
	void SetPower		(float val);

protected:

	// Update the light parameters
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draw the point light
	virtual void OnDrawLight (TemporaryStorage& storage, bool setStates);

protected:

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual bool OnSerializeFrom (const TreeNode& node);
};