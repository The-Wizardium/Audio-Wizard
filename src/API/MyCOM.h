/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: COM Automation and ActiveX Interface                    * //
// * Description:    MyCOM Header File                                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


///////////////
// * MyCOM * //
///////////////
#pragma region MyCOM
class MyCOM : public IDispatch {
public:
	MyCOM();
	~MyCOM();

	using CLSIDFromProgID_t = HRESULT(WINAPI*)(LPCOLESTR, LPCLSID);

	// * STATIC VARS * //
	static inline IID MyCOMAPI_IID = { 0xb8142e5d, 0xce24, 0x4243, { 0x91, 0x6, 0x2f, 0xc7, 0x7b, 0x13, 0x44, 0xfa } };
	static inline IID MyCOMLIB_IID = { 0xe7c1cada, 0x4e64, 0x4ae0, { 0xb4, 0xd0, 0x7d, 0x31, 0xfe, 0x87, 0x34, 0xbd } };
	static inline CLSID MyCOM_CLSID = { 0xf8dd1b13, 0xd097, 0x45fc, { 0xa7, 0xa6, 0xab, 0x90, 0xf, 0xc8, 0xb0, 0x19 } };
	static inline LPCOLESTR MyCOM_PROGID = OLESTR("AudioWizard");

	static inline LONG activeObjects = 0;
	static inline DWORD dwRegister = 0;
	static inline LONG serverLocks = 0;
	static inline CLSIDFromProgID_t original_CLSIDFromProgID = nullptr;
	static inline CComPtr<ITypeLib> typeLib;
	static inline CComPtr<ITypeInfo> typeInfo;

	// * PUBLIC STATIC METHODS * //
	static HRESULT InitMyCOM();
	static HRESULT InitTypeLibrary(HMODULE hModule);
	static HRESULT RegisterMyCOM(std::wstring_view regMethod = L"RegFree"); // "RegFree" or "RegEntry"
	static HRESULT UnregisterMyCOM(std::wstring_view regMethod = L"RegFree"); // "RegFree" or "RegEntry"
	static HRESULT QuitMyCOM(std::wstring_view regMethod = L"RegFree"); // "RegFree" or "RegEntry"

	// * IUNKNOWN METHODS * //
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

	// * PUBLIC API - REAL-TIME METRIC PROPERTIES * //
	STDMETHOD(get_RawAudioData)(SAFEARRAY** data) const;
	STDMETHOD(get_MomentaryLUFS)(double* value) const;
	STDMETHOD(get_ShortTermLUFS)(double* value) const;
	STDMETHOD(get_RMS)(double* value) const;
	STDMETHOD(get_LeftRMS)(double* value) const;
	STDMETHOD(get_RightRMS)(double* value) const;
	STDMETHOD(get_LeftSamplePeak)(double* value) const;
	STDMETHOD(get_RightSamplePeak)(double* value) const;
	STDMETHOD(get_TruePeak)(double* value) const;
	STDMETHOD(get_PSR)(double* value) const;
	STDMETHOD(get_PLR)(double* value) const;
	STDMETHOD(get_CrestFactor)(double* value) const;
	STDMETHOD(get_DynamicRange)(double* value) const;
	STDMETHOD(get_PureDynamics)(double* value) const;
	STDMETHOD(get_PhaseCorrelation)(double* value) const;
	STDMETHOD(get_StereoWidth)(double* value) const;

	// * PUBLIC API - REAL-TIME PEAKMETER PROPERTIES * //
	STDMETHOD(get_PeakmeterOffset)(double* value) const;
	STDMETHOD(put_PeakmeterOffset)(LONG value) const;
	STDMETHOD(get_PeakmeterAdjustedLeftRMS)(double* value) const;
	STDMETHOD(get_PeakmeterAdjustedRightRMS)(double* value) const;
	STDMETHOD(get_PeakmeterAdjustedLeftSamplePeak)(double* value) const;
	STDMETHOD(get_PeakmeterAdjustedRightSamplePeak)(double* value) const;

