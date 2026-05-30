/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Header File                                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.4.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "AW_DialogFullTrack.h"
#include "AW_DialogRealTime.h"
#include "AW_Main.h"
#include "AW_Peakmeter.h"
#include "AW_Waveform.h"


//////////////////////
// * AUDIO WIZARD * //
//////////////////////
#pragma region Audio Wizard
class AudioWizard {
public:
	AudioWizard() = default;
	~AudioWizard() = default;

	// * PUBLIC METHODS * //
	static void InitAudioWizard();
	static void QuitAudioWizard();

	// * PUBLIC GETTERS - SINGLETON ACCESSORS * //
	static AudioWizardMain* Main() { return audioWizardMain.get(); }
	static AudioWizardPeakmeter* Peakmeter() { return audioWizardPeakmeter.get(); }
	static AudioWizardWaveform* Waveform() { return audioWizardWaveform.get(); }
	static AudioWizardDialogFullTrack* DialogFullTrack() { return audioWizardDialogFullTrack.get(); }
	static AudioWizardDialogRealTime* DialogRealTime() { return audioWizardDialogRealTime.get(); }

	// * PUBLIC SETTERS * //
	static void SetFullTrackDialog(std::unique_ptr<AudioWizardDialogFullTrack> dialog) {
		audioWizardDialogFullTrack = std::move(dialog);
	}

private:
	static inline std::unique_ptr<AudioWizardMain> audioWizardMain = nullptr;
	static inline std::unique_ptr<AudioWizardPeakmeter> audioWizardPeakmeter = nullptr;
	static inline std::unique_ptr<AudioWizardWaveform> audioWizardWaveform = nullptr;
	static inline std::unique_ptr<AudioWizardDialogFullTrack> audioWizardDialogFullTrack = nullptr;
	static inline std::unique_ptr<AudioWizardDialogRealTime> audioWizardDialogRealTime = nullptr;

	friend class AudioWizardMain;
};
#pragma endregion
