/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Tag Source File                            * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/TT-ReBORN/Georgia-ReBORN             * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW_Helpers.h"
#include "AW_Tag.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardTag::AudioWizardTag(const char* tag, metadb_handle_list handles, OperationType opType) :
	tag(tag), handles(std::move(handles)), opType(opType) {
}
#pragma endregion


////////////////////////
// * PUBLIC METHODS * //
////////////////////////
#pragma region Public Methods
void AudioWizardTag::WriteTag(const char* tag, const std::vector<FullTrackResults>& results,
	const std::function<double(const FullTrackResults&)>& valueGetter, int precision, int maxDecimals,
	const std::function<void()>& completionHandler) {
	metadb_handle_list handles;
	std::vector<std::unique_ptr<file_info_impl>> infos;

	for (const auto& result : results) {
		handles.add_item(result.handle);
		auto info = std::make_unique<file_info_impl>();
		metadb_info_container::ptr info_container;

		if (result.handle->get_info_ref(info_container)) {
			info->copy(info_container->info());
			const double value = valueGetter(result);
			info->meta_set(tag, pfc::format_float(value, precision, maxDecimals));
			infos.push_back(std::move(info));
		}
	}

	if (handles.get_count() == 0) {
		if (completionHandler) {
			completionHandler();
		}
		return;
	}

	pfc::list_t<const file_info*> infos_raw;
	for (auto const& info : infos) {
		infos_raw.add_item(info.get());
	}

	auto instance = Create(tag, handles, OperationType::Write);
	if (completionHandler) {
		instance->SetCompletionHandler(completionHandler);
	}

	metadb_io_v2::get()->update_info_async_simple(
		handles, infos_raw, core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, nullptr
	);
}

void AudioWizardTag::WriteMultipleTags(const std::vector<std::pair<std::string, std::function<double(const FullTrackResults&)>>>& tagMappings,
	const std::vector<FullTrackResults>& results, int precision, int maxDecimals, const std::function<void()>& completionHandler) {
	metadb_handle_list handles;
	std::vector<std::unique_ptr<file_info_impl>> infos;

	for (const auto& result : results) {
		handles.add_item(result.handle);
		auto info = std::make_unique<file_info_impl>();
		metadb_info_container::ptr info_container;

		if (result.handle->get_info_ref(info_container)) {
			info->copy(info_container->info());
			for (const auto& [tag, getter] : tagMappings) {
				if (!tag.empty()) { // Skip if custom tag name is empty
					const double value = getter(result);
					info->meta_set(tag.c_str(), pfc::format_float(value, precision, maxDecimals));
				}
			}
			infos.push_back(std::move(info));
		}
	}

	if (handles.get_count() == 0) {
		if (completionHandler) {
			completionHandler();
		}
		return;
	}

	pfc::list_t<const file_info*> infos_raw;
	for (auto const& info : infos) {
		infos_raw.add_item(info.get());
	}

	auto instance = Create("All Tags", handles, OperationType::Write);
	if (completionHandler) {
		instance->SetCompletionHandler(completionHandler);
	}

	metadb_io_v2::get()->update_info_async_simple(
		handles, infos_raw, core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, nullptr
	);
}

void AudioWizardTag::ClearTag(const char* tag, const metadb_handle_list& handles,
	const std::function<void()>& completionHandler) {
	std::vector<std::unique_ptr<file_info_impl>> infos;

	for (const auto& handle : handles) {
		auto info = std::make_unique<file_info_impl>();
		metadb_info_container::ptr info_container;

		if (handle->get_info_ref(info_container)) {
			info->copy(info_container->info());
			info->meta_remove_field(tag);
			infos.push_back(std::move(info));
		}
	}

	if (handles.get_count() == 0) {
		if (completionHandler) {
			completionHandler();
		}
		return;
	}

	pfc::list_t<const file_info*> infos_raw;
	for (auto const& info : infos) {
		infos_raw.add_item(info.get());
	}

	auto instance = Create(tag, handles, OperationType::Clear);
	if (completionHandler) {
		instance->SetCompletionHandler(completionHandler);
	}

	metadb_io_v2::get()->update_info_async_simple(
		handles, infos_raw, core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, nullptr
	);
}

void AudioWizardTag::ClearMultipleTags(const std::vector<std::string>& tags, const metadb_handle_list& handles, const std::function<void()>& completionHandler) {
	std::vector<std::unique_ptr<file_info_impl>> infos;

	for (const auto& handle : handles) {
		auto info = std::make_unique<file_info_impl>();
		metadb_info_container::ptr info_container;

		if (handle->get_info_ref(info_container)) {
			info->copy(info_container->info());
			for (const auto& tag : tags) {
				if (!tag.empty()) { // Skip if custom tag name is empty
					info->meta_remove_field(tag.c_str());
				}
			}
			infos.push_back(std::move(info));
		}
	}

	if (handles.get_count() == 0) {
		if (completionHandler) {
			completionHandler();
		}
		return;
	}

	pfc::list_t<const file_info*> infos_raw;
	for (auto const& info : infos) {
		infos_raw.add_item(info.get());
	}

	auto instance = Create("All Tags", handles, OperationType::Clear);
	if (completionHandler) {
		instance->SetCompletionHandler(completionHandler);
	}

	metadb_io_v2::get()->update_info_async_simple(
		handles, infos_raw, core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, nullptr
	);
}

AudioWizardTag* AudioWizardTag::Create(const char* tag, metadb_handle_list handles, OperationType opType) {
	auto instance = std::make_unique<AudioWizardTag>(tag, std::move(handles), opType);
	instance->register_callback();
	return instance.release();
}

void AudioWizardTag::SetCompletionHandler(std::function<void()> handler) {
	completionHandler = std::move(handler);
}
#pragma endregion


/////////////////////////
// * PRIVATE METHODS * //
/////////////////////////
#pragma region Public Methods
void AudioWizardTag::on_changed_sorted_v2(metadb_handle_list_cref itemsSorted, metadb_io_callback_v2_data& data, bool bFromHook) {
	for (const auto& handle : handles) {
		if (metadb_handle_list_helper::bsearch_by_pointer(itemsSorted, handle) == pfc_infinite) {
			return;
		}
	}

	if (completionHandler) {
		completionHandler();
	}
	else {
		ShowMessage();
	}

	this->unregister_callback();
}

void AudioWizardTag::ShowMessage() const {
	pfc::string8 message;

	message << tag << (opType == OperationType::Write
		? " tags have been successfully written."
		: " tags have been successfully cleared."
	);

	MessageBox(core_api::get_main_window(),
		pfc::stringcvt::string_wide_from_utf8(message).get_ptr(),
		L"Success", MB_OK | MB_ICONINFORMATION
	);
}
#pragma endregion
