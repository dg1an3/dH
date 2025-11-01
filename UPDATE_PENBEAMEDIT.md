# PenBeamEdit Relocation Update

## Date: 2025-10-31

## Change

**PenBeamEdit** has been moved from `deprecated/` to `src_classic/`

## Rationale

PenBeamEdit is a classic-era MFC application (DevStudio 6.0, 2004) that fits better with the classic application stack rather than being isolated in deprecated/.

### Classification:
- **Type**: MFC GUI Application
- **Era**: 2004 (DevStudio 6.0 format)
- **Dependencies**: Uses MTL (Math Template Library)
- **Purpose**: Pencil beam editing tool
- **Stack**: Part of classic RT workflow

### Previous Location:
`deprecated/PenBeamEdit/` - Initially placed here as an "old utility"

### New Location:
`src_classic/PenBeamEdit/` - Now with other classic applications

## Current src_classic/ Contents (6 applications)

1. **Brimstone_original** - Main legacy RT planning GUI
2. **RT_MODEL** - Original modular RT library (with utilities)
3. **Graph_original** - Legacy DVH visualization
4. **VSIM_OGL** - Visual simulation with OpenGL
5. **DivFluence** - Divergent fluence calculations
6. **PenBeamEdit** - Pencil beam editor ← NEW

All use DevStudio 6.0 or Visual Studio 2005 project formats (.dsp or .vcproj).

## Directory Structure Update

```
dH/
├── src_classic/              → Classic applications (6)
│   ├── Brimstone_original/
│   ├── RT_MODEL/
│   ├── Graph_original/
│   ├── VSIM_OGL/
│   ├── DivFluence/
│   └── PenBeamEdit/          ← MOVED HERE
│
└── deprecated/               → Deprecated code (empty - ready for other items)
```

## Benefits

1. **Better organization** - Classic applications grouped together
2. **Consistent categorization** - All classic-era apps in src_classic/
3. **deprecated/ available** - Can be used for truly obsolete/non-functional code
4. **Historical context** - Shows complete classic application suite

## Documentation Updated

- ✅ STRUCTURE.md updated - PenBeamEdit now shown in src_classic/
- ✅ File counts updated - src_classic/ now shows 6 applications
- ✅ Original directories list updated
- ✅ deprecated/ marked as empty

## No Build Impact

Moving PenBeamEdit doesn't affect:
- Modern stack (src/) - Never used PenBeamEdit
- Main solution (Brimstone_src.sln) - Doesn't reference PenBeamEdit
- Classic stack builds - PenBeamEdit is standalone

PenBeamEdit original directory in root still pending deletion with other originals.
