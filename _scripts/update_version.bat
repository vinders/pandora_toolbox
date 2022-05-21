@echo off
setlocal EnableDelayedExpansion

:: Identify project directory
set filename="Version.cmake"
if not exist CMakeLists.txt cd ..
if not exist CMakeLists.txt (echo "project directory not found in current directory nor in parent directory!" && exit /B 1)

set MAJOR_KEYWORD=major-
set MINOR_KEYWORD=feat-
set PATCH_KEYWORD=fix-

:: Get major version - count breaking changes
for /f "delims=" %%a in ('git.exe shortlog -sn --grep="^%MAJOR_KEYWORD%" -i') do echo %%a >> _tmp_MAJOR
set /a MAJOR=0
if exist _tmp_MAJOR (
  for /f %%a in (_tmp_MAJOR) do (
    set /a MAJOR+=%%a
  )
  del /f _tmp_MAJOR

  for /f "delims=" %%a in ('git.exe log --grep="^%MAJOR_KEYWORD%" -i -1') do (
    set MAJOR_REVLINE=%%a
    goto end_major
  )
)
:end_major
if "%MAJOR_REVLINE%" == "" (
  echo "No major commit found."
) else (
  for %%s in (%MAJOR_REVLINE%) do set MAJOR_REV=%%s
)

:: Get minor version - count feature changes since last break
set /a MINOR=0
if "%MAJOR_REV%" == "" (
  for /f "delims=" %%a in ('git.exe shortlog -sn --grep="^%MINOR_KEYWORD%" -i') do echo %%a >> _tmp_MINOR
) else (
  for /f "delims=" %%a in ('git.exe shortlog -sn --grep="^%MINOR_KEYWORD%" -i %MAJOR_REV%..HEAD') do echo %%a >> _tmp_MINOR
)
set MINOR=0
if exist _tmp_MINOR (
  for /f %%a in (_tmp_MINOR) do (
    set /a MINOR+=%%a
  )
  del /f _tmp_MINOR
  
  for /f "delims=" %%a in ('git.exe log --grep="^%MINOR_KEYWORD%" -i -1') do (
    set MINOR_REVLINE=%%a
    goto end_minor
  )
)
:end_minor
if "%MINOR_REVLINE%" == "" (
  echo "No minor commit found."
) else (
  for %%s in (%MINOR_REVLINE%) do set MINOR_REV=%%s
)
if "%MINOR_REV%" == "" set MINOR_REV=%MAJOR_REV%

:: Get patch version - count fixes since last feature
set /a PATCH=0
if "%MINOR_REV%" == "" (
  for /f "delims=" %%a in ('git.exe shortlog -sn --grep="^%PATCH_KEYWORD%" -i') do echo %%a >> _tmp_PATCH
) else (
  for /f "delims=" %%a in ('git.exe shortlog -sn --grep="^%PATCH_KEYWORD%" -i %MINOR_REV%..HEAD') do echo %%a >> _tmp_PATCH
)
set PATCH=0
if exist _tmp_PATCH (
  for /f %%a in (_tmp_PATCH) do (
    set /a PATCH+=%%a
  )
  del /f _tmp_PATCH
)

:: Total number of commits
call git rev-list -n 999999999 --count HEAD >> _tmp_TOTAL
set /p TOTAL=<_tmp_TOTAL
del /f _tmp_TOTAL

:: Update version file
set VERSION_STRING=%MAJOR%.%MINOR%.%PATCH%.%TOTAL%

if exist "./%filename%" del /f %filename%
echo # %VERSION_STRING% >> %filename%
echo set(CWORK_BUILD_VERSION %VERSION_STRING%) >> %filename%
echo %VERSION_STRING%

exit /B %errorlevel%
