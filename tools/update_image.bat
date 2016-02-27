@echo off

REM - Author: Steve Tranby
REM - Dependencies: Virtual Floppy Disk 
REM - Used for when building on Win32

REM - TODO: need to create installer for VFD (chocolaty/nuget??)

set VFD=c:\_tools\vfd\vfd.exe
set IMG=c:\_img\tranix_kernel_grub.img
set KERNEL=c:\_projects\tranixos\kernel.bin

echo Updating Kernel Image
echo Removing VFD From the System

%VFD% CLOSE
%VFD% STOP
%VFD% REMOVE

echo Installing VFD

%VFD% INSTALL
%VFD% START
%VFD% LINK 1 B /L

echo Opening Kernel Floppy Image

%VFD% OPEN b: %IMG% /1.44

echo Copying Kernel To Floppy

REM Try to copy if Drive Exists

IF not exist B:\ goto ERROR
IF not exist %KERNEL% goto ERROR

copy %KERNEL% B:\

ERROR:

echo Saving and Closing

%VFD% SAVE %IMG%
%VFD% ULINK B
%VFD% CLOSE
%VFD% REMOVE

pause