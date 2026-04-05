#! /usr/bin/python3

import os
import shutil
import subprocess
import sys

engineFolder = "./"
packageFolder = os.path.join(engineFolder, "packedBuild")
executable = ""

def GetOSSpecifics():
    global executable
    if sys.platform.startswith("win"):
        executable = os.path.join(engineFolder, "bin/Windows-x64/Project_Kaunas-Package.exe")
    elif sys.platform.startswith("linux"):
        executable = os.path.join(engineFolder, "bin/Linux-x64/Project_Kaunas-Package")
    else:
        print("Unsupported OS: " + sys.platform + ", aborting packaging!")
        exit(-1)


def SetupFolder():
    os.makedirs(packageFolder, exist_ok=True)

def CopyProject():
    source = os.path.join(engineFolder, "GameData")
    if os.path.exists(source):
        shutil.copytree(source, packageFolder, dirs_exist_ok=True)
        print(f"Copied game data into package")
    else:
        print(f"Source folder '{source}' does not exist.")

def CompileExecutable(): # please do this inside of CLion...
    pass
    

def CopyExecutable():
    print(executable)
    if not os.path.exists(executable):
        print(f"Executable folder '{executable}' does not exist.")
        return
    
    shutil.copy(executable, packageFolder)
    

GetOSSpecifics()
SetupFolder()
CopyProject()
CompileExecutable()
CopyExecutable()