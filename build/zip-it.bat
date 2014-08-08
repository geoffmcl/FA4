@setlocal
@set TMPZIP=FA4-07.zip
@call cmake-clean
@cd ..
@call delallbak
@cd ..
@call zip-fa4
@cd zips
@if EXIST %TMPZIP% (
@call copy2tmp %TMPZIP%
) else (
@echo Can NOT locate %TMPZIP% in %CD%!
)

