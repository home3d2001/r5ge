#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the label's alignment
//============================================================================================================

void UILabel::SetAlignment (char alignment)
{
	if (mAlignment != alignment)
	{
		mAlignment = alignment;
		SetDirty();
	}
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UILabel::OnFill (UIQueue* queue)
{
	if (queue->mLayer != mLayer || queue->mWidget != 0) return;

	const ITexture* tex ( GetTexture() );
	byte height ( GetFontSize() );

	if (tex != 0 && tex == queue->mTex)
	{
		float wf = mRegion.GetCalculatedWidth();
		uint width = Float::RoundToUInt(wf);

		Color4ub textColor ( mTextColor, mRegion.GetCalculatedAlpha() );
		Color4ub shadowColor ( mShadowColor, mRegion.GetCalculatedAlpha() );
		Vector2f pos ( mRegion.GetCalculatedLeft(), mRegion.GetCalculatedTop() );

		const IFont* font = GetFont();
		ASSERT(font != 0, "Font is null? What?");

		if ( mAlignment == Alignment::Right )
		{
			mEnd = mText.GetLength();
			mStart = mEnd - font->CountChars( mText, width, 0, 0xFFFFFFFF, true, false, mTags );

			// Position needs to be adjusted by the difference between label's width and the length of the text
			pos.x += wf - font->GetLength( mText, mStart, mEnd, mTags );
		}
		else
		{
			mStart = 0;
			mEnd = font->CountChars( mText, Float::RoundToUInt(mRegion.GetCalculatedWidth()),
				0, 0xFFFFFFFF, false, false, mTags );

			// For centered alignment simply figure out the bounding size of the text and adjust the position
			if ( mAlignment == Alignment::Center )
			{
				uint size = font->GetLength( mText, mStart, mEnd, mTags );
				if (size < width) pos.x += (wf - size) * 0.5f;
			}
		}

		// Adjust the height in order to center the text as necessary
		float difference = mRegion.GetCalculatedHeight() - height;
		pos.y += difference * 0.5f;

		// Make the font pixel-perfect so it's as clear as possible
		pos.x = Float::Floor(pos.x);
		pos.y = Float::Floor(pos.y);

		// Drop a shadow if requested
		if (shadowColor.a != 0)
		{
			font->Print( queue->mVertices, pos + 1.0f, shadowColor, mText, mStart, mEnd,
				(mTags == IFont::Tags::Ignore) ? IFont::Tags::Ignore : IFont::Tags::Skip );
		}

		// Print the text directly into the buffer
		if (textColor.a != 0) font->Print( queue->mVertices, pos, textColor, mText, mStart, mEnd, mTags );
	}
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UILabel::OnSerializeFrom (const TreeNode& node)
{
	if (UITextLine::OnSerializeFrom(node))
	{
		return true;
	}
	else if (node.mTag == "Alignment")
	{
		String alignment;

		if (node.mValue >> alignment)
		{
			if		(alignment == "Left")	SetAlignment(Alignment::Left);
			else if (alignment == "Right")	SetAlignment(Alignment::Right);
			else							SetAlignment(Alignment::Center);
		}
	}
	return false;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UILabel::OnSerializeTo (TreeNode& node) const
{
	UITextLine::OnSerializeTo(node);

	if		(mAlignment == Alignment::Left)		node.AddChild("Alignment", "Left");
	else if (mAlignment == Alignment::Right)	node.AddChild("Alignment", "Right");
	else										node.AddChild("Alignment", "Center");
}