@echo off
setlocal

set ROOT=%~dp0
set OUT=%ROOT%build
set SRC=%ROOT%src

if not exist "%OUT%" mkdir "%OUT%"

echo Building 3D Product Showroom...
echo.

g++ -std=c++11 -O2 -Wall -Wextra -mwindows -static -static-libgcc -static-libstdc++ -I"%SRC%" "%SRC%\main.cpp" "%SRC%\app.cpp" -o "%OUT%\ProductShowroom.exe" -lopengl32 -lgdi32 -luser32 -lcomdlg32

if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build successful: "%OUT%\ProductShowroom.exe"
exit /b 0
