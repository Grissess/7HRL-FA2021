#ifndef ANSI_H
#define ANSI_H

#ifdef NO_COLOR
#define C_RESET ""
#define C_CREATURE ""
#define C_ITEM ""
#define C_WEAPON ""
#define C_EVENT_DAMAGE ""
#define C_EVENT_MISS ""
#define C_EVENT_NEUTRAL ""
#define A_HOME ""
#define A_CLEAR ""
#else
#define C_RESET "\x1b[m"
#define C_CREATURE "\x1b[36m"
#define C_ITEM "\x1b[35m"
#define C_WEAPON "\x1b[1;35m"
#define C_EVENT_DAMAGE "\x1b[1;31m"
#define C_EVENT_MISS "\x1b[31m"
#define C_EVENT_NEUTRAL "\x1b[37m"
#define A_HOME "\x1b[H"
#define A_CLEAR "\x1b[2J"
#endif

#endif
