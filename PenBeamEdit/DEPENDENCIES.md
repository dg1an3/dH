# PenBeamEdit Dependencies Analysis

## Overview
PenBeamEdit is a Windows MFC application for editing and visualizing pencil beam radiotherapy dose distributions. This document provides a complete dependency analysis.

## Project Type
- **Application Type**: Win32 (x86) GUI Application
- **Framework**: MFC (Microsoft Foundation Classes) 6.0
- **Build System**: Visual Studio 6.0 Project (.dsp/.dsw)
- **Configurations**: Debug/Release for Win32

## System & Framework Dependencies

### Microsoft Foundation Classes (MFC)
- **Version**: MFC 6.0 (shared DLL)
- **Component**: AFXDLL
- **Purpose**: Core GUI framework
- **Headers Used**:
  - `<afxwin.h>` - Core MFC components
  - `<afxext.h>` - MFC extensions
  - `<afxdisp.h>` - MFC Automation/OLE
  - `<afxdtctl.h>` - Internet Explorer 4 Common Controls
  - `<afxtempl.h>` - MFC template classes
  - `<afxcmn.h>` - Windows Common Controls

### ActiveX/COM Dependencies
The project includes COM wrapper classes for **MSChart** ActiveX control (Microsoft Chart):
- `CMSChart` - Main chart control
- `CVcAxis` - Chart axis configuration
- `CVcAxisGrid` - Grid lines
- `CVcAxisTitle` - Axis titles
- `CVcBackdrop` - Chart background
- `CVcBrush` - Drawing brushes
- `CVcCategoryScale` - Category axis scale
- `CVcDataGrid` - Data grid overlay
- `CVcDataPoint` - Individual data points
- `CVcDataPointLabel` - Data point labels
- `CVcDataPoints` - Collection of data points
- `CVcFill` - Fill styles
- `CVcFootnote` - Chart footnotes
- `CVcLabel` - Text labels
- `CVcLabels` - Label collections
- `CVcLCoor` - Coordinate system
- `CVcLegend` - Chart legend
- `CVcLightSources` - 3D lighting
- `CVcLocation` - Position/location
- `CVcMarker` - Data markers
- `CVcPen` - Drawing pens
- `CVcPlot` - Plot area
- `CVcPlotBase` - Base plot functionality
- `CVcRect` - Rectangle primitives
- `CVcSeriesCollection` - Data series collection
- `CVcSeriesMarker` - Series markers
- `CVcSeriesPosition` - Series positioning
- `CVcStatLine` - Statistical lines
- `CVcTextLayout` - Text layout/formatting
- `CVcTick` - Axis tick marks
- `CVcTitle` - Chart title
- `CVcValueScale` - Value axis scale
- `CVcView3d` - 3D view support
- `COleFont` - OLE font support

**MSChart Control GUID**: `{3A2B370A-BA0A-11D1-B137-0000F8753F5D}`

## Internal Project Dependencies

### Direct Dependencies (from PenBeamEdit.dsw)
PenBeamEdit depends on the following internal library projects:

1. **GEOM_BASE** (`../GEOM_BASE/GEOM_BASE.dsp`)
   - Basic geometric primitives and utilities

2. **GUI_BASE** (`../GUI_BASE/GUI_BASE.dsp`)
   - Base GUI components and utilities

3. **MODEL_BASE** (`../MODEL_BASE/MODEL_BASE.dsp`)
   - Base data model classes

4. **GEOM_MODEL** (`../GEOM_MODEL/GEOM_MODEL.dsp`)
   - Geometric modeling classes

5. **GEOM_VIEW** (`../GEOM_VIEW/GEOM_VIEW.dsp`)
   - Geometric visualization and rendering
   - Provides: `CDib`, `CGraph` classes

6. **RT_MODEL** (`../RT_MODEL/RT_MODEL.dsp`)
   - Radiotherapy-specific model classes
   - Provides: `CPlan`, `CSeries`, `CBeam`, `CStructure`, `CVolume`, `CHistogram`

7. **OPTIMIZER_BASE** (`../OptimizeN/OPTIMIZER_BASE.dsp`)
   - Optimization algorithms and utilities

8. **XMLLogging** (`../XMLLogging/XMLLogging.dsp`)
   - XML-based logging framework
   - Enabled via: `USE_XMLLOGGING`, `XMLLOGGING_ON` defines

### Include Paths
```
..\RT_MODEL\include
..\GEOM_VIEW\include
..\GEOM_MODEL\include
..\OptimizeN\include
..\MTL
..\XMLLogging
```

### Key Classes Used from Dependencies

#### From RT_MODEL:
- `CPlan` - Treatment plan document
- `CSeries` - CT image series
- `CBeam` - Radiation beam
- `CStructure` - Anatomical structure/ROI
- `CVolume<T>` - 3D volume template class
- `CHistogram` - Dose histogram

#### From GEOM_VIEW:
- `CDib` - Device-independent bitmap handling
- `CGraph` - 2D graph visualization

