#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Authors: Philip Cosgrave, Michael Lyashenko, and Eugene Gorodinsky
//============================================================================================================

class SoundInstance;
class Sound;

class Audio : public IAudio
{
private:

	typedef Sound* SoundPtr;
	typedef PointerArray<ISoundInstance> PlayingSounds;

	struct AudioLayer
	{
		uint			mLayer;
		float			mVolume;
		PlayingSounds	mSounds;

		AudioLayer(uint layer, float volume) : mLayer(layer), mVolume(volume) {} 
		~AudioLayer() 
		{
			mSounds.Release(); 
		}
	};

	typedef AudioLayer* AudioLayerPtr;

private:

	PointerHash<AudioLayer>	mLayers;
	PointerArray<Sound>		mLibrary;
	Vector3f				mPos;
	Thread::Lockable		mLock;

public:

	R5_DECLARE_INHERITED_CLASS (Audio, IAudio, IAudio);

	Audio();
	virtual ~Audio();

	// Release all audio resources
	virtual void Release();

	// Update notification
	virtual void Update();

	// Release all resources associated with the specified sound
	virtual void Release (ISound* sound);

	// Retrieves the audio listener position
	virtual const Vector3f& GetListener() const { return mPos; }

	// Sets the sound listener pos/dir/up (usually should be the camera)
	virtual void SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up);

	// Changes the volume of an entire sound layer (music layer, for example)
	virtual void SetLayerVolume (uint layer, float volume, float duration = 0.25f);

	// Gets the volume of the specified layer
	virtual const float	GetLayerVolume (uint layer) const;

	// Adds a new sound to the library
	virtual ISound* GetSound (const String& name, bool createIfMissing = true);

	// Release the created sound instance
	virtual void ReleaseInstance (ISoundInstance* sound);

	// Create a 2D sound instance.
	virtual ISoundInstance* Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat);

	// Create a 3D sound instance.
	virtual ISoundInstance* Instantiate (ISound* sound, const Vector3f& position, uint layer, float fadeInTime, bool repeat);

private:

	Sound* _GetOrCreate (const String& name, bool createIfMissing);
	AudioLayer* _GetAudioLayer  (uint layer, float volume);
	SoundInstance* _Instantiate (Sound* sound, uint layer, float fadeInTime, bool repeat);
};
