@echo Update the pcre zip source ...
@set TEMPSD=D:
@set TEMP1=pcre-02.zip
@REM set TEMP1=pcre-01.zip - 20070108 - fixed a LOT of warnings
@set TEMPX=%TEMP%\TEMPEXCL.TXT
@if EXIST %TEMPX% goto GotEx
Call QSEX %TEMPX%
@if NOT EXIST %TEMPX% goto Err2
:GotEx
@set TEMP2= -a -o -r -p -x@%TEMPX% -x*.zip -x*.exe -x*.lib
@if NOT EXIST %TEMP1% goto DoIt
@set TEMP2= -i -o -r -p -x@%TEMPX% -x*.zip -x*.exe -x*.lib
:DoIt
call Zip8 %TEMP2% %1 %2 %3 %4 %TEMP1% *.*
@if NOT EXIST %TEMP1% goto ERR1
dir %TEMP1%
@IF NOT EXIST %TEMPSD%\SAVES\nul goto NOSAVE
copy %TEMP1% %TEMPSD%\SAVES\.
@goto END

:NOSAVE
@echo WARNING: No Save to second disk ...
@goto END

:ERR1
@echo	ERROR: Can not create %TEMP1% zip file ...
@goto END

:ERR2
@echo ERROR: Can not make exclude file list ...
@goto END

:END
