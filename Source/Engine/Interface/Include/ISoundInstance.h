#pragma once

//============================================================================================================
//           R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko / Philip Cosgrave. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Basic interface for the SoundInstance class
//============================================================================================================

struct ISound;

struct ISoundInstance
{
	R5_DECLARE_INTERFACE_CLASS("ISoundInstance");
	
	virtual const ISound*	GetSound()		const=0;
	virtual const bool		Is3D()			const=0;
	virtual const bool		IsPlaying()		const=0;
	virtual const bool		IsPaused()		const=0;

	// Destory the sound
	virtual void DestroySelf()=0;

	// Play the sound
	virtual void Play()=0;

	// Pause the sound
	virtual void Pause (float duration = 0.25f)=0;

	// Stop the sound playback
	virtual void Stop (float duration = 0.25f)=0;

	// Sets the 3D position of the specified sound
	virtual void SetPosition (const Vector3f& position)=0;
	
	// Changes the volume of the specified sound
	virtual void SetVolume (float volume, float duration = 0.25f)=0;

	// Sets whether the sound will repeat after it ends
	virtual void SetRepeat (bool repeat)=0;

	// Gets the volume of the specified sound
	virtual const float	GetVolume () const=0;

	// Gets the volume of the specified sound
	virtual const bool	GetRepeat () const=0;
	
	// Gets the volume of the specified sound
	virtual const Vector3f&	GetPosition	() const=0;

	// Gets the layer of the specified sound
	virtual const uint	GetLayer () const=0;
};