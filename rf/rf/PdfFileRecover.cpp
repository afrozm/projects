#include "stdafx.h"
#include "PdfFileRecover.h"


CPdfFileRecover::CPdfFileRecover()
{
    mObjFind.SetFindPattern(" obj");
}


CPdfFileRecover::~CPdfFileRecover()
{
}

bool CPdfFileRecover::ParseBuffer(BinaryData &inData)
{
    switch (GetState())
    {
    case ReadTillEndOffset:
        {
            mObjFind.SetFindBuffer(inData, true);
            long long findPos(mObjFind.FindNext());
            bool bEndFound(findPos < 0);
            // obj found
            // check all character from start till obj should be printable
            for (size_t i = 0; i < (size_t)findPos && !bEndFound; ++i) {
                char ch(inData[i]);
                if (!STR_CHAR_IS_SPACE(ch) && !isprint(ch))
                    bEndFound = true;
            }
            if (!bEndFound) {
                m_iCurrrentEndPatternSkipCount = 0;
                SetState(FindEnd);
            }
            else {
                for (m_iCurrentEndOffset = 0; m_iCurrentEndOffset < (int)inData.DataSize(); ++m_iCurrentEndOffset) {
                    char ch(inData[m_iCurrentEndOffset]);
                    if (!(STR_CHAR_IS_SPACE(ch)))
                        break;
                }
                if (m_iCurrentEndOffset > 0 && m_iCurrentEndOffset == inData.DataSize()) // Reach end of buffer and all isprint found
                    ++m_iCurrentEndOffset; // increment 1 to check more isprint from next buffer
            }
        }
        break;
    default:
        break;
    }
    return __super::ParseBuffer(inData);
}
