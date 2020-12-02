@ECHO OFF
GOTO START

******************************************************************************
*
* File:        MakeMovie.bat
*
* Parameters:  4 (3 optional)
*              %1: Movie file to be converted
*              %2: Size
*              %3: Quality
*              %4: Framerate
*
* Requirement: The free available tool FFmpeg is required for the conversion
*              operation. FFmpeg is available under the following link:
*
*              http://www.ffmpeg.org/
*
*              Please note that PREP.bat needs to be adapted before using
*              MakeMovie.bat.
*
*              This file (MakeMovie.bat) normally do not need to be modified.
*
* Purpose:     This batch file converts a video file to an MJPEG AVI.
*
*              For details about all supported file types and the parameters
*              resolution, quality and frame rate please refer to the FFmpeg
*              documentation.
*
******************************************************************************

:START

CALL %~dp0PREP.BAT

IF "%2" == "" GOTO CONT2
SET SIZE=%2
GOTO NEXT2
:CONT2
SET SIZE=%DEFAULT_SIZE%
:NEXT2

IF "%3" == "" GOTO CONT3
SET QUALITY=%3
GOTO NEXT3
:CONT3
SET QUALITY=%DEFAULT_QUALITY%
:NEXT3

IF "%4" == "" GOTO CONT4
SET FRAMERATE=%4
GOTO NEXT4
:CONT4
SET FRAMERATE=%DEFAULT_FRAMERATE%
:NEXT4

"%FFMPEG%" -y -i %1 -vcodec rawvideo -f rawvideo -pix_fmt rgb565le -s %SIZE% %~n1_%SIZE%.bin

IF ERRORLEVEL 1 GOTO ERROR
GOTO NOERROR
:ERROR
ECHO Error
PAUSE
:NOERROR

SET SIZE=
SET QUALITY=
SET FRAMERATE=
SET FFMPEG=