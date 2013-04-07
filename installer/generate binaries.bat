@echo off
mkdir 32bit
mkdir 32bit\locale
mkdir 32bit\shaders
mkdir 32bit\plugins
mkdir 32bit\plugins\DShowPlugin
mkdir 32bit\plugins\DShowPlugin\locale
mkdir 32bit\plugins\DShowPlugin\shaders
mkdir 32bit\plugins\GraphicsCapture

mkdir 64bit
mkdir 64bit\locale
mkdir 64bit\shaders
mkdir 64bit\plugins
mkdir 64bit\plugins\DShowPlugin
mkdir 64bit\plugins\DShowPlugin\locale
mkdir 64bit\plugins\DShowPlugin\shaders
mkdir 64bit\plugins\GraphicsCapture

copy ..\release\obs.exe .\32bit\
copy ..\obsapi\release\obsapi.dll .\32bit\
copy ..\rundir\OBSHelp.chm .\32bit\
copy ..\rundir\services.xconfig .\32bit\
copy ..\rundir\pdb32\stripped\*.pdb .\32bit\
copy ..\rundir\locale\*.txt .\32bit\locale\
copy ..\rundir\shaders\*.?Shader .\32bit\shaders\
copy ..\dshowplugin\release\dshowplugin.dll .\32bit\plugins
copy ..\noisegate\release\noisegate.dll .\32bit\plugins
copy ..\rundir\plugins\dshowplugin\locale\*.txt .\32bit\plugins\dshowplugin\locale\
copy ..\rundir\plugins\dshowplugin\shaders\*.?Shader .\32bit\plugins\dshowplugin\shaders\
copy ..\graphicscapture\release\graphicscapture.dll .\32bit\plugins
copy ..\graphicscapture\graphicscapturehook\release\graphicscapturehook.dll .\32bit\plugins\graphicscapture
copy ..\graphicscapture\graphicscapturehook\x64\release\graphicscapturehook64.dll .\32bit\plugins\graphicscapture
copy ..\injectHelper\x64\release\injectHelper64.exe .\32bit\plugins\graphicscapture
copy ..\x264\libs\32bit\libx264-130.dll .\32bit

copy ..\x64\release\obs.exe .\64bit\
copy ..\obsapi\x64\release\obsapi.dll .\64bit\
copy ..\rundir\OBSHelp.chm .\64bit\
copy ..\rundir\services.xconfig .\64bit\
copy ..\rundir\pdb64\stripped\*.pdb .\64bit\
copy ..\rundir\locale\*.txt .\64bit\locale\
copy ..\rundir\shaders\*.?Shader .\64bit\shaders\
copy ..\dshowplugin\x64\release\dshowplugin.dll .\64bit\plugins
copy ..\noisegate\x64\release\noisegate.dll .\64bit\plugins
copy ..\rundir\plugins\dshowplugin\locale\*.txt .\64bit\plugins\dshowplugin\locale\
copy ..\rundir\plugins\dshowplugin\shaders\*.?Shader .\64bit\plugins\dshowplugin\shaders\
copy ..\graphicscapture\x64\release\graphicscapture.dll .\64bit\plugins
copy ..\graphicscapture\graphicscapturehook\release\graphicscapturehook.dll .\64bit\plugins\graphicscapture
copy ..\graphicscapture\graphicscapturehook\x64\release\graphicscapturehook64.dll .\64bit\plugins\graphicscapture
copy ..\injectHelper\release\injectHelper.exe .\64bit\plugins\graphicscapture
copy ..\x264\libs\64bit\libx264-130.dll .\64bit

mkdir upload
mkdir upload\DirectShowPlugin
mkdir upload\DirectShowPlugin\32bit
mkdir upload\DirectShowPlugin\32bit\DShowPlugin\locale
mkdir upload\DirectShowPlugin\32bit\DShowPlugin\shaders
mkdir upload\DirectShowPlugin\64bit
mkdir upload\DirectShowPlugin\64bit\DShowPlugin\locale
mkdir upload\DirectShowPlugin\64bit\DShowPlugin\shaders
mkdir upload\GraphicsCapturePlugin
mkdir upload\GraphicsCapturePlugin\32bit
mkdir upload\GraphicsCapturePlugin\32bit\GraphicsCapture
mkdir upload\GraphicsCapturePlugin\64bit
mkdir upload\GraphicsCapturePlugin\64bit\GraphicsCapture
mkdir upload\OBS
mkdir upload\OBS\32bit
mkdir upload\OBS\64bit
mkdir upload\OBS\locale
mkdir upload\OBS\services
mkdir upload\OBS\shaders

copy ..\dshowplugin\release\dshowplugin.dll .\upload\DirectShowPlugin\32bit\
copy ..\rundir\plugins\dshowplugin\locale\*.txt .\upload\DirectShowPlugin\32bit\DShowPlugin\locale\
copy ..\rundir\plugins\dshowplugin\shaders\*.?Shader .\upload\DirectShowPlugin\32bit\DShowPlugin\shaders\

copy ..\dshowplugin\x64\release\dshowplugin.dll .\upload\DirectShowPlugin\64bit\
copy ..\rundir\plugins\dshowplugin\locale\*.txt .\upload\DirectShowPlugin\64bit\DShowPlugin\locale\
copy ..\rundir\plugins\dshowplugin\shaders\*.?Shader .\upload\DirectShowPlugin\64bit\DShowPlugin\shaders\

copy ..\graphicscapture\release\graphicscapture.dll .\upload\GraphicsCapturePlugin\32bit\
copy ..\graphicscapture\graphicscapturehook\release\graphicscapturehook.dll .\upload\GraphicsCapturePlugin\32bit\GraphicsCapture\
copy ..\graphicscapture\graphicscapturehook\x64\release\graphicscapturehook64.dll .\upload\GraphicsCapturePlugin\32bit\GraphicsCapture\
copy ..\injectHelper\x64\release\injectHelper64.exe .\upload\GraphicsCapturePlugin\32bit\GraphicsCapture\

copy ..\graphicscapture\x64\release\graphicscapture.dll .\upload\GraphicsCapturePlugin\64bit\
copy ..\graphicscapture\graphicscapturehook\release\graphicscapturehook.dll .\upload\GraphicsCapturePlugin\64bit\GraphicsCapture\
copy ..\graphicscapture\graphicscapturehook\x64\release\graphicscapturehook64.dll .\upload\GraphicsCapturePlugin\64bit\GraphicsCapture\
copy ..\injectHelper\release\injectHelper.exe .\upload\GraphicsCapturePlugin\64bit\GraphicsCapture\

copy ..\release\obs.exe .\upload\OBS\32bit\
copy ..\obsapi\release\obsapi.dll .\upload\OBS\32bit\
copy ..\rundir\OBSHelp.chm .\upload\OBS\32bit\
copy ..\rundir\pdb32\stripped\*.pdb .\upload\OBS\32bit\
copy ..\x264\libs\32bit\libx264-130.dll .\upload\OBS\32bit

copy ..\x64\release\obs.exe .\upload\OBS\64bit\
copy ..\obsapi\x64\release\obsapi.dll .\upload\OBS\64bit\
copy ..\rundir\OBSHelp.chm .\upload\OBS\64bit\
copy ..\rundir\pdb64\stripped\*.pdb .\upload\OBS\64bit\
copy ..\x264\libs\64bit\libx264-130.dll .\upload\OBS\64bit

copy ..\rundir\locale\*.txt .\upload\OBS\locale\

copy ..\rundir\services.xconfig .\upload\OBS\services\

copy ..\rundir\shaders\*.?Shader .\upload\OBS\shaders\

