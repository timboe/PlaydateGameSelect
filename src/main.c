#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "game.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

#define VERSION "1.0"

void init(void) {

}

static void deinit(void) {
}

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
    case kEventInit:;
      #ifdef DEV
      playdate->system->logToConsole("EH: init %s", VERSION);
      #endif
      setPDPtr(playdate);
      init();
      playdate->display->setRefreshRate(50);
      playdate->system->setUpdateCallback(gameLoop, NULL);
      break;
    case kEventTerminate:; case kEventLock:; case kEventLowPower:;
      #ifdef DEV
      playdate->system->logToConsole("EH: terminate/lock/low-p, %i", event);
      #endif
      //
      //
      if (event == kEventTerminate) {
        deinit();
      }
      break;
    case kEventUnlock:;
      #ifdef DEV
      playdate->system->logToConsole("EH: unlock");
      #endif
      break;
    case kEventPause:;
      #ifdef DEV
      playdate->system->logToConsole("EH: pause");
      #endif
      break;
    case kEventResume:;
      #ifdef DEV
      playdate->system->logToConsole("EH: resume");
      #endif
      break;
    case kEventKeyPressed:;
      #ifdef DEV
      playdate->system->logToConsole("EH: pressed %i", arg);
      #endif
      break;
    case kEventKeyReleased:;
      #ifdef DEV
      playdate->system->logToConsole("EH: released %i", arg);
      #endif
      break;
    default:
      #ifdef DEV
      playdate->system->logToConsole("EH: unknown event %i with arg %i", event, arg);
      #endif
      break;
  }
  
  return 0;
}