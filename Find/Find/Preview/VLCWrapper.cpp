/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapper.cpp
    Desc.:  VLCWrapper Implementation.

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
#include "ThreadManager.h"

VLCWrapper::VLCWrapper(void) : mbStopping(false)
{
    m_pImpl=new VLCWrapperImpl();
}

VLCWrapper::~VLCWrapper(void)
{
	delete m_pImpl;
}

void VLCWrapper::SetOutputWindow(void* pHwnd)
{
    m_pImpl->SetOutputWindow(pHwnd);
}

void VLCWrapper::SetEventHandler(VLCEventHandler evt, libvlc_event_type_t eventType, void *pUserData)
{
    m_pImpl->SetEventHandler(evt, eventType, pUserData);
}

int VLCWrapper::Play()
{
    return m_pImpl->Play();
}

void VLCWrapper::Pause()
{
    m_pImpl->Pause();
}
static int ThreadProcFn_StopMedia(LPVOID *pThreadData)
{
	VLCWrapperImpl* pImpl((VLCWrapperImpl*)pThreadData);
	pImpl->Stop();
	return 0;
}
void VLCWrapper::Stop()
{
	TRACE("Stopping %d\n", mbStopping);
	if (!mbStopping) {
		mbStopping = true;
		m_pImpl->Stop();
		mbStopping = false;
	}
	TRACE("Stop %d\n", mbStopping);
	//ThreadManager::GetInstance().CreateThread(ThreadProcFn_StopMedia, m_pImpl, 127);
}

int64_t VLCWrapper::GetLength()
{
    return m_pImpl->GetLength();
}

int64_t VLCWrapper::GetTime()
{
    return m_pImpl->GetTime();
}

void VLCWrapper::SetTime(int64_t llNewTime)
{
    m_pImpl->SetTime(llNewTime);
}

void VLCWrapper::Mute(bool bMute)
{
    m_pImpl->Mute(bMute);
}

bool VLCWrapper::GetMute()
{
    return m_pImpl->GetMute();
}

int  VLCWrapper::GetVolume()
{
    return m_pImpl->GetVolume();
}

void VLCWrapper::SetVolume(int iVolume)
{
    m_pImpl->SetVolume(iVolume);
}

void VLCWrapper::OpenMedia(const char* pszMediaPathName)
{
	TRACE("opening %s\n", pszMediaPathName);
    m_pImpl->OpenMedia(pszMediaPathName);
	TRACE("opened %s\n", pszMediaPathName);
}

int  VLCWrapper::HasVOut()
{
    return m_pImpl->HasVOut();
}
void  VLCWrapper::ToggleFullScreen()
{
    m_pImpl->ToggleFullScreen();
}
void  VLCWrapper::SetFullScreen(bool bSetFullScreen)
{
    m_pImpl->SetFullScreen(bSetFullScreen);
}
bool  VLCWrapper::GetFullScreen()
{
    return m_pImpl->GetFullScreen();
}
