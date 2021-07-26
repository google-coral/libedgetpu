:: Copyright 2019 Google LLC
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     https://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.
@echo off
setlocal enabledelayedexpansion

rem Check for Admin privileges
fsutil dirty query %systemdrive% >NUL
if not %ERRORLEVEL% == 0 (
    powershell Start-Process -FilePath '%0' -ArgumentList "elevated" -verb runas
    exit /b
)

if exist "%~dp0\libedgetpu" (
    rem Running with the script in the root
    set ROOTDIR=%~dp0
) else (
    rem Running with the script in scripts\windows
    set ROOTDIR=%~dp0\..\..\
)

cd /d "%ROOTDIR%"
set ROOTDIR=%CD%

echo Deleting edgetpu and libusb from System32
del c:\windows\system32\edgetpu.dll
del c:\windows\system32\libusb-1.0.dll

echo Unistalling WinUSB drivers
for /f "tokens=3" %%a in ('pnputil /enum-devices /class {88bae032-5a81-49f0-bc3d-a4ff138216d6} ^| findstr /b "Driver Name:"') do (
    set infs=%%a !infs!
)
set infs=%infs:---=inf%
echo %infs%
for %%a in (%infs%) do (
    echo %%a
    pnputil /delete-driver %%a /uninstall
)

echo Uninstalling UsbDk
start /wait msiexec /x "%ROOTDIR%\third_party\usbdk\UsbDk_*.msi" /quiet /qb! /norestart

echo Uninstall complete!
rem If %1 is elevated, this means we were re-invoked to gain Administrator.
rem In this case, we're in a new window, so call pause to allow the user to view output.
if "%1" == "elevated" pause
