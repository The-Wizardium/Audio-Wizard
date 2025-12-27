/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Menu Source File                           * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_Menu.h"


////////////////////////////
// * MENU CONTEXT GROUP * //
////////////////////////////
#pragma region Menu Context Group
AudioWizardMenuContextGroup::AudioWizardMenuContextGroup() :
	contextmenu_group_popup_factory(AudioWizardMenuGUID::guid_audioWizardMenuContextGroup,
	contextmenu_groups::root, "Audio Wizard", contextmenu_priorities::root_utilities) {
}
#pragma endregion


///////////////////////////////
// * MENU CONTEXT COMMANDS * //
///////////////////////////////
#pragma region Menu Context Command
GUID AudioWizardMenuContextCommands::get_parent() {
	return AudioWizardMenuGUID::guid_audioWizardMenuContextGroup;
}

unsigned AudioWizardMenuContextCommands::get_num_items() {
	return 2;
}

void AudioWizardMenuContextCommands::get_item_name(unsigned p_index, pfc::string_base& p_out) {
	switch (p_index) {
		case 0: {
			p_out = "Open analysis";
			break;
		}

		case 1: {
			p_out = "Open monitor";
			break;
		}

		default: uBugCheck();
	}
}

void AudioWizardMenuContextCommands::context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller) {
	switch (p_index) {
		case 0: { // Full track dialog
			if (p_data.get_count() > 0) AudioWizard::Main()->GetFullTrackAnalysis();
			break;
		}

		case 1: { // Real time dialog
			auto dlg = std::make_unique<AudioWizardDialogRealTime>();
			auto* rawDlg = dlg.get();

			rawDlg->SetWindowLongPtr(GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dlg.release()));
			rawDlg->Create(core_api::get_main_window());
			rawDlg->CenterWindow();
			rawDlg->ShowWindow(SW_SHOW);

			break;
		}

		default: uBugCheck();
	}
}

GUID AudioWizardMenuContextCommands::get_item_guid(unsigned p_index) {
	switch (p_index) {
		case 0: {
			return AudioWizardMenuGUID::guid_audioWizardMenuContextAnalysis;
		}

		case 1: {
			return AudioWizardMenuGUID::guid_audioWizardMenuContextMonitor;
		}

		default: uBugCheck();
	}
}

bool AudioWizardMenuContextCommands::get_item_description(unsigned p_index, pfc::string_base& p_out) {
	switch (p_index) {
		case 0: {
			p_out = "Opens the Full-Track Analysis Audio Wizard dialog.";
			break;
		}

		case 1: {
			p_out = "Opens the Real-time Monitoring Audio Wizard dialog.";
			break;
		}

		default: uBugCheck();
	}

	return true;
}

contextmenu_item::t_enabled_state AudioWizardMenuContextCommands::get_enabled_state(unsigned) {
	return contextmenu_item::DEFAULT_ON;
}
#pragma endregion
