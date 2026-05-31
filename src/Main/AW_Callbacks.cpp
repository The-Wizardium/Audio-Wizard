/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Callbacks Source File                      * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.5.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_Callbacks.h"


////////////////////////
// * PUBLIC METHODS * //
////////////////////////
#pragma region Public Methods
unsigned AudioWizardCallbacks::get_flags() {
	return flags;
}

void AudioWizardCallbacks::ApplyFadeIn() const {
	if (!AudioWizard::Peakmeter()) return;
	AudioWizard::Peakmeter()->ApplyFadeIn();
}
#pragma endregion


////////////////////////////////
// * AUDIO WIZARD CALLBACKS * //
////////////////////////////////
#pragma region Audio Wizard Callbacks
void AudioWizardCallbacks::on_playback_dynamic_info(const file_info& p_info) noexcept {
	// Placeholder, current design doesn't require this action...
}

void AudioWizardCallbacks::on_playback_dynamic_info_track(const file_info& p_info) noexcept {
	// Placeholder, current design doesn't require this action...
}

void AudioWizardCallbacks::on_playback_edited(metadb_handle_ptr p_track) noexcept {
	// Placeholder, current design doesn't require this action...
}

void AudioWizardCallbacks::on_playback_new_track(metadb_handle_ptr p_track) noexcept {
	ApplyFadeIn();
}

void AudioWizardCallbacks::on_playback_pause(bool b_state) noexcept {
	ApplyFadeIn();
}

void AudioWizardCallbacks::on_playback_seek(double p_time) noexcept {
	ApplyFadeIn();
}

void AudioWizardCallbacks::on_playback_starting(play_control::t_track_command p_command, bool p_paused) noexcept {
	ApplyFadeIn();
	AudioWizard::Main()->mainRealTime->ResetMetrics();
}

void AudioWizardCallbacks::on_playback_stop(play_control::t_stop_reason p_reason) noexcept {
	ApplyFadeIn();
	AudioWizard::Main()->mainRealTime->ResetMetrics();
}

void AudioWizardCallbacks::on_playback_time(double p_time) noexcept {
	// Placeholder, current design doesn't require this action...
}
void AudioWizardCallbacks::on_volume_change(float p_new_val) noexcept {
	// Placeholder, current design doesn't require this action...
}
#pragma endregion
