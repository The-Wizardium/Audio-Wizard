/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Precompiled Header File                    * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#define NOMINMAX
#define STRICT
#define WIN32_LEAN_AND_MEAN


// * System headers * //
#include <Windows.h> // WIN32_LEAN_AND_MEAN must precede Windows.h
#include <commdlg.h>
#include <malloc.h>
#include <mmsystem.h> // When WIN32_LEAN_AND_MEAN macro is used, mmsystem.h is needed for timeGetTime in pfc/timers.h
#include <ShObjIdl.h>

// * Standard C++ libraries * //
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <ctime>
#include <deque>
#include <execution>
#include <functional>
#include <future>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// * External libraries * //
#include <atlbase.h>
#include <atlconv.h>
#include <atltime.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <MinHook.h>
#include <Pdh.h>
#include <TlHelp32.h>

// * foobar2000 SDK * //
#ifdef __cplusplus
#include <helpers/foobar2000+atl.h>
#endif
#include <helpers/advconfig_impl.h>
#include <helpers/atl-misc.h>
#include <helpers/window_placement_helper.h>
#include <SDK/cfg_var.h>
#include <SDK/coreDarkMode.h>
using audioType = audio_sample; // Alias for foobar2000 SDK `audio_sample` type => float (x86) or double (x64)

// * macOS-specific includes * //
#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

// * Linking directives //
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Pdh.lib")
