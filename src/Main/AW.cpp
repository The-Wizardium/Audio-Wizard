/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Source File                                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    27-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "MyCOM.h"


/////////////////////////
// * COMPONENT SETUP * //
/////////////////////////
#pragma region Component Setup
DECLARE_COMPONENT_VERSION(
	"Audio Wizard", "0.3.0",

	"Audio Wizard \n"
	"The Ruby Spell Of Sonic Mastery \n"
	"Sapientia Flamma Divina \n\n"

	"A Sacred Chapter Of The Wizardium \n"
	"https://www.The-Wizardium.org \n"
	"https://github.com/The-Wizardium \n\n"

	"Sealed within the blazing Rubynar Sanctum of the Holy Foobar Land, "
	"Audio Wizard commands sound's divine essence. "
	"Guided by sacred standards of ITU-R BS.1770-5 and EBU Tech 3341-3343, "
	"it masters audio across 24 channels, "
	"wielding full-track analysis, real-time monitoring, and COM scripting. "
	"Its Pure Dynamics (PD) metric, a unique psychoacoustic flame, "
	"reveals audio as mortals hear it. \n\n"

	"Wield the arcane arts: \n"
	" - Offline analysis and Real-time monitoring \n"
	" - Loudness: M LUFS, S LUFS, I LUFS \n"
	" - Peaks: Sample and True Peak \n"
	" - Dynamics: PSR, PLR, CF, LRA, DR, PD \n\n"

	"The Wizardium Chooses Only The Anointed"
);

DEFINE_GUID(AudioWizard, 0xaca3ee3f, 0x75d2, 0x415b, 0xbe, 0xcb, 0xa6, 0x8e, 0xc1, 0x6d, 0x16, 0x84);
VALIDATE_COMPONENT_FILENAME("foo_audio_wizard.dll");
FOOBAR2000_IMPLEMENT_CFG_VAR_DOWNGRADE;
#pragma endregion


/////////////////////////////
// * MAIN INITIALIZATION * //
/////////////////////////////
#pragma region Main Initialization
void AudioWizard::InitAudioWizard() {
	audioWizardMain = std::make_unique<AudioWizardMain>();
	audioWizardPeakmeter = std::make_unique<AudioWizardPeakmeter>();
	audioWizardWaveform = std::make_unique<AudioWizardWaveform>();
	audioWizardDialogRealTime = std::make_unique<AudioWizardDialogRealTime>();
}

void AudioWizard::QuitAudioWizard() {
	AWHDarkMode::Cleanup();

	audioWizardMain->StopFullTrackAudioProcessor();
	audioWizardMain->StopRealTimeAudioProcessor();

	audioWizardMain.reset();
	audioWizardPeakmeter.reset();
	audioWizardWaveform.reset();
	audioWizardDialogFullTrack.reset();
	audioWizardDialogRealTime.reset();

	MyCOM::QuitMyCOM();
}

namespace {
	FB2K_ON_INIT_STAGE(MyCOM::InitMyCOM, init_stages::after_config_read);
	FB2K_ON_INIT_STAGE(AudioWizard::InitAudioWizard, init_stages::after_ui_init);
	FB2K_RUN_ON_QUIT(AudioWizard::QuitAudioWizard);
}
#pragma endregion
