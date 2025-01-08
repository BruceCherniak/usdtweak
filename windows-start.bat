@echo off
REM Set the USDROOT environment variable to the relative path
set USDROOT=.\build\usd

REM Set the PYTHON_DIR to the Python directory in the USD build
set PYTHON_DIR=%USDROOT%\python

REM Update the PATH variable to include USD binaries, libraries, and Python
set PATH=%USDROOT%\bin;%USDROOT%\lib;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PATH%

REM Set the PYTHONPATH to include the USD Python libraries
set PYTHONPATH=%USDROOT%\lib\python;%PYTHONPATH%

REM Start usdtweak with any arguments passed to the batch file
.\build\RelWithDebInfo\usdtweak.exe %*