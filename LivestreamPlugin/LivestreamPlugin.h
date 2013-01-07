#pragma once

#include "OBSApi.h"

extern LocaleStringLookup *pluginLocale;
extern HINSTANCE hinstMain;
#define PluginStr(text) pluginLocale->LookupString(TEXT2(text))

#include "LivestreamService.h"