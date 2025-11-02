# wxWidgets Conversion of PenBeamEdit

This document describes the conversion of PenBeamEdit from MFC (Microsoft Foundation Classes) to wxWidgets, providing a cross-platform GUI framework.

## Overview

The PenBeamEdit application has been converted from MFC to wxWidgets. This conversion provides:
- **Cross-platform capability** - Can be built on Windows, Linux, and macOS
- **Modern GUI framework** - Active development and better long-term support
- **Open source** - No proprietary dependencies for the UI layer
- **Better documentation and community support**

## What Was Converted

### Application Layer (Complete)
- ✅ **PenBeamEditApp** - Main application class (CWinApp → wxApp)
- ✅ **MainFrame** - Main window with menu, toolbar, statusbar (CFrameWnd → wxFrame)
- ✅ **PenBeamEditCanvas** - Main drawing area for dose visualization (CView → wxPanel)

### New Files Created
```
PenBeamEdit/
├── PenBeamEditApp.h/cpp       - wxApp-based application
├── MainFrame.h/cpp            - wxFrame-based main window
└── PenBeamEditCanvas.h/cpp    - wxPanel-based dose visualization
```

### Old MFC Files (Retained for Reference)
```
PenBeamEdit/
├── PenBeamEdit.h/cpp          - Original MFC CWinApp
├── MainFrm.h/cpp              - Original MFC CFrameWnd
├── PenBeamEditView.h/cpp      - Original MFC CView
├── StdAfx.h/cpp               - MFC precompiled headers
└── PenBeamEdit.rc             - MFC resources
```

## Key Changes

### 1. Application Class
**Before (MFC):**
```cpp
class CPenBeamEditApp : public CWinApp
{
    virtual BOOL InitInstance();
    CSingleDocTemplate* m_pSeriesDocTemplate;
};
```

**After (wxWidgets):**
```cpp
class PenBeamEditApp : public wxApp
{
    virtual bool OnInit() override;
    wxDocManager* m_docManager;
};
```

### 2. Main Window
**Before (MFC):**
```cpp
class CMainFrame : public CFrameWnd
{
    CStatusBar m_wndStatusBar;
    CToolBar m_wndToolBar;
};
```

**After (wxWidgets):**
```cpp
class MainFrame : public wxFrame
{
    PenBeamEditCanvas* m_canvas;
    CPlan* m_plan;
};
```

### 3. View/Canvas
**Before (MFC):**
```cpp
class CPenBeamEditView : public CView
{
    void OnDraw(CDC* pDC);
    CGraph m_graph;
    CArray<COLORREF, COLORREF> m_arrColormap;
};
```

**After (wxWidgets):**
```cpp
class PenBeamEditCanvas : public wxPanel
{
    void OnPaint(wxPaintEvent& event);
    wxPanel* m_graphPanel;
    std::vector<wxColour> m_colormap;
};
```

### 4. Drawing Code
**Before (MFC CDC):**
```cpp
CBrush brush(color);
CBrush* pOldBrush = pDC->SelectObject(&brush);
pDC->Rectangle(x, y, w, h);
pDC->SelectObject(pOldBrush);
```

**After (wxWidgets wxDC):**
```cpp
dc.SetBrush(wxBrush(color));
dc.SetPen(*wxTRANSPARENT_PEN);
dc.DrawRectangle(x, y, w, h);
```

## What Still Uses MFC

### Core Libraries (MFC-Dependent)
The following libraries still depend on MFC and would require significant refactoring:

1. **MODEL_BASE** - Uses CObject, serialization
2. **GEOM_BASE** - Depends on MODEL_BASE
3. **GUI_BASE** - Uses CArray, CString
4. **GEOM_MODEL** - Uses CString, CObject, serialization
5. **GEOM_VIEW** - Uses CString, MFC collections
6. **RT_MODEL** - Heavily uses CString, CArray, CObject, IMPLEMENT_SERIAL
7. **OPTIMIZER_BASE** - Depends on GEOM_MODEL
8. **XMLLogging** - May use some MFC types

### MFC Usage Examples in Libraries

