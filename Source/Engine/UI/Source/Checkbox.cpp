#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UICheckbox::OnUpdate (bool dimensionsChanged)
{
	if (dimensionsChanged)
	{
		float height = mRegion.GetCalculatedHeight();

		if (GetAlignment() == UILabel::Alignment::Right)
		{
			mImage.GetRegion().SetLeft (1.0f, -height);
			mLabel.GetRegion().SetRight(1.0f, -height);
		}
		else
		{
			mImage.GetRegion().SetRight(0.0f, height);
			mLabel.GetRegion().SetLeft (0.0f, height);
		}
	}
	mImage.Update(mRegion, dimensionsChanged);
	mLabel.Update(mRegion, dimensionsChanged);
	return false;
}

//============================================================================================================
// Fills the rendering queue
//============================================================================================================

void UICheckbox::OnFill (UIQueue* queue)
{
	if (queue->mLayer	== mLayer &&
		queue->mTex		== mImage.GetTexture() &&
		queue->mArea	== 0)
	{
		static String faceName[] = {"Checkbox: Disabled", "Checkbox: Unchecked", "Checkbox: Highlighted", "Checkbox: Checked"};

		if (mState & State::Enabled)
		{
			mImage.SetFace(faceName[1], false);
			mImage.OnFill(queue);

			if (mState & State::Highlighted)
			{
				mImage.SetFace(faceName[2], false);
				mImage.OnFill(queue);
			}

			if (mState & State::Checked)
			{
				mImage.SetFace(faceName[3], false);
				mImage.OnFill(queue);
			}
		}
		else
		{
			mImage.SetFace(faceName[0], false);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UICheckbox::OnSerializeFrom (const TreeNode& node)
{
	if ( mImage.OnSerializeFrom(node) )
	{
		return true;
	}
	else if (node.mTag == "State")
	{
		if (node.mValue.IsString())
		{
			const String& state = node.mValue.AsString();

			if		(state == "Disabled")	SetState(State::Enabled, false);
			else if (state == "Checked")	SetState(State::Enabled | State::Checked, true);
			else if (state == "Enabled")	SetState(State::Enabled, true);
		}
		return true;
	}
	return mLabel.OnSerializeFrom (node);
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UICheckbox::OnSerializeTo (TreeNode& node) const
{
	const UISkin* skin = mImage.GetSkin();
	TreeNode& child = node.AddChild("Skin");
	if (skin) child.mValue = skin->GetName();

	mLabel.OnSerializeTo(node);

	if		(mState & State::Checked)	node.AddChild("State", "Checked");
	else if (mState & State::Enabled)	node.AddChild("State", "Enabled");
	else								node.AddChild("State", "Disabled");
}