#### From XMLLogging:
- `XMLLogging.h` - Logging macros and utilities
- `USES_FMT` macro

## Source Files

### Application Files
- **PenBeamEdit.cpp/h** - Application class (`CPenBeamEditApp`)
- **PenBeamEditView.cpp/h** - Main view class (`CPenBeamEditView`)
- **MainFrm.cpp/h** - Main frame window (`CMainFrame`)
- **StdAfx.cpp/h** - Precompiled headers

### Resource Files
- **PenBeamEdit.rc** - Windows resources
- **resource.h** - Resource ID definitions
- **res/PenBeamEdit.ico** - Application icon
- **res/PenBeamEditDoc.ico** - Document icon
- **res/PenBeamEdit.rc2** - Non-editable resources
- **res/Toolbar.bmp** - Toolbar bitmap
- **res/cursor1.cur** - Custom cursor
- **rainbow.bmp** - Colormap bitmap (for dose visualization)

### Documentation
- **ReadMe.txt** - Project overview
- **TransitionPlan.txt** - Migration/transition notes

## Preprocessor Defines

### Release Configuration
```cpp
WIN32
NDEBUG
_WINDOWS
_AFXDLL
_MBCS
USE_XMLLOGGING
XMLLOGGING_ON
```

### Debug Configuration
```cpp
WIN32
_DEBUG
_WINDOWS
_AFXDLL
_MBCS
USE_XMLLOGGING
XMLLOGGING_ON
```

## Compiler Settings

### Common
- **Compiler**: Intel C++ Compiler 6 (`xicl6.exe`)
- **Linker**: Intel Linker 6 (`xilink6.exe`)
- **Precompiled Header**: `stdafx.h`
- **Exception Handling**: Enabled (`/GX`)
- **Runtime Type Info**: Enabled

### Debug-Specific
- `/MDd` - Multithreaded DLL debug runtime
- `/Gm` - Minimal rebuild
- `/ZI` - Edit and continue debug info
- `/Od` - Disable optimizations
- `/GZ` - Catch release build errors
- `/FR` - Generate browse info

### Release-Specific
- `/MD` - Multithreaded DLL runtime
- `/O2` - Maximize speed optimizations

## Dependency Graph

```
PenBeamEdit.exe
├── MFC 6.0 (afxdll)
│   ├── afxwin.h (core)
│   ├── afxext.h (extensions)
│   ├── afxdisp.h (automation)
│   ├── afxdtctl.h (IE controls)
│   ├── afxtempl.h (templates)
│   └── afxcmn.h (common controls)
├── MSChart ActiveX Control
│   └── ~40 COM wrapper classes
├── XMLLogging
│   └── XMLLogging.h
├── RT_MODEL
│   ├── CPlan
│   ├── CSeries
│   ├── CBeam
│   ├── CStructure
│   ├── CVolume<T>
│   └── CHistogram
├── GEOM_VIEW
│   ├── CDib
│   └── CGraph
├── GEOM_MODEL
├── GEOM_BASE
├── GUI_BASE
├── MODEL_BASE
└── OPTIMIZER_BASE
```

## Runtime Requirements

### Required DLLs
- **MFC DLLs**: `MFC42.DLL` (or `MFC42D.DLL` for debug)
- **MSVCRT**: Microsoft Visual C++ Runtime
- **MSChart**: MSChart20 ActiveX control (must be registered)

### Optional Locale Support
- **MFCLOC.DLL**: Localized MFC resources (for non-English systems)
  - Copy `MFC42XXX.DLL` from VC++ CD-ROM and rename to `MFCLOC.DLL`

## Platform Requirements
- **Operating System**: Windows XP or later
- **Architecture**: x86 (32-bit)
- **WINVER**: 0x0501 (Windows XP)

## Build Order

To build PenBeamEdit successfully, the following projects must be built first:
1. MTL (mathematical template library)
2. XMLLogging
3. MODEL_BASE
4. GEOM_BASE
5. GEOM_MODEL
6. GEOM_VIEW
7. GUI_BASE
8. OPTIMIZER_BASE
9. RT_MODEL
10. PenBeamEdit (final executable)

## Key Features & Functionality

Based on the dependencies, PenBeamEdit provides:
- **Visualization**: 2D dose distribution display with colormap
- **Data Import**: ASCII file import for density and dose data
- **Beam Editing**: Pencil beam weight manipulation
- **Histogram Display**: DVH (dose-volume histogram) graphing via MSChart
- **Document Management**: MFC SDI pattern for plan/series documents
- **3D Volume Support**: Volume rendering and manipulation via CVolume<T>

## Notes

1. **Legacy Codebase**: Uses Visual Studio 6.0 format (pre-2000 era)
2. **Intel Compiler**: Uses Intel C++ 6 instead of Microsoft compiler
3. **ActiveX Dependency**: MSChart control must be registered in Windows registry
4. **Template Usage**: Heavy use of MFC template classes (CArray, etc.)
5. **Document Types**: Supports both CPlan (.pln) and CSeries (.ser) documents