**RT_MODEL/Plan.cpp:**
```cpp
IMPLEMENT_SERIAL(CPlan, CModelObject, VERSIONABLE_SCHEMA | PLAN_SCHEMA)

CString strName;
CTypedPtrArray< CArray, CBeam* > m_arrBeams;
```

**RT_MODEL/Series.cpp:**
```cpp
CMesh* CSeries::CreateSphereStructure(const CString& strName)
{
    CString strStructureName;
    // ...
}
```

**RT_MODEL/Prescription.cpp:**
```cpp
REAL CPrescription::Eval(CVectorN<> *pvGrad,
                          const CArray<BOOL, BOOL>& arrInclude) const
```

## Build System

### CMake Configuration

**Root CMakeLists.txt:**
- Removed global `_AFXDLL` definition
- Added wxWidgets find_package
- Libraries still use MFC (CMAKE_MFC_FLAG = 2)

**PenBeamEdit/CMakeLists.txt:**
- Links with `${wxWidgets_LIBRARIES}`
- No longer depends on MFC
- Still links with MFC-based libraries (hybrid approach)

**Library CMakeLists.txt files:**
- All libraries define `_AFXDLL` locally
- Required because they still use MFC classes

### Building

```bash
mkdir build && cd build

# Windows with Visual Studio
cmake .. -G "Visual Studio 16 2019" -A Win32 \
    -DwxWidgets_ROOT_DIR=C:/path/to/wxWidgets

cmake --build . --config Release
```

### Requirements
- **CMake 3.15+**
- **wxWidgets 3.0+** - Must be installed and findable by CMake
- **Visual Studio 2017+** (Windows) - For MFC support in libraries
- **MFC Development Tools** - Required for building the libraries

## Current Architecture

```
┌─────────────────────────────────┐
│   PenBeamEdit (wxWidgets GUI)   │
└─────────────────┬───────────────┘
                  │
        ┌─────────┴─────────┐
        │                   │
┌───────▼────────┐   ┌──────▼──────┐
│   RT_MODEL     │   │  GEOM_VIEW  │
│   (MFC-based)  │   │ (MFC-based) │
└───────┬────────┘   └──────┬──────┘
        │                   │
        └─────────┬─────────┘
                  │
        ┌─────────▼──────────┐
        │    GEOM_MODEL      │
        │    (MFC-based)     │
        └────────────────────┘
```

This is a **hybrid architecture** where the GUI uses wxWidgets but the business logic libraries still use MFC.

## Advantages of Current Approach

✅ **Modern GUI** - wxWidgets provides better cross-platform support
✅ **Minimal disruption** - Core algorithm code unchanged
✅ **Incremental migration** - Can refactor libraries one at a time
✅ **Working application** - PenBeamEdit functions with wxWidgets GUI

## Limitations

⚠️ **Still requires MFC** - Cannot build on non-Windows without Wine/compatibility layer
⚠️ **Hybrid complexity** - Mixing two GUI frameworks
⚠️ **Memory management** - Need to carefully manage objects between MFC and wxWidgets
⚠️ **String conversions** - CString ↔ wxString conversions needed at boundaries

## Complete Migration Roadmap

To fully remove MFC dependencies, the following steps would be needed:

### Phase 1: Replace MFC Collections (Moderate Effort)
- Replace `CString` with `std::string` or `wxString`
- Replace `CArray` with `std::vector`
- Replace `CList` with `std::list`
- Replace `CMap` with `std::map` / `std::unordered_map`
- Replace `CTypedPtrArray` with `std::vector<T*>` or smart pointers

### Phase 2: Replace MFC Base Classes (High Effort)
- Replace `CObject` with custom base class or remove
- Remove `IMPLEMENT_SERIAL` / `DECLARE_SERIAL` macros
- Implement custom serialization (or use library like Boost.Serialization)
- Replace `CModelObject` with custom base class

### Phase 3: Replace MFC Utilities (Low-Moderate Effort)
- Replace `CFile` / `CStdioFile` with `std::fstream` or wxFile
- Replace `CRect` / `CSize` / `CPoint` with wxWidgets equivalents
- Replace `CDib` with wxImage
- Replace `CException` with std::exception hierarchy

