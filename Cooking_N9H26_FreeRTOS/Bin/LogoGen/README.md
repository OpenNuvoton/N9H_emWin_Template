# Description for LogoGen Materials

### File

```c
ffmpeg.exe ffmpeg comes from FFmpeg, official website https://ffmpeg.org/, and compilered from Gyan Doshi, official website https://www.gyan.dev/ffmpeg/builds/
README.txt Description for how to use LgoGen
```

### Parameter
1. INPUT:   copy image file here
2. PROCESS: drag-and-drop this image file on 800x480.bat
3. OUTPUT:  n_800x480.bin

### Note
n is image file name
n_800x480.bin is a output file (raw data) and its format is RGB565 with resolution 800x480

You can check raw data by below command:
ffplay.exe -i Logo_800x480_800x480.bin -pixel_format rgb565le -video_size 080x480 -f rawvideo -loop 0
