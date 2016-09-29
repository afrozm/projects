#pragma once


// CNEXDlg dialog
#include "afxdialogex.h"

#ifdef NEX_DLG

#include "NEXDlg.h"

#define CMFCBaseDlg CNEXDlg

#else

#define CMFCBaseDlg CDialogEx

#endif