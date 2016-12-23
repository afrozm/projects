#include "stdafx.h"
#include "Property.h"
#include "TextReader.h"
#include "StringUtils.h"
#include <assert.h>

Property::Property()
{
}


Property::~Property()
{
}

static const lstring kStringPropertyEmptyString;

const lstring & Property::GetValue(const lstring & inKey) const
{
    auto cit(mProperties.find(StringUtils::ToLower(inKey)));
    if (cit != mProperties.end())
        return cit->second;
    return kStringPropertyEmptyString;
}

bool Property::HasKey(const lstring & inKey) const
{
    return mProperties.find(StringUtils::ToLower(inKey)) != mProperties.end();
}

bool Property::SetValue(const lstring & inKey, const lstring & inValue, bool bOverwrite)
{
    bool bAdded(bOverwrite || !HasKey(inKey));

    if (bAdded)
        mProperties[StringUtils::ToLower(inKey)] = inValue;

    return bAdded;
}

bool Property::RemoveKey(const lstring & inKey)
{
    size_t nElemmetsRemoved(mProperties.erase(StringUtils::ToLower(inKey)));
    return nElemmetsRemoved > 0;
}

void Property::RemoveAll()
{
    mProperties.clear();
}

bool Property::IsEmpty() const
{
    return mProperties.empty();
}

size_t Property::GetCount() const
{
    return mProperties.size();
}

void Property::SetProperties(const Property & inProperties, bool bMerger, bool bOverwrite)
{
    if (!bMerger)
        mProperties = inProperties.mProperties;
    else if (!bOverwrite)
        mProperties.insert(inProperties.mProperties.begin(), inProperties.mProperties.end());
    else for (auto cit : inProperties.mProperties)
        mProperties[cit.first] = cit.second;
}

const Property::MapStrStr & Property::GetMapStrStr() const
{
    return mProperties;
}


////////////////////////////// PropertySet //////////////////////////////


PropertySet::PropertySet()
{
}

PropertySet::~PropertySet()
{
}

const lstring & PropertySet::GetValue(const lstring & inSection, const lstring & inKey) const
{
    return GetProperty(inSection).GetValue(inKey);
}

bool PropertySet::SetValue(const lstring & inSection, const lstring & inKey, const lstring & inValue, bool bOverwrite)
{
    if (bOverwrite && !HasSection(inSection))
        mMapProperty[inSection];
    Property *prop(GetPropertyInt(inSection));
    if (prop)
        return prop->SetValue(inKey, inValue, bOverwrite);
    return false;
}

bool PropertySet::RemoveKey(const lstring & inSection, const lstring & inKey)
{
    Property *prop(GetPropertyInt(inSection));
    if (prop)
        return prop->RemoveKey(inKey);
    return false;
}

const Property & PropertySet::GetProperty(const lstring & inSection) const
{
    static const Property kPropertyEmptyProperty;
    auto cit(mMapProperty.find(inSection));
    if (cit != mMapProperty.end())
        return cit->second;
    return kPropertyEmptyProperty;
}
bool PropertySet::HasSection(const lstring & inSection) const
{
    return mMapProperty.find(inSection) != mMapProperty.end();
}

bool PropertySet::RemoveSection(const lstring &inSection)
{
    size_t nElemmetsRemoved(mMapProperty.erase(inSection));
    return nElemmetsRemoved > 0;
}

void PropertySet::RemoveAll()
{
    mMapProperty.clear();
}

bool PropertySet::IsEmpty() const
{
    return mMapProperty.empty();
}

size_t PropertySet::GetCount() const
{
    return mMapProperty.size();
}

const PropertySet::MapProperty& PropertySet::GetMapProperty() const
{
    return mMapProperty;
}

PropertySet::MapProperty & PropertySet::GetMapProperty()
{
    return mMapProperty;
}

Property* PropertySet::GetPropertyInt(const lstring & inSection)
{
    auto cit(mMapProperty.find(inSection));
    if (cit != mMapProperty.end())
        return &cit->second;
    return NULL;
}



////////////////////////////// PropertySetStreamer //////////////////////////////

PropertySetStreamer::PropertySetStreamer(LPCTSTR sep/* =_T("") */)
    : m_pPropertySet(NULL), mPropSep(sep ? sep : _T("\r\n"))
{
}

PropertySetStreamer::~PropertySetStreamer()
{
}

bool PropertySetStreamer::ReadFromFile(const lstring & inFile)
{
    bool bSuccess(false);
    if (m_pPropertySet != NULL) {
        CTextReader textReader(inFile.c_str());
        bSuccess = !!textReader;
        while (true)
        {
            lstring lineStr(textReader.ReadLine());
            if (lineStr.empty())
                break;
            bSuccess |= ReadFromString(lineStr);
        }
    }
    return bSuccess;
}
static void RemoveComment(lstring &str, TCHAR commentChar = '#')
{
    size_t pos(0);
    while (true)
    {
        pos = str.find(commentChar, pos);
        if (pos != lstring::npos) {
            if (str[pos + 1] == commentChar) {// Skip
                str.erase(pos, 1);
                ++pos;
            }
            else {
                str.erase(pos);
                break;
            }
        }
        else break;
    }
}
bool PropertySetStreamer::ReadFromString(const lstring & inString)
{
    bool bRead(false);
    if (m_pPropertySet != NULL) {
        StringUtils::VecString lines;
        StringUtils::SplitString(lines, inString, mPropSep);
        for (auto line : lines) {
            StringUtils::TrimString(line);
            RemoveComment(line);
            if (line.empty())
                continue;
            StringUtils::VecString keyVal;
            if (line[0] == '[') {// Section
                StringUtils::SplitString(keyVal, line, _T("[]"));
                if (keyVal.size() > 0)
                    mCurrentSection = keyVal[0];
                StringUtils::TrimString(mCurrentSection);
                assert(!mCurrentSection.empty());
                bRead |= !mCurrentSection.empty();
                continue;
            }
            StringUtils::SplitString(keyVal, line, _T("="), false, 1);
            if (keyVal.size() > 0) {
                StringUtils::TrimString(keyVal[0]);
                assert(!keyVal[0].empty());
                const lstring *valueStr(&kStringPropertyEmptyString);
                if (keyVal.size() > 1) {
                    StringUtils::TrimString(keyVal[1]);
                    valueStr = &keyVal[1];
                }
                bRead |= m_pPropertySet->SetValue(mCurrentSection, keyVal[0], *valueStr);
            }
        }
    }
    return bRead;
}

void PropertySetStreamer::SetPropertySetStream(PropertySet & inReadInSet)
{
    m_pPropertySet = &inReadInSet;
}

void PropertySetStreamer::SetPropertySetStream(const PropertySet &inWriteInSet)
{
    SetPropertySetStream((PropertySet&)inWriteInSet);
}

bool PropertySetStreamer::WrtieToString(lstring &outString)
{
    bool bWritten(false);
    if (m_pPropertySet != NULL) {
        const PropertySet::MapProperty &mapProperty(m_pPropertySet->GetMapProperty());
        for (auto &prop : mapProperty) {
            outString += _T("[") + prop.first + _T("]\r\n");
            for (auto &keyVal : prop.second.GetMapStrStr())
                outString += keyVal.first + _T("=") + keyVal.second + _T("\r\n");
        }
    }
    return bWritten;
}
