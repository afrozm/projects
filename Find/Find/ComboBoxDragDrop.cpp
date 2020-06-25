// ComboBoxDragDrop.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "ComboBoxDragDrop.h"


// CComboBoxDragDrop

IMPLEMENT_DYNAMIC(CComboBoxDragDrop, CComboBox)

CComboBoxDragDrop::CComboBoxDragDrop()
{

}

CComboBoxDragDrop::~CComboBoxDragDrop()
{
}


void CComboBoxDragDrop::OnDropFiles(HDROP hDropInfo)
{
    int nFilesDropped = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    if (nFilesDropped > 0) {
        DWORD   nBuffer = DragQueryFile(hDropInfo, 0, NULL, 0);
        CString sFile;
        DragQueryFile(hDropInfo, 0, sFile.GetBuffer(nBuffer + 1), nBuffer + 1);
        sFile.ReleaseBuffer();
        SetWindowText(sFile);
    }
    DragFinish(hDropInfo);

}

BEGIN_MESSAGE_MAP(CComboBoxDragDrop, CComboBox)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CComboBoxDragDrop message handlers


