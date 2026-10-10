param(
    [string]$TestRegex = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = Join-Path $repoRoot "build/native"

function Invoke-Checked {
    param([string]$Executable, [string[]]$Arguments)
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Executable failed with exit code $LASTEXITCODE."
    }
}

try {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCommand) {
        $cmakePath = $cmakeCommand.Source
    } else {
        $vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio/Installer/vswhere.exe"
        if (-not (Test-Path -LiteralPath $vswherePath)) {
            throw "CMake is missing. Install Visual Studio 2022 C++ Build Tools with CMake support."
        }
        $vsPath = & $vswherePath -latest -version '[17.0,18.0)' -products '*' `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($LASTEXITCODE -ne 0 -or -not $vsPath) {
            throw "Visual Studio 2022 C++ Build Tools were not found."
        }
        $cmakePath = Join-Path $vsPath "Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    }
    $ctestPath = Join-Path (Split-Path -Parent $cmakePath) "ctest.exe"
    if (-not (Test-Path -LiteralPath $cmakePath) -or -not (Test-Path -LiteralPath $ctestPath)) {
        throw "Both CMake and CTest are required."
    }
    Invoke-Checked $cmakePath @('-S', (Join-Path $repoRoot 'native'), '-B', $buildPath,
        '-G', 'Visual Studio 17 2022', '-A', 'x64', '-DBUILD_TESTING=ON')
    Invoke-Checked $cmakePath @('--build', $buildPath, '--config', 'Release', '--', '/nodeReuse:false')
    $testArguments = @('--test-dir', $buildPath, '-C', 'Release', '--output-on-failure', '--no-tests=error')
    if ($TestRegex) { $testArguments += @('-R', $TestRegex) }
    Invoke-Checked $ctestPath $testArguments
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
exit 0
