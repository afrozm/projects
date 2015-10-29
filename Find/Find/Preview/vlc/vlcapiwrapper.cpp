#include "stdafx.h"
#include "vlcapiwrapper.h"
#include "SystemUtils.h"

HMODULE ghLibVLC(NULL);
HMODULE ghLibVLCCore(NULL);

static TCHAR sVLCPath[MAX_PATH] = {0};

bool VLC_GetVLCPath(LPTSTR outVLCPath)
{
	if (sVLCPath[0] == 0) {
		DWORD size(sizeof(sVLCPath));
		if (RegGetValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\VideoLAN\\VLC"), _T("InstallDir"),
			RRF_RT_REG_SZ, NULL, sVLCPath, &size)) {
			SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, sVLCPath);
			PathAppend(sVLCPath, _T("VideoLAN\\VLC"));
		}
	}
	lstrcpy(outVLCPath, sVLCPath);
	return PathFileExists(outVLCPath) == TRUE;
}

HMODULE LoadLibrary(LPCTSTR inDir, LPCTSTR inLibName)
{
	TCHAR libPath[MAX_PATH];
	lstrcpy(libPath, inDir);
	PathAppend(libPath, inLibName);
	return LoadLibrary(libPath);
}

bool VLC_InitLib(LPCTSTR vlcPath)
{
	if (ghLibVLC == NULL) {
		TCHAR defvlcPath[MAX_PATH];
		if (vlcPath == NULL && VLC_GetVLCPath(defvlcPath)) {
			vlcPath = defvlcPath;
		}
		// Compare version - VLC version should be higher or equal to 1.1.x.x
		if (vlcPath != NULL) {
			TCHAR libPath[MAX_PATH];
			lstrcpy(libPath, vlcPath);
			PathAppend(libPath, _T("libvlccore.dll"));
			DWORD ms(0),ls(0);
			
			if (!SystemUtils::GetFileVersion(libPath, ms, ls)
				|| ms < MAKELONG(1,1)) {
				vlcPath = NULL; // Lower version or not found - fail it
			}
		}
		if (vlcPath != NULL) {
			ghLibVLCCore = LoadLibrary(vlcPath, _T("libvlccore.dll"));
			ghLibVLC = LoadLibrary(vlcPath, _T("libvlc.dll"));
			if (ghLibVLC != NULL)
				lstrcpy(sVLCPath, vlcPath);
		}
	}
	return ghLibVLC != NULL;
}
FARPROC WINAPI VLC_GetProcAddress(LPCSTR lpProcName)
{
	FARPROC proc(NULL);
	if (ghLibVLC != NULL)
		proc = GetProcAddress(ghLibVLC, lpProcName);
	return proc;
}

