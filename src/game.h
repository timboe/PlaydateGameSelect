#pragma once

#include <stdio.h>
#include <stdbool.h> 

#include "pd_api.h"

#define DEVICE_PIX_X 400
#define DEVICE_PIX_Y 240

extern PlaydateAPI* pd;

void setPDPtr(PlaydateAPI* _p);

int gameLoop(void* _data);