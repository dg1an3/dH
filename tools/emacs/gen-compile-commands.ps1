<#
.SYNOPSIS
    Generates compile_commands.json (JSON Compilation Database) for the Brimstone
    MSBuild/.vcxproj solution, so clangd can provide code navigation in Emacs.

.DESCRIPTION
    This never invokes the compiler and never requires a successful build.  It injects
    emacs\DumpCompileInfo.targets into each .vcxproj via
    /p:CustomAfterMicrosoftCommonTargets and runs only the DumpCompileInfo target, so
    MSBuild performs the full property/item evaluation (toolset pin, property sheets,
    ItemDefinitionGroup inheritance, CharacterSet/UseOfMfc defines) and hands back the
    resolved ClCompile metadata.  The script then rewrites that as clang-cl flags.

    Output: <repo root>\compile_commands.json

.PARAMETER Configuration
    MSBuild configuration to evaluate.  Default: Debug

.PARAMETER Platform
    MSBuild platform to evaluate.  Default: ARM64

.PARAMETER Solution
    Path to the .sln whose projects should be scanned.  Default: Brimstone_src.sln at repo root.

.PARAMETER OutFile
    Path of the compilation database to write.  Default: <repo root>\compile_commands.json

.PARAMETER MSBuildPath
    Full path to MSBuild.exe.  Auto-detected via vswhere when omitted.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File emacs\gen-compile-commands.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File emacs\gen-compile-commands.ps1 -Configuration Release -Platform x64

.NOTES
    Windows PowerShell 5.1 compatible.  Does not modify any .vcxproj/.sln/source file.
#>
[CmdletBinding()]
param(
    [string] $Configuration = 'Debug',
    [string] $Platform      = 'ARM64',
    [string] $Solution,
    [string] $OutFile,
    [string] $MSBuildPath
)

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

$ScriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RepoRoot   = Split-Path -Parent $ScriptDir
$TargetsFile = Join-Path $ScriptDir 'DumpCompileInfo.targets'

if (-not (Test-Path $TargetsFile)) {
    throw "Missing helper targets file: $TargetsFile"
}
if (-not $Solution) { $Solution = Join-Path $RepoRoot 'Brimstone_src.sln' }
if (-not $OutFile)  { $OutFile  = Join-Path $RepoRoot 'compile_commands.json' }

# ---------------------------------------------------------------------------
# Locate MSBuild
# ---------------------------------------------------------------------------

function Find-MSBuild {
    param([string] $Explicit)

    if ($Explicit) {
        if (Test-Path $Explicit) { return $Explicit }
        throw "MSBuild not found at -MSBuildPath '$Explicit'"
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $root = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
        if ($root) {
            # Prefer a host-native MSBuild (arm64 on Windows-on-ARM), then amd64, then x86.
            $candidates = @(
                (Join-Path $root 'MSBuild\Current\Bin\arm64\MSBuild.exe'),
                (Join-Path $root 'MSBuild\Current\Bin\amd64\MSBuild.exe'),
                (Join-Path $root 'MSBuild\Current\Bin\MSBuild.exe')
            )
            if ($env:PROCESSOR_ARCHITECTURE -ne 'ARM64') {
                $candidates = @($candidates[2], $candidates[1], $candidates[0])
            }
            foreach ($c in $candidates) {
                if (Test-Path $c) { return $c }
            }
        }
    }

    $onPath = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
    if ($onPath) { return $onPath.Source }

    throw 'Could not locate MSBuild.exe. Pass -MSBuildPath explicitly.'
}

$MSBuild = Find-MSBuild -Explicit $MSBuildPath
Write-Host "MSBuild:       $MSBuild"
Write-Host "Solution:      $Solution"
Write-Host "Configuration: $Configuration|$Platform"

# ---------------------------------------------------------------------------
# Enumerate the .vcxproj files referenced by the solution
# ---------------------------------------------------------------------------

if (-not (Test-Path $Solution)) { throw "Solution not found: $Solution" }
$SolutionDir = Split-Path -Parent $Solution

