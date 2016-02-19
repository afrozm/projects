#pragma once

#include "Common.h"
#include <map>

class Property
{
public:
    Property();
    ~Property();
    const lstring& GetValue(const lstring &inKey) const;
    bool HasKey(const lstring &inKey) const;
    bool SetValue(const lstring &inKey, const lstring &inValue, bool bOverwrite = true);
    bool RemoveKey(const lstring &inKey);
    void RemoveAll();
    bool IsEmpty() const;
    size_t GetCount() const;
    void SetProperties(const Property &inProperties, bool bMerger = true, bool bOverwrite = true);
private:
    typedef std::map<lstring, lstring> MapStrStr;
    MapStrStr mProperties;
};


////////////////////////////// PropertySet //////////////////////////////

class PropertySet
{
public:
    PropertySet();
    ~PropertySet();
    const lstring& GetValue(const lstring &inSection, const lstring &inKey) const;
    bool SetValue(const lstring &inSection, const lstring &inKey, const lstring &inValue, bool bOverwrite = true);
    bool RemoveKey(const lstring &inSection, const lstring &inKey);
    const Property& GetProperty(const lstring &inSection) const;
    bool HasSection(const lstring &inSection) const;
    bool RemoveSection(const lstring &inSection);
    void RemoveAll();
    bool IsEmpty() const;
    size_t GetCount() const;
    typedef std::map<lstring, Property> MapProperty;
    const MapProperty& GetMapProperty() const; // only to iterate
    MapProperty& GetMapProperty(); // only to iterate
private:
    Property* GetProperty(const lstring &inSection);
    MapProperty mMapProperty;
};


////////////////////////////// PropertySetStreamer //////////////////////////////

class PropertySetStreamer
{
public:
    PropertySetStreamer();
    ~PropertySetStreamer();
    bool ReadFromFile(const lstring &inFile);
    bool ReadFromString(const lstring &inString);
    void SetPropertySetStream(PropertySet &inReadInSet);
    void SetPropertySetStream(const PropertySet &inWriteInSet);
    bool WriteToFile(const lstring &inFile);
    bool WrtieToString(lstring &outString);
private:
    PropertySet *m_pPropertySet;
    lstring mCurrentSection;
};

