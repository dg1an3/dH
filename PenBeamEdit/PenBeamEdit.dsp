# Microsoft Developer Studio Project File - Name="PenBeamEdit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PenBeamEdit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PenBeamEdit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PenBeamEdit.mak" CFG="PenBeamEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PenBeamEdit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PenBeamEdit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PenBeamEdit - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\MODEL_BASE" /I "..\GEOM_BASE" /I "..\GUI_BASE" /I "..\VSIM_MODEL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
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

!ELSEIF  "$(CFG)" == "PenBeamEdit - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\MODEL_BASE" /I "..\GEOM_BASE" /I "..\GUI_BASE" /I "..\VSIM_MODEL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
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

# Name "PenBeamEdit - Win32 Release"
# Name "PenBeamEdit - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\PenBeamEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\PenBeamEdit.rc
# End Source File
# Begin Source File

SOURCE=.\PenBeamEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\PenBeamEdit.h
# End Source File
# Begin Source File

SOURCE=.\PenBeamEditView.h
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

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\PenBeamEdit.ico
# End Source File
# Begin Source File

SOURCE=.\res\PenBeamEdit.rc2
# End Source File
# Begin Source File

SOURCE=.\res\PenBeamEditDoc.ico
# End Source File
# Begin Source File

SOURCE=.\rainbow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\TransitionPlan.txt
# End Source File
# End Target
# End Project
# Section PenBeamEdit : {E9E0752A-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcDataPoints
# 	2:10:HeaderFile:vcdatapoints.h
# 	2:8:ImplFile:vcdatapoints.cpp
# End Section
# Section PenBeamEdit : {E9E074EE-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLightSources
# 	2:10:HeaderFile:vclightsources.h
# 	2:8:ImplFile:vclightsources.cpp
# End Section
# Section PenBeamEdit : {E9E074E2-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcBackdrop
# 	2:10:HeaderFile:vcbackdrop.h
# 	2:8:ImplFile:vcbackdrop.cpp
# End Section
# Section PenBeamEdit : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font.h
# 	2:8:ImplFile:font.cpp
# End Section
# Section PenBeamEdit : {E9E074F2-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcView3d
# 	2:10:HeaderFile:vcview3d.h
# 	2:8:ImplFile:vcview3d.cpp
# End Section
# Section PenBeamEdit : {E9E07527-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcPlot
# 	2:10:HeaderFile:vcplot.h
# 	2:8:ImplFile:vcplot.cpp
# End Section
# Section PenBeamEdit : {E9E07517-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcCategoryScale
# 	2:10:HeaderFile:vccategoryscale.h
# 	2:8:ImplFile:vccategoryscale.cpp
# End Section
# Section PenBeamEdit : {E9E074D6-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLocation
# 	2:10:HeaderFile:vclocation.h
# 	2:8:ImplFile:vclocation.cpp
# End Section
# Section PenBeamEdit : {E9E07502-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcSeriesPosition
# 	2:10:HeaderFile:vcseriesposition.h
# 	2:8:ImplFile:vcseriesposition.cpp
# End Section
# Section PenBeamEdit : {E9E0750E-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcDataPoint
# 	2:10:HeaderFile:vcdatapoint.h
# 	2:8:ImplFile:vcdatapoint.cpp
# End Section
# Section PenBeamEdit : {E9E074CD-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcRect
# 	2:10:HeaderFile:vcrect.h
# 	2:8:ImplFile:vcrect.cpp
# End Section
# Section PenBeamEdit : {E9E074E6-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcFootnote
# 	2:10:HeaderFile:vcfootnote.h
# 	2:8:ImplFile:vcfootnote.cpp
# End Section
# Section PenBeamEdit : {E9E074D1-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcPen
# 	2:10:HeaderFile:vcpen.h
# 	2:8:ImplFile:vcpen.cpp
# End Section
# Section PenBeamEdit : {3A2B370A-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CMSChart
# 	2:10:HeaderFile:mschart.h
# 	2:8:ImplFile:mschart.cpp
# End Section
# Section PenBeamEdit : {E9E07506-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcStatLine
# 	2:10:HeaderFile:vcstatline.h
# 	2:8:ImplFile:vcstatline.cpp
# End Section
# Section PenBeamEdit : {E9E0751D-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcIntersection
# 	2:10:HeaderFile:vcintersection.h
# 	2:8:ImplFile:vcintersection.cpp
# End Section
# Section PenBeamEdit : {E9E074D5-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcTextLayout
# 	2:10:HeaderFile:vctextlayout.h
# 	2:8:ImplFile:vctextlayout.cpp
# End Section
# Section PenBeamEdit : {E9E07511-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcSeries
# 	2:10:HeaderFile:vcseries.h
# 	2:8:ImplFile:vcseries.cpp
# End Section
# Section PenBeamEdit : {E9E07521-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcAxisScale
# 	2:10:HeaderFile:vcaxisscale.h
# 	2:8:ImplFile:vcaxisscale.cpp
# End Section
# Section PenBeamEdit : {E9E074DC-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcShadow
# 	2:10:HeaderFile:vcshadow.h
# 	2:8:ImplFile:vcshadow.cpp
# End Section
# Section PenBeamEdit : {E9E074EC-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLightSource
# 	2:10:HeaderFile:vclightsource.h
# 	2:8:ImplFile:vclightsource.cpp
# End Section
# Section PenBeamEdit : {E9E074E0-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcFrame
# 	2:10:HeaderFile:vcframe.h
# 	2:8:ImplFile:vcframe.cpp
# End Section
# Section PenBeamEdit : {E9E074FC-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcWall
# 	2:10:HeaderFile:vcwall.h
# 	2:8:ImplFile:vcwall.cpp
# End Section
# Section PenBeamEdit : {E9E074F0-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLight
# 	2:10:HeaderFile:vclight.h
# 	2:8:ImplFile:vclight.cpp
# End Section
# Section PenBeamEdit : {E9E074C9-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcCoor
# 	2:10:HeaderFile:vccoor.h
# 	2:8:ImplFile:vccoor.cpp
# End Section
# Section PenBeamEdit : {E9E07515-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcAxisTitle
# 	2:10:HeaderFile:vcaxistitle.h
# 	2:8:ImplFile:vcaxistitle.cpp
# End Section
# Section PenBeamEdit : {E9E0750C-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcDataPointLabel
# 	2:10:HeaderFile:vcdatapointlabel.h
# 	2:8:ImplFile:vcdatapointlabel.cpp
# End Section
# Section PenBeamEdit : {E9E07525-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcAxis
# 	2:10:HeaderFile:vcaxis.h
# 	2:8:ImplFile:vcaxis.cpp
# End Section
# Section PenBeamEdit : {E9E074E4-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcTitle
# 	2:10:HeaderFile:vctitle.h
# 	2:8:ImplFile:vctitle.cpp
# End Section
# Section PenBeamEdit : {E9E074F4-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcPlotBase
# 	2:10:HeaderFile:vcplotbase.h
# 	2:8:ImplFile:vcplotbase.cpp
# End Section
# Section PenBeamEdit : {E9E074CB-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLCoor
# 	2:10:HeaderFile:vclcoor.h
# 	2:8:ImplFile:vclcoor.cpp
# End Section
# Section PenBeamEdit : {E9E074D8-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcFont
# 	2:10:HeaderFile:vcfont.h
# 	2:8:ImplFile:vcfont.cpp
# End Section
# Section PenBeamEdit : {E9E074E8-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLegend
# 	2:10:HeaderFile:vclegend.h
# 	2:8:ImplFile:vclegend.cpp
# End Section
# Section PenBeamEdit : {E9E07504-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcSeriesMarker
# 	2:10:HeaderFile:vcseriesmarker.h
# 	2:8:ImplFile:vcseriesmarker.cpp
# End Section
# Section PenBeamEdit : {E9E074CF-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcColor
# 	2:10:HeaderFile:vccolor.h
# 	2:8:ImplFile:vccolor.cpp
# End Section
# Section PenBeamEdit : {E9E074D3-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcMarker
# 	2:10:HeaderFile:vcmarker.h
# 	2:8:ImplFile:vcmarker.cpp
# End Section
# Section PenBeamEdit : {E9E0751B-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcAxisGrid
# 	2:10:HeaderFile:vcaxisgrid.h
# 	2:8:ImplFile:vcaxisgrid.cpp
# End Section
# Section PenBeamEdit : {3A2B370C-BA0A-11D1-B137-0000F8753F5D}
# 	2:21:DefaultSinkHeaderFile:mschart.h
# 	2:16:DefaultSinkClass:CMSChart
# End Section
# Section PenBeamEdit : {E9E074DA-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcBrush
# 	2:10:HeaderFile:vcbrush.h
# 	2:8:ImplFile:vcbrush.cpp
# End Section
# Section PenBeamEdit : {E9E074EA-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcDataGrid
# 	2:10:HeaderFile:vcdatagrid.h
# 	2:8:ImplFile:vcdatagrid.cpp
# End Section
# Section PenBeamEdit : {E9E074FA-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcWeighting
# 	2:10:HeaderFile:vcweighting.h
# 	2:8:ImplFile:vcweighting.cpp
# End Section
# Section PenBeamEdit : {E9E07508-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLabel
# 	2:10:HeaderFile:vclabel.h
# 	2:8:ImplFile:vclabel.cpp
# End Section
# Section PenBeamEdit : {E9E074DE-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcFill
# 	2:10:HeaderFile:vcfill.h
# 	2:8:ImplFile:vcfill.cpp
# End Section
# Section PenBeamEdit : {E9E07513-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcSeriesCollection
# 	2:10:HeaderFile:vcseriescollection.h
# 	2:8:ImplFile:vcseriescollection.cpp
# End Section
# Section PenBeamEdit : {E9E07523-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcValueScale
# 	2:10:HeaderFile:vcvaluescale.h
# 	2:8:ImplFile:vcvaluescale.cpp
# End Section
# Section PenBeamEdit : {E9E0750A-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcLabels
# 	2:10:HeaderFile:vclabels.h
# 	2:8:ImplFile:vclabels.cpp
# End Section
# Section PenBeamEdit : {E9E0751F-BA0A-11D1-B137-0000F8753F5D}
# 	2:5:Class:CVcTick
# 	2:10:HeaderFile:vctick.h
# 	2:8:ImplFile:vctick.cpp
# End Section
