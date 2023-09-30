@echo off

call build pch

del GNOP.zip
copy build\GNOP.exe GNOP.exe
7z a GNOP.zip GNOP.exe assets shaders