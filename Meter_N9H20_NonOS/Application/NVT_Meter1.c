#include "N9H20.h"
#include "GUI.h"
#include "WM.h"
#include "IMAGE.h"
#include "BUTTON.h"

#include "NVT_Config.h"

#define NVT_LOGO1 "D:\\Logo_128x64.dta"
#define NVT_METER1 "D:\\Meter1_480x272.dta"

static UINT8 s_u8Logo1ImageBuf[18 * 1024] __attribute__((aligned(32)));
static UINT8 s_u8Meter1ImageBuf[257 * 1024] __attribute__((aligned(32)));

void NVT_Logo1(int x, int y)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_LOGO1, s_u8Logo1ImageBuf);

    GUI_DrawStreamedBitmapAuto(s_u8Logo1ImageBuf, x, y);
}

void NVT_Meter1(int x, int y)
{
    static int i32FileSize;

    if (i32FileSize == 0)
        i32FileSize = NVT_GetImage(NVT_METER1, s_u8Meter1ImageBuf);

    GUI_DrawStreamedBitmapAuto(s_u8Meter1ImageBuf, x, y);
}

#define Circle_AA_Factor 6
#define Circle_StartDegreePoint -90
#define Circle_EndDegreePoint 360
#define BigCircle_rad 52
#define SmallCircle_rad 37
#define RPMSpeedCircle_rad 70
#define RPMSpeed_dregreeS -31
#define RPMSpeed_dregreeE 210

#define FuelVolume_PositionX 370
#define FuelVolume_PositionY 200
#define FuelVolume_Point_X0Y0  0
#define FuelVolume_Point_X1Y1 -40

#define VehicleSpeed_PositionX 168
#define VehicleSpeed_PositionY 138
#define Vehicle_Point_X0Y0  -(RPMSpeedCircle_rad-5)
#define Vehicle_Point_X1Y1 -(RPMSpeedCircle_rad+60)

#define IntakeAirTemp_PositionX 370
#define IntakeAirTemp_PositionY 70
#define IntakeAirTemp_Point_X0Y0  -(BigCircle_rad-10)
#define IntakeAirTemp_Point_X1Y1 -(BigCircle_rad-15)

#define FuelVolumeText_PositionX 370
#define FuelVolumeText_PositionY 240

int Vehicle_Speed, Fuel_Volume, IntakeAir_Temp;