#define VLCGETPROC(fn)\
		static fn_##fn pfn_##fn = NULL;\
		if (pfn_##fn == NULL)\
			pfn_##fn = (fn_##fn)VLC_GetProcAddress(#fn);\
		if (pfn_##fn != NULL)\

#define DEFINE_VLC_RET_FN_1ARG(rt, fn, argType, argName) \
	typedef rt (*fn_##fn)(argType);\
	LIBVLC_API rt fn(argType argName)\
	{\
		rt retVal(0);\
		VLCGETPROC(fn)\
			retVal = pfn_##fn(argName);\
		return retVal;\
	}

#define DEFINE_VLC_RET_FN_2ARG(rt, fn, argType, argName, argType2, argName2) \
	typedef rt (*fn_##fn)(argType,argType2);\
	LIBVLC_API rt fn(argType argName, argType2 argName2)\
	{\
		rt retVal(0);\
		VLCGETPROC(fn)\
			retVal = pfn_##fn(argName,argName2);\
		return retVal;\
	}

#define DEFINE_VLC_RET_FN_3ARG(rt, fn, argType, argName, argType2, argName2, argType3, argName3) \
	typedef rt (*fn_##fn)(argType,argType2,argType3);\
	LIBVLC_API rt fn(argType argName, argType2 argName2, argType3 argName3)\
	{\
		rt retVal(0);\
		VLCGETPROC(fn)\
			retVal = pfn_##fn(argName,argName2,argName3);\
		return retVal;\
	}

#define DEFINE_VLC_RET_FN_4ARG(rt, fn, argType, argName, argType2, argName2, argType3, argName3, argType4, argName4) \
	typedef rt (*fn_##fn)(argType,argType2,argType3,argType4);\
	LIBVLC_API rt fn(argType argName, argType2 argName2, argType3 argName3, argType4 argName4)\
	{\
		rt retVal(0);\
		VLCGETPROC(fn)\
			retVal = pfn_##fn(argName,argName2,argName3,argName4);\
		return retVal;\
	}

#define DEFINE_VLC_FN_1ARG(fn, argType, argName) \
	typedef void (*fn_##fn)(argType);\
	LIBVLC_API void fn(argType argName)\
	{\
		VLCGETPROC(fn)\
			pfn_##fn(argName);\
	}

#define DEFINE_VLC_FN_2ARG(fn, argType, argName, argType2, argName2) \
	typedef void (*fn_##fn)(argType, argType2);\
	LIBVLC_API void fn(argType argName, argType2 argName2)\
	{\
		VLCGETPROC(fn)\
			pfn_##fn(argName, argName2);\
	}

#define DEFINE_VLC_FN_3ARG(fn, argType, argName, argType2, argName2, argType3, argName3) \
	typedef void (*fn_##fn)(argType, argType2, argType3);\
	LIBVLC_API void fn(argType argName, argType2 argName2, argType3 argName3)\
	{\
		VLCGETPROC(fn)\
			pfn_##fn(argName, argName2, argName3);\
	}

#define DEFINE_VLC_FN_5ARG(fn, argType, argName, argType2, argName2, argType3, argName3, argType4, argName4, argType5, argName5) \
	typedef void (*fn_##fn)(argType, argType2, argType3, argType4, argType5);\
	LIBVLC_API void fn(argType argName, argType2 argName2, argType3 argName3, argType4 argName4, argType5 argName5)\
	{\
		VLCGETPROC(fn)\
			pfn_##fn(argName, argName2, argName3, argName4, argName5);\
	}

DEFINE_VLC_RET_FN_2ARG(libvlc_instance_t*, libvlc_new, int, size, const char *const *, vlc_args)
DEFINE_VLC_RET_FN_1ARG(libvlc_media_player_t*, libvlc_media_player_new, libvlc_instance_t*, libvlc_ins)
DEFINE_VLC_RET_FN_1ARG(libvlc_event_manager_t*, libvlc_media_player_event_manager, libvlc_media_player_t*, mp)
DEFINE_VLC_FN_1ARG(libvlc_media_release, libvlc_media_t*, p_meta_desc)
DEFINE_VLC_FN_1ARG(libvlc_media_player_release, libvlc_media_player_t*, mp)
DEFINE_VLC_FN_1ARG(libvlc_release, libvlc_instance_t*, li)
DEFINE_VLC_FN_2ARG(libvlc_media_player_set_hwnd, libvlc_media_player_t*, mp, void*, drawable)
DEFINE_VLC_RET_FN_4ARG(int, libvlc_event_attach, libvlc_event_manager_t*, p_event_manager, libvlc_event_type_t, i_event_type, libvlc_callback_t, f_callback, void*, user_data)
DEFINE_VLC_RET_FN_1ARG(int, libvlc_media_player_play, libvlc_media_player_t*, mp)
DEFINE_VLC_FN_1ARG(libvlc_media_player_pause, libvlc_media_player_t*, mp)
DEFINE_VLC_FN_1ARG(libvlc_media_player_stop, libvlc_media_player_t*, mp)
DEFINE_VLC_RET_FN_1ARG(int64_t, libvlc_media_player_get_length, libvlc_media_player_t*, mp)
DEFINE_VLC_RET_FN_1ARG(int64_t, libvlc_media_player_get_time, libvlc_media_player_t*, mp)
DEFINE_VLC_FN_2ARG(libvlc_media_player_set_time, libvlc_media_player_t*, mp, libvlc_time_t, nt)
DEFINE_VLC_FN_2ARG(libvlc_audio_set_mute, libvlc_media_player_t*, p_mi, int, status)
DEFINE_VLC_RET_FN_1ARG(int, libvlc_audio_get_mute, libvlc_media_player_t*, p_mi)
DEFINE_VLC_RET_FN_1ARG(int, libvlc_audio_get_volume, libvlc_media_player_t*, p_mi)
DEFINE_VLC_RET_FN_2ARG(int, libvlc_audio_set_volume, libvlc_media_player_t*, p_mi, int, i_volume)
DEFINE_VLC_RET_FN_2ARG(libvlc_media_t*, libvlc_media_new_path, libvlc_instance_t*, p_instance, const char*, path)
DEFINE_VLC_FN_2ARG(libvlc_media_player_set_media, libvlc_media_player_t*, mp, libvlc_media_t*, media)
DEFINE_VLC_RET_FN_1ARG(unsigned, libvlc_media_player_has_vout, libvlc_media_player_t*, p_mi)
DEFINE_VLC_FN_1ARG(libvlc_toggle_fullscreen, libvlc_media_player_t*, p_mi)
DEFINE_VLC_FN_2ARG(libvlc_set_fullscreen, libvlc_media_player_t*, p_mi, int, b_fullscreen)
DEFINE_VLC_RET_FN_1ARG(int, libvlc_get_fullscreen, libvlc_media_player_t*, p_mi)