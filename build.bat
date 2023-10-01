
@echo off

cls

if NOT defined VSCMD_ARG_TGT_ARCH (
	@REM Replace with your path
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

if not exist build\NUL mkdir build

set comp=-nologo -std:c++20 -Zc:strictStrings- -W4 -FC -Gm- -GR- -EHa- -wd 4324 -wd 4127 -wd 4505 -D_CRT_SECURE_NO_WARNINGS -Dm_app
set linker=-INCREMENTAL:NO
set comp=%comp% -wd4201

set debug=2
if %debug%==0 (
	set comp=%comp% -O2 -MT
	set linker=%linker% -SUBSYSTEM:windows
	rc.exe /nologo icon.rc
)
if %debug%==1 (
	set comp=%comp% -O2 -Dm_debug -MTd
)
if %debug%==2 (
	set comp=%comp% -Od -Dm_debug -Zi -MTd
)

SETLOCAL ENABLEDELAYEDEXPANSION
pushd build

	if "%1%"=="pch" (
		cl /Ycpch_client.h ..\src\pch_client.cpp %comp% /c
		cl /Ycpch_platform.h ..\src\pch_platform.cpp %comp% /c
	)

	if %debug%==0 (
		cl ..\src\win32_platform.cpp ..\src\client.cpp -FeDigHard.exe %comp% -link %linker% -PDB:platform_client.pdb ..\icon.res > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt
	) else (
		cl ..\src\client.cpp /Yupch_client.h -LD -FeDigHard.dll %comp% -link %linker% pch_client.obj -PDB:client.pdb > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt

		tasklist /fi "ImageName eq DigHard.exe" /fo csv 2>NUL | find /I "DigHard.exe">NUL
		if NOT !ERRORLEVEL!==0 (
			cl ..\src\win32_platform.cpp /Yupch_platform.h -FeDigHard.exe %comp% -link %linker% pch_platform.obj -PDB:platform_client.pdb > temp_compiler_output.txt
			if NOT !ErrorLevel! == 0 (
				type temp_compiler_output.txt
				popd
				goto fail
			)
			type temp_compiler_output.txt
		)
	)

popd
if %errorlevel%==0 goto success
goto fail

:success
copy build\DigHard.exe DigHard.exe > NUL
goto end

:fail

:end
copy build\temp_compiler_output.txt compiler_output.txt > NUL

:foo