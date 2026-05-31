param(
    [string]$QtBin = "E:\1_Code\QT\6.11.1\mingw_64\bin",
    [string]$MingwBin = "E:\1_Code\QT\Tools\mingw1310_64\bin",
    [string]$Configuration = "release"
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path "$PSScriptRoot\..").Path
$Project = Join-Path $Root "Statistical_Analysis\Statistical_Analysis.pro"
$BuildDir = Join-Path $Root ".release-build"
$PackageDir = Join-Path $Root "dist\InsightQt-AI-Workbench"

if (-not (Test-Path (Join-Path $QtBin "qmake.exe"))) {
    throw "qmake.exe was not found under $QtBin"
}
if (-not (Test-Path (Join-Path $MingwBin "mingw32-make.exe"))) {
    throw "mingw32-make.exe was not found under $MingwBin"
}

Remove-Item -LiteralPath $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $PackageDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Path $PackageDir | Out-Null

$env:Path = "$QtBin;$MingwBin;$env:Path"

Push-Location $BuildDir
try {
    & (Join-Path $QtBin "qmake.exe") $Project "CONFIG+=$Configuration"
    & (Join-Path $MingwBin "mingw32-make.exe") "-j$([Environment]::ProcessorCount)"
}
finally {
    Pop-Location
}

$exe = Get-ChildItem -Path $BuildDir -Recurse -Filter "Statistical_Analysis.exe" | Select-Object -First 1
if (-not $exe) {
    throw "Build completed but Statistical_Analysis.exe was not found."
}

$ReleaseDir = Join-Path $PackageDir "release"
New-Item -ItemType Directory -Path $ReleaseDir | Out-Null
Copy-Item -LiteralPath $exe.FullName -Destination (Join-Path $ReleaseDir "InsightQt AI Workbench.exe")

& (Join-Path $QtBin "windeployqt.exe") (Join-Path $ReleaseDir "InsightQt AI Workbench.exe")

Copy-Item -LiteralPath (Join-Path $Root "docker-compose.yml") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $Root "README.md") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "start-insightqt.ps1") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "README_RELEASE.md") -Destination $PackageDir

$ServiceDir = Join-Path $PackageDir "analysis_service"
New-Item -ItemType Directory -Path $ServiceDir | Out-Null
Copy-Item -LiteralPath (Join-Path $Root "analysis_service\Dockerfile") -Destination $ServiceDir
Copy-Item -LiteralPath (Join-Path $Root "analysis_service\requirements.txt") -Destination $ServiceDir
Copy-Item -LiteralPath (Join-Path $Root "analysis_service\app") -Destination $ServiceDir -Recurse
Copy-Item -LiteralPath (Join-Path $Root "samples") -Destination $PackageDir -Recurse

Compress-Archive -LiteralPath $PackageDir -DestinationPath (Join-Path $Root "dist\InsightQt-AI-Workbench.zip") -Force
Write-Host "Release package created: $Root\dist\InsightQt-AI-Workbench.zip"
