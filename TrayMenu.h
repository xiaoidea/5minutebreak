// Copyright (c) 2012-2013 Czarek Tomczak. All rights reserved.
// License: New BSD License.
// Project website: http://code.google.com/p/5minutebreak/

#pragma once

#include <Windows.h>

// This enumeration must be global, as TRAYMENU_EXIT is used in WinMain().
enum
{
    WM_FIRST = WM_APP,
    WM_TRAYICON,
    TRAYMENU_EXIT,
    TRAYMENU_STARTUP,
    TRAYMENU_HELP,
    TRAYMENU_TIMER, // "Alarm is off" or "Alarm in 20 minutes"
    TRAYMENU_ALARM_SOUND,
    TRAYMENU_5_MINUTES,
    TRAYMENU_10_MINUTES,
    TRAYMENU_15_MINUTES,
    TRAYMENU_20_MINUTES,
    TRAYMENU_25_MINUTES,
    TRAYMENU_30_MINUTES,
    TRAYMENU_45_MINUTES,
    TRAYMENU_1_HOUR,
    TRAYMENU_2_HOURS,
    TRAYMENU_3_HOURS,
    TRAYMENU_4_HOURS,
    // -- language menu
    LANGMENU_ENGLISH,
    LANGMENU_POLISH,
    // -- subtimer menu
    DEFAULTTIMER_4_HOURS,
    DEFAULTTIMER_3_HOURS,
    DEFAULTTIMER_2_HOURS,
    DEFAULTTIMER_1_HOUR,
    DEFAULTTIMER_45_MINUTES,
    DEFAULTTIMER_30_MINUTES,
    DEFAULTTIMER_25_MINUTES,
    DEFAULTTIMER_20_MINUTES,
    DEFAULTTIMER_15_MINUTES,
    // -- play sound menu
    ALARM_SOUND_DEFAULT,
    ALARM_SOUND_ANNOYING,
    ALARM_SOUND_DISABLED
};
