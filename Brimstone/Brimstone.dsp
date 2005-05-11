# Microsoft Developer Studio Project File - Name="Brimstone" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Brimstone - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Brimstone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Brimstone.mak" CFG="Brimstone - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Brimstone - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Brimstone - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Brimstone - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\dcmtk-3.5.3\config\include" /I "..\..\dcmtk-3.5.3\dcmimgle\include" /I "..\..\dcmtk-3.5.3\ofstd\include" /I "..\..\dcmtk-3.5.3\dcmdata\include" /I "..\dcmtk-3.5.3\config\include" /I "..\dcmtk-3.5.3\dcmimgle\include" /I "..\dcmtk-3.5.3\ofstd\include" /I "..\dcmtk-3.5.3\dcmdata\include" /I "..\RT_MODEL\DicomImEx" /I "..\RT_MODEL\include" /I "..\OptimizeN\include" /I "..\GEOM_MODEL\include" /I "..\MTL" /I "..\XMLLogging" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /D "USE_IPP" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ippi20.lib ippcv20.lib ipps20.lib ippm20.lib ws2_32.lib netapi32.lib /nologo /subsystem:windows /profile /machine:I386 /libpath:"$(IPPROOT)\lib" /libpath:"$(IPPROOT)\stublib"

!ELSEIF  "$(CFG)" == "Brimstone - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\dcmtk-3.5.3\config\include" /I "..\dcmtk-3.5.3\dcmimgle\include" /I "..\dcmtk-3.5.3\ofstd\include" /I "..\dcmtk-3.5.3\dcmdata\include" /I "..\RT_MODEL\DicomImEx" /I "..\RT_MODEL\include" /I "..\OptimizeN\include" /I "..\GEOM_MODEL\include" /I "..\MTL" /I "..\XMLLogging" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "USE_XMLLOGGING" /D "XMLLOGGING_ON" /D "USE_IPP" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ippi20.lib ippcv20.lib ipps20.lib ippm20.lib ws2_32.lib netapi32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"$(IPPROOT)\lib" /libpath:"$(IPPROOT)\stublib"

!ENDIF 

# Begin Target

# Name "Brimstone - Win32 Release"
# Name "Brimstone - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Brimstone.cpp
# End Source File
# Begin Source File

SOURCE=.\Brimstone.rc
# End Source File
# Begin Source File

SOURCE=.\BrimstoneDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\BrimstoneView.cpp
# End Source File
# Begin Source File

SOURCE=.\Graph.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanarView.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanSetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PrescDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PrescriptionToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Brimstone.h
# End Source File
# Begin Source File

SOURCE=.\BrimstoneDoc.h
# End Source File
# Begin Source File

SOURCE=.\BrimstoneView.h
# End Source File
# Begin Source File

SOURCE=.\Graph.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\PlanarView.h
# End Source File
# Begin Source File

SOURCE=.\PlanSetupDlg.h
# End Source File
# Begin Source File

SOURCE=.\PrescDlg.h
# End Source File
# Begin Source File

SOURCE=.\PrescriptionToolbar.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Brimstone.ico
# End Source File
# Begin Source File

SOURCE=.\res\Brimstone.rc2
# End Source File
# Begin Source File

SOURCE=.\res\BrimstoneDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\legend.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rainbow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
