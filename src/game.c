#include "game.h"

#ifdef SDL2API
  #include "../srcgame_ycgb/minimal.h"
  #include "../srcgame_cascada/minimal.h"
  #include "../srcgame_ff/minimal.h"
#endif

PlaydateAPI* pd = NULL;

int32_t m_fCount = 0;
int32_t m_animateOut = 0;

enum kGameType{
  kYCGB,
  kFactoryFarming,
  kCascada,
  kNGameTypes
};

enum kGameType m_selected = kYCGB;

LCDBitmap* m_catImg = NULL;
LCDBitmap* m_gameImg[kNGameTypes] = {NULL};
LCDBitmap* m_banner[kNGameTypes] = {NULL};
FilePlayer* m_tracks[kNGameTypes] = {NULL};
SamplePlayer* m_samplePlayerA = NULL;
SamplePlayer* m_samplePlayerB = NULL;
AudioSample* m_audioSampleA = NULL;
AudioSample* m_audioSampleB = NULL; 

////////////

void setPDPtr(PlaydateAPI* _p) {
  pd = _p;
}

///////////

#ifdef SDL2API

  void initSubgame(void) {
    if (m_selected == kYCGB) {

      initSprite_ycgb(pd);
      initSound_ycgb(pd);
      gameWindowLoad_ycgb();

      pd->display->setRefreshRate(20);
      pd->system->setUpdateCallback(gameLoop_ycgb, NULL);

    } else if (m_selected == kFactoryFarming) {
      initLocalisation();
      initSprite();
      initWorld();
      initMap();
      initCargo();
      initChunk();
      initLocation();
      initPlayer();
      initiUI();
      initBuilding();
      initSound();
      initGame();
      for (int32_t s = 0; s < N_SAVES; ++s) {
        setSave(s);
        scanSlots();
      }
      doIO(kDoTitle, /*and then*/ kDoNothing, /*and finally*/ kDoNothing);

      #define TICK_FREQUENCY 50
      pd->display->setRefreshRate(TICK_FREQUENCY);
      pd->system->setUpdateCallback(gameLoop_ff, NULL);

    } else if (m_selected == kCascada) {

      boardDoInit_cascada();
      bitmapDoInit_cascada();
      soundDoInit_cascada();
      physicsDoInit_cascada();
      inputDoInit_cascada();
      FSMDo_cascada(kTitlesFSM_DisplayTitles);

      #define TICK_FREQUENCY 50
      pd->display->setRefreshRate(TICK_FREQUENCY);
      pd->system->setUpdateCallback(gameLoop_cascada, NULL);

    }
  }

#endif

///////////

void chkErr(const char* _outErr) {
  if (_outErr != NULL) {
    pd->system->error("Error loading image: %s", _outErr);
  }
}

bool gameIsSelected(void) { return m_animateOut > DEVICE_PIX_X * 2; }

