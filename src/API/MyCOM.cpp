/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: COM Automation and ActiveX Interface                    * //
// * Description:    MyCOM Source File                                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    27-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "MyCOM.h"


//////////////////////////////////////////
// * MyCOM - CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////////////
#pragma region MyCOM - Constructor & Destructor
MyCOM::MyCOM() {
	InterlockedIncrement(&activeObjects);
}

MyCOM::~MyCOM() {
	InterlockedDecrement(&activeObjects);
}
#pragma endregion


///////////////////////////////////////
// * MyCOM - PUBLIC STATIC METHODS * //
///////////////////////////////////////
#pragma region MyCOM - Public Static Methods
HRESULT MyCOM::InitMyCOM() {
	HMODULE hModule = nullptr;
	HRESULT hr = OleInitialize(nullptr);

	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::InitMyCOM", L"OleInitialize failed", true);
	}

	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(static_cast<void*>(&InitMyCOM)), &hModule)) {
		OleUninitialize();
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::InitMyCOM", L"Failed to get module handle", true);
	}

	hr = InitTypeLibrary(hModule);
	if (FAILED(hr)) {
		OleUninitialize();
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::InitMyCOM", L"InitTypeLibrary failed", true);
	}

	hr = RegisterMyCOM();
	if (FAILED(hr)) {
		OleUninitialize();
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::InitMyCOM", L"RegisterMyCOM failed", true);
	}

	return S_OK;
}

HRESULT MyCOM::InitTypeLibrary(HMODULE hModule) {
	std::wstring module_path(MAX_PATH, L'\0');

	DWORD len = GetModuleFileNameW(hModule, &module_path[0], MAX_PATH);
	if (len == 0 || len >= MAX_PATH) {
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::InitTypeLibrary", L"Failed to retrieve module path", true);
	}

	module_path.resize(len);

	HRESULT hr = LoadTypeLibEx(module_path.c_str(), REGKIND_NONE, &MyCOM::typeLib);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::InitTypeLibrary", L"LoadTypeLibEx failed", true);
	}

	return S_OK;
}

HRESULT MyCOM::RegisterMyCOM(std::wstring_view regMethod) {
	auto hr = S_OK;
	CComPtr<MyCOMClassFactory> pClassFactory = new MyCOMClassFactory();

	if (!pClassFactory) {
		return AWHCOM::LogError(E_OUTOFMEMORY, L"Audio Wizard => MyCOM::RegisterMyCOM", L"Failed to create class factory", true);
	}

	hr = CoRegisterClassObject(MyCOM_CLSID, pClassFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterMyCOM", L"CoRegisterClassObject failed", true);
	}

	if (regMethod == L"RegEntry") {
		hr = RegisterCLSID();
		if (FAILED(hr)) {
			CoRevokeClassObject(dwRegister);
			return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterMyCOM", L"RegisterCLSID failed", true);
		}
	}
	else {
		hr = HookCLSIDFromProgID();
		if (FAILED(hr)) {
			CoRevokeClassObject(dwRegister);
			return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterMyCOM", L"HookCLSIDFromProgID failed", true);
		}
	}

	return S_OK;
}

HRESULT MyCOM::UnregisterMyCOM(std::wstring_view regMethod) {
	if (dwRegister != 0) {
		HRESULT hr = CoRevokeClassObject(dwRegister);

		if (FAILED(hr)) {
			return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::UnregisterMyCOM", L"CoRevokeClassObject failed", true);
		}

		if (regMethod == L"RegFree" && original_CLSIDFromProgID) {
			MH_DisableHook(&CLSIDFromProgID);
			MH_Uninitialize();
			original_CLSIDFromProgID = nullptr;
		}
		else if (regMethod == L"RegEntry") {
			UnregisterCLSID();
		}

		dwRegister = 0;
	}

	return S_OK;
}

HRESULT MyCOM::QuitMyCOM(std::wstring_view regMethod) {
	HRESULT hr = UnregisterMyCOM(regMethod);
	OleUninitialize();
	return hr;
}
#pragma endregion


