#pragma once
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../session/PlayerManager.h"
#include "../games/pixel_derby/PixelDerbyGame.h"
#include "../presentation/ArcadeDirector.h"
#include "../presentation/SpriteRenderer.h"
#include "../assets/sprites/ArcadeSprites.h"

class MatrixRenderer {
public:
  CRGB leds[LED_COUNT];
  void begin(){FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,LED_COUNT);FastLED.setBrightness(LED_BRIGHTNESS);clearPixels();FastLED.show();}
  void clearPixels(){fill_solid(leds,LED_COUNT,CRGB::Black);}
  void render(const PlayerManager& players, PixelDerbyGame& game, const ArcadeDirector& director){
    clearPixels();
    if (drawPresentationCue(game, director)) { FastLED.show(); return; }
    switch(game.stage){
      case ArcadeStage::PLATFORM_SELECT: drawPlatformSelect(); break;
      case ArcadeStage::GAME_SELECT: drawGameSelect(game); break;
      case ArcadeStage::LOBBY: drawLobby(players,game); break;
      case ArcadeStage::ANNOUNCE: drawAnnouncement(game); break;
      case ArcadeStage::COUNTDOWN: drawCountdown(game.countdownValue); break;
      case ArcadeStage::RACING:
        if(game.selectedGame==GameId::TRON_ARENA) drawTron(players,game);
        else if(game.selectedGame==GameId::PIXEL_RAIDER) drawRaider(game);
        else if(game.selectedGame==GameId::COLOR_CLASH) drawClash(players,game);
        else if(game.selectedGame==GameId::PIXEL_PONG) drawPong(players,game);
        else drawRace(players);
        break;
      case ArcadeStage::RESULT: drawResult(players,game); break;
      case ArcadeStage::BOSS: drawBoss(players,game); break;
      case ArcadeStage::BOSS_RESULT: drawBossResult(players,game); break;
    }
    FastLED.show();
  }
