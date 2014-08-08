@setlocal
@set TMPPRJ=FA4
@set TMPLOG=bldlog-1.txt
@set TMPSRC=..
@REM ############################################
@REM NOTE: SPECIAL INSTALL LOCATION
@REM Adjust to suit your environment
@REM ##########################################
@set TMPINST=C:\MDOS
@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=%TMPINST%

@call chkmsvc %TMPPRJ%

@echo Begin %DATE% %TIME%, output to %TMPLOG%
@echo Begin %DATE% %TIME% > %TMPLOG%

@echo Doing: 'cmake %TMPSRC% %TMPOPTS%'
@echo Doing: 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config debug'
@echo Doing: 'cmake --build . --config debug' >> %TMPLOG%
@cmake --build . --config debug >> %TMPLOG%
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config release'
@echo Doing: 'cmake --build . --config release' >> %TMPLOG%
@cmake --build . --config release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@echo Appears a successful build
@echo.
@echo No install at this time...
@goto END

@echo Note install location %TMPINST%
@echo *** CONTINUE with install? *** Only Ctrl+C aborts
@pause

cmake -P cmake_install.cmake
@REM echo Doing: 'cmake --build . --config release --target INSTALL'
@REM echo Doing: 'cmake --build . --config release --target INSTALL' >> %TMPLOG%
@REM cmake --build . --config release --target INSTALL >> %TMPLOG% 2>&1
@REM fa4 " -- " %TMPLOG%

@echo Done build and install of %TMPPRJ%...

@goto END

:ERR1
@echo cmake config, generation error
@goto ISERR

:ERR2
@echo debug build error
@goto ISERR

:ERR3
@echo release build error
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
