#pragma once


// CComboBoxDragDrop

class CComboBoxDragDrop : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxDragDrop)

public:
	CComboBoxDragDrop();
	virtual ~CComboBoxDragDrop();
    afx_msg void OnDropFiles(HDROP hDropInfo);

protected:
	DECLARE_MESSAGE_MAP()
};


