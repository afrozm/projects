#include "stdafx.h"
#include "IAccessibleHelper.h"
#include <STLUtils.h>


#pragma comment(lib, "Oleacc.lib")

using namespace STLUtils;

IAccessibleHelper::MapGetterAPI IAccessibleHelper::mMapGetterAPI;

IAccessibleHelper::IAccessibleHelper()
{
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

void IAccessibleHelper::InitFromPoint(int x, int y)
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
    if (x >= 0 && y >= 0)
        SUCCEEDED(AccessibleObjectFromPoint({ x,y }, &m_pIAcc, &m_vt));
}

bool IAccessibleHelper::GetChild(_variant_t &childVt, IAccessibleHelper &outChild) const
{
    bool bSuccess(false);
    if (m_pIAcc) {
        outChild.m_vt = childVt;
        IDispatch *disp = NULL;
        m_pIAcc->get_accChild(outChild.m_vt, &disp);
        if (disp)
            disp->QueryInterface(IID_IAccessible, (void **)&outChild.m_pIAcc);
        if (outChild) {
            //outChild.m_vt.lVal = CHILDID_SELF;
            bSuccess = true;
        }
    }
    return bSuccess;
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
    if (m_pIAcc) {
        long childCount(0);
        m_pIAcc->get_accChildCount(&childCount);
        ChangeType(childCount, outStr);
    }
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
        m_pIAcc->get_accFocus(&vt);
        if (vt) {
            if (vt.lVal == CHILDID_SELF)
                outStr = Name();
            else {
                IAccessibleHelper child;
                if (GetChild(vt, child))
                    outStr = child.Name();
            }
            if (outStr.empty())
                ChangeType(vt.lVal, outStr);
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
    lstring outStr;
    if (m_pIAcc) {
        long pos[4] = { 0 };
        if (SUCCEEDED(m_pIAcc->accLocation(pos, pos+1, pos+2, pos+3, m_vt))) {
            for (auto p : pos)
            {
                lstring str;
                ChangeType(p, str);
                if (!outStr.empty())
                    outStr += _T(",");
                outStr += str;
            }
        }
    }
    return outStr;
}
