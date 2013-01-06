
#include "LivestreamPlugin.h"

extern "C" __declspec(dllexport) bool LoadPlugin();
extern "C" __declspec(dllexport) void UnloadPlugin();
extern "C" __declspec(dllexport) CTSTR GetPluginName();
extern "C" __declspec(dllexport) CTSTR GetPluginDescription();

LocaleStringLookup *pluginLocale = NULL;
HINSTANCE hinstMain = NULL;

#define LIVESTREAM_CLASSNAME TEXT("Livestream.com")

RTMPService* STDCALL CreateLivestreamService(XElement* data){
	LivestreamService* livestreamService = new LivestreamService();

}

bool LoadPlugin(){
    InitColorControl(hinstMain);

    pluginLocale = new LocaleStringLookup;

    if(!pluginLocale->LoadStringFile(TEXT("plugins/LivestreamPlugin/locale/en.txt")))
        AppWarning(TEXT("Could not open locale string file '%s'"), TEXT("plugins/LivestreamPlugin/locale/en.txt"));

    if(scmpi(API->GetLanguage(), TEXT("en")) != 0)
    {
        String pluginStringFile;
        pluginStringFile << TEXT("plugins/LivestreamPlugin/locale/") << API->GetLanguage() << TEXT(".txt");
        if(!pluginLocale->LoadStringFile(pluginStringFile))
            AppWarning(TEXT("Could not open locale string file '%s'"), pluginStringFile.Array());
    }

    API->RegisterImageSourceClass(LIVESTREAM_CLASSNAME, PluginStr("ClassName"), (OBSCREATEPROC)CreateLivestreamService, (OBSCONFIGPROC)ConfigureLivestreamService);

    return true;
}

void UnloadPlugin(){
    delete pluginLocale;
}

CTSTR GetPluginName(){
    return PluginStr("Plugin.Name");
}

CTSTR GetPluginDescription(){
    return PluginStr("Plugin.Description");
}


BOOL CALLBACK DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpBla){
    if(dwReason == DLL_PROCESS_ATTACH)
        hinstMain = hInst;

    return TRUE;
}