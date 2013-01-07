
#include "LivestreamPlugin.h"

extern "C" __declspec(dllexport) bool LoadPlugin();
extern "C" __declspec(dllexport) void UnloadPlugin();
extern "C" __declspec(dllexport) CTSTR GetPluginName();
extern "C" __declspec(dllexport) CTSTR GetPluginDescription();

LocaleStringLookup *pluginLocale = NULL;
HINSTANCE hinstMain = NULL;

#define LIVESTREAM_CLASSNAME TEXT("LivestreamPlugin")

RTMPService* STDCALL CreateLivestreamService(XElement* data){
	LivestreamService* livestreamService = new LivestreamService();
	return livestreamService;
}

bool STDCALL SettingsProc(HWND hwnd, UINT mode) //Codes 0 = init, 1 = select, 2 = save
{
	String s = "Called with mode: ";
	s+= mode;
	MessageBox(hwnd, s.Array(), NULL, MB_ICONEXCLAMATION | MB_YESNO);
	return FALSE;
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

    API->RegisterServiceClass(LIVESTREAM_CLASSNAME, PluginStr("ClassName"), (OBSCREATEPROC)CreateLivestreamService, (OBSSETTINGPROC)SettingsProc);

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