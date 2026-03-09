[CmdletBinding()]
param(
    [string]$Preset = "x64-debug",
    [string]$SourceDir = $PSScriptRoot,
    [string]$BuildDir = "",
    [string]$Target = "eds_server_new_mediasoup_app",
    [ValidateSet("ON", "OFF")]
    [string]$BuildServer = "OFF",
    [ValidateSet("ON", "OFF")]
    [string]$BuildCli = "OFF",
    [ValidateSet("ON", "OFF")]
    [string]$BuildServerNew = "ON",
    [int]$Jobs = 8,
    [switch]$ConfigureOnly,
    [switch]$BuildOnly,
    [switch]$Run,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $PSScriptRoot ("out/build/{0}-mediasoup" -f $Preset)
}

function Resolve-VsWherePath {
    $defaultPath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $defaultPath) {
        return $defaultPath
    }

    $found = Get-Command vswhere.exe -ErrorAction SilentlyContinue
    if ($null -ne $found) {
        return $found.Source
    }

    throw "vswhere.exe not found. Install Visual Studio Installer components, or add vswhere.exe to PATH."
}

function Resolve-VcpkgRoot {

    if ($env:VCPKG_ROOT -and (Test-Path $env:VCPKG_ROOT)) {
        $resolved = (Resolve-Path $env:VCPKG_ROOT).Path
        $env:VCPKG_ROOT = $resolved
        return $resolved
    }

    $candidates = @(
        "D:\vcpkg",
        "C:\vcpkg",
        (Join-Path $PSScriptRoot "vcpkg"),
        (Join-Path $env:USERPROFILE "vcpkg")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            $resolved = (Resolve-Path $candidate).Path
            $env:VCPKG_ROOT = $resolved
            return $resolved
        }
    }

    throw "VCPKG_ROOT is not set and no fallback vcpkg path was found. Set VCPKG_ROOT to your vcpkg directory."
}

function Import-MsvcEnvironment {
    param(
        [Parameter(Mandatory = $true)]
        [string]$VsDevCmdPath
    )

    $setOutput = & cmd.exe /d /c "`"$VsDevCmdPath`" -arch=x64 -host_arch=x64 >nul && set"
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to initialize MSVC environment via VsDevCmd.bat."
    }

    foreach ($line in $setOutput) {
        if ($line -match "^([^=]+)=(.*)$") {
            $name = $matches[1]
            $value = $matches[2]
            Set-Item -Path "Env:$name" -Value $value
        }
    }
}

function Invoke-External {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Exe,
        [Parameter(Mandatory = $true)]
        [string[]]$Args
    )

    Write-Host ">> $Exe $($Args -join ' ')"
    & $Exe @Args
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed: $Exe (exit code $LASTEXITCODE)"
    }
}

$SourceDir = [System.IO.Path]::GetFullPath($SourceDir)
$BuildDir = [System.IO.Path]::GetFullPath($BuildDir)

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host ">> Removing build directory: $BuildDir"
    Remove-Item -Path $BuildDir -Recurse -Force
}


$resolvedVcpkgRoot = Resolve-VcpkgRoot
Write-Host "Using VCPKG_ROOT: $resolvedVcpkgRoot"
$msvcAlreadyReady = $null -ne (Get-Command cl.exe -ErrorAction SilentlyContinue)
if ($msvcAlreadyReady) {
    Write-Host "MSVC environment already initialized, skipping VsDevCmd."
} else {
    $vswherePath = Resolve-VsWherePath
    $vsInstallPath = (& $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath).Trim()
    if ([string]::IsNullOrWhiteSpace($vsInstallPath)) {
        throw "No Visual Studio installation with C++ tools was found."
    }
    Write-Host "Using Visual Studio: $vsInstallPath"
    $vsDevCmd = Join-Path $vsInstallPath "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
        throw "VsDevCmd.bat not found at expected path: $vsDevCmd"
    }
    Import-MsvcEnvironment -VsDevCmdPath $vsDevCmd
}
$env:VCPKG_ROOT = $resolvedVcpkgRoot
Write-Host "Re-applied VCPKG_ROOT after VsDevCmd: $env:VCPKG_ROOT"

$vcpkgTriplet = ""
$isWindowsHost = $false
if (Get-Variable -Name IsWindows -Scope Global -ErrorAction SilentlyContinue) {
    $isWindowsHost = [bool]$IsWindows
} elseif ($env:OS -eq "Windows_NT") {
    $isWindowsHost = $true
}
if ($env:VCPKG_TARGET_TRIPLET) {
    $vcpkgTriplet = $env:VCPKG_TARGET_TRIPLET
} elseif ($isWindowsHost) {
    if ($Preset -match "x86") {
        $vcpkgTriplet = "x86-windows"
    } else {
        $vcpkgTriplet = "x64-windows"
    }
}
if (-not [string]::IsNullOrWhiteSpace($vcpkgTriplet)) {
    Write-Host "Using VCPKG_TARGET_TRIPLET: $vcpkgTriplet"
}

if (-not $BuildOnly) {
    $configureArgs = @(
        "--preset", $Preset,
        "-S", $SourceDir,
        "-B", $BuildDir,
        "-DEDUSPACE_BUILD_SERVER=$BuildServer",
        "-DEDUSPACE_BUILD_CLI=$BuildCli",
        "-DEDUSPACE_BUILD_SERVER_NEW=$BuildServerNew"
    )
    if ($vcpkgTriplet) {
        $configureArgs += "-DVCPKG_TARGET_TRIPLET=$vcpkgTriplet"
    }
    Invoke-External -Exe "cmake" -Args $configureArgs
}

if (-not $ConfigureOnly) {
    $buildArgs = @(
        "--build", $BuildDir,
        "--target", $Target
    )
    if ($Jobs -gt 0) {
        $buildArgs += @("-j", "$Jobs")
    }

    Invoke-External -Exe "cmake" -Args $buildArgs
}

if ($Run) {
    $exeCandidates = @(
        (Join-Path $BuildDir "EDS_serverNew\$Target.exe"),
        (Join-Path $BuildDir "$Target.exe")
    )
    $exePath = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ([string]::IsNullOrWhiteSpace($exePath)) {
        throw "Built executable not found. Checked: $($exeCandidates -join ', ')"
    }

    Write-Host ">> $exePath"
    & $exePath
    if ($LASTEXITCODE -ne 0) {
        throw "Executable returned non-zero exit code: $LASTEXITCODE"
    }
}

Write-Host "Build script completed successfully."