////////////////////////////////////////
// * MyCOM - PRIVATE STATIC METHODS * //
////////////////////////////////////////
#pragma region MyCOM - COM Registration-Free with MinHook
HRESULT WINAPI MyCOM::MyCLSIDFromProgID(LPCOLESTR lpszProgID, LPCLSID lpclsid) {
	static std::map<std::wstring, CLSID> progidToClsidMap = {
		{ MyCOM_PROGID, MyCOM_CLSID }
	};

	auto it = progidToClsidMap.find(lpszProgID);
	if (it != progidToClsidMap.end()) {
		*lpclsid = it->second;
		return S_OK;
	}

	// Call the original function for other ProgIDs
	if (original_CLSIDFromProgID != nullptr) {
		return original_CLSIDFromProgID(lpszProgID, lpclsid);
	}

	return AWHCOM::LogError(REGDB_E_CLASSNOTREG, L"Audio Wizard => MyCOM::MyCLSIDFromProgID", L"Class not registered", true);
}

HRESULT MyCOM::HookCLSIDFromProgID() {
	if (MH_Initialize() != MH_OK) {
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::HookCLSIDFromProgID", L"MinHook initialization failed", true);
	}

	if (MH_CreateHook(&CLSIDFromProgID, &MyCLSIDFromProgID, reinterpret_cast<void**>(&original_CLSIDFromProgID)) != MH_OK) {
		MH_Uninitialize();
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::HookCLSIDFromProgID", L"MinHook creation failed", true);
	}

	if (MH_EnableHook(&CLSIDFromProgID) != MH_OK) {
		MH_Uninitialize();
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::HookCLSIDFromProgID", L"MinHook enable failed", true);
	}

	return S_OK;
}
#pragma endregion

#pragma region MyCOM - COM Registration with Reg Entry
HRESULT MyCOM::RegisterCLSID() {
	HRESULT hr;
	LPOLESTR clsidString = nullptr;
	hr = StringFromCLSID(MyCOM_CLSID, &clsidString);

	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterCLSID", L"StringFromCLSID failed", true);
	}

	CComBSTR clsidBSTR(clsidString);
	CoTaskMemFree(clsidString);

	// Register ProgID
	hr = SetRegistry(HKEY_CLASSES_ROOT, L"AudioWizard\\CLSID", nullptr, clsidBSTR);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterCLSID", L"Failed to set registry for ProgID", true);
	}

	// Register CLSID key
	std::wstring clsidKeyPath = L"CLSID\\" + std::wstring(clsidBSTR);
	std::wstring modulePath(MAX_PATH, L'\0');
	DWORD len = GetModuleFileNameW(core_api::get_my_instance(), &modulePath[0], MAX_PATH);

	if (len == 0) {
		return AWHCOM::LogError(E_FAIL, L"Audio Wizard => MyCOM::RegisterCLSID", L"GetModuleFileNameW failed", true);
	}
	modulePath.resize(len);

	// Set InProcServer32
	hr = SetRegistry(HKEY_CLASSES_ROOT, clsidKeyPath.c_str(), L"InProcServer32", modulePath.c_str());
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterCLSID", L"Failed to set InProcServer32 registry", true);
	}

	// Set ThreadingModel
	hr = SetRegistry(HKEY_CLASSES_ROOT, clsidKeyPath.c_str(), L"ThreadingModel", L"Apartment");
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::RegisterCLSID", L"Failed to set ThreadingModel registry", true);
	}

	return S_OK;
}

HRESULT MyCOM::UnregisterCLSID() {
	LPOLESTR clsidString = nullptr;
	HRESULT hr = StringFromCLSID(MyCOM_CLSID, &clsidString);

	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::UnregisterCLSID", L"StringFromCLSID failed");
	}

	std::wstring clsidKeyPath = L"CLSID\\" + std::wstring(clsidString);
	CoTaskMemFree(clsidString);

	LONG result = RegDeleteTree(HKEY_CLASSES_ROOT, clsidKeyPath.c_str());
	if (result != ERROR_SUCCESS) {
		AWHCOM::LogError(HRESULT_FROM_WIN32(result), L"Audio Wizard => MyCOM::UnregisterCLSID", L"Failed to delete CLSID registry key");
	}

	result = RegDeleteTree(HKEY_CLASSES_ROOT, L"AudioWizard");
	if (result != ERROR_SUCCESS) {
		AWHCOM::LogError(HRESULT_FROM_WIN32(result), L"Audio Wizard => MyCOM::UnregisterCLSID", L"Failed to delete AudioWizard registry key");
	}

	return S_OK;
}

