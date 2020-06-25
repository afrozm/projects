/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapperImpl.cpp
    Desc.:  VLCWrapperImpl Implementation.

    Author: Alex Skoruppa
    Date:   08/10/2009
    eM@il:  alex.skoruppa@googlemail.com

    VLCWrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
     
    VLCWrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
     
    You should have received a copy of the GNU General Public License
    along with VLCWrapper.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/
#include "stdafx.h"
#include "VLCWrapperImpl.h"
#include <stdio.h>
#include "SystemUtils.h"

VLCWrapperImpl::VLCWrapperImpl(void)
:	m_pVLCInstance(0),
	m_pMediaPlayer(0),
	m_pMedia(0),
    m_pEvtManager(0),
    m_EvtH(0)
{
	TCHAR pluginPath[MAX_PATH];
	VLC_GetVLCPath(pluginPath);
	char pluginPathArg[2*MAX_PATH];
	sprintf_s(pluginPathArg, 2*MAX_PATH, "--plugin-path=%s",
		SystemUtils::UnicodeToUTF8(pluginPath).c_str());

	const char * const vlc_args[] = {
		"-I", "dumy",      // No special interface
		"--ignore-config", // Don't use VLC's config
		pluginPathArg };

	// init vlc modules, should be done only once
	m_pVLCInstance = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
     
    // Create a media player playing environement
    m_pMediaPlayer = libvlc_media_player_new(m_pVLCInstance);

    // Create an event manager for the player for handling e.g. time change events
    m_pEvtManager=libvlc_media_player_event_manager(m_pMediaPlayer);
}

VLCWrapperImpl::~VLCWrapperImpl(void)
{
	if (m_pMedia) {
		libvlc_media_release(m_pMedia);
		m_pMedia = NULL;
	}
    // Free the media_player
    libvlc_media_player_release (m_pMediaPlayer);
	libvlc_release (m_pVLCInstance);
}

void VLCWrapperImpl::SetOutputWindow(void* pHwnd)
{
    // Set the output window    
	libvlc_media_player_set_hwnd(m_pMediaPlayer, pHwnd);
}

int VLCWrapperImpl::SetEventHandler(VLCEventHandler evt, libvlc_event_type_t eventType, void *pUserData)
{
    m_EvtH=evt;
    return libvlc_event_attach(m_pEvtManager,        
                        eventType,
                        m_EvtH,
                        pUserData);//void *user_data,
}

int VLCWrapperImpl::Play()
{
	// play the media_player
    return libvlc_media_player_play (m_pMediaPlayer);
}

void VLCWrapperImpl::Pause()
{
	// Pause playing
    libvlc_media_player_pause (m_pMediaPlayer);
}

void VLCWrapperImpl::Stop()
{
    // Stop playing
    libvlc_media_player_stop (m_pMediaPlayer);
}

int64_t VLCWrapperImpl::GetLength()
{
    int64_t iLength=libvlc_media_player_get_length(m_pMediaPlayer);
    return iLength;
}

int64_t VLCWrapperImpl::GetTime()
{
    int64_t iTime=libvlc_media_player_get_time(m_pMediaPlayer);
    return iTime;
}

void VLCWrapperImpl::SetTime(int64_t llNewTime)
{
    libvlc_media_player_set_time(m_pMediaPlayer,(libvlc_time_t)llNewTime);
}

void VLCWrapperImpl::Mute(bool bMute)
{
    libvlc_audio_set_mute(m_pMediaPlayer, bMute);
}

bool VLCWrapperImpl::GetMute()
{
    bool bMuteState=!!libvlc_audio_get_mute(m_pMediaPlayer);
    return bMuteState;
}

int VLCWrapperImpl::GetVolume()
{
    int iVolume=libvlc_audio_get_volume(m_pMediaPlayer);
    return iVolume;
}

int VLCWrapperImpl::SetVolume(int iVolume)
{
    return libvlc_audio_set_volume(m_pMediaPlayer, iVolume);
}

void VLCWrapperImpl::OpenMedia(const char* pszMediaPathName)
{
	// Load a new item
	if (m_pMedia) {
		libvlc_media_release(m_pMedia);
		m_pMedia = NULL;
	}
	m_pMedia = libvlc_media_new_path (m_pVLCInstance, pszMediaPathName);
    libvlc_media_player_set_media (m_pMediaPlayer, m_pMedia);    
}
int VLCWrapperImpl::HasVOut()
{
    int hasVOut = libvlc_media_player_has_vout(m_pMediaPlayer);
	return hasVOut;
}
void VLCWrapperImpl::ToggleFullScreen()
{
	libvlc_toggle_fullscreen(m_pMediaPlayer);
}
void VLCWrapperImpl::SetFullScreen(bool bSetFullScreen)
{
	libvlc_set_fullscreen(m_pMediaPlayer, bSetFullScreen);
}
bool VLCWrapperImpl::GetFullScreen()
{
	return libvlc_get_fullscreen(m_pMediaPlayer) != 0;
}