@ECHO OFF
GOTO START

******************************************************************************
*
* File:        PREP.bat
*
* Parameters:  none
*
* Purpose:     This batch file is called by MakeMovie.bat and sets the default
*              values for working folder, FFmpeg path. It needs
*              to be adapted before MakeMovie.bat can be used.
*
******************************************************************************

:START

REM **************************************************************************
REM
REM   Configuration: Default values for size, quality and framerate
REM
REM **************************************************************************

SET DEFAULT_SIZE=320x240
SET DEFAULT_QUALITY=1
SET DEFAULT_FRAMERATE=25

REM **************************************************************************
REM
REM   Configuration: FFmpeg
REM
REM **************************************************************************

SET FFMPEG=ffmpeg.exe
