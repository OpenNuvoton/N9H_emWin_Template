#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <stdint.h>

#include "NVTMedia.h"

#define DEF_PATH_MEDIA_FOLDER "D:\\DCIM\\"

typedef enum
{
    ePlayerCmd_Play,
    ePlayerCmd_Stop,
    ePlayerCmd_Seek,
    ePlayerCmd_Previous,
    ePlayerCmd_Next,
    ePlayerCmd_Status,
    ePlayerCmd_Cnt
} E_PLAYER_CMD;

typedef struct
{
    E_PLAYER_CMD m_ePlayerCmd;
    void *m_pData;
} S_PLAYER_CMD;

void *worker_player(void *pvArgs);
int player_commander(S_PLAYER_CMD *psPlayCmd);

int player_start(void);
int player_stop(void);
E_NM_PLAY_STATUS player_status(void);
E_NM_ERRNO player_pause(void);
int player_play(void);
int player_seek(uint32_t u32Percent);
int player_next(void);
int player_previous(void);
int player_playselect(int u32ListIndex);
uint64_t player_playtime_current(void);
uint64_t player_playtime_total(void);

#endif  // Avoid multiple inclusion