$projects = New-Object System.Collections.ArrayList
foreach ($line in (Get-Content -LiteralPath $Solution)) {
    if ($line -match '^\s*Project\(') {
        # Project("{GUID}") = "Name", "Relative\Path.vcxproj", "{GUID}"
        $fields = [regex]::Matches($line, '"([^"]*)"')
        foreach ($f in $fields) {
            $val = $f.Groups[1].Value
            if ($val -like '*.vcxproj') {
                $full = [System.IO.Path]::GetFullPath((Join-Path $SolutionDir $val))
                if (Test-Path $full) { [void]$projects.Add($full) }
                else { Write-Warning "Solution references a missing project: $full" }
            }
        }
    }
}

if ($projects.Count -eq 0) { throw "No .vcxproj files found in $Solution" }
Write-Host ("Projects:      {0}" -f $projects.Count)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Resolve-Dir {
    # Make a possibly-relative include dir absolute w.r.t. the project directory.
    param([string] $Dir, [string] $BaseDir)

    $d = $Dir.Trim().Trim('"')
    if (-not $d) { return $null }
    # Drop unexpanded MSBuild macros / vcpkg-style vars that survived evaluation.
    if ($d -match '\$\(') { return $null }

    try {
        if ([System.IO.Path]::IsPathRooted($d)) {
            return [System.IO.Path]::GetFullPath($d)
        }
        return [System.IO.Path]::GetFullPath((Join-Path $BaseDir $d))
    } catch {
        return $null
    }
}

function Convert-LanguageStandard {
    # MSBuild <LanguageStandard> token -> clang-cl /std: switch
    param([string] $Std)

    switch ($Std) {
        'stdcpp14'      { return '/std:c++14' }
        'stdcpp17'      { return '/std:c++17' }
        'stdcpp20'      { return '/std:c++20' }
        'stdcpp23'      { return '/std:c++23' }
        'stdcpplatest'  { return '/std:c++latest' }
        default         { return '/std:c++14' }   # MSVC's own default
    }
}

# Cache of "does this source already #include its PCH header" lookups.
$script:PchIncludeCache = @{}
function Test-SourceIncludesPch {
    param([string] $SourceFile, [string] $PchHeader)

    $key = $SourceFile + '|' + $PchHeader
    if ($script:PchIncludeCache.ContainsKey($key)) { return $script:PchIncludeCache[$key] }

    $result = $false
    if (Test-Path -LiteralPath $SourceFile) {
        $leaf = [System.IO.Path]::GetFileName($PchHeader)
        $pattern = '^\s*#\s*include\s*["<].*' + [regex]::Escape($leaf) + '[">]'
        $head = Get-Content -LiteralPath $SourceFile -TotalCount 60 -ErrorAction SilentlyContinue
        foreach ($l in $head) {
            if ($l -match $pattern) { $result = $true; break }
        }
    }
    $script:PchIncludeCache[$key] = $result
    return $result
}

# clang-cl needs an -fms-compatibility-version high enough that the pinned MSVC STL
# headers do not reject it.  Derive _MSC_VER from the toolset's own header if possible.
function Get-MsCompatibilityVersion {
    param([string[]] $SystemIncludeDirs)

    # The toolset directory name carries the version: ...\VC\Tools\MSVC\14.51.36231\include
    # MSVC toolset 14.NN corresponds to _MSC_VER 19NN, i.e. -fms-compatibility-version=19.NN.
    foreach ($d in $SystemIncludeDirs) {
        if ($d -match '\\MSVC\\14\.(\d+)\.') {
            return ('19.{0}' -f $matches[1])
        }
    }
    return $null
}

# ---------------------------------------------------------------------------
# Per-project: run the injected DumpCompileInfo target and parse its output
# ---------------------------------------------------------------------------

$tmpDir = Join-Path ([System.IO.Path]::GetTempPath()) ('gencc_' + [System.Guid]::NewGuid().ToString('N'))
[void](New-Item -ItemType Directory -Path $tmpDir)

$entries      = New-Object System.Collections.ArrayList
$missingDirs  = New-Object System.Collections.ArrayList
$failedProj   = New-Object System.Collections.ArrayList

