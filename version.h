#pragma once

#define APP_NAME "FrameFLow"

#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0

#define APP_MODULE  APP_NAME
#define APP_PRODUCT "FrameFlow"
#define APP_COMPANY "AVIO"
#define APP_YEAR    "2026"

#define APP__makestr_aux(y) #y
#define APP__makestr(x)     APP__makestr_aux(x)
#define APP_VERSION         APP__makestr(APP_VERSION_MAJOR) "." \
                            APP__makestr(APP_VERSION_MINOR) "." \
                            APP__makestr(APP_VERSION_PATCH)

#define APP_VERSION_STR     APP_VERSION ".1"
#define APP_VERSION_NUM     APP_VERSION_MAJOR,   \
                            APP_VERSION_MINOR,   \
                            APP_VERSION_PATCH,   \
                            1
