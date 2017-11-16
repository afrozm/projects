// DialogPreviewImages.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewImages.h"
#include <gdiplus.h>
using namespace Gdiplus;

// CDialogPreviewImages dialog

IMPLEMENT_DYNAMIC(CDialogPreviewImages, CDialogPreviewBase)

CDialogPreviewImages::CDialogPreviewImages(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewImages::IDD, pParent), m_pImage(NULL)
{

}

CDialogPreviewImages::~CDialogPreviewImages()
{
	StopPreview();
}

void CDialogPreviewImages::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewImages, CDialogPreviewBase)
	ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

ULONG_PTR           gdiplusToken(0);

void GDI_Init()
{
	if (gdiplusToken == 0) {
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
}

// CDialogPreviewImages message handlers
BOOL CDialogPreviewImages::ShowPreview(const CString &path)
{
	GDI_Init();
	m_pImage = new Image(path);
	InvalidateRect(NULL);
	return TRUE;
}
void CDialogPreviewImages::StopPreview()
{
	if (m_pImage != NULL)
		delete (Image*)m_pImage;
	m_pImage = NULL;
}
void CDialogPreviewImages::OnPaint( )
{
	PAINTSTRUCT ps;
	CDC* pDC = BeginPaint(&ps);
    CDC MemDC;
    MemDC.CreateCompatibleDC(pDC);
    CRect cr;
    GetClientRect(&cr);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(pDC, cr.Width(), cr.Height());
    MemDC.SelectObject(&bmp);
    Graphics g(MemDC);
    Gdiplus::SolidBrush br((ARGB)Gdiplus::Color::White);
    g.FillRectangle(&br, cr.left, cr.top, cr.Width(), cr.Height());
	Image *pImage((Image*)m_pImage);
	RectF destRect(0.0f, 0.0f, (REAL)(cr.right - cr.left), (REAL)(cr.bottom - cr.top));
	REAL srcwidth = (REAL)pImage->GetWidth();
	REAL srcheight = (REAL)pImage->GetHeight();
	Unit srcunit = UnitPixel;
	if (srcheight > 0 && destRect.Height > 0) {
		REAL imgRatio(srcwidth/srcheight);
		REAL newHeight = destRect.Width / imgRatio;
		REAL newWidth = destRect.Height * imgRatio;
		if (newHeight > destRect.Height) {
			destRect.Width = newWidth;
			destRect.X = ((REAL)(cr.right - cr.left) - destRect.Width)/2;
		}
		else {
			destRect.Height = newHeight;
			destRect.Y = ((REAL)(cr.bottom - cr.top) - destRect.Height)/2;
		}
	}
	// Draw the image.
	g.DrawImage(pImage, destRect, 0.0f, 0.0f, srcwidth, srcheight, srcunit);
    pDC->BitBlt(0, 0, cr.Width(), cr.Height(), &MemDC, 0, 0, SRCCOPY);
	EndPaint(&ps);
}

BOOL CDialogPreviewImages::OnEraseBkgnd(CDC* /*pDC*/)
{
    return TRUE;
}
