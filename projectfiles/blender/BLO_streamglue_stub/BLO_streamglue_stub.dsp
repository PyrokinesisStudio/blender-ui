# Microsoft Developer Studio Project File - Name="BLO_streamglue_stub" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=BLO_streamglue_stub - Win32 MT DLL Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BLO_streamglue_stub.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BLO_streamglue_stub.mak" CFG="BLO_streamglue_stub - Win32 MT DLL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BLO_streamglue_stub - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BLO_streamglue_stub - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "BLO_streamglue_stub - Win32 MT DLL Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BLO_streamglue_stub - Win32 MT DLL Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BLO_streamglue_stub - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\obj\windows\blender\streamglue_stub"
# PROP Intermediate_Dir "..\..\..\..\obj\windows\blender\streamglue_stub"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_streamglue_stub - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\obj\windows\blender\streamglue_stub\debug"
# PROP Intermediate_Dir "..\..\..\..\obj\windows\blender\streamglue_stub\debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_streamglue_stub - Win32 MT DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "BLO_streamglue_stub___Win32_MT_DLL_Release"
# PROP BASE Intermediate_Dir "BLO_streamglue_stub___Win32_MT_DLL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "BLO_streamglue_stub___Win32_MT_DLL_Release"
# PROP Intermediate_Dir "BLO_streamglue_stub___Win32_MT_DLL_Release"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "BLO_streamglue_stub - Win32 MT DLL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "BLO_streamglue_stub___Win32_MT_DLL_Debug"
# PROP BASE Intermediate_Dir "BLO_streamglue_stub___Win32_MT_DLL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BLO_streamglue_stub___Win32_MT_DLL_Debug"
# PROP Intermediate_Dir "BLO_streamglue_stub___Win32_MT_DLL_Debug"
# PROP Target_Dir ""
LINK32=link.exe -lib
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\source\blender\streamglue\\" /I "..\..\..\source\blender\writeblenfile\\" /I "..\..\..\source\blender\deflate\\" /I "..\..\..\source\blender\inflate\\" /I "..\..\..\lib\windows\zlib\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
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

# Name "BLO_streamglue_stub - Win32 Release"
# Name "BLO_streamglue_stub - Win32 Debug"
# Name "BLO_streamglue_stub - Win32 MT DLL Release"
# Name "BLO_streamglue_stub - Win32 MT DLL Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\source\blender\streamglue\stub\BLO_streamGlueControlSTUB.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\blender\streamglue\stub\BLO_streamGlueSTUB.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\source\blender\streamglue\BLO_streamglue.h
# End Source File
# End Group
# End Target
# End Project
