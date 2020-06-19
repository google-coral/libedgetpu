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

type bazel\WORKSPACE bazel\WORKSPACE.windows > WORKSPACE 2>NUL

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

set BAZEL_INFO_FLAGS=^
--experimental_repo_remote_exec
for /f %%i in ('bazel info %BAZEL_INFO_FLAGS% output_path') do set "BAZEL_OUTPUT_PATH=%%i"
set BAZEL_OUTPUT_PATH=%BAZEL_OUTPUT_PATH:/=\%
set BAZEL_OUT_DIR=%BAZEL_OUTPUT_PATH%\%CPU%-%COMPILATION_MODE%\bin


set TARGET=//tflite/public:edgetpu_direct_usb.dll
set BAZEL_VS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools
set BAZEL_VC=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC
set BAZEL_BUILD_FLAGS= ^
--experimental_repo_remote_exec ^
--compilation_mode %COMPILATION_MODE% ^
--define darwinn_portable=1 ^
--copt=/DSTRIP_LOG=1 ^
--copt=/DABSL_FLAGS_STRIP_NAMES ^
--copt=/D_HAS_DEPRECATED_RESULT_OF ^
--copt=/D_HAS_DEPRECATED_ADAPTOR_TYPEDEFS ^
--copt=/GR- ^
--copt=/DWIN32_LEAN_AND_MEAN ^
--copt=/D_WINSOCKAPI_ ^
--copt=/std:c++latest

call "%BAZEL_VC%\Auxiliary\Build\vcvars64.bat"

bazel build %BAZEL_BUILD_FLAGS% %LEFTOVER_ARGS% %TARGET%
md %OUT_DIR%\direct\%CPU%
copy %BAZEL_OUT_DIR%\tflite\public\edgetpu_direct_usb.dll ^
     %OUT_DIR%\direct\%CPU%\
python %~dp0\rename_library.py ^
  --input_dll %OUT_DIR%\direct\%CPU%\edgetpu_direct_usb.dll ^
  --output_dll %OUT_DIR%\direct\%CPU%\edgetpu.dll

set BAZEL_BUILD_FLAGS=%BAZEL_BUILD_FLAGS% --copt=/DTHROTTLE_EDGE_TPU
bazel build %BAZEL_BUILD_FLAGS% %LEFTOVER_ARGS% %TARGET%
md %OUT_DIR%\throttled\%CPU%
copy %BAZEL_OUT_DIR%\tflite\public\edgetpu_direct_usb.dll ^
     %OUT_DIR%\throttled\%CPU%\
python %~dp0\rename_library.py ^
  --input_dll %OUT_DIR%\throttled\%CPU%\edgetpu_direct_usb.dll ^
  --output_dll %OUT_DIR%\throttled\%CPU%\edgetpu.dll