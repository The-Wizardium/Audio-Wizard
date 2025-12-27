/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Tag Header File                            * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include <AW_Analysis.h>


//////////////////////////
// * AUDIO WIZARD TAG * //
//////////////////////////
#pragma region Audio Wizard Tag
class AudioWizardTag : public metadb_io_callback_v2_dynamic_impl_base {
public:
	using FullTrackResults = AudioWizardAnalysisFullTrack::FullTrackResults;
	enum class OperationType { Write, Clear };

	AudioWizardTag(const char* tag, metadb_handle_list handles, OperationType opType);
	virtual ~AudioWizardTag() = default;

	static void WriteTag(const char* tag, const std::vector<FullTrackResults>& results,
		const std::function<double(const FullTrackResults&)>& valueGetter, int precision, int maxDecimals,
		const std::function<void()>& completionHandler = nullptr
	);

	static void WriteMultipleTags(const std::vector<std::pair<std::string, std::function<double(const FullTrackResults&)>>>& tagMappings,
		const std::vector<FullTrackResults>& results, int precision, int maxDecimals,
		const std::function<void()>& completionHandler = nullptr
	);

	static void ClearTag(const char* tag, const metadb_handle_list& handles,
		const std::function<void()>& completionHandler = nullptr
	);

	static void ClearMultipleTags(const std::vector<std::string>& tags, const metadb_handle_list& handles,
		const std::function<void()>& completionHandler = nullptr
	);

	static AudioWizardTag* Create(const char* tag, metadb_handle_list handles, OperationType opType);

	void SetCompletionHandler(std::function<void()> handler);

private:
	OperationType opType;
	metadb_handle_list handles;
	pfc::string8 tag;
	std::function<void()> completionHandler;

	void on_changed_sorted_v2(metadb_handle_list_cref itemsSorted, metadb_io_callback_v2_data& data, bool bFromHook) override;
	void ShowMessage() const;
};
#pragma endregion
