//
//  MacUtil.hpp
//  key2
//
//  Created by Afroz Muzammil on 25/6/20.
//  Copyright © 2020 Autodesk. All rights reserved.
//

#pragma once

#include <string>

namespace MacUtil {
std::string WstringToString(const std::wstring &inSource);
bool HasPermissionForAccess();
bool GetFrontWindowTitle(std::string &outTitle);
bool GetClipboardString(std::string &outString);
};