HRESULT MyCOM::SetRegistry(HKEY rootKey, const wchar_t* subKey, const wchar_t* valueName, const wchar_t* value) {
	CRegKey hKey;
	LONG lResult = hKey.Create(rootKey, subKey);

	if (lResult != ERROR_SUCCESS) {
		return AWHCOM::LogError(HRESULT_FROM_WIN32(lResult), L"Audio Wizard => MyCOM::SetRegistry", L"Failed to create/open registry key", true);
	}

	lResult = hKey.SetStringValue(valueName, value);

	if (lResult != ERROR_SUCCESS) {
		return AWHCOM::LogError(HRESULT_FROM_WIN32(lResult), L"Audio Wizard => MyCOM::SetRegistry", L"Failed to set registry value", true);
	}

	return S_OK;
}
#pragma endregion


/////////////////////////////////////////
// * MyCOM - IUNKNOWN PUBLIC METHODS * //
/////////////////////////////////////////
#pragma region MyCOM - IUnknown Public Methods
STDMETHODIMP MyCOM::QueryInterface(REFIID riid, void** ppvObject) {
	if (riid == IID_IUnknown || riid == IID_IDispatch || riid == MyCOMAPI_IID) {
		*ppvObject = static_cast<IDispatch*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return AWHCOM::LogError(E_NOINTERFACE, L"Audio Wizard => MyCOM::QueryInterface", L"Interface not supported", true);
}

STDMETHODIMP_(ULONG) MyCOM::AddRef() {
	return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) MyCOM::Release() {
	LONG count = InterlockedDecrement(&refCount);

	if (count == 0) {
		delete this;
	}

	return count;
}
#pragma endregion


///////////////////////////////////////////
// * MyCOM - IDISPATCH PRIVATE METHODS * //
///////////////////////////////////////////
#pragma region MyCOM - IDispatch Private Methods
HRESULT MyCOM::InitTypeInfo() {
	if (MyCOM::typeInfo == nullptr && typeLib != nullptr) {
		HRESULT hr = typeLib->GetTypeInfoOfGuid(MyCOMAPI_IID, &MyCOM::typeInfo);

		if (FAILED(hr)) {
			return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::InitTypeInfo", L"GetTypeInfoOfGuid failed", true);
		}
	}

	return S_OK;
}

STDMETHODIMP MyCOM::GetTypeInfoCount(UINT* pctinfo) {
	*pctinfo = (typeInfo != nullptr) ? 1 : 0;
	return S_OK;
}

STDMETHODIMP MyCOM::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {
	if (iTInfo != 0) {
		return AWHCOM::LogError(DISP_E_BADINDEX, L"Audio Wizard => MyCOM::GetTypeInfo", L"Invalid type info index", true);
	}

	if (typeInfo == nullptr) {
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr)) {
			return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetTypeInfo", L"InitTypeInfo failed", true);
		}
	}

	*ppTInfo = typeInfo;
	if (*ppTInfo) {
		(*ppTInfo)->AddRef();
	}

	return S_OK;
}

STDMETHODIMP MyCOM::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	if (typeInfo == nullptr) {
		return AWHCOM::LogError(E_NOTIMPL, L"Audio Wizard => MyCOM::GetIDsOfNames", L"Type info not initialized", true);
	}

	return typeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);
}

STDMETHODIMP MyCOM::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	if (typeInfo == nullptr) {
		return AWHCOM::LogError(E_NOTIMPL, L"Audio Wizard => MyCOM::Invoke", L"Type info not initialized", true);
	}

	return typeInfo->Invoke(static_cast<IDispatch*>(this), dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}
#pragma endregion


//////////////////////////////////////////////////////////
// * MyCOM - PUBLIC API - REAL-TIME METRIC PROPERTIES * //
//////////////////////////////////////////////////////////
#pragma region MyCOM - Public API - Real-Time Metric Properties
STDMETHODIMP MyCOM::get_RawAudioData(SAFEARRAY** data) const {
	if (!data) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_RawAudioData", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_RawAudioData", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetRawAudioData(data);
	return S_OK;
}

