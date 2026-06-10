# Tower Defense

基于 SFML 的塔防游戏。

## 环境要求

- CMake >= 3.16（[下载地址](https://cmake.org/download/)）
- C++17 编译器（如 MinGW 或 Visual Studio）
- SFML 2.6（[下载地址](https://www.sfml-dev.org/download/sfml/2.6.1/)）

## 下载与安装

### 1. 安装 CMake
从 [cmake.org/download](https://cmake.org/download/) 下载安装包，安装时勾选"Add CMake to system PATH"。

### 2. 安装 SFML
从 [sfml-dev.org](https://www.sfml-dev.org/download/sfml/2.6.1/) 下载 **2.6.1** 版本，解压到 `D:\SFML-2.6.1`（目录结构应包含 `bin/`、`include/`、`lib/`、`share/`）。

如果已有 SFML DLL，说明已下载，可以跳过这一步。

### 3. 编译

```bash
# 进入项目目录
cd D:\ComputerGraphicsFinalProject

# 配置
cmake -S . -B build

# 编译
cmake --build build
```

编译完成后，可执行文件生成在 `build/tower_defense.exe`，所需 DLL 和资源文件会自动复制到同一目录。

## 运行

```bash
./build/tower_defense.exe
```

或者双击 `build/tower_defense.exe`。

## 项目结构

```
├── CMakeLists.txt          # CMake 构建配置
├── assets/                 # 游戏资源（地图等）
├── fonts/                  # 字体文件
├── textures/               # 纹理/图片
│   └── background/         # 背景图
├── sound/                  # 音效文件
├── include/                # 头文件
│   ├── Constants.h
│   ├── Game.h
│   ├── Enemy.h
│   ├── Tower.h
│   ├── Map.h
│   ├── UI.h
│   └── ...
├── main/
│   └── main.cpp            # 入口
├── src/                    # 源代码
│   ├── Game.cpp
│   ├── Enemy.cpp
│   ├── GamePlay.cpp
│   ├── GameScreens*.cpp    # 各界面逻辑
│   ├── Tower.cpp
│   └── ...
└── build/                  # 构建输出目录
    └── tower_defense.exe
```
