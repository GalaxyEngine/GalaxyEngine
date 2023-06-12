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
  echo namespace GALAXY {
  echo }
) >> "%filepath%".h

echo File "%filename%" created successfully.