STDMETHODIMP MyCOM::get_MomentaryLUFS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_MomentaryLUFS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_MomentaryLUFS", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetMomentaryLUFS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_ShortTermLUFS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_ShortTermLUFS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_ShortTermLUFS", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetShortTermLUFS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_RMS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_RMS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_RMS", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetRMS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_LeftRMS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_LeftRMS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_LeftRMS", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetLeftRMS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_RightRMS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_RightRMS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_RightRMS", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetRightRMS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_LeftSamplePeak(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_LeftSamplePeak", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_LeftSamplePeak", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetLeftSamplePeak(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_RightSamplePeak(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_RightSamplePeak", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_RightSamplePeak", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetRightSamplePeak(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_TruePeak(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_TruePeak", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_TruePeak", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetTruePeak(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PSR(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PSR", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PSR", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetPSR(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PLR(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PLR", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PLR", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetPLR(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_CrestFactor(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_CrestFactor", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_CrestFactor", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetCrestFactor(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_DynamicRange(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_DynamicRange", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_DynamicRange", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetDynamicRange(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PureDynamics(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PureDynamics", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PureDynamics", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetPureDynamics(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PhaseCorrelation(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PhaseCorrelation", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PhaseCorrelation", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetPhaseCorrelation(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_StereoWidth(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_StereoWidth", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_StereoWidth", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->GetStereoWidth(value);
	return S_OK;
}
#pragma endregion


/////////////////////////////////////////////////////////////
// * MyCOM - PUBLIC API - REAL-TIME PEAKMETER PROPERTIES * //
/////////////////////////////////////////////////////////////
#pragma region MyCOM - Public API - Real-Time Peakmeter Properties
STDMETHODIMP MyCOM::get_PeakmeterOffset(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PeakmeterOffset", L"Invalid pointer", false);
	}
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PeakmeterOffset", L"AudioWizard::Peakmeter not available", false);
	}

	AudioWizard::Peakmeter()->GetOffset(value);
	return S_OK;
}

STDMETHODIMP MyCOM::put_PeakmeterOffset(LONG value) const {
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::put_PeakmeterOffset", L"AudioWizard::Peakmeter not available", false);
	}

	AudioWizard::Peakmeter()->SetOffset(static_cast<double>(value));
	return S_OK;
}

STDMETHODIMP MyCOM::get_PeakmeterAdjustedLeftRMS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedLeftRMS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedLeftRMS", L"AudioWizard::Peakmeter not available", false);
	}
	AudioWizard::Peakmeter()->GetAdjustedLeftRMS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PeakmeterAdjustedRightRMS(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedRightRMS", L"Invalid pointer", false);
	}
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedRightRMS", L"AudioWizard::Peakmeter not available", false);
	}
	AudioWizard::Peakmeter()->GetAdjustedRightRMS(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PeakmeterAdjustedLeftSamplePeak(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedLeftSamplePeak", L"Invalid pointer", false);
	}
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedLeftSamplePeak", L"AudioWizard::Peakmeter not available", false);
	}
	AudioWizard::Peakmeter()->GetAdjustedLeftSamplePeak(value);
	return S_OK;
}

STDMETHODIMP MyCOM::get_PeakmeterAdjustedRightSamplePeak(double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedRightSamplePeak", L"Invalid pointer", false);
	}
	if (!AudioWizard::Peakmeter()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_PeakmeterAdjustedRightSamplePeak", L"AudioWizard::Peakmeter not available", false);
	}
	AudioWizard::Peakmeter()->GetAdjustedRightSamplePeak(value);
	return S_OK;
}
#pragma endregion


////////////////////////////////////////////////////
// * MyCOM - PUBLIC API - FULL-TRACK PROPERTIES * //
////////////////////////////////////////////////////
#pragma region MyCOM - Public API - Full-Track Properties
STDMETHODIMP MyCOM::get_FullTrackProcessing(VARIANT_BOOL* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::get_FullTrackProcessing", L"Invalid pointer", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::get_FullTrackProcessing", L"AudioWizard::Main not available", false);
	}

	*value = AudioWizard::Main()->mainFullTrack->fetcher.isFullTrackFetching.load(std::memory_order_acquire) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}
#pragma endregion


