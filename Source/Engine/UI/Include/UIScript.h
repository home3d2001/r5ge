#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script that gets executed on the UI widgets
//============================================================================================================

class UIWidget;
class UIScript
{
public:

	// Allow the widget class to set 'mWidget' directly
	friend class UIWidget;

	// Script creation function delegate
	typedef FastDelegate<UIScript* (void)>  CreateDelegate;

protected:

	// Widget this script belongs to
	UIWidget* mWidget;

protected:

	// It's not possible to create just plain scripts -- they need to be derived from
	UIScript() : mWidget(0) {}

private:

	// INTERNAL: Registers a new script of the specified type
	static void _Register(const String& type, const CreateDelegate& func);

	// INTERNAL: Creates a new script of the specified type
	static UIScript* _Create(const String& type);

public:

	// This is a top-level base class
	R5_DECLARE_INTERFACE_CLASS("UIScript");

	// Registers a new script
	template <typename Type> static void Register() { _Register( Type::ClassID(), &Type::_CreateNew ); }

	// Scripts should be removed via DestroySelf() or using the RemoveScript<> template
	virtual ~UIScript();

	// Destroys this script. The script is never deleted immediately, but is rather scheduled for deletion.
	void DestroySelf();

	// Returns the widget this script belongs to
	UIWidget* GetWidget() { return mWidget; }

	// Initialization function is called once the script has been created
	virtual void Init() {}

	// Virtual functions that should be overwritten to add functionality
	virtual bool OnMouseMove	(const Vector2i& pos, const Vector2i& delta)	{ return false; }
	virtual bool OnKeyPress		(const Vector2i& pos, byte key, bool isDown)	{ return false; }
	virtual bool OnScroll		(const Vector2i& pos, float delta)				{ return false; }
	virtual void OnMouseOver	(bool isMouseOver)								{}
	virtual void OnFocus		(bool gotFocus)									{}
	virtual void OnStateChange	(uint state, bool isSet)						{}
	virtual void OnValueChange	()												{}
	virtual void OnUpdate		()												{}
	virtual bool SerializeTo	(TreeNode& node) const { return false; }
	virtual bool SerializeFrom	(const TreeNode& node) { return false; }
};