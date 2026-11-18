@echo off
echo =======================================
echo Compiling ATM C++ Project...
echo =======================================
g++ -std=c++17 -Iinclude src/Transaction.cpp src/Account.cpp src/FileHandler.cpp src/ATM.cpp src/main.cpp -o atm.exe
if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Compilation successful!
    echo Run atm.exe to start the application.
) else (
    echo [ERROR] Compilation failed.
)
pause