////////////////////////////////////////////////
// * MyCOM - PUBLIC API - SYSTEM PROPERTIES * //
////////////////////////////////////////////////
#pragma region MyCOM - Public API - System Properties
STDMETHODIMP MyCOM::put_SystemDebugLog(bool value) const {
	bool oldValue = AudioWizardSettings::systemDebugLog;

	if (oldValue != value) {
		AudioWizardSettings::systemDebugLog = value;
		FB2K_console_formatter() << "Audio Wizard => Debug log " << (value ? "enabled" : "disabled");
	}

	return S_OK;
}
#pragma endregion


////////////////////////////////////////////////////////////
// * MyCOM - PUBLIC API - FULL-TRACK ANALYSIS CALLBACKS * //
////////////////////////////////////////////////////////////
#pragma region MyCOM - Public API - Full-Track Analysis Callbacks
STDMETHODIMP MyCOM::SetFullTrackAnalysisCallback(const VARIANT* callback) {
	if (!callback) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::SetFullTrackAnalysisCallback", L"Invalid callback pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::SetFullTrackAnalysisCallback", L"AudioWizard::Main not available", true);
	}

	AudioWizard::Main()->SetFullTrackAnalysisCallback(callback);
	return S_OK;
}

STDMETHODIMP MyCOM::SetFullTrackWaveformCallback(const VARIANT* callback) {
	if (!callback) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::SetFullTrackWaveformCallback", L"Invalid callback pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::SetFullTrackWaveformCallback", L"AudioWizard::Main not available", true);
	}

	AudioWizard::Main()->SetFullTrackWaveformCallback(callback);
	return S_OK;
}
#pragma endregion


/////////////////////////////////////////////////
// * MyCOM - PUBLIC API - FULL-TRACK METHODS * //
/////////////////////////////////////////////////
#pragma region MyCOM - Public API - Full-Track Methods
STDMETHODIMP MyCOM::StartWaveformAnalysis(VARIANT metadata, LONG pointsPerSec) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StartWaveformAnalysis", L"AudioWizard::Waveform not available", true);
	}

	auto resolution = static_cast<int>(pointsPerSec);
	metadb_handle_list metadb = AWHCOM::GetMetadbHandlesFromStringArray(metadata);

	if (metadb.get_count() == 0) {
		static_api_ptr_t<playlist_manager> playlistManager;
		const t_size playlistIndex = playlistManager->get_active_playlist();
		playlistManager->playlist_get_selected_items(playlistIndex, metadb);
	}

	AudioWizard::Waveform()->StartWaveformAnalysis(metadb, resolution);
	return S_OK;
}

STDMETHODIMP MyCOM::StopWaveformAnalysis() const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StopWaveformAnalysis", L"AudioWizard::Waveform not available", true);
	}

	AudioWizard::Waveform()->StopWaveformAnalysis();
	return S_OK;
}

STDMETHODIMP MyCOM::GetWaveformData(LONG trackIndex, SAFEARRAY** data) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetWaveformData", L"AudioWizard::Waveform not available", true);
	}
	if (!data) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetWaveformData", L"Invalid pointer", true);
	}
	if (trackIndex < 0 || trackIndex >= static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackCount())) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetWaveformData", L"Invalid track index", true);
	}

	AudioWizard::Waveform()->GetWaveformData(static_cast<size_t>(trackIndex), data);
	return S_OK;
}

STDMETHODIMP MyCOM::GetWaveformTrackChannels(LONG trackIndex, LONG* channels) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetWaveformTrackChannels", L"AudioWizard::Waveform not available", true);
	}
	if (!channels) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetWaveformTrackChannels", L"Invalid pointer", true);
	}

	if (trackIndex < 0 || trackIndex >= static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackCount())) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetWaveformData", L"Invalid track index", true);
	}

	*channels = static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackChannels(static_cast<size_t>(trackIndex)));
	return S_OK;
}

STDMETHODIMP MyCOM::GetWaveformTrackCount(LONG* count) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetWaveformTrackCount", L"AudioWizard::Waveform not available", true);
	}
	if (!count) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetWaveformTrackCount", L"Invalid pointer", true);
	}

	*count = static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackCount());
	return S_OK;
}

