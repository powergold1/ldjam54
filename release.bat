@echo off

call build pch

del DigHard.zip
copy build\DigHard.exe DigHard.exe
7z a DigHard.zip DigHard.exe assets shaders