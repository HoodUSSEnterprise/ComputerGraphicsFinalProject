# quick_sync.py - 快速同步，无确认
import os
import shutil

def quick_sync(source, target):
    
    print(f"Syncing {source} -> {target}...")
    
    # 确保目标目录存在
    os.makedirs(target, exist_ok=True)
    
    # 同步文件
    os.system(f'rsync -av --delete {source}/ {target}/' if os.name != 'nt' 
              else f'robocopy {source} {target} /MIR /NJH /NJS')
    
    print("Sync completed!")

if __name__ == "__main__":
    quick_sync("assets", "build/assets")
    quick_sync("sound", "build/sound")
    quick_sync("textures", "build/textures")