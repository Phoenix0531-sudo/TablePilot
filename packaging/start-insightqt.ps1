param(
    [string]$ProjectRoot = (Resolve-Path "$PSScriptRoot\..").Path,
    [switch]$SkipService
)

$ErrorActionPreference = "Stop"

if (-not $SkipService) {
    Push-Location $ProjectRoot
    try {
        docker compose up -d | Out-Host
    }
    finally {
        Pop-Location
    }
}

$exe = Join-Path $ProjectRoot "release\InsightQt AI Workbench.exe"
if (-not (Test-Path $exe)) {
    $exe = Join-Path $ProjectRoot "release\Statistical_Analysis.exe"
}

if (-not (Test-Path $exe)) {
    throw "Could not find the InsightQt executable under $ProjectRoot\release."
}

Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe)
