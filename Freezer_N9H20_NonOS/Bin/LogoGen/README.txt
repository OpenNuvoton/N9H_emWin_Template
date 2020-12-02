1. INPUT:   copy image file here
2. PROCESS: drag-and-drop this image file on 480x272.bat
3. OUTPUT:  n_480x272.bin

Please note that:
n is image file name
this output file is a raw data and its format is RGB565 with resolution 480x272

ffplay.exe -i logo_nuvoton_w480xh272_480x272.bin -pixel_format rgb565le -video_size 480x272 -f rawvideo -loop 0