int CircleColor1[72] =
{
    0x0093AF00, 0x0091AF0B, 0x008FAE16, 0x008DAE20, 0x0089AD36, 0x0087AD41, 0x0085AD4B, 0x0083AC56, 0x0081AC61, 0x007EAC6C, 0x007CAB77, 0x007AAB81, 0x0078AB8C, 0x0076AA97, 0x0074AAA2, 0x0072AAAC, 0x0070A9B7, 0x006EA9C2, 0x006FB5CD, 0x0071B5C2, 0x0075B4AB, 0x0077B49F, 0x0079B394, 0x007BB389, 0x007DB37D, 0x007FB272, 0x0081B267, 0x0083B25B, 0x0085B150, 0x0087B144, 0x0089B139, 0x008BB02E, 0x008DB022, 0x008FB017, 0x0091AF0B, 0x0093AF00, 0x008BAB09, 0x0083A713, 0x007AA21C, 0x00729E25, 0x006A9A2F, 0x00629638, 0x005A9141, 0x00528D4B, 0x00498954, 0x0041855D, 0x00398167, 0x00317C70, 0x00297879, 0x00217483, 0x0018708C, 0x00106B95, 0x0008679F, 0x000063A8, 0x000063A8, 0x0008679F, 0x00106B95, 0x0019708C, 0x00217483, 0x00297879, 0x00317C70, 0x00398167, 0x0041855D, 0x004A8954, 0x00528D4B, 0x005A9141, 0x00629638, 0x006A9A2F, 0x00729E25, 0x007BA21C, 0x0083A713, 0x008BAB09
};
int CircleColor2[72] =
{
    0x00D45B00, 0x00CE5F0B, 0x00C96416, 0x00C36820, 0x00BD6C2B, 0x00B87136, 0x00B27541, 0x00AC794B, 0x00A77E56, 0x00A18261, 0x009B866C, 0x00968B77, 0x00908F81, 0x008A938C, 0x00859897, 0x007F9CA2, 0x0079A0AC, 0x0074A5B7, 0x006FB5CD, 0x0072B4BA, 0x0074B3B1, 0x0076B2A8, 0x0078B19E, 0x0079B195, 0x007BB08C, 0x007DAF82, 0x007FAF79, 0x0080AE70, 0x0082AD66, 0x0084AC5D, 0x0085AC54, 0x0087AB4A, 0x0089AA41, 0x008BA938, 0x008CA92E, 0x008EA825, 0x0092A823, 0x0095A821, 0x0099A81F, 0x009DA81D, 0x00A1A71B, 0x00A4A719, 0x00A8A717, 0x00ACA715, 0x00B0A712, 0x00B3A710, 0x00B7A70E, 0x00BBA70C, 0x00BEA70A, 0x00C2A608, 0x00C6A606, 0x00CAA604, 0x00CDA602, 0x00D1A600, 0x00D1A600, 0x00D1A200, 0x00D19E00, 0x00D29A00, 0x00D29500, 0x00D29100, 0x00D28D00, 0x00D28900, 0x00D28500, 0x00D38000, 0x00D37C00, 0x00D37800, 0x00D37400, 0x00D37000, 0x00D36C00, 0x00D46700, 0x00D46300, 0x00D45F00
};
int CircleColor3[72] =
{
    0x0093AF00, 0x0091AF0B, 0x008FAE16, 0x008DAE20, 0x0089AD36, 0x0087AD41, 0x0085AD4B, 0x0083AC56, 0x0081AC61, 0x007EAC6C, 0x007CAB77, 0x007AAB81, 0x0078AB8C, 0x0076AA97, 0x0074AAA2, 0x0072AAAC, 0x0070A9B7, 0x006EA9C2, 0x006FB5CD, 0x0071B5C2, 0x0075B4AB, 0x0077B49F, 0x0079B394, 0x007BB389, 0x007DB37D, 0x007FB272, 0x0081B267, 0x0083B25B, 0x0085B150, 0x0087B144, 0x0089B139, 0x008BB02E, 0x008DB022, 0x008FB017, 0x0091AF0B, 0x0093AF00, 0x008BAB09, 0x0083A713, 0x007AA21C, 0x00729E25, 0x006A9A2F, 0x00629638, 0x005A9141, 0x00528D4B, 0x00498954, 0x0041855D, 0x00398167, 0x00317C70, 0x00297879, 0x00217483, 0x0018708C, 0x00106B95, 0x0008679F, 0x000063A8, 0x000063A8, 0x0008679F, 0x00106B95, 0x0019708C, 0x00217483, 0x00297879, 0x00317C70, 0x00398167, 0x0041855D, 0x004A8954, 0x00528D4B, 0x005A9141, 0x00629638, 0x006A9A2F, 0x00729E25, 0x007BA21C, 0x0083A713, 0x008BAB09
};
int CircleColor4[72] =
{
    0x00D45B00, 0x00CE5F0B, 0x00C96416, 0x00C36820, 0x00BD6C2B, 0x00B87136, 0x00B27541, 0x00AC794B, 0x00A77E56, 0x00A18261, 0x009B866C, 0x00968B77, 0x00908F81, 0x008A938C, 0x00859897, 0x007F9CA2, 0x0079A0AC, 0x0074A5B7, 0x006FB5CD, 0x0072B4BA, 0x0074B3B1, 0x0076B2A8, 0x0078B19E, 0x0079B195, 0x007BB08C, 0x007DAF82, 0x007FAF79, 0x0080AE70, 0x0082AD66, 0x0084AC5D, 0x0085AC54, 0x0087AB4A, 0x0089AA41, 0x008BA938, 0x008CA92E, 0x008EA825, 0x0092A823, 0x0095A821, 0x0099A81F, 0x009DA81D, 0x00A1A71B, 0x00A4A719, 0x00A8A717, 0x00ACA715, 0x00B0A712, 0x00B3A710, 0x00B7A70E, 0x00BBA70C, 0x00BEA70A, 0x00C2A608, 0x00C6A606, 0x00CAA604, 0x00CDA602, 0x00D1A600, 0x00D1A600, 0x00D1A200, 0x00D19E00, 0x00D29A00, 0x00D29500, 0x00D29100, 0x00D28D00, 0x00D28900, 0x00D28500, 0x00D38000, 0x00D37C00, 0x00D37800, 0x00D37400, 0x00D37000, 0x00D36C00, 0x00D46700, 0x00D46300, 0x00D45F00
};

void Draw_FuelVolumeLine(volatile int degree_idex)
{
    int x, y, x1, y1, FuelVolume_degree;
//    float a;
    GUI_AA_EnableHiRes(); /* Enable high resolution */
    GUI_AA_SetFactor(Circle_AA_Factor); /* Set quality factor */
    GUI_SetPenSize(2);

    if(degree_idex>240) 
        FuelVolume_degree=RPMSpeed_dregreeE;
    else 
        FuelVolume_degree=(degree_idex+RPMSpeed_dregreeS);
    
//    a = (FuelVolume_degree)*3.1415926/180;
    x = (FuelVolume_Point_X0Y0*GUI__CosHQ(FuelVolume_degree * 1000) >> 16)+FuelVolume_PositionX;
    y = (FuelVolume_Point_X0Y0*GUI__SinHQ(FuelVolume_degree * 1000) >> 16)+FuelVolume_PositionY;
    x1 = (FuelVolume_Point_X1Y1*GUI__CosHQ(FuelVolume_degree * 1000) >> 16)+FuelVolume_PositionX;
    y1 = (FuelVolume_Point_X1Y1*GUI__SinHQ(FuelVolume_degree * 1000) >> 16)+FuelVolume_PositionY;    
    
    GUI_SetColor( GUI_RED );
    GUI_AA_DrawLine(x*Circle_AA_Factor, y*Circle_AA_Factor, x1*Circle_AA_Factor, y1*Circle_AA_Factor);       
    GUI_AA_DisableHiRes(); /* Disable high resolution */
}

