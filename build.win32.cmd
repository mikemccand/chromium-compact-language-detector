REM **************************************************
REM To build use the following command
REM > build.win32.cmd vc90

REM As usual, to set up the environment variables, it is required to first
REM run "C:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\vcvarsall.bat" 
REM (replacing the specific parts of the path to meet your system's).

REM Pre-built versions of the .lib, .dll and the test file can be downloaded
REM from the following link, for quick testing:
REM **************************************************

echo off

SET OLDDIR=%CD%

IF "%~1"=="help" (
    ECHO Syntax: %~0 vc90 ^| vc100 ^| vc110 [amd64]
    ECHO vc90: For compiling with Visual C 2008 9.0 and .NET 3.5. At least, a working copy of Visual Studio 2008 is needed or a working copy of both Visual Express 2008 and 2010.
    ECHO vc100: For compiling with Visual C 2010 10.0 and .NET 4.0. At least, a working copy of Visual Express 2010 is needed.
    ECHO vc110: For compiling with Visual C 2012 11.0 and .NET 4.5. At least, a working copy of Visual Express 2012 is needed.
    ECHO amd64: For compiling in amd64 architectures when in a amd64 Windows installation. At least, a working copy of the correspoding toolchains for amd64 are needed. Option not thouroughly tested.
    GOTO end
)

REM IF "%~1"=="vc90" (
REM     CALL "C:\Program Files (x86)\Microsoft Visual Studio 9.0\vc\vcvarsall.bat" %~2
REM     IF ERRORLEVEL 1 GOTO end
REM ) ELSE (
REM     IF "%~1"=="vc100" (
REM         CALL "C:\Program Files (x86)\Microsoft Visual Studio 10.0\vc\vcvarsall.bat" %~2
REM         IF ERRORLEVEL 1 GOTO end
REM     ) ELSE (
REM         IF "%~1"=="vc110" (
REM             CALL "C:\Program Files (x86)\Microsoft Visual Studio 11.0\vc\vcvarsall.bat" %~2
REM             IF ERRORLEVEL 1 GOTO end
REM 	) ELSE (
REM             ECHO Syntax: %~0 vc90 ^| vc100 ^| vc110 [amd64]
REM             GOTO end
REM 	)
REM     )
REM )

set CFLAGS=/I. /DWIN32 /DNDEBUG /D_LIB /DCLD_WINDOWS /FD /EHsc /MD /W3 /O2 /nologo /c /Wp64 /Zi /TP /errorReport:prompt
set CC=cl.exe
set AR=lib.exe
set CSC=csc.exe
set LNK=link.exe
set MT=mt.exe

cd src/
del *.obj
del libcld.lib

set SOURCES=encodings\compact_lang_det\cldutil.cc encodings\compact_lang_det\cldutil_dbg_empty.cc encodings\compact_lang_det\compact_lang_det.cc encodings\compact_lang_det\compact_lang_det_impl.cc encodings\compact_lang_det\ext_lang_enc.cc encodings\compact_lang_det\getonescriptspan.cc encodings\compact_lang_det\letterscript_enum.cc encodings\compact_lang_det\tote.cc encodings\compact_lang_det\generated\cld_generated_score_quadchrome_0406.cc encodings\compact_lang_det\generated\compact_lang_det_generated_cjkbis_0.cc encodings\compact_lang_det\generated\compact_lang_det_generated_ctjkvz.cc encodings\compact_lang_det\generated\compact_lang_det_generated_deltaoctachrome.cc encodings\compact_lang_det\generated\compact_lang_det_generated_quadschrome.cc encodings\compact_lang_det\win\cld_htmlutils_windows.cc encodings\compact_lang_det\win\cld_unilib_windows.cc encodings\compact_lang_det\win\cld_utf8statetable.cc encodings\compact_lang_det\win\cld_utf8utils_windows.cc encodings\internal\encodings.cc languages\internal\languages.cc

echo Compile and link libcld.lib...
%CC% %CFLAGS% %SOURCES%
IF ERRORLEVEL 1 GOTO end
%AR% *.obj /NOLOGO /OUT:libcld.lib
IF ERRORLEVEL 1 GOTO end


echo Compile basic test...

set CFLAGS=/nologo /I. /I..\src\ /O2 /DCLD_WINDOWS /DWIN32 /EHsc
set LDFLAGS=/link /LIBPATH:..\src\ /NODEFAULTLIB:libcmt.lib

cd ../tests/
del *.obj
del basic_test.exe

%CC% %CFLAGS% basic_test.cc %LDFLAGS% libcld.lib
IF ERRORLEVEL 1 GOTO end

cd ..

REM Build .NET bindings library and example

cd bindings\dotnet\

del *.obj
del *.dll
del *.exe
del *.ilk
del *.pdb

set CFLAGS=/I..\..\src\ /DWIN32 /DNDEBUG /D_WINDOWS /D_USRDLL /DDOTNETBINDING_EXPORTS /DCLD_WINDOWS /D_WINDLL /D_AFXDLL /FD /EHa /MD /O2 /W3 /c /Zi /clr /TP /nologo /errorReport:prompt
set LDFLAGS=/NOLOGO /DLL /MANIFEST /MANIFESTFILE:ccld.dll.intermediate.manifest /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DYNAMICBASE /FIXED:No /NXCOMPAT /ERRORREPORT:PROMPT ..\..\src\libcld.lib


echo Compile, link and embed manifest in .NET bindings...
%CC% %CFLAGS% ccld.cpp
IF ERRORLEVEL 1 GOTO end
%LNK% %LDFLAGS% /out:ccld.dll ccld.obj
IF ERRORLEVEL 1 GOTO end
%MT% /outputresource:"ccld.dll;#2" /manifest ccld.dll.intermediate.manifest /nologo
IF ERRORLEVEL 1 GOTO end

echo Compile .NET bindings example...
set CSCFLAGS=/noconfig /unsafe+ /nowarn:1701,1702 /errorreport:prompt /warn:4 /define:TRACE /debug:pdbonly /filealign:512 /optimize+ /target:exe /reference:System.Core.dll

IF "%~2"=="amd64" (
    %CSC% %CSCFLAGS% /platform:x64 /t:exe /r:ccld.dll test.cs
    IF ERRORLEVEL 1 GOTO end
) ELSE (
    %CSC% %CSCFLAGS% /platform:x86 /t:exe /r:ccld.dll test.cs
    IF ERRORLEVEL 1 GOTO end
)
cd ..\..

:end
chdir /d %OLDDIR%