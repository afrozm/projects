
#pragma once

typedef __int64                         int64_t;                      ///< For old MS Compilers.
typedef unsigned __int32	uint32_t;
#include "vlc.h"

bool VLC_InitLib(LPCTSTR vlcPath = NULL);
bool VLC_GetVLCPath(LPTSTR outVLCPath);
