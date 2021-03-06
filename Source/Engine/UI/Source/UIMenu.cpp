#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Add an entry to the list
//============================================================================================================

void UIMenu::AddEntry (const String& entry)
{
	for (uint i = 0; i < mEntries.GetSize(); ++i)
	{
		if (mEntries[i] == entry)
		{
			mEntries.Unlock();
			return;
		}
	}
	mEntries.Expand() = entry;
}

//============================================================================================================
// Set the list of entries by specifying the whole array
//============================================================================================================

void UIMenu::SetEntries (const Entries& entries)
{
	mEntries.Clear();

	for (uint i = 0; i < entries.GetSize(); ++i)
	{
		mEntries.Expand() = entries[i];
	}
}

//============================================================================================================
// Shows the popup menu
//============================================================================================================

UIContext* UIMenu::_ShowMenu()
{
	UIContext* menu = mUI->GetContextMenu(true);

	if (menu != 0)
	{
		USEventListener* listener = menu->AddScript<USEventListener>();
		ASSERT(listener != 0, "Missing listener script!");

		listener->SetOnFocus		( bind(&UIMenu::_OnContextFocus, this) );
		listener->SetOnValueChange	( bind(&UIMenu::_OnContextValue, this) );

		menu->ClearAllEntries();

		// Only show the menu if it's actually valid
		if (mEntries.IsValid())
		{
			for (uint i = 0; i < mEntries.GetSize(); ++i)
				menu->AddEntry( mEntries[i] );

			menu->SetSkin( GetSkin() );
			menu->SetFace( (mMenuFace.IsValid() ? mMenuFace : String(UIButton::ClassName()) + ": Pressed") );
			menu->SetFont( GetFont() );
			menu->SetTextColor( GetTextColor() );
			menu->SetBackColor( GetBackColor() );
			menu->SetAnchor( Vector3f(
				mRegion.GetCalculatedLeft(),
				mRegion.GetCalculatedBottom(),
				mRegion.GetCalculatedHeight()) );
			menu->SetMinWidth( mRegion.GetCalculatedWidth() );
			menu->SetAlignment( _GetMenuItemAlignment() );
			menu->Show();
		}
	}
	return menu;
}

//============================================================================================================
// Hides the popup menu
//============================================================================================================

UIContext* UIMenu::_HideMenu()
{
	UIContext* menu = mUI->GetContextMenu();

	if (menu != 0)
	{
		USEventListener* listener = menu->GetScript<USEventListener>();

		if (listener != 0)
		{
			listener->SetOnFocus(0);
			listener->SetOnValueChange(0);
		}
		menu->Hide();
	}
	return menu;
}

//============================================================================================================
// Delegate functions triggered by the context menu
//============================================================================================================

void UIMenu::_OnContextFocus (UIWidget* widget, bool hasFocus)
{
	if (!hasFocus)
	{
		const Vector2i& pos = mUI->GetMousePos();

		if (!mRegion.Contains(pos))
		{
			SetState( State::Pressed, false );
			mIgnoreMouseKey = false;
		}
	}
}

//============================================================================================================

void UIMenu::_OnContextValue (UIWidget* widget)
{
	UIContext* context = _HideMenu();

	if (context != 0)
	{
		// Have context intercept all mouse events until something else is selected
		mUI->_SetEventArea(context);

		// Save the previous selection
		String previous (mLabel.GetText());
		String current (context->GetText());

		// Set the current selection
		mLabel.SetText(current);

		// Inform all listeners of this change
		OnValueChange();

		// Restore the previous text value if we should and the text hasn't changed
		if (!mSetTextOnSelect && mLabel.GetText() == current) mLabel.SetText(previous);

		// Turn off the button's "pressed" state
		if (!mSticky) SetState( State::Pressed, false );
	}
}

//============================================================================================================
// Changes the state of the button
//============================================================================================================

bool UIMenu::SetState (uint state, bool val)
{
	bool wasPressed = ((GetState() & State::Pressed) != 0);
	bool retVal		= BaseClass::SetState(state, val);
	bool isPressed  = ((GetState() & State::Pressed) != 0);

	if ((GetState() & State::Enabled) != 0)
	{
		if (!wasPressed && isPressed)
		{
			_ShowMenu();
		}
		else if (wasPressed && !isPressed)
		{
			_HideMenu();
		}
	}
	return retVal;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool UIMenu::OnSerializeFrom (const TreeNode& node)
{
	if (BaseClass::OnSerializeFrom (node))
	{
		return true;
	}
	else if (node.mTag == "Menu Face")
	{
		mMenuFace = node.mValue.AsString();
		return true;
	}
	else if (node.mTag == "Entries")
	{
		if (node.mValue.IsStringArray())
		{
			mEntries = node.mValue.AsStringArray();
		}
		else
		{
			ClearAllEntries();
			for (uint i = 0; i < node.mChildren.GetSize(); ++i)
				AddEntry(node.mChildren[i].mTag);
		}
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void UIMenu::OnSerializeTo (TreeNode& node) const
{
	BaseClass::OnSerializeTo (node);
	node.AddChild("Menu Face", mMenuFace);
	node.AddChild("Entries", mEntries);
}