STDMETHODIMP MyCOM::GetWaveformTrackDuration(LONG trackIndex, DOUBLE* duration) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetWaveformTrackDuration", L"AudioWizard::Waveform not available", true);
	}
	if (!duration) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetWaveformTrackDuration", L"Invalid pointer", true);
	}
	if (trackIndex < 0 || trackIndex >= static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackCount())) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetWaveformTrackDuration", L"Invalid track index", true);
	}

	pfc::string8 trackPath;
	AudioWizard::Waveform()->GetWaveformTrackInfo(static_cast<size_t>(trackIndex), trackPath, *duration);
	return S_OK;
}

STDMETHODIMP MyCOM::GetWaveformTrackPath(LONG trackIndex, BSTR* path) const {
	if (!AudioWizard::Waveform()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetWaveformTrackPath", L"AudioWizard::Waveform not available", true);
	}
	if (!path) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetWaveformTrackPath", L"Invalid pointer", true);
	}
	if (trackIndex < 0 || trackIndex >= static_cast<LONG>(AudioWizard::Waveform()->GetWaveformTrackCount())) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetWaveformTrackPath", L"Invalid track index", true);
	}

	pfc::string8 trackPath;
	double duration;
	AudioWizard::Waveform()->GetWaveformTrackInfo(static_cast<size_t>(trackIndex), trackPath, duration);
	*path = SysAllocString(pfc::stringcvt::string_wide_from_utf8(trackPath).get_ptr());

	return S_OK;
}

STDMETHODIMP MyCOM::StartFullTrackAnalysis(VARIANT metadata, LONG chunkDurationMs) const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StartFullTrackAnalysis",
			L"AudioWizard::Main not available", true);
	}

	metadb_handle_list metadb = AWHCOM::GetMetadbHandlesFromStringArray(metadata);

	// Explicit track lists from JavaScript cannot pass native metadb_handle_ptr across COM boundaries.
	// Spider Monkey Panel or JSplitter only supports basic types (BSTR, VARIANT arrays, etc.), so the standard foobar2000 pattern
	// is to serialize handles as "path\u001Fsubsong" strings and recreate them on the C++ side.
	// Recreation creates new handles without cached metadata > GetMetadataField() would return empty
	// album/artist until info is loaded. We therefore preload metadata asynchronously when explicit
	// tracks are provided, ensuring album metric calculations work correctly.
	if (metadb.get_count() > 0) {
		static_api_ptr_t<metadb_io_v2> io;

		io->load_info_async(
			metadb, metadb_io_v2::load_info_default, nullptr,
			metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui | metadb_io_v2::op_flag_no_errors,
			nullptr
		);
	}

	// If no valid explicit tracks were provided, use current selection fallback
	if (metadb.get_count() == 0) {
		static_api_ptr_t<playlist_manager> pm;
		t_size active = pm->get_active_playlist();
		if (active != pfc_infinite) {
			pm->playlist_get_selected_items(active, metadb);
		}
	}

	if (metadb.get_count() == 0) {
		FB2K_console_formatter() << "Audio Wizard => StartFullTrackAnalysis: No tracks provided or selected";
		AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackAnalysisCallback, false);
		return S_OK;
	}

	auto chunkDuration = static_cast<int>(chunkDurationMs);
	AudioWizard::Main()->StartFullTrackAnalysis(metadb, chunkDuration);
	return S_OK;
}

STDMETHODIMP MyCOM::GetFullTrackAnalysis(VARIANT_BOOL* pSuccess) const {
	if (!pSuccess) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetFullTrackAnalysis", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetFullTrackAnalysis", L"AudioWizard::Main not available", true);
	}

	*pSuccess = AudioWizard::Main()->GetFullTrackAnalysis() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP MyCOM::GetFullTrackMetrics(SAFEARRAY** metrics) {
	if (!metrics) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetFullTrackMetrics", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetFullTrackMetrics", L"AudioWizard::Main not available", true);
	}

	AudioWizard::Main()->GetFullTrackMetrics(metrics);
	return S_OK;
}