void Draw_VehicleLine(volatile int degree_idex)
{
    int x, y, x1, y1, Vehicle_degree;
//    float a;
    GUI_AA_EnableHiRes(); /* Enable high resolution */
    GUI_AA_SetFactor(Circle_AA_Factor); /* Set quality factor */
    GUI_SetPenSize(5);

    if(degree_idex>240)
        Vehicle_degree=RPMSpeed_dregreeE;
    else
        Vehicle_degree=(degree_idex+RPMSpeed_dregreeS);

//    a = (Vehicle_degree)*3.1415926/180;
    x = (Vehicle_Point_X0Y0*GUI__CosHQ(Vehicle_degree * 1000) >> 16)+VehicleSpeed_PositionX;
    y = (Vehicle_Point_X0Y0*GUI__SinHQ(Vehicle_degree * 1000) >> 16)+VehicleSpeed_PositionY;
    x1 = (Vehicle_Point_X1Y1*GUI__CosHQ(Vehicle_degree * 1000) >> 16)+VehicleSpeed_PositionX;
    y1 = (Vehicle_Point_X1Y1*GUI__SinHQ(Vehicle_degree * 1000) >> 16)+VehicleSpeed_PositionY;

    GUI_SetColor( GUI_RED );
    GUI_AA_DrawLine(x*Circle_AA_Factor, y*Circle_AA_Factor, x1*Circle_AA_Factor, y1*Circle_AA_Factor);
    GUI_AA_DisableHiRes(); /* Disable high resolution */
}

void Draw_IntakeAirRainbow(volatile int degree_idex)
{
    int i, x, y, x1, y1, IntakeAir_degree;
//    float a;

    IntakeAir_degree = (degree_idex+40)/3.542;
    if(degree_idex==215)    IntakeAir_degree = 72;
    GUI_AA_EnableHiRes(); /* Enable high resolution */
    GUI_AA_SetFactor(Circle_AA_Factor); /* Set quality factor */
    GUI_SetPenSize( 2 );

    for (i=0; i< IntakeAir_degree; i++)
    {
//        a = (Circle_StartDegreePoint+i*5)*3.1415926/180;
        x = (IntakeAirTemp_Point_X0Y0*GUI__CosHQ((Circle_StartDegreePoint+i*5) * 1000) >> 16)+IntakeAirTemp_PositionX;
        y = (IntakeAirTemp_Point_X0Y0*GUI__SinHQ((Circle_StartDegreePoint+i*5) * 1000) >> 16)+IntakeAirTemp_PositionY;
        x1 = (IntakeAirTemp_Point_X1Y1*GUI__CosHQ((Circle_StartDegreePoint+i*5) * 1000) >> 16)+IntakeAirTemp_PositionX;
        y1 = (IntakeAirTemp_Point_X1Y1*GUI__SinHQ((Circle_StartDegreePoint+i*5) * 1000) >> 16)+IntakeAirTemp_PositionY;

        if ((i + 18) > 71)
            break;
        GUI_SetColor( CircleColor2[i + 18] );
        GUI_AA_DrawLine(x*Circle_AA_Factor, y*Circle_AA_Factor, x1*Circle_AA_Factor, y1*Circle_AA_Factor);
    }

    GUI_AA_DisableHiRes(); /* Disable high resolution */
}

void Show_IntakeAirTemp_Position(volatile int degree_idex)
{
    char ac[10];

    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);

    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_WHITE);

    sprintf(ac,"%02d\xB0\x43", degree_idex);
    GUI_DispStringHCenterAt(ac, IntakeAirTemp_PositionX, IntakeAirTemp_PositionY);
    Draw_IntakeAirRainbow(degree_idex);
}

void Show_FuelVolume(volatile int degree_idex)
{
    char ac[10];
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);

    GUI_SetFont(GUI_FONT_16B_ASCII);
    GUI_SetColor(GUI_WHITE);

    sprintf(ac,"%02dL", Fuel_Volume);
    GUI_DispStringHCenterAt(ac, FuelVolumeText_PositionX, FuelVolumeText_PositionY);
    GUI_SetColor(GUI_RED);
    GUI_DispStringAt("E", 340, 220);
    GUI_SetColor(GUI_GREEN);
    GUI_DispStringAt("F", 400, 221);
    Draw_FuelVolumeLine(degree_idex);
}

void Show_VehicleSpeed(volatile int degree_idex)
{
    char ac[10];
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);

    GUI_SetFont(GUI_FONT_D32);
    GUI_SetColor(GUI_WHITE);
    sprintf(ac,"%02d", Vehicle_Speed);
    GUI_DispStringHCenterAt(ac, VehicleSpeed_PositionX, VehicleSpeed_PositionY);
    Draw_VehicleLine(degree_idex);
}

void NVT_CxtUpdate(void)
{
    Show_VehicleSpeed(Vehicle_Speed);
    Show_IntakeAirTemp_Position(IntakeAir_Temp);
    Show_FuelVolume(Fuel_Volume);
}
