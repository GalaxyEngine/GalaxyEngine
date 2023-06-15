@echo off
set /p "filename=Enter the file name: "

set "filepath=src\%filename%"
mkdir "%filepath%\.."
(  
  echo #include "pch.h"
  echo #include "%filename%.h"
) >> "%filepath%".cpp


set "filepath=include\%filename%"
mkdir "%filepath%\.."
(  
  echo #pragma once
  echo #include "GalaxyAPI.h"
  echo namespace GALAXY 
  echo {
  echo }
) >> "%filepath%".h

set /p "create_inl=Do you want to create the .inl file? (Y/N): "
if /i "%create_inl%"=="Y" (
  mkdir "%filepath%\.."
  (  
    echo #pragma once
    echo #include "%filename%.h"
    echo namespace GALAXY 
    echo {
    echo }
  ) >> "%filepath%".inl

  echo #include "%filename%.inl" >> "%filepath%".h
)


echo File "%filename%" created successfully.
