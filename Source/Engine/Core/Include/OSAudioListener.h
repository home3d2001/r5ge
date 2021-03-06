#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Updates the sound listener position every update (attach to your main camera)
// Author: Michael Lyashenko
//============================================================================================================

class OSAudioListener : public Script
{
protected:

	IAudio* mAudio;

	OSAudioListener() : mAudio(0) {}

public:

	R5_DECLARE_INHERITED_CLASS(OSAudioListener, Script, Script);

	virtual void OnInit()
	{
		mAudio = mObject->GetCore()->GetAudio();
		if (mAudio == 0) DestroySelf();
	}

	virtual void OnUpdate()
	{
		const Quaternion& rot = mObject->GetAbsoluteRotation();
		mAudio->SetListener(mObject->GetAbsolutePosition(), rot.GetForward(), rot.GetUp(), mObject->GetAbsoluteVelocity());
	}
};
