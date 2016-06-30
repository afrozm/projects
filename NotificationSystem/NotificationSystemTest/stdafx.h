// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
#include "targetver.h"
#include <conio.h>
#else
#include <curses.h>
#define scanf_s(p,d,s) scanf(p,d)
#endif

#include <stdio.h>
#include <stdlib.h>
