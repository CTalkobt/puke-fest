/* ctl.h — the controls: a joystick in either port, W A S D or the cursor keys */
#pragma once

#define C_UP    0x01
#define C_DOWN  0x02
#define C_LEFT  0x04
#define C_RIGHT 0x08
#define C_FIRE  0x10                 /* FIRE, SPACE or RETURN */
#define C_QUIT  0x20                 /* RUN/STOP */
#define C_PAUSE 0x40                 /* P */
#define C_OPTIONS 0x80               /* O */

unsigned char ctl_read(void);        /* the controls held now */