	// * PUBLIC API - WAVEFORM PROPERTIES * //
	STDMETHOD(get_WaveformData)(SAFEARRAY** data) const;
	STDMETHOD(put_WaveformMetric)(LONG metric) const;

	// * PUBLIC API - FULL-TRACK ANALYSIS CALLBACKS * //
	STDMETHOD(SetFullTrackAnalysisCallback)(const VARIANT* callback);
	STDMETHOD(SetFullTrackWaveformCallback)(const VARIANT* callback);

	// * PUBLIC API - FULL-TRACK METHODS * //
	STDMETHOD(StartWaveformAnalysis)(LONG resolutionSec) const;
	STDMETHOD(StopWaveformAnalysis)() const;
	STDMETHOD(StartFullTrackAnalysis)(LONG chunkDurationMs) const;
	STDMETHOD(GetFullTrackAnalysis)(VARIANT_BOOL* pSuccess) const;
	STDMETHOD(GetFullTrackMetrics)(SAFEARRAY** metrics);
	STDMETHOD(GetMomentaryLUFSFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetShortTermLUFSFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetIntegratedLUFSFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetRMSFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetSamplePeakFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetTruePeakFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetPSRFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetPLRFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetCrestFactorFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetLoudnessRangeFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetDynamicRangeFull)(VARIANT* trackIndex, double* value) const;
	STDMETHOD(GetPureDynamicsFull)(VARIANT* trackIndex, double* value) const;

	// * PUBLIC API - FULL-TRACK ALBUM METHODS * //
	STDMETHOD(GetDynamicRangeAlbumFull)(BSTR albumName, double* value) const;
	STDMETHOD(GetPureDynamicsAlbumFull)(BSTR albumName, double* value) const;

	// * PUBLIC API - REAL-TIME METHODS * //
	STDMETHOD(SetMonitoringRefreshRate)(LONG refreshRateMs) const;
	STDMETHOD(StartRealTimeMonitoring)(LONG refreshRateMs, LONG chunkDurationMs) const;
	STDMETHOD(StopRealTimeMonitoring)() const;
	STDMETHOD(StartRawAudioMonitoring)(LONG refreshRateMs, LONG chunkDurationMs) const;
	STDMETHOD(StopRawAudioMonitoring)() const;
	STDMETHOD(StartPeakmeterMonitoring)(LONG refreshRateMs, LONG chunkDurationMs) const;
	STDMETHOD(StopPeakmeterMonitoring)() const;

private:
	LONG refCount = 0;

	// * STATIC METHODS * //
	static HRESULT WINAPI MyCLSIDFromProgID(LPCOLESTR lpszProgID, LPCLSID lpclsid); // "RegFree" with MinHook method
	static HRESULT HookCLSIDFromProgID(); // "RegFree" with MinHook method
	static HRESULT RegisterCLSID(); // "RegEntry" method
	static HRESULT UnregisterCLSID(); // "RegEntry" method
	static HRESULT SetRegistry(HKEY rootKey, const wchar_t* subKey, const wchar_t* valueName, const wchar_t* value); // "RegEntry" method

	// * IDISPATCH METHODS * //
	static HRESULT InitTypeInfo();
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) override;
	STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
	STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override;
	STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override;
};
#pragma endregion


/////////////////////////////
// * MyCOM CLASS FACTORY * //
/////////////////////////////
#pragma region MyCOM Class Factory
class MyCOMClassFactory : public IClassFactory {
public:
	MyCOMClassFactory() = default;
	~MyCOMClassFactory() = default;

	// * IUNKNOWN METHODS * //
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

private:
	LONG refCount = 0;

	// * ICLASSFACTORY METHODS * //
	STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
	STDMETHODIMP LockServer(BOOL fLock) override;
};
#pragma endregion
