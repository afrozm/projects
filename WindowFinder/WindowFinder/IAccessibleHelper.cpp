#include "stdafx.h"
#include "IAccessibleHelper.h"
#include <STLUtils.h>
#include <vector>

#pragma comment(lib, "Oleacc.lib")

using namespace STLUtils;

IAccessibleHelper::MapGetterAPI IAccessibleHelper::mMapGetterAPI;

IAccessibleHelper::IAccessibleHelper()
{
    m_vt.vt = VT_I4;
}


IAccessibleHelper::~IAccessibleHelper()
{
}

IAccessibleHelper::operator bool() const
{
    if (m_pIAcc)
        return true;
    return false;
}


#define ADD_MAP_GETTER_API(a) mMapGetterAPI[_T(#a)]=&IAccessibleHelper::a

void IAccessibleHelper::CommonInit()
{
    if (mMapGetterAPI.empty()) {
        ADD_MAP_GETTER_API(ChildCount);
        ADD_MAP_GETTER_API(Name);
        ADD_MAP_GETTER_API(Value);
        ADD_MAP_GETTER_API(Description);
        ADD_MAP_GETTER_API(Role);
        ADD_MAP_GETTER_API(State);
        ADD_MAP_GETTER_API(Help);
        ADD_MAP_GETTER_API(KeyboardShortcut);
        ADD_MAP_GETTER_API(Focus);
        ADD_MAP_GETTER_API(DefaultAction);
        ADD_MAP_GETTER_API(Location);
    }
    if (m_pIAcc)
        m_pIAcc.Release();
    m_vt.Clear();
}

bool IAccessibleHelper::InitFromPoint(int x, int y)
{
    bool bSuccess(false);
    CommonInit();
    if (x >= 0 && y >= 0) {
        HRESULT hr(AccessibleObjectFromPoint({ x,y }, &m_pIAcc, &m_vt));
        bSuccess = SUCCEEDED(hr);
    }

    return bSuccess;
}

bool IAccessibleHelper::InitFromWindow(HWND hWnd, DWORD objID /*= OBJID_WINDOW*/)
{
    bool bSuccess(false);
    CommonInit();
    IAccessible *pIAccessible(NULL);
    bSuccess = SUCCEEDED(AccessibleObjectFromWindow(hWnd, objID, IID_IAccessible, (void **)&pIAccessible));
    if (pIAccessible) {
        m_pIAcc = pIAccessible;
        m_vt.vt = VT_I4;
        m_vt.lVal = CHILDID_SELF;
    }
    return bSuccess;
}

long IAccessibleHelper::GetChildCount() const
{
    long childCount(0);
    if (m_pIAcc && m_vt.llVal == CHILDID_SELF)
        m_pIAcc->get_accChildCount(&childCount);
    return childCount;
}

bool IAccessibleHelper::GetChild(unsigned childIndex, IAccessibleHelper &outChild) const
{
    bool bSuccess(false);
    if (m_pIAcc) {
		long childCount(0);
		_variant_t childItem;
		AccessibleChildren(m_pIAcc, childIndex - 1, 1, &childItem, &childCount);
		if (childCount == 1) {
			if (childItem.vt == VT_DISPATCH) {
				outChild.m_pIAcc = childItem;
				outChild.m_vt.vt = VT_I4;
				outChild.m_vt.lVal = CHILDID_SELF;
			}
			else {
				outChild.m_pIAcc = m_pIAcc;
				outChild.m_vt = childItem;
			}
			bSuccess = true;
		}
    }
    return bSuccess;
}


IAccessibleHelper IAccessibleHelper::GetParent() const
{
    IAccessibleHelper parent;
    if (m_pIAcc) {
        IDispatch *disp = NULL;
        m_pIAcc->get_accParent(&disp);
        if (disp)
            parent.m_pIAcc = disp;
    }
    return parent;
}

bool IAccessibleHelper::GetRect(RECT &outRect, unsigned rectFlags /* = GRF_None */)
{
    bool bSuccess(false);
    long mLocation[4] = { 0 };
    if (m_pIAcc && SUCCEEDED(m_pIAcc->accLocation(mLocation, mLocation + 1, mLocation + 2, mLocation + 3, m_vt))) {
        bSuccess = true;
        outRect.left = mLocation[0];
        outRect.top = mLocation[1];
        outRect.right = outRect.left + mLocation[2];
        outRect.bottom = outRect.top + mLocation[3];
        if (rectFlags & (GRF_WRTParent | GRF_WRTSelf)) {
            HWND hWnd(GetWindow());
            if (hWnd && (rectFlags&GRF_WRTParent)) {
                HWND hWndp = ::GetParent(hWnd);
                if (hWndp)
                    hWnd = hWndp;
            }
            if (hWnd) {
                LPPOINT pt((LPPOINT)&outRect);
                ScreenToClient(hWnd, pt);
                ScreenToClient(hWnd, pt + 1);
            }
        }
    }
    return bSuccess;
}

