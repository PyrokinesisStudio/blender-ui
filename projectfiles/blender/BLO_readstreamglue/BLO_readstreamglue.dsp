# Microsoft Developer Studio Project File - Name="BLO_readstreamglue" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=BLO_readstreamglue - Win32 Release
!MESSAGE Dies ist kein g�ltiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und f�hren Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "BLO_readstreamglue.mak".
!MESSAGE 
!MESSAGE Sie k�nnen beim Ausf�hren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "BLO_readstreamglue.mak" CFG="BLO_readstreamglue - Win32 Release"
!MESSAGE 
!MESSAGE F�r die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "BLO_readstreamglue - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "BLO_readstreamglue - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "BLO_readstreamglue - Win32 MT DLL Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "BLO_readstreamglue - Win32 MT DLL Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BLO_readstreamglue - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\obj\windows\blender\readstreamglue"
# PROP Intermediate_Dir "..\..\..\obj\windows\blender\readstreamglue"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_readstreamglue - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\obj\windows\blender\readstreamglue\debug"
# PROP Intermediate_Dir "..\..\..\obj\windows\blender\readstreamglue\debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\lib\windows\zlib\include" /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_readstreamglue - Win32 MT DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BLO_readstreamglue___Win32_MT_DLL_Release"
# PROP BASE Intermediate_Dir "BLO_readstreamglue___Win32_MT_DLL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\obj\windows\blender\readstreamglue\mtdll"
# PROP Intermediate_Dir "..\..\..\obj\windows\blender\readstreamglue\mtdll"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_readstreamglue - Win32 MT DLL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BLO_readstreamglue___Win32_MT_DLL_Debug"
# PROP BASE Intermediate_Dir "BLO_readstreamglue___Win32_MT_DLL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\obj\windows\blender\readstreamglue\mtdll_debug"
# PROP Intermediate_Dir "..\..\..\obj\windows\blender\readstreamglue\mtdll_debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\lib\windows\zlib\include" /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\lib\windows\zlib\include" /I "..\..\..\source\kernel\gen_messaging" /I "..\..\..\source\blender\readstreamglue" /I "..\..\..\source\blender\readstreamglue\stubs" /I "..\..\..\..\lib\windows\zlib\include" /I "..\..\..\source\blender\blenloader" /I "..\..\..\source\blender\inflate" /I "..\..\..\source\blender\decrypt" /I "..\..\..\source\blender\verify" /I "..\..\..\..\lib\windows\blenkey\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "BLO_readstreamglue - Win32 Release"
# Name "BLO_readstreamglue - Win32 Debug"
# Name "BLO_readstreamglue - Win32 MT DLL Release"
# Name "BLO_readstreamglue - Win32 MT DLL Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\source\blender\readstreamglue\intern\BLO_readStreamGlue.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\blender\readstreamglue\intern\BLO_readStreamGlueLoopBack.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
