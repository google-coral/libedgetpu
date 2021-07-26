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
echo off

setlocal

if not defined PYTHON set PYTHON=python
set BAZEL_VS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools
set BAZEL_VC=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC
call "%BAZEL_VC%\Auxiliary\Build\vcvars64.bat"

for /f %%i in ('%PYTHON% -c "import sys;print(sys.executable)"') do set PYTHON_BIN_PATH=%%i

set THROTTLED=0
set COMPILATION_MODE=opt
set OUT_DIR=%~dp0\out
set CPU=x64_windows

:PROCESSARGS
set ARG=%1
if defined ARG (
  if "%ARG%"=="/DBG" (
    set COMPILATION_MODE=dbg
  ) else (
    set LEFTOVER_ARGS=%LEFTOVER_ARGS% %ARG%
  )
  shift
  goto PROCESSARGS
)

for /f %%i in ('bazel info output_path') do set "BAZEL_OUTPUT_PATH=%%i"
set BAZEL_OUTPUT_PATH=%BAZEL_OUTPUT_PATH:/=\%
set BAZEL_OUT_DIR=%BAZEL_OUTPUT_PATH%\%CPU%-%COMPILATION_MODE%\bin

:: Supported EdgeTPU devices, one of: usb, pci or all.
set EDGETPU_BUS=all

for /f "tokens=1" %%i in ('bazel query "@libedgetpu_properties//..." ^| findstr /C:"tensorflow_commit" ^| cut -d# -f2') do set "TENSORFLOW_COMMIT=%%i"
set TARGET=//tflite/public:edgetpu_direct_%EDGETPU_BUS%.dll
set BAZEL_BUILD_FLAGS= ^
--compilation_mode %COMPILATION_MODE% ^
--embed_label='TENSORFLOW_COMMIT=%TENSORFLOW_COMMIT%' --stamp

bazel build %BAZEL_BUILD_FLAGS% %LEFTOVER_ARGS% %TARGET%
md %OUT_DIR%\direct\%CPU%
copy %BAZEL_OUT_DIR%\tflite\public\edgetpu_direct_%EDGETPU_BUS%.dll ^
     %OUT_DIR%\direct\%CPU%\
%PYTHON_BIN_PATH% %~dp0\rename_library.py ^
  --input_dll %OUT_DIR%\direct\%CPU%\edgetpu_direct_%EDGETPU_BUS%.dll ^
  --output_dll %OUT_DIR%\direct\%CPU%\edgetpu.dll

set BAZEL_BUILD_FLAGS=%BAZEL_BUILD_FLAGS% --copt=/DTHROTTLE_EDGE_TPU
bazel build %BAZEL_BUILD_FLAGS% %LEFTOVER_ARGS% %TARGET%
md %OUT_DIR%\throttled\%CPU%
copy %BAZEL_OUT_DIR%\tflite\public\edgetpu_direct_%EDGETPU_BUS%.dll ^
     %OUT_DIR%\throttled\%CPU%\
%PYTHON_BIN_PATH% %~dp0\rename_library.py ^
  --input_dll %OUT_DIR%\throttled\%CPU%\edgetpu_direct_%EDGETPU_BUS%.dll ^
  --output_dll %OUT_DIR%\throttled\%CPU%\edgetpu.dll