STDMETHODIMP MyCOM::GetMomentaryLUFSFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetMomentaryLUFSFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetMomentaryLUFSFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetMomentaryLUFSFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetMomentaryLUFSFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetShortTermLUFSFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetShortTermLUFSFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetShortTermLUFSFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetShortTermLUFSFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetShortTermLUFSFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetIntegratedLUFSFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetIntegratedLUFSFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetIntegratedLUFSFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetIntegratedLUFSFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetIntegratedLUFSFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetRMSFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetRMSFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetRMSFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetRMSFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetRMSFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetSamplePeakFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetSamplePeakFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetSamplePeakFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetSamplePeakFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetSamplePeakFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetTruePeakFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetTruePeakFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetTruePeakFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetTruePeakFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetTruePeakFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetPSRFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetPSRFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetPSRFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetPSRFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetPSRFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetPLRFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetPLRFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetPLRFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetPLRFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetPLRFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetCrestFactorFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetCrestFactorFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetCrestFactorFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetCrestFactorFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetCrestFactorFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetLoudnessRangeFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetLoudnessRangeFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetLoudnessRangeFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetLoudnessRangeFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetLoudnessRangeFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetDynamicRangeFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetDynamicRangeFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetDynamicRangeFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetDynamicRangeFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetDynamicRangeFull(trackIndexValue);
	return S_OK;
}

STDMETHODIMP MyCOM::GetPureDynamicsFull(VARIANT* trackIndex, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetPureDynamicsFull", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetPureDynamicsFull", L"AudioWizard::Main not available", true);
	}

	LONG trackIndexValue;
	HRESULT hr = AWHCOM::GetOptionalLong(trackIndex, trackIndexValue);
	if (FAILED(hr)) {
		return AWHCOM::LogError(hr, L"Audio Wizard => MyCOM::GetPureDynamicsFull", L"Invalid track index, must be a valid integer", true);
	}

	*value = AudioWizard::Main()->GetPureDynamicsFull(trackIndexValue);
	return S_OK;
}
#pragma endregion


///////////////////////////////////////////////////////
// * MyCOM - PUBLIC API - FULL-TRACK ALBUM METHODS * //
///////////////////////////////////////////////////////
#pragma region MyCOM - Public API - Full-Track Album Methods
STDMETHODIMP MyCOM::GetDynamicRangeAlbumFull(BSTR albumName, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetDynamicRangeAlbum", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetDynamicRangeAlbum", L"AudioWizard::Main not available", true);
	}
	if (!albumName) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetDynamicRangeAlbum", L"Invalid album name", true);
	}

	auto dynamicRangeAlbum = AudioWizard::Main()->GetAlbumMetricFull(
		[](const AudioWizardAnalysisFullTrack::FullTrackResults& r) { return r.dynamicRange; }
	);
	auto it = dynamicRangeAlbum.find(albumName);
	*value = (it != dynamicRangeAlbum.end()) ? it->second : -INFINITY;

	return S_OK;
}

STDMETHODIMP MyCOM::GetPureDynamicsAlbumFull(BSTR albumName, double* value) const {
	if (!value) {
		return AWHCOM::LogError(E_POINTER, L"Audio Wizard => MyCOM::GetPureDynamicsAlbum", L"Invalid pointer", true);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::GetPureDynamicsAlbum", L"AudioWizard::Main not available", true);
	}
	if (!albumName) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::GetPureDynamicsAlbum", L"Invalid album name", true);
	}

	auto pureDynamicsAlbum = AudioWizard::Main()->GetAlbumMetricFull(
		[](const AudioWizardAnalysisFullTrack::FullTrackResults& r) { return r.pureDynamics; }
	);
	auto it = pureDynamicsAlbum.find(albumName);
	*value = (it != pureDynamicsAlbum.end()) ? it->second : -INFINITY;

	return S_OK;
}
#pragma endregion


////////////////////////////////////////////////
// * MyCOM - PUBLIC API - REAL-TIME METHODS * //
////////////////////////////////////////////////
#pragma region MyCOM - Public API - Real-Time Methods
STDMETHODIMP MyCOM::SetMonitoringRefreshRate(LONG refreshRateMs) const {
	if (refreshRateMs < 10 || refreshRateMs > 1000) {
		return AWHCOM::LogError(E_INVALIDARG, L"Audio Wizard => MyCOM::SetMonitoringRefreshRate", L"Invalid refresh rate, must be 10-1000", false);
	}
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::SetMonitoringRefreshRate", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->SetMonitoringRefreshRate(refreshRateMs);
	return S_OK;
}