void renderGameSelect(void) {
  if (gameIsSelected()) return;

  pd->graphics->drawBitmap(m_gameImg[kYCGB], 1, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(m_gameImg[kFactoryFarming], 132 + 2, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(m_gameImg[kCascada], 132 + 132 + 3, 0, kBitmapUnflipped);

  const bool flash = (m_fCount % 50) < 25 && !m_animateOut;

  pd->graphics->setDrawMode((flash && m_selected == kYCGB) ? kDrawModeInverted : kDrawModeCopy);
  pd->graphics->drawBitmap(m_catImg, 1, 0, kBitmapUnflipped);
  pd->graphics->setDrawMode((flash && m_selected == kFactoryFarming) ? kDrawModeInverted : kDrawModeCopy);
  pd->graphics->drawBitmap(m_catImg, 132 + 2, 0, kBitmapUnflipped);
  pd->graphics->setDrawMode((flash && m_selected == kCascada) ? kDrawModeInverted : kDrawModeCopy);
  pd->graphics->drawBitmap(m_catImg, 132 + 132 + 3, 0, kBitmapUnflipped);
  pd->graphics->setDrawMode(kDrawModeCopy);

  switch (m_selected) {
    case kYCGB: pd->graphics->drawBitmap(m_banner[m_selected], 0, 16, kBitmapUnflipped); break;
    case kFactoryFarming: pd->graphics->drawBitmap(m_banner[m_selected], 0, 0, kBitmapUnflipped); break;
    case kCascada: pd->graphics->drawBitmap(m_banner[m_selected], 0, 16, kBitmapUnflipped); break;
    default:
  }

  if (m_animateOut) {
    pd->graphics->fillEllipse(DEVICE_PIX_X/2 - m_animateOut/2, DEVICE_PIX_Y/2 - m_animateOut/2, m_animateOut, m_animateOut, 0, 360.0f, kColorBlack);
    uint32_t inner = round( m_animateOut * 0.9f );
    pd->graphics->fillEllipse(DEVICE_PIX_X/2 - inner/2, DEVICE_PIX_Y/2 - inner/2, inner, inner, 0, 360.0f, kColorWhite);
  }
}

void stopS(void) {
  for (enum kGameType gt = kYCGB; gt < kNGameTypes; ++gt) pd->sound->fileplayer->stop(m_tracks[gt]);
}

void butL(void) {
  m_selected = (m_selected + 1) % kNGameTypes;
  stopS();
  pd->sound->fileplayer->play(m_tracks[m_selected], 0);
  pd->sound->sampleplayer->play(m_samplePlayerB, 1, 1.0f);
}

void butR(void) {
  m_selected = (!m_selected ? kNGameTypes-1 : m_selected - 1);
  stopS();
  pd->sound->fileplayer->play(m_tracks[m_selected], 0);
  pd->sound->sampleplayer->play(m_samplePlayerB, 1, 1.0f);
}

void butA(void) {
  ++m_animateOut;
  m_fCount = 0;
  stopS();
  pd->sound->sampleplayer->play(m_samplePlayerA, 1, 1.0f);
}

int gameLoop(void* _data) {

  if (gameIsSelected()) {
    static bool doneInit = false;
    #ifdef SDL2API
    if (!doneInit) {
      stopS();
      initSubgame();
    }
    #endif
    doneInit = true;
    return 1;
  }

  ++m_fCount;
  pd->graphics->setBackgroundColor(kColorWhite);

  if (!m_catImg) {
    const char* _outErr = NULL;
    m_catImg = pd->graphics->loadBitmap("images/game_select_frame.png", &_outErr); chkErr(_outErr);
    m_gameImg[kYCGB] = pd->graphics->loadBitmap("images/game_select_ycgb.png", &_outErr); chkErr(_outErr);
    m_gameImg[kFactoryFarming] = pd->graphics->loadBitmap("images/game_select_ff.png", &_outErr); chkErr(_outErr);
    m_gameImg[kCascada] = pd->graphics->loadBitmap("images/game_select_c.png", &_outErr); chkErr(_outErr);
    m_banner[kYCGB] = pd->graphics->loadBitmap("images/splash_ycgb.png", &_outErr); chkErr(_outErr);
    m_banner[kFactoryFarming] = pd->graphics->loadBitmap("images/splash_ff.png", &_outErr); chkErr(_outErr);
    m_banner[kCascada] = pd->graphics->loadBitmap("images/splash_c.png", &_outErr); chkErr(_outErr);

    m_tracks[kYCGB] = pd->sound->fileplayer->newPlayer();
    pd->sound->fileplayer->loadIntoPlayer(m_tracks[kYCGB], "sounds/8bitDungeonLevel");
    m_tracks[kFactoryFarming] = pd->sound->fileplayer->newPlayer();
    pd->sound->fileplayer->loadIntoPlayer(m_tracks[kFactoryFarming], "music/1985");
    m_tracks[kCascada] = pd->sound->fileplayer->newPlayer();
    pd->sound->fileplayer->loadIntoPlayer(m_tracks[kCascada], "tracks/690224__nox_sound__ambiance_waterfall_big_skogafoss_loop_stereo");

    m_audioSampleA = pd->sound->sample->load("sounds/a");
    m_audioSampleB = pd->sound->sample->load("sounds/b");
    m_samplePlayerA = pd->sound->sampleplayer->newPlayer();
    m_samplePlayerB = pd->sound->sampleplayer->newPlayer();
    pd->sound->sampleplayer->setSample(m_samplePlayerA, m_audioSampleA);
    pd->sound->sampleplayer->setSample(m_samplePlayerB, m_audioSampleB);

    pd->sound->fileplayer->play(m_tracks[kYCGB], 0);
  }

  renderGameSelect();

  if (!m_animateOut) {
    PDButtons current, pushed, released = 0;
    pd->system->getButtonState(&current, &pushed, &released);
    if (pushed & kButtonRight) butL();
    if (pushed & kButtonLeft) butR();
    if (pushed & kButtonA) butA();
  } else {
    m_animateOut += m_fCount;
  }

  return 1;
}


void processPause(void) {
  if (!gameIsSelected()) return;

  #ifdef SDL2API
    if (m_selected == kFactoryFarming) {
      pd->system->setMenuImage(getPauseImage(), 0);
    }
  #endif

}

void processTerminate(void) {

  if (!gameIsSelected()) return;

  #ifdef SDL2API
    if (m_selected == kFactoryFarming) {
      if (getGameMode() != kTitles && !IOOperationInProgress()) {
        synchronousSave();
      }
    } else if (m_selected == kCascada) {
      IODoSave();
    }
  #endif

}