@echo off
@setlocal enabledelayedexpansion
pushd %~dp0

rem Get parameters
:PARSE
set arg=%1
if defined arg (
	if "%arg%"=="/?" (
		goto :HELP
		exit /b
	)
	if "%arg%"=="/G" (
		set cmake_gen=%2
		shift
	)
	if "%arg%"=="/A" (
		set cmake_arch=%2
		shift
	)
	shift
	goto :PARSE
)

rem Get CMake generator
if not defined cmake_gen (
	set "default_cmake_gen=Visual Studio 17 2022"
	set /p "cmake_gen=Enter CMake generator (default: !default_cmake_gen!): "
	if not defined cmake_gen (
		set cmake_gen="!default_cmake_gen!"
	) else set cmake_gen="%cmake_gen%"
)

rem Get target architecture
if not defined cmake_arch (
	set "default_cmake_arch=64"
	set /p "cmake_arch=Enter target architecture (32 or 64, default: !default_cmake_arch!): "
	if not defined cmake_arch set cmake_arch=!default_cmake_arch!
)

rem Validate parameters
if not "!cmake_arch!"=="64" ( 
	if not "!cmake_arch!"=="32" (
		echo Invalid target architecture ^(!cmake_arch!^)
		goto :END
	)
)
where /q cmake
if errorlevel 1 (
	echo Could not find CMake on PATH
	goto :END
)

rem Get submodules
set module_path=
set module_url=
set module_branch=
for /F "tokens=*" %%L in (.gitmodules) do (
	echo %%L | >nul findstr /R "path" && (
		for /F "tokens=3" %%i in ('echo %%L') do (
			set module_path=%%i
			set "module_path=!module_path:/=\!"
		)
	)
	echo %%L | >nul findstr /R "url" && (
		for /F "tokens=3" %%i in ('echo %%L') do set module_url=%%i
	)
	echo %%L | >nul findstr /R "branch" && (
		for /F "tokens=3" %%i in ('echo %%L') do set module_branch=%%i
	)
	if not "!module_path!"=="" (
		if not "!module_url!"=="" (
			if not "!module_branch!"=="" (
				rem Clone submodule
				echo Cloning submodule "!module_path!"...
				git clone --filter=blob:none !module_url! !module_path! -b !module_branch! --recursive
				set module_path=
				set module_url=
				set module_branch=
			)
		)
	)
)

rem Run generator scripts for submodules
if exist "vendor\SDL_image\external\Get-GitModules.ps1" (
	echo Running gen script for SDL_image...
	powershell -executionpolicy remotesigned -File "vendor\SDL_image\external\Get-GitModules.ps1"
)

rem Modify base64 CMake policy (temp workaround for outdated behavior)
set target_file="vendor\base64\CMakeLists.txt"
if not exist %target_file% (
	echo Could not find base64 CMakeLists
	goto :MODIFY_BASE64_DONE
)
set temp_file="vendor\base64\CMakeLists.txt.tmp"
if exist %temp_file% (
	goto :MODIFY_BASE64_DONE
)

echo Modifying base64 CMakeLists...
ren %target_file% CMakeLists.txt.tmp
set prev=
for /f "usebackq tokens=*" %%L in (%temp_file%) do (
	echo !prev! | >nul findstr /R "cmake_minimum_required" && (
		echo(cmake_policy^(SET CMP0077 NEW^) >> %target_file%
	)
	echo %%L >> %target_file%
	set prev=%%L
)
:MODIFY_BASE64_DONE

rem Run CMake
echo Running CMake...
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S./ -B./build -G !cmake_gen! -T host=x64 -A x!cmake_arch!
:END
popd
exit /b

rem Print help message
:HELP
echo.
echo GenerateSolution.bat [/?]
echo.
echo Description:
echo    Generate a Visual Studio solution for the project using CMake.
echo    If a parameter is not given, the script will ask the user for
echo    the value.
echo.
echo Parameter List:
echo    /G    Generator string passed to CMake. (defaults to "Visual
echo          Studio 17 2022")
echo.
echo    /A    Target architecture. Valid values are 32 or 64. 
echo          (defaults to 64)
echo.
echo.   /?    Displays this help message.
exit /b