private:
  uint16_t xy(uint8_t x,uint8_t y)const{return (x&1)==0?x*MATRIX_HEIGHT+y:x*MATRIX_HEIGHT+(MATRIX_HEIGHT-1-y);}
  void set(int x,int y,CRGB c){if(x>=0&&x<MATRIX_WIDTH&&y>=0&&y<MATRIX_HEIGHT)leds[xy(x,y)]=c;}
  CRGB playerColor(uint8_t slot)const{static const CRGB c[MAX_PLAYERS]={CRGB::Blue,CRGB::DeepPink,CRGB::Green,CRGB::Yellow,CRGB::Cyan,CRGB::Orange,CRGB::Purple,CRGB::White};return c[slot%MAX_PLAYERS];}
  const char* colorName(uint8_t slot)const{static const char* tr[MAX_PLAYERS]={"MAVI","PEMBE","YESIL","SARI","TURKUAZ","TURUNCU","MOR","BEYAZ"};static const char* en[MAX_PLAYERS]={"BLUE","PINK","GREEN","YELLOW","CYAN","ORANGE","PURPLE","WHITE"};return DISPLAY_LANGUAGE_TR?tr[slot%MAX_PLAYERS]:en[slot%MAX_PLAYERS];}
  uint8_t glyph(char ch,uint8_t row)const{
    ch=toupper(ch); const char* p=nullptr;
    switch(ch){
      case 'A':p="010101111101101";break;case 'B':p="110101110101110";break;case 'C':p="011100100100011";break;case 'D':p="110101101101110";break;
      case 'E':p="111100110100111";break;case 'F':p="111100110100100";break;case 'G':p="011100101101011";break;case 'H':p="101101111101101";break;
      case 'I':p="111010010010111";break;case 'J':p="001001001101010";break;case 'K':p="101101110101101";break;case 'L':p="100100100100111";break;
      case 'M':p="101111111101101";break;case 'N':p="101111111111101";break;case 'O':p="010101101101010";break;case 'P':p="110101110100100";break;
      case 'R':p="110101110101101";break;case 'S':p="011100010001110";break;case 'T':p="111010010010010";break;case 'U':p="101101101101111";break;
      case 'V':p="101101101101010";break;case 'W':p="101101111111101";break;case 'Y':p="101101010010010";break;case 'Z':p="111001010100111";break;
      case '1':p="010110010010111";break;case '2':p="110001010100111";break;case '3':p="110001010001110";break;default:p="000000000000000";break;
    }
    uint8_t bits=0;for(uint8_t col=0;col<3;col++)if(p[row*3+col]=='1')bits|=(1<<(2-col));return bits;
  }
  void drawText(const char* text,CRGB color){
    const int len=strlen(text);const int width=len*4-1;int ox=(MATRIX_WIDTH-width)/2; if(ox<0)ox=0;
    for(int i=0;i<len;i++)for(uint8_t r=0;r<5;r++){uint8_t bits=glyph(text[i],r);for(uint8_t c=0;c<3;c++)if(bits&(1<<(2-c)))set(ox+i*4+c,1+r,color);}
  }
  uint8_t activeCount(const PlayerManager&p)const{uint8_t c=0;for(uint8_t i=0;i<MAX_PLAYERS;i++)if(p.players[i].occupied&&p.players[i].connected&&!p.players[i].waiting)c++;return c;}
  int8_t activeOrder(const PlayerManager&p,uint8_t slot)const{int8_t o=0;for(uint8_t i=0;i<MAX_PLAYERS;i++){if(!p.players[i].occupied||!p.players[i].connected||p.players[i].waiting)continue;if(i==slot)return o;o++;}return -1;}
  uint8_t displayRow(const PlayerManager&p,uint8_t slot)const{static const uint8_t l[MAX_PLAYERS][MAX_PLAYERS]={{3,3,3,3,3,3,3,3},{2,5,5,5,5,5,5,5},{1,4,6,6,6,6,6,6},{0,2,5,7,7,7,7,7},{0,2,3,5,7,7,7,7},{0,1,3,4,6,7,7,7},{0,1,2,4,5,6,7,7},{0,1,2,3,4,5,6,7}};uint8_t c=activeCount(p);int8_t o=activeOrder(p,slot);return(c==0||o<0)?3:l[c-1][o];}
  static void spriteSet(void* ctx,int x,int y,CRGB c){static_cast<MatrixRenderer*>(ctx)->set(x,y,c);}
  bool drawPresentationCue(const PixelDerbyGame& g,const ArcadeDirector& d){
    using Cue=ArcadeDirector::VisualCue;
    const uint32_t age=d.cueAgeMs();
    if(g.stage==ArcadeStage::PLATFORM_SELECT||g.stage==ArcadeStage::GAME_SELECT){
      if(d.isAttractRunning()){
        if(d.cue()==Cue::ATTRACT_CHOMPER)drawAttract(age);
        else if(d.cue()==Cue::ATTRACT_DERBY_DEMO)drawDerbyDemo(age);
        else if(d.cue()==Cue::ATTRACT_RAIDER_DEMO)drawRaiderDemo(age);
        else if(d.cue()==Cue::ATTRACT_PAINT_DEMO)drawPaintDemo(age);
        else if(d.cue()==Cue::ATTRACT_JOIN)drawJoinMessage(age);
        else drawAttract(age);
      } else drawAttract(age);
      return true;
    }
    if(g.stage==ArcadeStage::LOBBY&&d.cueActive(1100)){
      if(d.cue()==Cue::DERBY)drawRacingTeaser(age);
      else if(d.cue()==Cue::RAIDER)drawShipTeaser(age);
      else if(d.cue()==Cue::PAINT)drawPaintTeaser(age);
      else if(d.cue()==Cue::PONG)drawPongTeaser(age);
      else return false;
      return true;
    }
    return false;
  }
  void drawAttract(uint32_t age){
    const int travel=MATRIX_WIDTH+10;const int x=int(age/90%travel)-8;
    for(uint8_t px=0;px<MATRIX_WIDTH;px+=4)set(px,4,CRGB(16,12,2));
    const bool open=((age/180)%2)==0;
    SpriteRenderer::drawRows(this,spriteSet,x,1,open?CHOMPER_OPEN:CHOMPER_CLOSED,6,CRGB::Yellow);
    SpriteRenderer::drawRows(this,spriteSet,x-10,2,GHOST,5,CRGB::DeepPink);
  }
  void drawRacingTeaser(uint32_t age){
    const int x=int(age/45)-8;
    for(uint8_t px=0;px<MATRIX_WIDTH;px++){set(px,6,CRGB(12,12,12));if((px+age/80)%5==0)set(px,5,CRGB(35,35,35));}
    SpriteRenderer::drawRows(this,spriteSet,x,2,CAR,4,CRGB::Blue);
    SpriteRenderer::drawRows(this,spriteSet,x-9,2,CAR,4,CRGB::DeepPink);
  }
  void drawShipTeaser(uint32_t age){
    const int x=int(age/38)-7;
    SpriteRenderer::drawRows(this,spriteSet,x,1,SHIP,5,CRGB::Cyan);
    for(int b=x+7;b<MATRIX_WIDTH;b+=6)set(b,3,CRGB::Orange);
    const int ex=25-int(age/70)%8;set(ex,2,CRGB::Red);set(ex,3,CRGB::Red);set(ex,4,CRGB::Red);
  }
  void drawPaintTeaser(uint32_t age){
    const int split=min<int>(MATRIX_WIDTH,int(age/30));
    for(int x=0;x<MATRIX_WIDTH;x++)for(int y=0;y<MATRIX_HEIGHT;y++){
      if(x<split){CRGB c=((x+y)&1)?CRGB::Blue:CRGB::DeepPink;c.nscale8_video(55+((x*13+y*17)%70));set(x,y,c);}
    }
    const int cx=min<int>(MATRIX_WIDTH-1,split);for(int dy=-2;dy<=2;dy++)set(cx,3+dy,dy&1?CRGB::White:CRGB::Gold);
  }
  void drawPongTeaser(uint32_t age){
    const int y=1+int((age/170)%6);
    for(uint8_t i=0;i<PONG_PADDLE_HEIGHT;i++){set(PONG_LEFT_X,2+i,CRGB::Blue);set(PONG_RIGHT_X,3+i,CRGB::DeepPink);}
    for(uint8_t yy=0;yy<MATRIX_HEIGHT;yy+=2){set(15,yy,CRGB(12,12,18));set(16,yy,CRGB(12,12,18));}
    const int travel=2*(PONG_RIGHT_X-PONG_LEFT_X-2);
    int phase=int((age/70)%travel);
    int x=phase<(travel/2)?PONG_LEFT_X+2+phase:PONG_RIGHT_X-2-(phase-travel/2);
    set(x,y,CRGB::White);
  }
  void drawDerbyDemo(uint32_t age){
    for(uint8_t y=1;y<7;y+=4)for(uint8_t x=0;x<MATRIX_WIDTH;x++)set(x,y,CRGB(4,4,8));
    const uint8_t a=(age/85)%MATRIX_WIDTH;
    const uint8_t b=(age/110+7)%MATRIX_WIDTH;
    set(a,2,CRGB::Blue); if(a>0)set(a-1,2,CRGB(0,0,45));
    set(b,5,CRGB::DeepPink); if(b>0)set(b-1,5,CRGB(45,0,20));
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++)set(31,y,((age/150+y)&1)?CRGB::White:CRGB(25,25,25));
  }
  void drawRaiderDemo(uint32_t age){
    const int shipY=2+int((age/420)%4);
    set(3,shipY,CRGB::Cyan);set(2,shipY,CRGB(0,50,60));
    for(int x=6+int((age/70)%5);x<MATRIX_WIDTH;x+=7)set(x,shipY,CRGB::Orange);
    const int enemyX=30-int((age/95)%24);
    set(enemyX,1,CRGB::Red);set(enemyX,2,CRGB(75,0,0));
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++)if(y<shipY-1||y>shipY+1)set(31,y,CRGB(10,25,45));
  }
  void drawPaintDemo(uint32_t age){
    const uint8_t split=(age/75)%MATRIX_WIDTH;
    for(uint8_t x=0;x<MATRIX_WIDTH;x++)for(uint8_t y=0;y<MATRIX_HEIGHT;y++){
      CRGB c=x<split?CRGB::Blue:CRGB::DeepPink;
      c.nscale8_video(((x+y+age/120)&1)?90:45);set(x,y,c);
    }
    set(split,2,CRGB::White);set((MATRIX_WIDTH-1-split),5,CRGB::White);
  }
  void drawTextAt(const char* text,int ox,CRGB color){
    const int len=strlen(text);
    for(int i=0;i<len;i++)for(uint8_t r=0;r<5;r++){
      uint8_t bits=glyph(text[i],r);
      for(uint8_t c=0;c<3;c++)if(bits&(1<<(2-c)))set(ox+i*4+c,1+r,color);
    }
  }
  void drawJoinMessage(uint32_t age){
    const char* text=DISPLAY_LANGUAGE_TR?"OYUNA KATIL":"JOIN GAME";
    const int width=int(strlen(text))*4-1;
    const int travel=MATRIX_WIDTH+width+4;
    const int ox=MATRIX_WIDTH-int((age/80)%travel);
    drawTextAt(text,ox,CRGB::Gold);
    const uint8_t pulse=80+uint8_t((age/8)%150);
    CRGB wifi(0,pulse,pulse);
    set(0,6,wifi);set(1,5,wifi);set(2,4,wifi);set(3,5,wifi);set(4,6,wifi);
  }
  void drawPlatformSelect(){drawAttract(millis());}
  void drawGameSelect(const PixelDerbyGame&g){drawAttract(millis());}
  void drawLobby(const PlayerManager&p,const PixelDerbyGame&g){for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;CRGB c=playerColor(i);c.nscale8_video(x.ready?180:50);uint8_t row=displayRow(p,i);for(uint8_t xx=0;xx<(x.ready?8:3);xx++)set(xx,row,c);}}
  void drawAnnouncement(const PixelDerbyGame&g){if(g.announcePhase%2==0)drawText("BOSS",CRGB::Gold);else if(g.bossSlot>=0)drawText(colorName(g.bossSlot),playerColor(g.bossSlot));}
  void drawCountdown(uint8_t v){if(v==0){for(uint8_t x=0;x<MATRIX_WIDTH;x++)for(uint8_t y=0;y<MATRIX_HEIGHT;y++)set(x,y,CRGB::Green);}else{char t[2]={char('0'+v),0};drawText(t,CRGB::White);}}
  void drawRace(const PlayerManager&p){for(uint8_t y=0;y<MATRIX_HEIGHT;y++){set(FINISH_X,y,CRGB(45,45,45));set(TURBO_X_1,y,CRGB(55,34,0));set(TURBO_X_2,y,CRGB(55,34,0));}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;set(x.position,displayRow(p,i),x.turboTaps?CRGB::Gold:playerColor(i));}}
  void drawTron(const PlayerManager&p,const PixelDerbyGame&g){for(uint16_t idx=0;idx<LED_COUNT;idx++){uint8_t owner=g.tronTrail[idx];if(!owner)continue;uint8_t y=idx/MATRIX_WIDTH,x=idx%MATRIX_WIDTH;CRGB c=playerColor(owner-1);c.nscale8_video(90);set(x,y,c);}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(x.tronAlive)set(x.tronX,x.tronY,playerColor(i));}}
  void drawClash(const PlayerManager& p,const PixelDerbyGame& g){
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++)for(uint8_t x=0;x<MATRIX_WIDTH;x++){
      const uint8_t owner=g.clashPaint[y*MATRIX_WIDTH+x];
      if(owner){CRGB c=playerColor(owner-1);c.nscale8_video(75);set(x,y,c);}else set(x,y,CRGB(2,2,4));
    }
    for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto& pl=p.players[i];if(!pl.occupied||!pl.connected||pl.waiting||pl.clashX<0)continue;set(pl.clashX,pl.clashY,CRGB::White);}
    const uint32_t remain=g.clashRemainingMs();
    if(remain>0&&remain<=5000){const uint8_t lights=(remain+999)/1000;for(uint8_t i=0;i<5;i++)set(27+i,0,i<lights?CRGB::White:CRGB(12,12,12));}
  }
  void drawPong(const PlayerManager& p,const PixelDerbyGame& g){
    const PixelPongGame& pong=g.pong;
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++){
      if((y&1)==0){set(15,y,CRGB(10,10,16));set(16,y,CRGB(10,10,16));}
    }
    if(pong.leftSlot>=0&&pong.leftSlot<MAX_PLAYERS){
      CRGB c=playerColor(pong.leftSlot);if(p.players[pong.leftSlot].isCpu)c.nscale8_video(150);
      for(uint8_t i=0;i<PONG_PADDLE_HEIGHT;i++)set(PONG_LEFT_X,pong.leftPaddleY+i,c);
    }
    if(pong.rightSlot>=0&&pong.rightSlot<MAX_PLAYERS){
      CRGB c=playerColor(pong.rightSlot);if(p.players[pong.rightSlot].isCpu)c.nscale8_video(150);
      for(uint8_t i=0;i<PONG_PADDLE_HEIGHT;i++)set(PONG_RIGHT_X,pong.rightPaddleY+i,c);
    }
    CRGB ball=pong.pointPause?CRGB::Gold:CRGB::White;
    if(!pong.pointPause)set(pong.ballX-pong.ballDx,pong.ballY,CRGB(35,35,45));
    set(pong.ballX,pong.ballY,ball);
    for(uint8_t i=0;i<PONG_SCORE_TO_WIN;i++){
      set(5+i,0,i<pong.leftScore?playerColor(pong.leftSlot):CRGB(8,8,12));
      set(26-i,0,i<pong.rightScore?playerColor(pong.rightSlot):CRGB(8,8,12));
    }
  }
  void drawRaider(const PixelDerbyGame&g){
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++)for(uint8_t x=0;x<MATRIX_WIDTH;x++){
      uint8_t cell=g.raiderCells[y*MATRIX_WIDTH+x];
      if(cell==1)set(x,y,CRGB(20,40,70));
      else if(cell==2)set(x,y,CRGB::Red);
      else if(cell==3)set(x,y,CRGB::Yellow);
      else if(cell==4)set(x,y,CRGB::Purple);
      else if(cell==5)set(x,y,CRGB::White);
    }
    for(uint8_t i=0;i<RAIDER_MAX_BULLETS;i++)if(g.raiderBulletX[i]>=0)set(g.raiderBulletX[i],g.raiderBulletY[i],CRGB::Orange);
    CRGB ship=g.raiderShield?CRGB::White:CRGB::Cyan;
    set(RAIDER_PLAYER_X,g.raiderPlayerY,ship); set(RAIDER_PLAYER_X-1,g.raiderPlayerY,ship);
  }
  void drawResult(const PlayerManager&p,const PixelDerbyGame&g){if(g.selectedGame==GameId::PIXEL_RAIDER){if(g.raiderNewRecord){drawText(DISPLAY_LANGUAGE_TR?"REKOR":"RECORD",CRGB::Gold);}else{drawText(((millis()/900)%2)==0?"GAME":"OVER",CRGB::Red);}return;}if(g.selectedGame==GameId::COLOR_CLASH){if(g.winner<0){drawText(DISPLAY_LANGUAGE_TR?"BERABERE":"DRAW",CRGB::White);return;}const char* word=DISPLAY_LANGUAGE_TR?"KAZANAN":"WINNER";if((millis()/900)%2==0)drawText(word,CRGB::White);else drawText(colorName(g.winner),playerColor(g.winner));return;}if(g.winner<0){drawText(DISPLAY_LANGUAGE_TR?"BERABERE":"DRAW",CRGB::White);return;}const char* word=DISPLAY_LANGUAGE_TR?"KAZANAN":"WINNER";if((millis()/900)%2==0)drawText(word,CRGB::White);else drawText(colorName(g.winner),playerColor(g.winner));}
  void drawBoss(const PlayerManager&p,const PixelDerbyGame&g){const uint8_t bs=25;if(g.bossSlot>=0){CRGB c=playerColor(g.bossSlot);c.nscale8_video((millis()/110)%2?230:110);for(uint8_t x=bs;x<MATRIX_WIDTH;x++)for(uint8_t y=1;y<7;y++)if(x==bs||x==31||y==1||y==6||((x+y+millis()/140)%3==0))set(x,y,c);set(27,0,CRGB::Gold);set(29,0,CRGB::Gold);set(31,0,CRGB::Gold);}uint8_t hp=g.bossMaxHp?uint8_t((uint32_t(g.bossHp)*23+g.bossMaxHp-1)/g.bossMaxHp):0;for(uint8_t x=0;x<23;x++)set(x,0,x<hp?CRGB::Red:CRGB(18,0,0));for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting||i==g.bossSlot)continue;uint8_t row=displayRow(p,i);CRGB c=g.stunRemainingMs(x)?CRGB::White:playerColor(i);set(0,row,c);set(2+((millis()/75+x.bossDamage*3)%21),row,c);}}
  void drawBossResult(const PlayerManager&p,const PixelDerbyGame&g){if(g.bossDefeated)drawText(DISPLAY_LANGUAGE_TR?"TAKIM":"TEAM",CRGB::Green);else if(g.bossSlot>=0)drawText(colorName(g.bossSlot),playerColor(g.bossSlot));}
};
