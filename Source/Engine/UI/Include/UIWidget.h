#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Expanded region, holding a pointer to an associated Widget, parent, and children
//============================================================================================================

class UIManager;
class UIWidget
{
	friend class UIManager;
	friend class UIFrame;
	friend class UIScript;

public:

	typedef FastDelegate<UIWidget* (void)>  CreateDelegate;
	typedef PointerArray<UIWidget> Children;
	typedef PointerArray<UIScript> Scripts;

protected:

	String		mName;				// Every widget needs a name
	UIRegion	mRegion;			// As well as a region
	String		mTooltip;			// All areas can have a tooltip
	bool		mReceivesEvents;	// Whether the widget will receive events
	bool		mIsFading;			// Whether the widget is fading (will not respond to events)
	UIManager*	mUI;				// Pointer to the UI root that controls this widget
    UIWidget*	mParent;			// Pointer to the parent widget
	Children	mChildren;			// Array of children nodes
	Scripts		mScripts;			// Array of attached scripts
	int			mLayer;				// Layer used by this widget
	bool		mSerializable;		// Whether the widget is saved along with everything else

	// Scripts slated for removal
	Array<UIScript*> mDestroy;

public:

	UIWidget() :	mReceivesEvents (true),
					mIsFading		(false),
					mUI				(0),
					mParent			(0),
					mLayer			(0),
					mSerializable	(true) {}

	virtual ~UIWidget() { RemoveAllScripts(); }

public:

	// Area creation
	R5_DECLARE_BASE_CLASS("Area", UIWidget);

	// Property retrieval functions
	const String&	GetName()		 const	{ return mName;			}
	const String&	GetTooltip()	 const	{ return mTooltip;		}
	const Children& GetChildren()	 const	{ return mChildren;		}
	UIWidget*		GetParent()				{ return mParent;		}
	int				GetLayer()		 const	{ return mLayer;		}
	bool			IsSerializable() const	{ return mSerializable; }

	// Returns whether the specified parent is reachable by going up the hierarchy tree
	bool IsChildOf (const UIWidget* parent) const
	{
		return (parent == this || (mParent == 0 ? false : mParent->IsChildOf(parent)));
	}

	// Registers a new widget type or script
	template <typename Type> static void Register() { _Register( Type::ClassID(), &Type::_CreateNew ); }

	// Returns an existing script of given type
	template <typename Type> Type* GetScript()
	{
		UIScript* script = _GetScript(Type::ClassID());
		return (script != 0) ? (Type*)script : 0;
	}

	// Adds a new or returns an existing script of given type
	template <typename Type> Type* AddScript()
	{
		UIScript* script = _AddScript(Type::ClassID());
		return (script != 0) ? (Type*)script : 0;
	}

	// Adds a new or returns an existing script of given type
	template <typename Type> bool RemoveScript()
	{
		UIScript* script = _GetScript(Type::ClassID());
		if (script == 0) return false;
		_RemoveScript(script);
		return true;
	}

	// Finds a child widget at the specified position
	template <typename Type> Type* FindWidget (const Vector2i& pos)
	{
		UIWidget* obj = _FindWidget(pos);
		return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
	}

	// Finds a child widget of specified type
	template <typename Type> Type* FindWidget (const String& name, bool recursive = true)
	{
		UIWidget* obj = _FindWidget(name, recursive);
		return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
	}

	// Adds a new widget of specified type
	template <typename Type> Type* AddWidget (const String& name, bool unique = true)
	{
		UIWidget* obj = _AddWidget(Type::ClassID(), name, unique);
		return ( obj != 0 && obj->IsOfClass(Type::ClassID()) ) ? (Type*)obj : 0;
	}

private:

	// INTERNAL: Registers a new widget type
	static void _Register(const String& type, const CreateDelegate& callback);

	// INTERNAL: Find existing areas or add new ones -- should not be used directly.
	// Use these templates instead: FindWidget<> or AddWidget<>
	UIWidget* _FindWidget (const Vector2i& pos);
	UIWidget* _FindWidget (const String& name, bool recursive = true);
	UIWidget* _AddWidget  (const String& type, const String& name, bool unique = true);

	// INTERNAL: Retrieve an existing or add a new script (only unique script types can exist on the widget)
	UIScript* _GetScript(const String& type);
	UIScript* _AddScript(const String& type);

	// INTERNAL: Schedules the specified script for deletion
	void _RemoveScript (UIScript* ptr);

public:

