#pragma once


class CWindowEntry {
public:
    CWindowEntry(HWND hWnd = NULL);
    enum Desc
    {
        Short,
        Long
    };
    const CString& GetDesc(Desc desc = Short) const;
    operator HWND() const;
    void UpdateDesc(LPCTSTR procName = NULL);
private:
    HWND m_hWnd;
    CString mDesc, mDescLong;
};


class CWindowIterator
{
public:
    CWindowIterator();
    ~CWindowIterator();
    typedef int(*Callback)(const CWindowEntry& we);
    void SetCallback(Callback callback = NULL);
    int Iterate(HWND hWnd = NULL);
    int Callback_Iterator(HWND hWnd);
    const CArray<CWindowEntry>& GetWindowList() const;
    DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, Searching)
private:
    CArray<CWindowEntry> mWindows;
    Callback mCalllback;
    CString mProcessName;
    enum {
        Searching
    };
    DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, Searching)
    UINT m_uFlags;
    HWND m_hWnd;
};

