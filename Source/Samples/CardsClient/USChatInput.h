#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Chat input field behavior
// Author: Michael Lyashenko
//============================================================================================================

class USChatInput : public UIScript
{
	UIInput* mInput;

	USChatInput() : mInput(0) {}

public:

	R5_DECLARE_INHERITED_CLASS(USChatInput, UIScript, UIScript);

	// Ensure the script is attached to an input and register a key listener
	virtual void OnInit();

	// Remove bound callbacks
	virtual void OnDestroy();

public:

	// Key event callback registered with the core
	uint OnKey (const Vector2i& pos, byte key, bool isDown);

	// Submit the text
	virtual void OnValueChange();
};