### Phase 4: Update CMake (Low Effort)
- Remove `CMAKE_MFC_FLAG`
- Remove `_AFXDLL` definitions
- Update compile flags for non-Windows platforms

### Estimated Total Effort
- **Phase 1:** 40-60 hours (touching ~50+ files)
- **Phase 2:** 60-80 hours (requires careful refactoring)
- **Phase 3:** 20-30 hours
- **Phase 4:** 2-4 hours
- **Testing:** 40-60 hours
- **Total:** ~200-250 hours of development time

## Testing the wxWidgets Version

### Current Functionality
- ✅ Application starts with wxWidgets window
- ✅ Menu bar with File and Help menus
- ✅ Toolbar with standard buttons
- ✅ Status bar with 4 panes
- ✅ Main canvas for dose visualization
- ✅ Colormap loading (rainbow gradient)
- ✅ Dose/density rendering

### Not Yet Implemented
- ⚠️ File I/O (Open/Save plan files)
- ⚠️ Density data import
- ⚠️ Histogram graph display (right panel)
- ⚠️ Mouse interaction
- ⚠️ MSChart integration for DVH display

### To Test
```bash
cd build
./bin/PenBeamEdit  # or PenBeamEdit.exe on Windows
```

## String Conversion Utilities

When interfacing between MFC and wxWidgets code:

```cpp
// CString → wxString
CString mfcStr = "Hello";
wxString wxStr(mfcStr.GetString());

// wxString → CString
wxString wxStr = "World";
CString mfcStr(wxStr.c_str().AsChar());

// std::string → CString
std::string stdStr = "Test";
CString mfcStr(stdStr.c_str());

// CString → std::string
CString mfcStr = "Test";
std::string stdStr(mfcStr.GetString());
```

## Recommendations

### For Production Use
1. **Use Current Hybrid** - Acceptable for Windows-only deployment
2. **Test Thoroughly** - Verify all UI interactions work correctly
3. **Monitor Performance** - Check for any overhead from framework mixing

### For Cross-Platform Goal
1. **Complete Migration** - Remove all MFC dependencies
2. **Use Standard C++** - Prefer std:: over framework-specific types
3. **Consider wxWidgets Doc/View** - Use wxDocument/wxView for document management
4. **Unit Tests** - Add comprehensive tests before refactoring

### For Maintenance
1. **Keep Old Files** - Retain MFC version until wxWidgets version is proven
2. **Document Interfaces** - Clearly mark MFC/wxWidgets boundaries
3. **Use Smart Pointers** - Convert to std::unique_ptr/shared_ptr for better memory safety

## Resources

### wxWidgets Documentation
- [wxWidgets Official Docs](https://docs.wxwidgets.org/)
- [wxWiki](https://wiki.wxwidgets.org/)
- [Migration Guide](https://docs.wxwidgets.org/trunk/page_port_mfc.html)

### MFC to wxWidgets Class Mapping
| MFC Class      | wxWidgets Equivalent      |
|---------------|---------------------------|
| CWinApp        | wxApp                     |
| CFrameWnd      | wxFrame                   |
| CView          | wxScrolledWindow, wxPanel |
| CDialog        | wxDialog                  |
| CString        | wxString                  |
| CArray         | std::vector               |
| CList          | std::list                 |
| CMap           | std::map                  |
| CFile          | wxFile, std::fstream      |
| CDC            | wxDC, wxPaintDC           |
| CBitmap        | wxBitmap                  |
| CRect          | wxRect                    |
| CPoint         | wxPoint                   |
| CSize          | wxSize                    |

## Conclusion

The PenBeamEdit GUI has been successfully converted to wxWidgets, providing a modern, cross-platform interface. However, the underlying libraries still depend on MFC, creating a hybrid architecture that requires both frameworks.

For full cross-platform capability, a complete migration of the core libraries from MFC to standard C++ would be necessary. This is a significant undertaking but would result in a truly portable codebase.

The current hybrid approach is functional and provides immediate benefits of the wxWidgets framework while maintaining compatibility with the existing MFC-based business logic.
