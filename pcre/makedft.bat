@echo Make of pcre_chartables.c ... that is build and run dftables ...
@set TEMP1=makefile.dft
@if "%INCLUDE%." == "." goto ERR1
@if "%LIB%." == "." goto ERR2
@if "%FrameworkDir%." == "." goto ERR3
@if "%VCINSTALLDIR%." == "." goto ERR4
@if "%VSINSTALLDIR%." == "." goto ERR5
@if NOT EXIST %TEMP1% goto ERR6
nmake /nologo /f %TEMP1%
@echo	Error level = %ERRORLEVEL%
@goto END

:ERR1
@echo	No INCLUDE enviroment items ...
@goto NOTVC8
:ERR2
@echo	No LIB enviroment items ...
@goto NOTVC8
:ERR3
@echo 	No FrameworkDir environment item ...
@goto NOTVC8
:ERR4
@echo 	No VCINSTALLDIR environment item ...
@goto NOTVC8
:ERR5
@echo 	No VSINSTALLDIR environment item ...
@goto NOTVC8

:NOTVC8
@echo	***** ERROR ERROR ERROR *****
@echo	Do NOT appear to be in a MSVC8 console ...
@echo	That is C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat
@echo	***** HAS NOT BEEN RUN ***** ...
@echo	Try running vsvars first, or open a MSVC8 console ...
@goto END

:ERR6
@echo 	Can NOT locate %TEMP1% file ... check name, location ...
@goto END

:END
