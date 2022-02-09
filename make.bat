del /s /q BUILD\*.*
rmdir /s /q BUILD
mkdir BUILD
cd BUILD
"C:\Program Files\CMake\bin\cmake" ../WINDOWS
"C:\Program Files\CMake\bin\cmake" --build . --config Release
copy Release\enea.exe ..\enea.exe
copy ..\XERCES\xerces-c-3.2.3\BUILD\src\Release\xerces-c_3_2.dll ..\xerces-c_3_2.dll
cd ..
pause
