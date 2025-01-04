# Ensure the window stays open
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
Write-Host "Starting build script..." -ForegroundColor Cyan
Start-Sleep -Seconds 1

try {
    # Ensure we stop on any errors
    $ErrorActionPreference = "Stop"

    Write-Host "USDTweak Automated Build Script" -ForegroundColor Cyan
    Write-Host "================================" -ForegroundColor Cyan

    # Function to check if a command exists
    function Test-Command($cmdname) {
        return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
    }

    # Check prerequisites
    $prerequisites = @{
        "git" = "Git is required. Download from: https://git-scm.com/download/win"
        "cmake" = "CMake is required. Download from: https://cmake.org/download/"
    }

    foreach ($cmd in $prerequisites.Keys) {
        if (-not (Test-Command $cmd)) {
            Write-Host "Error: $($prerequisites[$cmd])" -ForegroundColor Red
            pause
            exit 1
        }
    }

    # Create build directory
    $buildDir = Join-Path $PSScriptRoot "build"
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

    # Define USD directory path
    $usdDir = Join-Path $buildDir "usd"

    # Check if USD directory exists first
    $usdPresent = Test-Path $usdDir
    if ($usdPresent) {
        # Only verify files if directory exists
        $usdValidationPaths = @(
            (Join-Path $usdDir "pxrConfig.cmake"),  # Corrected path for pxrConfig.cmake
            (Join-Path $usdDir "lib\cmake\MaterialX\MaterialXConfig.cmake"),  # Corrected path for MaterialXConfig.cmake
            (Join-Path $usdDir "python\python.exe")
        )

        $missingFiles = @()
        foreach ($path in $usdValidationPaths) {
            if (-not (Test-Path $path)) {
                $missingFiles += $path
            }
        }

        if ($missingFiles.Count -gt 0) {
            Write-Host "Warning: USD directory exists but missing required files:" -ForegroundColor Yellow
            foreach ($file in $missingFiles) {
                Write-Host "  - $file" -ForegroundColor Yellow
            }
            # Removed the deletion of the USD directory
            # $usdPresent = $false
            # Remove-Item -Path $usdDir -Recurse -Force
        }
    }

    if (-not $usdPresent) {
        # Create USD directory only if we need to download
        New-Item -ItemType Directory -Force -Path $usdDir | Out-Null
        
        # Download USD from NVIDIA
        $usdUrl = "https://developer.nvidia.com/downloads/USD/usd_binaries/24.11/usd.py311.windows-x86_64.usdview.release-0.24.11-4d81dd85.zip"
        $usdZip = Join-Path $buildDir "usd.zip"

        Write-Host "`nDownloading USD from NVIDIA..." -ForegroundColor Yellow
        try {
            $ProgressPreference = 'SilentlyContinue'
            Invoke-WebRequest -Uri $usdUrl -OutFile $usdZip
            $ProgressPreference = 'Continue'
        } catch {
            Write-Host "Error: Failed to download USD. Please check your internet connection or download manually from:" -ForegroundColor Red
            Write-Host $usdUrl -ForegroundColor Yellow
            pause
            exit 1
        }

        # Extract USD - Try to use 7-Zip if available
        Write-Host "Extracting USD..." -ForegroundColor Yellow
        $sevenZip = "${env:ProgramFiles}\7-Zip\7z.exe"
        if (Test-Path $sevenZip) {
            Write-Host "Using 7-Zip for extraction..." -ForegroundColor Green
            $extractProcess = Start-Process -FilePath $sevenZip -ArgumentList "x", "`"$usdZip`"", "-o`"$usdDir`"", "-y" -NoNewWindow -Wait -PassThru
            if ($extractProcess.ExitCode -ne 0) {
                Write-Host "7-Zip extraction failed, falling back to PowerShell extraction..." -ForegroundColor Yellow
                Expand-Archive -Path $usdZip -DestinationPath $usdDir -Force
            }
        } else {
            Write-Host "7-Zip not found, using PowerShell extraction..." -ForegroundColor Yellow
            Expand-Archive -Path $usdZip -DestinationPath $usdDir -Force
        }

        # Remove the zip file after extraction
        Remove-Item -Path $usdZip -Force

        # Diagnostic: Check what's in the extracted directory
        Write-Host "Checking USD directory structure..." -ForegroundColor Yellow
        Write-Host "Contents of ${usdDir}:" -ForegroundColor Yellow
        Get-ChildItem -Path ${usdDir} | ForEach-Object {
            Write-Host "  - $($_.Name)"
        }

        # Search for pxrConfig.cmake
        Write-Host "`nSearching for pxrConfig.cmake..." -ForegroundColor Yellow
        $pxrConfigFiles = Get-ChildItem -Path $usdDir -Recurse -Filter "pxrConfig.cmake"
        if ($pxrConfigFiles) {
            foreach ($file in $pxrConfigFiles) {
                Write-Host "  Found at: $($file.FullName)"
            }
        } else {
            Write-Host "  No pxrConfig.cmake found!"
        }
    } else {
        Write-Host "`nUsing existing USD installation in build/usd" -ForegroundColor Green
    }

    # Move to build directory
    Push-Location $buildDir

    # Configure with CMake
    Write-Host "`nConfiguring with CMake..." -ForegroundColor Yellow
    $cmakeArgs = @(
        "-G`"Visual Studio 17 2022`"",
        "-A", "x64",
        "-Dpxr_DIR=`"$usdDir`"",
        "-DMaterialX_DIR=`"$usdDir\lib\cmake\MaterialX`"",
        "-DPython3_EXECUTABLE=`"$usdDir\python\python.exe`"",
        "-DPython3_LIBRARY=`"$usdDir\python\libs\python311.lib`"",
        "-DPython3_INCLUDE_DIR=`"$usdDir\python\include`"",
        "-DPython3_VERSION=`"3.11`"",
        "-DImath_DIR=`"$usdDir\lib\cmake\Imath`"",
        ".."
    )

    $cmakeProcess = Start-Process cmake -ArgumentList ($cmakeArgs -join ' ') -NoNewWindow -Wait -PassThru
    if ($cmakeProcess.ExitCode -ne 0) {
        Write-Host "Error: CMake configuration failed" -ForegroundColor Red
        pause
        exit 1
    }

    # Fix the osdGPU.lib and osdCPU.lib issue in pxrTargets.cmake
    $pxrTargetsPath = Join-Path $usdDir "cmake\pxrTargets.cmake"
    if (Test-Path $pxrTargetsPath) {
        $content = Get-Content $pxrTargetsPath
        $content = $content -replace '\$\{_IMPORT_PREFIX\}/lib/osdGPU\.lib;', ''
        $content = $content -replace '\$\{_IMPORT_PREFIX\}/lib/osdCPU\.lib;', ''
        Set-Content $pxrTargetsPath $content
    }

    # Build the project
    Write-Host "`nBuilding project..." -ForegroundColor Yellow
    $buildProcess = Start-Process cmake -ArgumentList "--build", ".", "--config", "RelWithDebInfo", "--parallel" -NoNewWindow -Wait -PassThru
    if ($buildProcess.ExitCode -ne 0) {
        Write-Host "Error: Build failed" -ForegroundColor Red
        pause
        exit 1
    }

    # Install
    Write-Host "`nInstalling..." -ForegroundColor Yellow
    $installProcess = Start-Process cmake -ArgumentList "--install", ".", "--prefix", "./install", "--config", "RelWithDebInfo" -NoNewWindow -Wait -PassThru
    if ($installProcess.ExitCode -ne 0) {
        Write-Host "Error: Installation failed" -ForegroundColor Red
        pause
        exit 1
    }

    # Return to original directory
    Pop-Location

    Write-Host "`nBuild completed successfully!" -ForegroundColor Green
    Write-Host "The built files can be found in: $buildDir\install" -ForegroundColor Green
} catch {
    Write-Host "`nError occurred:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host "`nStack trace:" -ForegroundColor Red
    Write-Host $_.ScriptStackTrace -ForegroundColor Red
    pause
    exit 1
} finally {
    # Make sure we're back in the original directory
    if ($buildDir -and (Get-Location).Path -eq $buildDir) {
        Pop-Location
    }
}

# Move pause outside of try-catch so it always executes
Write-Host "`nPress any key to exit..." -ForegroundColor Cyan
pause 