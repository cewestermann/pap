@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

set CODE_DIR=.\code
set BUILD_DIR=.\build

set SOURCE_FILE=parser

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cl /EHsc /Zi /WX /W4 /wd4101 /wd4505 /wd4189 /wd4100 /I"%CODE_DIR%" /Fe:"%BUILD_DIR%\%SOURCE_FILE%.exe" /Fd:"%BUILD_DIR%\%SOURCE_FILE%.pdb" "%CODE_DIR%\%SOURCE_FILE%.c"

