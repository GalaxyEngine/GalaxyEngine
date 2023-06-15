@echo off

set /p "filename=Enter the file name: "
set "filepath=src\%filename%"
set "cppfile=%filepath%.cpp"

mkdir "%filepath%\.."
if not exist "%cppfile%" (
  (  
    echo #include "pch.h"
    echo #include "%filename%.h"
  ) >> "%cppfile%"
)

set "filepath=include\%filename%"
set "hfile=%filepath%.h"

mkdir "%filepath%\.."
if not exist "%hfile%" (
  (  
    echo #pragma once
    echo #include "GalaxyAPI.h"
    echo namespace GALAXY 
    echo {
    echo }
  ) >> "%hfile%"
)

set /p "create_inl=Do you want to create the .inl file? (Y/N): "
if /i "%create_inl%"=="Y" (
  if not exist "include/%filename%.inl" (
    (  
      echo #pragma once
      echo #include "%filename%.h"
      echo namespace GALAXY 
      echo {
      echo }
    ) >> "include/%filename%.inl"
	echo #include "%filename%.inl" >> "%hfile%"
  )
)

pause
echo File "%filename%" created successfully.