try {
    foreach ($proj in $projects) {

        $dump = Join-Path $tmpDir ((Split-Path -Leaf $proj) + '.dci.txt')

        $msbArgs = @(
            $proj,
            '/nologo',
            '/v:quiet',
            '/nr:false',
            '/t:DumpCompileInfo',
            ('/p:Configuration=' + $Configuration),
            ('/p:Platform=' + $Platform),
            ('/p:CustomAfterMicrosoftCommonTargets=' + $TargetsFile),
            ('/p:DciOutFile=' + $dump)
        )

        Write-Host ("  evaluating {0} ..." -f (Split-Path -Leaf $proj))
        $msbOut = & $MSBuild $msbArgs
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $dump)) {
            [void]$failedProj.Add((Split-Path -Leaf $proj))
            Write-Warning ("MSBuild evaluation failed for {0} (exit {1}). Skipping." -f $proj, $LASTEXITCODE)
            if ($msbOut) { $msbOut | Select-Object -First 15 | ForEach-Object { Write-Warning "    $_" } }
            continue
        }

        # --- parse the dump ------------------------------------------------
        $projDir   = ''
        $projName  = ''
        $sysInc    = New-Object System.Collections.ArrayList
        $addInc    = New-Object System.Collections.ArrayList
        $defines   = New-Object System.Collections.ArrayList
        $forcedInc = New-Object System.Collections.ArrayList
        $files     = New-Object System.Collections.ArrayList

        foreach ($line in (Get-Content -LiteralPath $dump)) {
            if (-not $line) { continue }
            $tag  = $line.Substring(0, 1)
            $rest = $line.Substring(2)
            switch ($tag) {
                'D' { $projDir  = $rest }
                'N' { $projName = $rest }
                'S' { [void]$sysInc.Add($rest) }
                'I' { [void]$addInc.Add($rest) }
                'P' { [void]$defines.Add($rest) }
                'Q' { [void]$forcedInc.Add($rest) }
                'F' { [void]$files.Add(($rest -split '\|')) }
            }
        }

        if (-not $projDir) { $projDir = Split-Path -Parent $proj }

        # --- normalize include dirs ---------------------------------------
        $sysAbs = New-Object System.Collections.ArrayList
        foreach ($d in $sysInc) {
            $r = Resolve-Dir -Dir $d -BaseDir $projDir
            if (-not $r) { continue }
            if (Test-Path -LiteralPath $r) {
                if (-not $sysAbs.Contains($r)) { [void]$sysAbs.Add($r) }
            } else {
                [void]$missingDirs.Add($r)
            }
        }

        $incAbs = New-Object System.Collections.ArrayList
        foreach ($d in $addInc) {
            $r = Resolve-Dir -Dir $d -BaseDir $projDir
            if (-not $r) { continue }
            if (Test-Path -LiteralPath $r) {
                if (-not $incAbs.Contains($r)) { [void]$incAbs.Add($r) }
            } else {
                [void]$missingDirs.Add($r)
            }
        }

        # --- assemble the flags shared by every file in this project -------
        $msCompat = Get-MsCompatibilityVersion -SystemIncludeDirs $sysAbs

        $common = New-Object System.Collections.ArrayList
        [void]$common.Add('clang-cl.exe')
        [void]$common.Add('--driver-mode=cl')

        switch ($Platform) {
            'ARM64' { [void]$common.Add('--target=aarch64-pc-windows-msvc') }
            'x64'   { [void]$common.Add('--target=x86_64-pc-windows-msvc') }
            'Win32' { [void]$common.Add('--target=i686-pc-windows-msvc') }
            'ARM'   { [void]$common.Add('--target=arm-pc-windows-msvc') }
        }
        if ($msCompat) { [void]$common.Add('-fms-compatibility-version=' + $msCompat) }

        foreach ($d in $defines) {
            $t = $d.Trim()
            if ($t) { [void]$common.Add('/D' + $t) }
        }
        foreach ($d in $incAbs)  { [void]$common.Add('/I' + $d) }
        # /imsvc marks the toolset + SDK headers as *system* headers so clangd does not
        # spray diagnostics from inside <windows.h> / afxwin.h.
        foreach ($d in $sysAbs)  { [void]$common.Add('/imsvc'); [void]$common.Add($d) }
        foreach ($f in $forcedInc) {
            $t = $f.Trim()
            if ($t) { [void]$common.Add('/FI' + $t) }
        }

        # --- one entry per source file -------------------------------------
        foreach ($f in $files) {
            $path     = $f[0]
            $pchMode  = ''
            $pchFile  = ''
            $std      = ''
            $excluded = ''
            $compAs   = ''
            $rtLib    = ''
            $eh       = ''
            $rtti     = ''
            if ($f.Count -gt 1) { $pchMode  = $f[1] }
            if ($f.Count -gt 2) { $pchFile  = $f[2] }
            if ($f.Count -gt 3) { $std      = $f[3] }
            if ($f.Count -gt 4) { $excluded = $f[4] }
            if ($f.Count -gt 5) { $compAs   = $f[5] }
            if ($f.Count -gt 6) { $rtLib    = $f[6] }
            if ($f.Count -gt 7) { $eh       = $f[7] }
            if ($f.Count -gt 8) { $rtti     = $f[8] }

            if ($excluded -eq 'true') { continue }
            if (-not (Test-Path -LiteralPath $path)) {
                Write-Warning "Source listed in project but missing on disk: $path"
                continue
            }

            $cmdArgs = New-Object System.Collections.ArrayList
            foreach ($c in $common) { [void]$cmdArgs.Add($c) }

            if ($compAs -eq 'CompileAsC') {
                [void]$cmdArgs.Add('/TC')
            } else {
                [void]$cmdArgs.Add('/TP')
                [void]$cmdArgs.Add((Convert-LanguageStandard -Std $std))
            }

            # /MDd matters beyond codegen: afxver_.h #errors out when _AFXDLL is defined
            # without a DLL runtime, which would break every MFC translation unit.
            switch ($rtLib) {
                'MultiThreaded'          { [void]$cmdArgs.Add('/MT')  }
                'MultiThreadedDebug'     { [void]$cmdArgs.Add('/MTd') }
                'MultiThreadedDLL'       { [void]$cmdArgs.Add('/MD')  }
                'MultiThreadedDebugDLL'  { [void]$cmdArgs.Add('/MDd') }
            }
            switch ($eh) {
                'Sync'       { [void]$cmdArgs.Add('/EHsc') }
                'Async'      { [void]$cmdArgs.Add('/EHa')  }
                'SyncCThrow' { [void]$cmdArgs.Add('/EHs')  }
                default      { [void]$cmdArgs.Add('/EHsc') }
            }
            if ($rtti -eq 'false') { [void]$cmdArgs.Add('/GR-') } else { [void]$cmdArgs.Add('/GR') }

            # Precompiled headers: clangd does not consume MSVC .pch files, so /Yu and /Yc
            # are dropped.  What matters is that the PCH header's contents are visible.
            # Sources compiled with /Yu normally #include it themselves; if one does not,
            # force-include it so clangd sees the same translation unit MSVC would.
            if ($pchFile -and ($pchMode -eq 'Use' -or $pchMode -eq 'Create')) {
                if (-not (Test-SourceIncludesPch -SourceFile $path -PchHeader $pchFile)) {
                    [void]$cmdArgs.Add('/FI' + $pchFile)
                }
            }

            [void]$cmdArgs.Add($path)

            $entry = New-Object psobject -Property ([ordered]@{
                directory = $projDir
                file      = $path
                arguments = @($cmdArgs.ToArray())
            })
            [void]$entries.Add($entry)
        }

        Write-Host ("    {0}: {1} source file(s), {2} include dir(s)" -f $projName, $files.Count, ($incAbs.Count + $sysAbs.Count))
    }
}
finally {
    Remove-Item -LiteralPath $tmpDir -Recurse -Force -ErrorAction SilentlyContinue
}

if ($entries.Count -eq 0) { throw 'No compile entries were produced.' }

# ---------------------------------------------------------------------------
# Emit JSON
# ---------------------------------------------------------------------------

$json = ConvertTo-Json -InputObject @($entries.ToArray()) -Depth 8
# PowerShell 5.1 renders a single-element array as a bare object; force array form.
if ($json -notmatch '^\s*\[') { $json = "[`r`n" + $json + "`r`n]" }

Set-Content -LiteralPath $OutFile -Value $json -Encoding UTF8

Write-Host ''
Write-Host ("Wrote {0}" -f $OutFile)
Write-Host ("Entries: {0}" -f $entries.Count)

if ($missingDirs.Count -gt 0) {
    $uniqMissing = $missingDirs | Sort-Object -Unique
    Write-Host ''
    Write-Warning ("{0} configured include director(ies) do not exist and were omitted:" -f $uniqMissing.Count)
    foreach ($d in $uniqMissing) { Write-Warning "  $d" }
}
if ($failedProj.Count -gt 0) {
    Write-Host ''
    Write-Warning ('Projects skipped due to MSBuild evaluation failure: ' + ($failedProj -join ', '))
}
