/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Menu Header File                           * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


///////////////////
// * MENU GUID * //
///////////////////
#pragma region Menu GUID
namespace AudioWizardMenuGUID {
	constexpr GUID guid_audioWizardMenuContextGroup = { 0x12326e6e, 0x1ef0, 0x4f49, { 0x8f, 0x24, 0xb4, 0x4c, 0x3c, 0xc1, 0x88, 0x20 } };
	constexpr GUID guid_audioWizardMenuContextAnalysis = { 0x86353369, 0xec6d, 0x4c11, { 0xb3, 0x40, 0x6a, 0xd, 0xfe, 0xd0, 0xd1, 0x9d } };
	constexpr GUID guid_audioWizardMenuContextMonitor = { 0xdde539c2, 0x7f84, 0x40e6, { 0xb4, 0x37, 0x35, 0x16, 0x86, 0x9a, 0x81, 0x27 } };
}
#pragma endregion


////////////////////////////
// * MENU CONTEXT GROUP * //
////////////////////////////
#pragma region Menu Context Group
class AudioWizardMenuContextGroup : public contextmenu_group_popup_factory {
public:
	AudioWizardMenuContextGroup();
};

static AudioWizardMenuContextGroup audioWizardMenuContextGroup;
#pragma endregion


///////////////////////////////
// * MENU CONTEXT COMMANDS * //
///////////////////////////////
#pragma region Menu Context Commands
class AudioWizardMenuContextCommands : public contextmenu_item_simple {
public:
	GUID get_parent() override;
	unsigned get_num_items() override;
	void get_item_name(unsigned p_index, pfc::string_base& p_out) override;
	void context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller) override;
	GUID get_item_guid(unsigned p_index) override;
	bool get_item_description(unsigned p_index, pfc::string_base& p_out) override;
	contextmenu_item::t_enabled_state get_enabled_state(unsigned p_index) override;
};

static contextmenu_item_factory_t<AudioWizardMenuContextCommands> audioWizardMenuContextCommands;
#pragma endregion
