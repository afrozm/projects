#pragma once

#include "StringUtils.h"
#include <map>

class Property
{
public:
    Property();
    ~Property();
    const StdString& GetValue(const StdString &inKey) const;
    bool HasKey(const StdString &inKey) const;
    bool SetValue(const StdString &inKey, const StdString &inValue, bool bOverwrite = true);
    bool RemoveKey(const StdString &inKey);
    void RemoveAll();
    bool IsEmpty() const;
    size_t GetCount() const;
    void SetProperties(const Property &inProperties, bool bMerger = true, bool bOverwrite = true);
    typedef std::map<StdString, StdString> MapStrStr;
    const MapStrStr& GetMapStrStr() const; // only to iterate
private:
    MapStrStr mProperties;
};


////////////////////////////// PropertySet //////////////////////////////

class PropertySet
{
public:
    PropertySet();
    ~PropertySet();
    const StdString& GetValue(const StdString &inSection, const StdString &inKey) const;
    bool SetValue(const StdString &inSection, const StdString &inKey, const StdString &inValue, bool bOverwrite = true);
    bool RemoveKey(const StdString &inSection, const StdString &inKey);
    const Property& GetProperty(const StdString &inSection) const;
    bool HasSection(const StdString &inSection) const;
    bool RemoveSection(const StdString &inSection);
    void RemoveAll();
    bool IsEmpty() const;
    size_t GetCount() const;
    typedef std::map<StdString, Property> MapProperty;
    const MapProperty& GetMapProperty() const; // only to iterate
    MapProperty& GetMapProperty(); // only to iterate
private:
    Property* GetPropertyInt(const StdString &inSection);
    MapProperty mMapProperty;
};


////////////////////////////// PropertySetStreamer //////////////////////////////

class PropertySetStreamer
{
public:
    PropertySetStreamer(LPCTSTR sep=_T("\r\n"));
    ~PropertySetStreamer();
    bool ReadFromFile(const StdString &inFile);
    bool ReadFromString(const StdString &inString);
    void SetPropertySetStream(PropertySet &inReadInSet);
    void SetPropertySetStream(const PropertySet &inWriteInSet);
    bool WriteToFile(const StdString &inFile);
    bool WrtieToString(StdString &outString);
private:
    PropertySet *m_pPropertySet;
    StdString mCurrentSection, mPropSep;
};

