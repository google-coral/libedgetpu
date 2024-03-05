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
    set ROOTDIR=%~dp0\..\
)

cd /d "%ROOTDIR%"
set ROOTDIR=%CD%

echo Warning: During normal operation, the Edge TPU Accelerator may heat up,
echo depending on the computation workloads and operating frequency. Touching the
echo metal part of the device after it has been operating for an extended period of
echo time may lead to discomfort and/or skin burns. As such, when running at the
echo default operating frequency, the device is intended to safely operate at an
echo ambient temperature of 35C or less. Or when running at the maximum operating
echo frequency, it should be operated at an ambient temperature of 25C or less.
echo.
echo Google does not accept any responsibility for any loss or damage if the device
echo is operated outside of the recommended ambient temperature range.
echo ................................................................................
set /p USE_MAX_FREQ="Would you like to enable the maximum operating frequency for the USB Accelerator? Y/N "
if "%USE_MAX_FREQ%" == "y" set FREQ_DIR=direct
if "%USE_MAX_FREQ%" == "Y" set FREQ_DIR=direct
if not defined FREQ_DIR set FREQ_DIR=throttled

echo Installing UsbDk
start /wait msiexec /i "%ROOTDIR%\third_party\usbdk\UsbDk_*.msi" /quiet /qb! /norestart

echo Installing Windows drivers
if exist %SystemRoot%\System32\pnputil.exe (
    set "SystemPath=%SystemRoot%\System32"
) else if exist %SystemRoot%\Sysnative\pnputil.exe (
    set "SystemPath=%SystemRoot%\Sysnative"
) else (
    echo ERROR: Cannot find pnputil.exe to install the driver.
    echo/
    pause
    goto :EOF
)
%SystemPath%\pnputil.exe /add-driver "%ROOTDIR%\third_party\coral_accelerator_windows\*.inf" /install

echo Installing performance counters
lodctr /M:"%ROOTDIR%\third_party\coral_accelerator_windows\coral.man"

echo Copying edgetpu and libusb to System32
copy "%ROOTDIR%\libedgetpu\%FREQ_DIR%\x64_windows\edgetpu.dll" c:\windows\system32
copy "%ROOTDIR%\third_party\libusb_win\libusb-1.0.dll" c:\windows\system32

echo Install complete!
rem If %1 is elevated, this means we were re-invoked to gain Administrator.
rem In this case, we're in a new window, so call pause to allow the user to view output.
if "%1" == "elevated" pause
