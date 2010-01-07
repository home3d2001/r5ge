#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// 32-bit color
//============================================================================================================

struct Vector3f;
struct Color4ub
{
	union
	{
		uint mVal;
		struct { byte r, g, b, a; };
	};

	Color4ub()													: mVal(0) {}
	Color4ub(float val)											: mVal(0) { PackFloat(val); }
	Color4ub(byte R,
			 byte G,
			 byte B,
			 byte A = 255)										: r(R), g(G), b(B), a(A) {}
	Color4ub(int rgba)											: mVal((uint)rgba) {}
	Color4ub(uint rgba)											: mVal(rgba) {}
	Color4ub(const Color4ub& c)									{ mVal = c.mVal;	}
	Color4ub(const Color4f& c)									{ r = Float::ToRangeByte(c.r);
																  g = Float::ToRangeByte(c.g);
																  b = Float::ToRangeByte(c.b);
																  a = Float::ToRangeByte(c.a);	}

	Color4ub(const Color3f& c, float alpha = 1.0f)				{ r = Float::ToRangeByte(c.r);
																  g = Float::ToRangeByte(c.g);
																  b = Float::ToRangeByte(c.b);
																  a = Float::ToRangeByte(alpha); }

	Color4ub(const Color4f& c, float alpha)						{ r = Float::ToRangeByte(c.r);
																  g = Float::ToRangeByte(c.g);
																  b = Float::ToRangeByte(c.b);
																  a = Float::ToRangeByte(c.a*alpha); }
	Color4ub(const Vector3f& v)									{ *this = v; }

			byte& operator [] (uint i)							{ return (&r)[i]; }
	const	byte& operator [] (uint i) const					{ return (&r)[i]; }

	operator uint&()											{ return mVal; }
	uint GetRGB() const											{ return mVal & 0xFFFFFF; }
	
	float GetAlpha() const										{ return Float::FromRangeByte(a); }
	void  SetAlpha(float val)									{ a = Float::ToRangeByte(val); }

	void operator = (float val)									{ PackFloat(val); }
	void operator = (const Vector3f& v);						// Inlined in Vector3f.h
	void operator = (int val)									{ mVal = (uint)val; }
	void operator = (uint val)									{ mVal = val;	}
	void operator = (const Color3f& c)							{ r = Float::ToRangeByte(c.r);
																  g = Float::ToRangeByte(c.g);
																  b = Float::ToRangeByte(c.b);
																  a = 255; }
	void operator = (const Color4ub& c)							{ mVal = c.mVal; }
	void operator = (const Color4f& c)							{ r = Float::ToRangeByte(c.r);
																  g = Float::ToRangeByte(c.g);
																  b = Float::ToRangeByte(c.b);
																  a = Float::ToRangeByte(c.a);	}

	void     operator *=(float f)								{ Set((byte)(f * r), (byte)(f * g), (byte)(f * b));				}
	Color4ub operator  *(float f) const							{ return Color4ub((byte)(f * r), (byte)(f * g), (byte)(f * b));	}
	Color4ub operator  *(const Color4ub& v) const				{ return Color4ub(r * v.r, g * v.g, b * v.b, a * v.a);			}
	bool	 operator ==(const Color4ub& c) const				{ return c.mVal == mVal; }
	bool	 operator !=(const Color4ub& c) const				{ return c.mVal != mVal; }
	bool	 operator ==(uint iValue) const						{ return iValue == mVal; }
	bool	 operator !=(uint iValue) const						{ return iValue != mVal; }
	bool	 operator ==(int iValue) const						{ return (uint)iValue == mVal; }
	bool	 operator !=(int iValue) const						{ return (uint)iValue != mVal; }
	
	void SetToAverage (const Color4ub& c1, const Color4ub& c2)	{ r = ((ushort)c1.r + c2.r) / 2;
																  g = ((ushort)c1.g + c2.g) / 2;
																  b = ((ushort)c1.b + c2.b) / 2;
																  a = ((ushort)c1.a + c2.a) / 2; }

	void Set (byte R, byte G, byte B, byte A = 255)	{ r = R;  g = G;  b = B;  a = A; }

	// Sets the color based on the specified text value in FFFFFFFF format
	void SetByHexString (const char* text, uint length = 8);

	void PackFloat (float val);	// Packs a float into 4-component RGBA
	float UnpackFloat() const;	// Unpacks a previously packed float
};

//============================================================================================================
// Convenience conversion functions
//============================================================================================================

inline Color4f::Color4f(const Color4ub& c)
{
	r = 0.00392156862f * c.r;
	g = 0.00392156862f * c.g;
	b = 0.00392156862f * c.b;
	a = 0.00392156862f * c.a;
}

inline void Color4f::operator =(const Color4ub& c)
{
	r = 0.00392156862f * c.r;
	g = 0.00392156862f * c.g;
	b = 0.00392156862f * c.b;
	a = 0.00392156862f * c.a;
}