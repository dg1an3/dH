# Microsoft Developer Studio Project File - Name="ClusterDlg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ClusterDlg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClusterDlg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClusterDlg.mak" CFG="ClusterDlg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClusterDlg - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ClusterDlg - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClusterDlg - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\VecMat" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "ClusterDlg - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\VecMat" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ClusterDlg - Win32 Release"
# Name "ClusterDlg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ClusterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ClusterDlg.rc
# End Source File
# Begin Source File

SOURCE=.\ClusterDlgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Cluster.h
# End Source File
# Begin Source File

SOURCE=.\ClusterDlg.h
# End Source File
# Begin Source File

SOURCE=.\ClusterDlgDlg.h
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

SOURCE=.\res\ClusterDlg.ico
# End Source File
# Begin Source File

SOURCE=.\res\ClusterDlg.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section ClusterDlg : {5356F2F0-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CPane
# 	2:10:HeaderFile:pane.h
# 	2:8:ImplFile:pane.cpp
# End Section
# Section ClusterDlg : {0719D4A0-5BCA-11D1-8051-00600893B445}
# 	2:5:Class:Cnterior
# 	2:10:HeaderFile:nterior.h
# 	2:8:ImplFile:nterior.cpp
# End Section
# Section ClusterDlg : {A2BD023C-D428-11D1-8837-0060089BA39A}
# 	2:5:Class:CFilter
# 	2:10:HeaderFile:filter.h
# 	2:8:ImplFile:filter.cpp
# End Section
# Section ClusterDlg : {5C638330-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CPanes
# 	2:10:HeaderFile:panes.h
# 	2:8:ImplFile:panes.cpp
# End Section
# Section ClusterDlg : {9F6AA700-D188-11CD-AD48-00AA003C9CB6}
# 	2:5:Class:CRowCursor
# 	2:10:HeaderFile:rowcursor.h
# 	2:8:ImplFile:rowcursor.cpp
# End Section
# Section ClusterDlg : {29CB89F0-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:COWCFont
# 	2:10:HeaderFile:owcfont.h
# 	2:8:ImplFile:owcfont.cpp
# End Section
# Section ClusterDlg : {1F40F4D0-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CBorders
# 	2:10:HeaderFile:borders.h
# 	2:8:ImplFile:borders.cpp
# End Section
# Section ClusterDlg : {C0846D11-D402-11D1-823A-0060083316C7}
# 	2:5:Class:CAutoFilter
# 	2:10:HeaderFile:autofilter.h
# 	2:8:ImplFile:autofilter.cpp
# End Section
# Section ClusterDlg : {2D3BEF00-2C50-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CWorksheet
# 	2:10:HeaderFile:worksheet.h
# 	2:8:ImplFile:worksheet.cpp
# End Section
# Section ClusterDlg : {193F4B98-1CB8-11D1-9923-00C04FB937E1}
# 	2:5:Class:CWebCalc
# 	2:10:HeaderFile:webcalc.h
# 	2:8:ImplFile:webcalc.cpp
# End Section
# Section ClusterDlg : {0FB45160-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CBorder
# 	2:10:HeaderFile:border.h
# 	2:8:ImplFile:border.cpp
# End Section
# Section ClusterDlg : {A7D3C540-2CED-11D1-B4D6-444553540000}
# 	2:5:Class:CRange
# 	2:10:HeaderFile:range.h
# 	2:8:ImplFile:range.cpp
# End Section
# Section ClusterDlg : {4030E170-2DE7-11D1-B1CE-00A0C9056619}
# 	2:5:Class:CHyperlink
# 	2:10:HeaderFile:hyperlink.h
# 	2:8:ImplFile:hyperlink.cpp
# End Section
# Section ClusterDlg : {D7846FF1-91CE-11D1-9DE4-00600893B542}
# 	2:5:Class:CProtection
# 	2:10:HeaderFile:protection.h
# 	2:8:ImplFile:protection.cpp
# End Section
# Section ClusterDlg : {A2BD023A-D428-11D1-8837-0060089BA39A}
# 	2:5:Class:CCriteria
# 	2:10:HeaderFile:criteria.h
# 	2:8:ImplFile:criteria.cpp
# End Section
# Section ClusterDlg : {5F4DF280-531B-11CF-91F6-C2863C385E30}
# 	2:5:Class:CMSFlexGrid
# 	2:10:HeaderFile:msflexgrid.h
# 	2:8:ImplFile:msflexgrid.cpp
# End Section
# Section ClusterDlg : {6262D3A0-531B-11CF-91F6-C2863C385E30}
# 	2:21:DefaultSinkHeaderFile:msflexgrid.h
# 	2:16:DefaultSinkClass:CMSFlexGrid
# End Section
# Section ClusterDlg : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font.h
# 	2:8:ImplFile:font.cpp
# End Section
# Section ClusterDlg : {0002E510-0000-0000-C000-000000000046}
# 	2:21:DefaultSinkHeaderFile:webcalc.h
# 	2:16:DefaultSinkClass:CWebCalc
# End Section
# Section ClusterDlg : {BB917B40-2CED-11D1-B4D6-444553540000}
# 	2:5:Class:CTitleBar
# 	2:10:HeaderFile:titlebar.h
# 	2:8:ImplFile:titlebar.cpp
# End Section
# Section ClusterDlg : {A2BD023D-D428-11D1-8837-0060089BA39A}
# 	2:5:Class:CFilters
# 	2:10:HeaderFile:filters.h
# 	2:8:ImplFile:filters.cpp
# End Section
# Section ClusterDlg : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
