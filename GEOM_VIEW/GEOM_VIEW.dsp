# Microsoft Developer Studio Project File - Name="GEOM_VIEW" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GEOM_VIEW - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GEOM_VIEW.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GEOM_VIEW.mak" CFG="GEOM_VIEW - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GEOM_VIEW - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GEOM_VIEW - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "GEOM_VIEW - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /I "..\GEOM_MODEL\include" /I "..\MTL" /I "..\XMLLogging" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GEOM_VIEW - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /I "..\GEOM_MODEL\include" /I "..\MTL" /I "..\XMLLogging" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GEOM_VIEW - Win32 Release"
# Name "GEOM_VIEW - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\DibView.cpp
# End Source File
# Begin Source File

SOURCE=.\Graph.cpp
# End Source File
# Begin Source File

SOURCE=.\GUI_BASE.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "include"
# End Source File
# Begin Source File

SOURCE=.\Light.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectExplorer.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectTreeItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Renderable.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderContext.cpp
# End Source File
# Begin Source File

SOURCE=.\RotateTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SurfaceRenderable.cpp
# End Source File
# Begin Source File

SOURCE=.\TabControlBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Texture.cpp
# End Source File
# Begin Source File

SOURCE=.\Tracker.cpp
# End Source File
# Begin Source File

SOURCE=.\ZoomTracker.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\Camera.h
# End Source File
# Begin Source File

SOURCE=.\include\DibView.h
# End Source File
# Begin Source File

SOURCE=.\include\Graph.h
# End Source File
# Begin Source File

SOURCE=.\include\GUI_BASE_resource.h
# End Source File
# Begin Source File

SOURCE=.\include\Light.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectExplorer.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectTreeItem.h
# End Source File
# Begin Source File

SOURCE=.\include\Renderable.h
# End Source File
# Begin Source File

SOURCE=.\include\RenderContext.h
# End Source File
# Begin Source File

SOURCE=.\include\RotateTracker.h
# End Source File
# Begin Source File

SOURCE=.\include\SceneView.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\SurfaceRenderable.h
# End Source File
# Begin Source File

SOURCE=.\include\TabControlBar.h
# End Source File
# Begin Source File

SOURCE=.\include\Texture.h
# End Source File
# Begin Source File

SOURCE=.\include\Tracker.h
# End Source File
# Begin Source File

SOURCE=.\include\ZoomTracker.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\folder_closed.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_opened.bmp
# End Source File
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
