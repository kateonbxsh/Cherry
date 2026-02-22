param(
    [string]$BuildDir = "build-wasm",
    [string]$OutDir = "web/public/wasm",
    [string]$Config = "Release",
    [switch]$UsePreset,
    [switch]$Reconfigure,
    [switch]$UseSccache
)

$ErrorActionPreference = "Stop"

function Require-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Required command '$Name' was not found in PATH."
    }
}

Require-Command "cmake"

if ($UsePreset) {
    $BuildDir = "build-wasm"
}

$cachePath = Join-Path $BuildDir "CMakeCache.txt"
$needsConfigure = $Reconfigure -or (-not (Test-Path $cachePath))

$sccacheArgs = @()
if ($UseSccache) {
    $sccache = Get-Command "sccache" -ErrorAction SilentlyContinue
    if ($null -eq $sccache) {
        Write-Host "sccache was requested but not found in PATH. Continuing without it."
    } else {
        Write-Host "Using sccache compiler launcher."
        $sccacheArgs = @(
            "-DCMAKE_C_COMPILER_LAUNCHER=sccache",
            "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
        )
    }
}

if ($UsePreset) {
    if ($needsConfigure) {
        Write-Host "Configuring Emscripten build using CMake preset 'wasm'..."
        cmake --preset wasm @sccacheArgs
    } else {
        Write-Host "Skipping configure (cache exists at '$cachePath'). Use -Reconfigure to force."
    }
    Write-Host "Building Cherry (WASM) using CMake build preset 'wasm'..."
    cmake --build --preset wasm
} else {
    Require-Command "emcmake"
    if ($needsConfigure) {
        Write-Host "Configuring Emscripten build..."
        emcmake cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=$Config @sccacheArgs
    } else {
        Write-Host "Skipping configure (cache exists at '$cachePath'). Use -Reconfigure to force."
    }

    Write-Host "Building Cherry (WASM)..."
    cmake --build $BuildDir --config $Config
}

New-Item -ItemType Directory -Path $OutDir -Force | Out-Null

$candidates = @(
    (Join-Path $BuildDir "Cherry.js"),
    (Join-Path (Join-Path $BuildDir $Config) "Cherry.js")
)

$jsPath = $null
foreach ($candidate in $candidates) {
    if (Test-Path $candidate) {
        $jsPath = $candidate
        break
    }
}

if (-not $jsPath) {
    throw "Could not find Cherry.js in '$BuildDir'."
}

$base = [System.IO.Path]::GetDirectoryName($jsPath)
Copy-Item $jsPath (Join-Path $OutDir "Cherry.js") -Force

foreach ($asset in @("Cherry.wasm", "Cherry.data", "Cherry.worker.js")) {
    $src = Join-Path $base $asset
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $OutDir $asset) -Force
    }
}

Write-Host "WASM artifacts copied to '$OutDir'."