HWND IAccessibleHelper::GetWindow()
{
    HWND hWnd(NULL);

    if (m_pIAcc)
        WindowFromAccessibleObject(m_pIAcc, &hWnd);

    return hWnd;
}

lstring IAccessibleHelper::GetValue(const lstring & fieldName) const
{
    lstring outStr;
    auto cit(mMapGetterAPI.find(fieldName));
    if (cit != mMapGetterAPI.end())
        outStr = (this->*cit->second)();
    return outStr;
}

lstring IAccessibleHelper::ChildCount() const
{
    lstring outStr;
    long childCount(GetChildCount());
    if (childCount > 0)
        ChangeType(childCount, outStr);
    return outStr;
}

lstring IAccessibleHelper::Name() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accName(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Value() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accValue(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Description() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accDescription(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Role() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;
        m_pIAcc->get_accRole(m_vt, &vt);
        if (vt) {
            TCHAR roleText[256];
            roleText[0] = 0;
            GetRoleText(vt.lVal, roleText, _countof(roleText));
            outStr = roleText;
            if (outStr.empty())
                ChangeType(vt.lVal, outStr);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::State() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;
        m_pIAcc->get_accState(m_vt, &vt);
        if (vt) {
            TCHAR roleText[256];
            roleText[0] = 0;
            GetStateText(vt.lVal, roleText, _countof(roleText));
            outStr = roleText;
            if (outStr.empty())
                ChangeType(vt.lVal, outStr);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Help() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accHelp(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::KeyboardShortcut() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accKeyboardShortcut(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Focus() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;

        if (SUCCEEDED(m_pIAcc->get_accFocus(&vt))
            && vt.vt != VT_EMPTY) {
            IAccessibleHelper iah;
            const IAccessibleHelper *pIAH(&iah);

            if (vt.vt == VT_I4) {
                if (vt.lVal == CHILDID_SELF)
                    pIAH = this;
                else GetChild(vt, iah);
            }
            else if (vt.vt == VT_DISPATCH){
                iah.m_pIAcc = vt;
                iah.m_vt.vt = VT_I4;
                iah.m_vt.lVal = CHILDID_SELF;
            }
            if (*pIAH) {
                outStr = pIAH->Value();
                if (outStr.empty())
                    outStr = pIAH->Name();
            }
        }
    }
    return outStr;
}

lstring IAccessibleHelper::DefaultAction() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accDefaultAction(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Location() const
{
    long mLocation[4] = { 0 };
    lstring outStr;
    if (m_pIAcc) {
        if (SUCCEEDED(m_pIAcc->accLocation(mLocation, mLocation +1, mLocation +2, mLocation +3, m_vt))) {
            if (mLocation[0] || mLocation[1] || mLocation[2] || mLocation[3]) {
                for (auto p : mLocation)
                {
                    lstring str;
                    ChangeType(p, str);
                    if (!outStr.empty())
                        outStr += _T(",");
                    outStr += str;
                }
            }
        }
    }
    return outStr;
}

int IAccessibleHelper::IterateChildren(IterCallback callback, void *pUserData /* = NULL */, int maxDepth /* = 0 */) const
{
    IterCallbackDataInt data = { 0 };
    data.callback = callback;
    data.itData.pUserData = pUserData;
    data.maxDepth = maxDepth;
    data.itData.rootItem = this;
    return IterateChildrenInt(data);
}

class AutoIncrDecrCount
{
public:
    AutoIncrDecrCount(int &c) : m_iCount(c) { ++m_iCount; }
    ~AutoIncrDecrCount() { --m_iCount; }

private:
    int &m_iCount;
};

int IAccessibleHelper::IterateChildrenInt(IterCallbackDataInt &inData) const
{
    inData.itData.parentItem = this;
    AutoIncrDecrCount incrDeptCount(inData.itData.childDepth);
    if (inData.maxDepth > 0 && inData.itData.childDepth > inData.maxDepth)
        return 0;
    if (!this || !*this)
        return 0;
    long childCount(0), childIndex(0);
    m_pIAcc->get_accChildCount(&childCount);
    if (childCount <= 0)
        return 0;
    std::vector<_variant_t> childVariants(childCount);
    AccessibleChildren(m_pIAcc, 0, childCount, childVariants.data(), &childCount);
    inData.itData.childCount = childCount;
    for (auto &childV : childVariants) {
        inData.itData.childIndex = childIndex++;
        IAccessibleHelper childItem;
        if (childV.vt == VT_DISPATCH) {
            childItem.m_pIAcc = childV;
            childItem.m_vt.vt = VT_I4;
            childItem.m_vt.lVal = CHILDID_SELF;
        }
        else {
            childItem.m_pIAcc = m_pIAcc;
            childItem.m_vt = childV;
        }
        inData.itData.childItem = &childItem;
        if (inData.callback) {
            int clabbackRes = inData.callback(inData.itData);
            if (clabbackRes != 0)
                break;
        }
        if (childV.vt == VT_DISPATCH)
            childCount += childItem.IterateChildrenInt(inData);
    }
    return childCount;
}

