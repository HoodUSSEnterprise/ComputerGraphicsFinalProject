@echo off
:: 检查参数
if "%1"=="" (
    echo Error: No filename provided
    exit /b 1
)

:: 创建目标目录（如果不存在）
if not exist "..\build\assets\maps\community" mkdir "..\build\assets\maps\community"

:: 复制文件
copy /Y "..\assets\maps\community\%1" "..\build\assets\maps\community\%1"

:: 检查是否成功
if errorlevel 1 (
    echo Error: Failed to copy %1
    exit /b 1
) else (
    echo Synced: %1
    exit /b 0
)