	// Whether the widget is actually saved with everything else is subject to change
	void SetSerializable(bool val)			{ mSerializable = val;	}
	void SetName	(const String& name)	{ mName = name;			}
	void SetTooltip	(const String& text)	{ mTooltip = text;		}
	void SetLayer	(int layer, bool setDirty = true);

	// Deletes all child areas
	void DeleteAllChildren();

	// Removes all attached scripts
	void RemoveAllScripts();

	// Brings the widget to the foreground
	void BringToFront (UIWidget* child = 0);

	// Gives this widget undivided attention
	void SetFocus (bool focus);

	// Gives this widget keyboard focus
	void SetKeyboardFocus();

	// Areas that don't receive events will not respond to anything
	bool ReceivesEvents() const			{ return mReceivesEvents && !mIsFading; }
	void SetReceivesEvents(bool val)	{ mReceivesEvents = val; }

	// Having access to the widget's dimensions can come in handy
	UIRegion& GetRegion() { return mRegion; }
	void SetRegion (float left, float top, float width, float height) { mRegion.SetRect(left, top, width, height); }

	// Calls OnTextureChanged() and recurses through children
	void _TextureChanged (const ITexture* ptr);

	// Forwards the event to _Update()
	bool Update (const Vector2i& size,   bool parentChanged = false) { return _Update( mRegion.Update(size,   parentChanged) ); }
	bool Update (const UIRegion& parent, bool parentChanged = false) { return _Update( mRegion.Update(parent, parentChanged) ); }

private:

	// Updates the widget
	bool _Update (bool areaChanged);

	// Serialization
	bool SerializeTo	(TreeNode& root) const;
	bool SerializeFrom	(const TreeNode& root);

	// Calls the virtual Area::OnDraw() and recurses through children
	uint Draw();

protected:

	// Calls OnFill() on itself then recurses through all non-UIFrame children.
	void Fill (UIQueue* queue);

	// Convenience function, seeing as most areas will use the built-in layer
	void OnDirty (const ITexture* tex) { OnDirty(tex, mLayer, 0); }

	// Should notify the listeners of state changes
	void OnStateChange(uint state, bool isSet);

	// Should notify the listeners of value changes
	void OnValueChange();

	// Functions overwritten by the Frame class
	virtual void OnDirty (const ITexture* tex, int layer, const UIWidget* widget = 0) { if (mParent != 0) mParent->OnDirty(tex, layer, widget); }
	virtual uint OnDraw() { return 0; }

public:

	// Internal UI functions. In most cases you won't need to use these.
	virtual void _SetParentPtr (UIWidget* ptr)	{ mParent	= ptr; }
	virtual void _SetRootPtr   (UIManager* ptr)	{ mUI		= ptr; }

	// Can be overwritten for additional functionality (see the Animated Frame class)
	virtual float GetAlpha() const { return mRegion.GetCalculatedAlpha(); }
	virtual void  SetAlpha (float val, float animTime = 0.0f) { mRegion.SetAlpha(val); }

	// UIArea::Update() function gets the sub-region, and that gets passed to all children
	// See UIWindow class for an example
	virtual const UIRegion& GetSubRegion() const { return mRegion; }

	// Marks this specific widget as needing to be rebuilt
	virtual void SetDirty() {}

	// Called when something changes in the texture
	virtual void OnTextureChanged (const ITexture* ptr) {}

	// Called when the layer changes
	virtual void OnLayerChanged() {}

	// Called before OnUpdate(); can be used to override the widget's alpha or dimensions
	virtual bool OnPreUpdate (bool dimensionsChanged) { return false; }

	// Any per-frame animation should go here
	virtual bool OnUpdate (bool dimensionsChanged) { return false; }

	// Called when a queue is being rebuilt
	virtual void OnFill (UIQueue* queue) {}

	// Called before and after rendering the queue, respectively
	virtual void OnPreDraw (IGraphics* graphics) const {}
	virtual void OnPostDraw(IGraphics* graphics) const {}

	// Serialization
	virtual void OnSerializeTo (TreeNode& root) const {}
	virtual bool OnSerializeFrom (const TreeNode& root) { return false; }

protected:

	// Events
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta);
	virtual bool OnKeyPress	(const Vector2i& pos, byte key, bool isDown);
	virtual bool OnScroll	(const Vector2i& pos, float delta);
	virtual void OnMouseOver(bool inside);
	virtual void OnFocus	(bool selected);
	virtual void OnChar		(byte character) {}
};