STDMETHODIMP MyCOM::StartRealTimeMonitoring(LONG refreshRateMs, LONG chunkDurationMs) const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StartRealTimeMonitoring", L"AudioWizard::Main not available", false);
	}

	auto refreshRate = static_cast<int>(refreshRateMs);
	auto chunkDuration = static_cast<int>(chunkDurationMs);

	AudioWizard::Main()->StartRealTimeMonitoring(refreshRate, chunkDuration);
	return S_OK;
}

STDMETHODIMP MyCOM::StopRealTimeMonitoring() const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StopRealTimeMonitoring", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->StopRealTimeMonitoring();
	return S_OK;
}

STDMETHODIMP MyCOM::StartRawAudioMonitoring(LONG refreshRateMs, LONG chunkDurationMs) const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StartRawAudioMonitoring", L"AudioWizard::Main not available", false);
	}

	auto refreshRate = static_cast<int>(refreshRateMs);
	auto chunkDuration = static_cast<int>(chunkDurationMs);

	AudioWizard::Main()->StartRawAudioMonitoring(refreshRate, chunkDuration);
	return S_OK;
}

STDMETHODIMP MyCOM::StopRawAudioMonitoring() const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StopRawAudioMonitoring", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->StopRawAudioMonitoring();
	return S_OK;
}

STDMETHODIMP MyCOM::StartPeakmeterMonitoring(LONG refreshRateMs, LONG chunkDurationMs) const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StartPeakmeterMonitoring", L"AudioWizard::Main not available", false);
	}

	auto refreshRate = static_cast<int>(refreshRateMs);
	auto chunkDuration = static_cast<int>(chunkDurationMs);

	AudioWizard::Main()->StartPeakmeterMonitoring(refreshRate, chunkDuration);
	return S_OK;
}

STDMETHODIMP MyCOM::StopPeakmeterMonitoring() const {
	if (!AudioWizard::Main()) {
		return AWHCOM::LogError(E_UNEXPECTED, L"Audio Wizard => MyCOM::StopPeakmeterMonitoring", L"AudioWizard::Main not available", false);
	}

	AudioWizard::Main()->StopPeakmeterMonitoring();
	return S_OK;
}
#pragma endregion


///////////////////////////////////////////////////////
// * MyCOM CLASS FACTORY - IUNKNOWN PUBLIC METHODS * //
///////////////////////////////////////////////////////
#pragma region MyCOM Class Factory - IUnknown Public Methods
STDMETHODIMP MyCOMClassFactory::QueryInterface(REFIID riid, void** ppvObject) {
	if (riid == IID_IUnknown || riid == IID_IClassFactory) {
		*ppvObject = static_cast<IClassFactory*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return AWHCOM::LogError(E_NOINTERFACE, L"Audio Wizard => MyCOMClassFactory::QueryInterface", L"Interface not supported", true);
}

STDMETHODIMP_(ULONG) MyCOMClassFactory::AddRef() {
	return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) MyCOMClassFactory::Release() {
	LONG count = InterlockedDecrement(&refCount);

	if (count == 0) {
		delete this;
	}

	return count;
}
#pragma endregion


/////////////////////////////////////////////////////////////
// * MyCOM CLASS FACTORY - ICLASSFACTORY PRIVATE METHODS * //
/////////////////////////////////////////////////////////////
#pragma region MyCOM Class Factory - IClassFactory Private Methods
STDMETHODIMP MyCOMClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) {
	if (pUnkOuter != nullptr) {
		return AWHCOM::LogError(CLASS_E_NOAGGREGATION, L"Audio Wizard => MyCOMClassFactory::CreateInstance", L"Aggregation not supported", true);
	}

	CComPtr<MyCOM> pMyCOM = new MyCOM();
	return pMyCOM->QueryInterface(riid, ppvObject);
}

STDMETHODIMP MyCOMClassFactory::LockServer(BOOL fLock) {
	fLock ? InterlockedIncrement(&MyCOM::serverLocks) :
		InterlockedDecrement(&MyCOM::serverLocks);

	return S_OK;
}
#pragma endregion
