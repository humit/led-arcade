#pragma once
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../session/PlayerManager.h"
#include "../games/pixel_derby/PixelDerbyGame.h"

class MatrixRenderer {
public:
  CRGB leds[LED_COUNT];
  void begin(){FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,LED_COUNT);FastLED.setBrightness(LED_BRIGHTNESS);FastLED.clear(true);}
  void render(const PlayerManager& players, PixelDerbyGame& game){
    FastLED.clear();
    switch(game.stage){
      case ArcadeStage::PLATFORM_SELECT: drawPlatformSelect(); break;
      case ArcadeStage::GAME_SELECT: drawGameSelect(game); break;
      case ArcadeStage::LOBBY: drawLobby(players,game); break;
      case ArcadeStage::ANNOUNCE: drawAnnouncement(game); break;
      case ArcadeStage::COUNTDOWN: drawCountdown(game.countdownValue); break;
      case ArcadeStage::RACING: game.selectedGame==GameId::TRON_ARENA?drawTron(players,game):drawRace(players); break;
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
  void drawPlatformSelect(){for(uint8_t x=0;x<MATRIX_WIDTH;x++)for(uint8_t y=0;y<MATRIX_HEIGHT;y++){CRGB c=((x+y)%3)?CRGB::Purple:CRGB::Blue;c.nscale8_video(x==(millis()/70)%MATRIX_WIDTH?180:25);set(x,y,c);}}
  void drawGameSelect(const PixelDerbyGame&g){for(uint8_t x=0;x<MATRIX_WIDTH;x++){set(x,2,CRGB(8,8,8));set(x,5,CRGB(8,8,8));}set((millis()/90)%MATRIX_WIDTH,2,CRGB::Blue);set(MATRIX_WIDTH-1-(millis()/90)%MATRIX_WIDTH,5,CRGB::DeepPink);}
  void drawLobby(const PlayerManager&p,const PixelDerbyGame&g){for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;CRGB c=playerColor(i);c.nscale8_video(x.ready?180:50);uint8_t row=displayRow(p,i);for(uint8_t xx=0;xx<(x.ready?8:3);xx++)set(xx,row,c);}}
  void drawAnnouncement(const PixelDerbyGame&g){if(g.announcePhase%2==0)drawText("BOSS",CRGB::Gold);else if(g.bossSlot>=0)drawText(colorName(g.bossSlot),playerColor(g.bossSlot));}
  void drawCountdown(uint8_t v){if(v==0){for(uint8_t x=0;x<MATRIX_WIDTH;x++)for(uint8_t y=0;y<MATRIX_HEIGHT;y++)set(x,y,CRGB::Green);}else{char t[2]={char('0'+v),0};drawText(t,CRGB::White);}}
  void drawRace(const PlayerManager&p){for(uint8_t y=0;y<MATRIX_HEIGHT;y++){set(FINISH_X,y,CRGB(45,45,45));set(TURBO_X_1,y,CRGB(55,34,0));set(TURBO_X_2,y,CRGB(55,34,0));}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;set(x.position,displayRow(p,i),x.turboTaps?CRGB::Gold:playerColor(i));}}
  void drawTron(const PlayerManager&p,const PixelDerbyGame&g){for(uint16_t idx=0;idx<LED_COUNT;idx++){uint8_t owner=g.tronTrail[idx];if(!owner)continue;uint8_t y=idx/MATRIX_WIDTH,x=idx%MATRIX_WIDTH;CRGB c=playerColor(owner-1);c.nscale8_video(90);set(x,y,c);}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(x.tronAlive)set(x.tronX,x.tronY,playerColor(i));}}
  void drawResult(const PlayerManager&p,const PixelDerbyGame&g){if(g.winner<0){drawText(DISPLAY_LANGUAGE_TR?"BERABERE":"DRAW",CRGB::White);return;}const char* word=DISPLAY_LANGUAGE_TR?"KAZANAN":"WINNER";if((millis()/900)%2==0)drawText(word,CRGB::White);else drawText(colorName(g.winner),playerColor(g.winner));}
  void drawBoss(const PlayerManager&p,const PixelDerbyGame&g){const uint8_t bs=25;if(g.bossSlot>=0){CRGB c=playerColor(g.bossSlot);c.nscale8_video((millis()/110)%2?230:110);for(uint8_t x=bs;x<MATRIX_WIDTH;x++)for(uint8_t y=1;y<7;y++)if(x==bs||x==31||y==1||y==6||((x+y+millis()/140)%3==0))set(x,y,c);set(27,0,CRGB::Gold);set(29,0,CRGB::Gold);set(31,0,CRGB::Gold);}uint8_t hp=g.bossMaxHp?uint8_t((uint32_t(g.bossHp)*23+g.bossMaxHp-1)/g.bossMaxHp):0;for(uint8_t x=0;x<23;x++)set(x,0,x<hp?CRGB::Red:CRGB(18,0,0));for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting||i==g.bossSlot)continue;uint8_t row=displayRow(p,i);CRGB c=g.stunRemainingMs(x)?CRGB::White:playerColor(i);set(0,row,c);set(2+((millis()/75+x.bossDamage*3)%21),row,c);}}
  void drawBossResult(const PlayerManager&p,const PixelDerbyGame&g){if(g.bossDefeated)drawText(DISPLAY_LANGUAGE_TR?"TAKIM":"TEAM",CRGB::Green);else if(g.bossSlot>=0)drawText(colorName(g.bossSlot),playerColor(g.bossSlot));}
};
