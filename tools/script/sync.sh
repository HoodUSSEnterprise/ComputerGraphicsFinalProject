#!/bin/bash

# 检查参数
if [ -z "$1" ]; then
    echo "Error: No filename provided"
    exit 1
fi

# 创建目标目录（如果不存在）
mkdir -p "../build/assets/maps/community"

# 复制文件
cp -f "../assets/maps/community/$1" "../build/assets/maps/community/$1"

# 检查是否成功
if [ $? -eq 0 ]; then
    echo "Synced: $1"
    exit 0
else
    echo "Error: Failed to copy $1"
    exit 1
fi