@echo off

if defined ProgramFiles(x86) (
   SET C2PROG_EXE="%ProgramFiles(x86)%\C2Prog\C2ProgShell.exe"
) else (
   SET C2PROG_EXE="%ProgramFiles%\C2Prog\C2ProgShell.exe"
)

set BUILDSTEP=step_%1
shift

set COMPILERPATH=%~1
shift

set PROJECTNAME=%~1
shift

goto %BUILDSTEP%
:step_0
    goto end_step
    
:step_1
   set C2PROGTARGET=%1
   rem trim \" surrounding target name, e.g. for \"28069,67,66_JTAG\"
   set C2PROGTARGET=%C2PROGTARGET:"=%
   set C2PROGTARGET=%C2PROGTARGET:\=%  
   if exist %C2PROG_EXE% (
      "%COMPILERPATH%/bin/hex2000" -romwidth 16 -memwidth 16 -i -o %PROJECTNAME%.hex %PROJECTNAME%.out
      if  errorlevel  1  goto  error
      %C2PROG_EXE% -hex=%PROJECTNAME%.hex -ehx=%PROJECTNAME%.ehx -target=%C2PROGTARGET% -ta=770 -sa=778 -baud=500000
      if  errorlevel  1  goto  error
   )
:end_step

if  errorlevel  1  goto  error
exit 0

:error
exit 1