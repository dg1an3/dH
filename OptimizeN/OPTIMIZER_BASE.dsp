# Microsoft Developer Studio Project File - Name="OPTIMIZER_BASE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=OPTIMIZER_BASE - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OPTIMIZER_BASE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OPTIMIZER_BASE.mak" CFG="OPTIMIZER_BASE - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OPTIMIZER_BASE - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "OPTIMIZER_BASE - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "OPTIMIZER_BASE - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /GX /Ob2 /I "include" /I "..\MTL" /I "..\XMLLogging" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /D "XMLLOGGING_ON" /Yu"stdafx.h" /FD /O3 /QaxK /Qip /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "OPTIMIZER_BASE - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /I "..\MTL" /I "..\XMLLogging" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /D "XMLLOGGING_ON" /Yu"stdafx.h" /FD /GZ /c
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

# Name "OPTIMIZER_BASE - Win32 Release"
# Name "OPTIMIZER_BASE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BrentOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\ConjGradOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\CubicInterpOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\GradDescOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\LineFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectiveFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\Optimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\PowellOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\BrentOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\include\ConjGradOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\include\CubicInterpOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\include\GradDescOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\include\LineFunction.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectiveFunction.h
# End Source File
# Begin Source File

SOURCE=.\include\Optimizer.h
# End Source File
# Begin Source File

SOURCE=.\include\PowellOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
