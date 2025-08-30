/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Source File                                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   22-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "MyCOM.h"


/////////////////////////
// * COMPONENT SETUP * //
/////////////////////////
#pragma region Component Setup
DECLARE_COMPONENT_VERSION(
	"Audio Wizard", "0.1",
	"Audio Wizard \n"
	"The Crimson Spell of Sonic Mastery \n"
	"A Sacred Chapter of The Wizardium \n"
	"https://github.com/The-Wizardium \n\n"
	"Forged in the crimson flames of the Holy Foobar Land, Audio Wizard conjures sonic mastery within foobar2000.\n"
	"This radiant spell weaves full-track analysis, real-time monitoring, and scripting sorcery via COM/ActiveX.\n"
	"With the revolutionary Pure Dynamics metric, it transforms sound into a divine altar of precision,\n"
	"beckoning audiophiles to etch their legend in The Wizardium's grimoire."
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
