#include "stdafx.h"
#include "Property.h"
#include "TextReader.h"
#include <assert.h>

Property::Property()
{
}


Property::~Property()
{
}

static const StdString kStringPropertyEmptyString;

const StdString & Property::GetValue(const StdString & inKey) const
{
    auto cit(mProperties.find(inKey.ToLower()));
    if (cit != mProperties.end())
        return cit->second;
    return kStringPropertyEmptyString;
}

bool Property::HasKey(const StdString & inKey) const
{
    return mProperties.find(inKey.ToLower()) != mProperties.end();
}

bool Property::SetValue(const StdString & inKey, const StdString & inValue, bool bOverwrite)
{
    bool bAdded(bOverwrite || !HasKey(inKey));

    if (bAdded)
        mProperties[inKey.ToLower()] = inValue;

    return bAdded;
}

bool Property::RemoveKey(const StdString & inKey)
{
    size_t nElemmetsRemoved(mProperties.erase(inKey.ToLower()));
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

const StdString & PropertySet::GetValue(const StdString & inSection, const StdString & inKey) const
{
    return GetProperty(inSection).GetValue(inKey);
}

bool PropertySet::SetValue(const StdString & inSection, const StdString & inKey, const StdString & inValue, bool bOverwrite)
{
    if (bOverwrite && !HasSection(inSection))
        mMapProperty[inSection];
    Property *prop(GetPropertyInt(inSection));
    if (prop)
        return prop->SetValue(inKey, inValue, bOverwrite);
    return false;
}

bool PropertySet::RemoveKey(const StdString & inSection, const StdString & inKey)
{
    Property *prop(GetPropertyInt(inSection));
    if (prop)
        return prop->RemoveKey(inKey);
    return false;
}

const Property & PropertySet::GetProperty(const StdString & inSection) const
{
    static const Property kPropertyEmptyProperty;
    auto cit(mMapProperty.find(inSection));
    if (cit != mMapProperty.end())
        return cit->second;
    return kPropertyEmptyProperty;
}
bool PropertySet::HasSection(const StdString & inSection) const
{
    return mMapProperty.find(inSection) != mMapProperty.end();
}

bool PropertySet::RemoveSection(const StdString &inSection)
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

Property* PropertySet::GetPropertyInt(const StdString & inSection)
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

bool PropertySetStreamer::ReadFromFile(const StdString & inFile)
{
    bool bSuccess(false);
    if (m_pPropertySet != NULL) {
        CTextReader textReader(inFile.c_str());
        bSuccess = !!textReader;
        while (true)
        {
            StdString lineStr(textReader.ReadLine());
            if (lineStr.empty())
                break;
            bSuccess |= ReadFromString(lineStr);
        }
    }
    return bSuccess;
}
static void RemoveComment(StdString &str, TCHAR commentChar = '#')
{
    size_t pos(0);
    while (true)
    {
        pos = str.find(commentChar, pos);
        if (pos != StdString::npos) {
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
bool PropertySetStreamer::ReadFromString(const StdString & inString)
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
                const StdString *valueStr(&kStringPropertyEmptyString);
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

bool PropertySetStreamer::WrtieToString(StdString &outString)
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
