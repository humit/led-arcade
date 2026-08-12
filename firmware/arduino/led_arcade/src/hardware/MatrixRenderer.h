#pragma once
#include <FastLED.h>
#include "../Config.h"
#include "../Types.h"
#include "../session/PlayerManager.h"
#include "../core/ArcadeGameEngine.h"
#include "../presentation/ArcadeDirector.h"
#include "../presentation/SpriteRenderer.h"
#include "../assets/sprites/ArcadeSprites.h"

class MatrixRenderer {
public:
  CRGB leds[LED_COUNT];
  void begin(){FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,LED_COUNT);FastLED.setBrightness(LED_BRIGHTNESS);clearPixels();FastLED.show();}
  void clearPixels(){fill_solid(leds,LED_COUNT,CRGB::Black);}
  void render(const PlayerManager& players, ArcadeGameEngine& game, const ArcadeDirector& director){
    // Stack Shift uses true black as part of the playfield language. Temporal
    // dithering can make very dim pixels appear to shimmer, so disable it for
    // the portrait game and restore the FastLED default for the other games.
    FastLED.setDither(game.selectedGame==GameId::STACK_SHIFT?0:1);
    clearPixels();
    if (drawPresentationCue(game, director)) {
      if (game.selectedGame == GameId::STACK_SHIFT) sanitizeStackPixels(game);
      FastLED.show();
      return;
    }
    switch(game.stage){
      case ArcadeStage::PLATFORM_SELECT: drawPlatformSelect(); break;
      case ArcadeStage::GAME_SELECT: drawGameSelect(game); break;
      case ArcadeStage::LOBBY: drawLobby(players,game); break;
      case ArcadeStage::ANNOUNCE: drawAnnouncement(game); break;
      case ArcadeStage::COUNTDOWN: drawCountdown(game); break;
      case ArcadeStage::RACING:
        if(game.selectedGame==GameId::TRON_ARENA) drawTron(players,game);
        else if(game.selectedGame==GameId::PIXEL_RAIDER) drawRaider(game);
        else if(game.selectedGame==GameId::COLOR_CLASH) drawClash(players,game);
        else if(game.selectedGame==GameId::PIXEL_PONG) drawPong(players,game);
        else if(game.selectedGame==GameId::STACK_SHIFT) drawStack(game);
        else drawRace(players);
        break;
      case ArcadeStage::RESULT: drawResult(players,game); break;
      case ArcadeStage::BOSS: drawBoss(players,game); break;
      case ArcadeStage::BOSS_RESULT: drawBossResult(players,game); break;
    }
    if (game.selectedGame == GameId::STACK_SHIFT) sanitizeStackPixels(game);
    FastLED.show();
  }
private:
  uint16_t xy(uint8_t x,uint8_t y)const{return (x&1)==0?x*MATRIX_HEIGHT+y:x*MATRIX_HEIGHT+(MATRIX_HEIGHT-1-y);}
  void set(int x,int y,CRGB c){if(x>=0&&x<MATRIX_WIDTH&&y>=0&&y<MATRIX_HEIGHT)leds[xy(x,y)]=c;}
  void setOriented(DisplayOrientation orientation,int x,int y,CRGB color){
    if(orientation==DisplayOrientation::HORIZONTAL){set(x,y,color);return;}
    if(x<0||x>=MATRIX_HEIGHT||y<0||y>=MATRIX_WIDTH)return;
    if(orientation==DisplayOrientation::VERTICAL_CLOCKWISE)set(y,MATRIX_HEIGHT-1-x,color);
    else set(MATRIX_WIDTH-1-y,x,color);
  }
  void setStack(int virtualX,int virtualY,CRGB color){
    if(virtualX<0||virtualX>=STACK_WIDTH||virtualY<0||virtualY>=STACK_HEIGHT)return;
    setOriented(gameOrientation(GameId::STACK_SHIFT),virtualX,virtualY,color);
  }
  CRGB stackColor(uint8_t piece)const{
    // Avoid CSS-style named colors such as Green and Purple: in FastLED those
    // are intentionally half-intensity (0x008000 / 0x800080). Stack Shift
    // uses a saturated palette so every gameplay cell has a 255 peak channel.
    static const CRGB colors[7]={
      CRGB(0,255,255), CRGB(255,255,0), CRGB(190,0,255),
      CRGB(0,255,70), CRGB(255,0,35), CRGB(0,90,255), CRGB(255,120,0)
    };
    return piece>=1&&piece<=7?colors[piece-1]:CRGB::Black;
  }
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
      case '0':p="111101101101111";break;case '1':p="010110010010111";break;case '2':p="110001010100111";break;case '3':p="110001010001110";break;
      case '4':p="101101111001001";break;case '5':p="111100110001110";break;case '6':p="011100111101111";break;case '7':p="111001010010010";break;
      case '8':p="111101111101111";break;case '9':p="111101111001110";break;case '+':p="000010111010000";break;default:p="000000000000000";break;
    }
    uint8_t bits=0;for(uint8_t col=0;col<3;col++)if(p[row*3+col]=='1')bits|=(1<<(2-col));return bits;
  }
  void drawText(const char* text,CRGB color){
    const int len=strlen(text);const int width=len*4-1;int ox=(MATRIX_WIDTH-width)/2; if(ox<0)ox=0;
    for(int i=0;i<len;i++)for(uint8_t r=0;r<5;r++){uint8_t bits=glyph(text[i],r);for(uint8_t c=0;c<3;c++)if(bits&(1<<(2-c)))set(ox+i*4+c,1+r,color);}
  }
  void drawVerticalText(DisplayOrientation orientation,const char* text,CRGB color){
    char compact[24]={0};uint8_t len=0;
    for(uint8_t i=0;text[i]&&len<sizeof(compact)-1;i++)if(text[i]!=' ')compact[len++]=text[i];
    if(len==0)return;
    if(len<=2){
      const int width=int(len)*4-1;
      const int ox=(MATRIX_HEIGHT-width)/2;
      const int oy=(MATRIX_WIDTH-5)/2;
      for(uint8_t i=0;i<len;i++)for(uint8_t r=0;r<5;r++){
        const uint8_t bits=glyph(compact[i],r);
        for(uint8_t c=0;c<3;c++)if(bits&(1<<(2-c)))setOriented(orientation,ox+i*4+c,oy+r,color);
      }
      return;
    }
    const uint8_t perPage=6;
    const uint8_t pages=(len+perPage-1)/perPage;
    const uint8_t page=pages>1?uint8_t((millis()/900)%pages):0;
    const uint8_t start=page*perPage;
    const uint8_t remaining=len-start;
    const uint8_t count=remaining<perPage?remaining:perPage;
    const int ox=(MATRIX_HEIGHT-3)/2;
    const int oy=(MATRIX_WIDTH-int(count)*5)/2;
    for(uint8_t i=0;i<count;i++)for(uint8_t r=0;r<5;r++){
      const uint8_t bits=glyph(compact[start+i],r);
      for(uint8_t c=0;c<3;c++)if(bits&(1<<(2-c)))setOriented(orientation,ox+c,oy+i*5+r,color);
    }
  }
  void drawTextOriented(GameId game,const char* text,CRGB color){
    if(gameOrientation(game)==DisplayOrientation::HORIZONTAL)drawText(text,color);
    else drawVerticalText(gameOrientation(game),text,color);
  }
  void drawPortraitGlyph(char ch,int top,CRGB color,uint8_t scale=1){
    const int glyphWidth=3*scale;
    const int left=(STACK_WIDTH-glyphWidth)/2;
    for(uint8_t r=0;r<5;r++){
      const uint8_t bits=glyph(ch,r);
      for(uint8_t c=0;c<3;c++){
        if(!(bits&(1<<(2-c))))continue;
        for(uint8_t sy=0;sy<scale;sy++)for(uint8_t sx=0;sx<scale;sx++)
          setStack(left+c*scale+sx,top+r*scale+sy,color);
      }
    }
  }
  void drawPortraitNumber(uint16_t value,CRGB color){
    char text[6];snprintf(text,sizeof(text),"%u",value);
    const uint8_t len=strlen(text);
    const uint8_t scale=len<=2?2:1;
    const int digitHeight=5*scale;
    const int gap=scale+1;
    const int totalHeight=int(len)*digitHeight+int(len-1)*gap;
    int top=(STACK_HEIGHT-totalHeight)/2;
    for(uint8_t i=0;i<len;i++){
      drawPortraitGlyph(text[i],top,color,scale);
      top+=digitHeight+gap;
    }
  }
  void drawPortraitGo(uint32_t age){
    const uint8_t phase=(age/85)%10;
    for(uint8_t band=0;band<3;band++){
      const int center=STACK_HEIGHT-4-int(phase)-int(band)*8;
      CRGB color=CRGB::Green;color.nscale8_video(uint8_t(255-band*60));
      for(int d=0;d<4;d++){
        setStack(3-d/2,center+d,color);
        setStack(4+d/2,center+d,color);
      }
    }
  }
  void drawStackLockedBoard(const StackShiftGame& stack){
    for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++){
      const uint8_t piece=stack.cell(x,y);
      if(piece)setStack(x,y,stackColor(piece));
    }
  }
  void drawStackPerfectClear(const StackShiftGame& stack){
    clearStackPixels();
    const uint32_t age=stack.perfectClearAgeMs(millis());
    static const CRGB rainbow[7]={CRGB::Red,CRGB::Orange,CRGB::Yellow,CRGB::Green,CRGB::Cyan,CRGB::Blue,CRGB::Purple};
    if(age<1500){
      const uint8_t travel=(age/55)%(STACK_HEIGHT+8);
      for(uint8_t y=0;y<STACK_HEIGHT;y++){
        const uint8_t distance=travel>y?travel-y:y-travel;
        if(distance>3)continue;
        CRGB color=rainbow[(y+age/110)%7];
        if(distance>0)color.nscale8_video(uint8_t(255-distance*45));
        for(uint8_t x=0;x<STACK_WIDTH;x++)setStack(x,y,color);
      }
      return;
    }
    const bool pulse=((age/130)&1)==0;
    const CRGB gold=pulse?CRGB::Gold:CRGB::White;
    for(uint8_t x=0;x<STACK_WIDTH;x++){setStack(x,0,gold);setStack(x,STACK_HEIGHT-1,gold);}
    for(uint8_t y=0;y<STACK_HEIGHT;y++){setStack(0,y,gold);setStack(STACK_WIDTH-1,y,gold);}
    for(uint8_t d=0;d<4;d++){
      setStack(3-d/2,14+d,gold);
      setStack(4+d/2,14+d,gold);
      setStack(3-d/2,18-d,gold);
      setStack(4+d/2,18-d,gold);
    }
  }
  void drawPortraitGameOver(uint32_t age){
    if((age/700)%2==0){
      for(uint8_t i=0;i<8;i++){
        setStack(i,12+i,CRGB::Red);
        setStack(7-i,12+i,CRGB::Red);
      }
      return;
    }
    for(uint8_t y=10;y<=21;y++){
      setStack(1,y,CRGB(130,10,0));setStack(6,y,CRGB(130,10,0));
    }
    for(uint8_t x=1;x<=6;x++){
      setStack(x,10,CRGB::OrangeRed);setStack(x,21,CRGB::OrangeRed);
    }
  }
  void drawPortraitRecord(uint32_t age){
    const bool pulse=((age/220)&1)==0;
    const CRGB crown=pulse?CRGB::Gold:CRGB(120,75,0);
    for(uint8_t x=1;x<=6;x++)setStack(x,18,crown);
    for(uint8_t x=1;x<=6;x++)setStack(x,19,crown);
    setStack(1,15,crown);setStack(3,13,crown);setStack(4,13,crown);setStack(6,15,crown);
    setStack(2,17,crown);setStack(5,17,crown);
  }
  void clearStackPixels(){
    for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++)setStack(x,y,CRGB::Black);
  }
  void sanitizeStackPixels(const ArcadeGameEngine& game){
    const StackShiftGame& stack=game.stack;
    const bool strictPlayfield=game.stage==ArcadeStage::RACING && stack.running &&
        !stack.paused && !stack.lineClearActive && !stack.levelBreakActive &&
        !stack.perfectClearActive && !stack.matchFinished;
    const uint8_t threshold=strictPlayfield
        ? STACK_PLAYFIELD_BLACK_CLAMP_MAX
        : STACK_BLACK_CLAMP_MAX;
    for(uint16_t i=0;i<LED_COUNT;i++){
      const uint8_t peak=leds[i].r>leds[i].g
          ? (leds[i].r>leds[i].b?leds[i].r:leds[i].b)
          : (leds[i].g>leds[i].b?leds[i].g:leds[i].b);
      if(peak<=threshold)leds[i]=CRGB::Black;
    }
  }
  uint8_t activeCount(const PlayerManager&p)const{uint8_t c=0;for(uint8_t i=0;i<MAX_PLAYERS;i++)if(p.players[i].occupied&&p.players[i].connected&&!p.players[i].waiting)c++;return c;}
  int8_t activeOrder(const PlayerManager&p,uint8_t slot)const{int8_t o=0;for(uint8_t i=0;i<MAX_PLAYERS;i++){if(!p.players[i].occupied||!p.players[i].connected||p.players[i].waiting)continue;if(i==slot)return o;o++;}return -1;}
  uint8_t displayRow(const PlayerManager&p,uint8_t slot)const{static const uint8_t l[MAX_PLAYERS][MAX_PLAYERS]={{3,3,3,3,3,3,3,3},{2,5,5,5,5,5,5,5},{1,4,6,6,6,6,6,6},{0,2,5,7,7,7,7,7},{0,2,3,5,7,7,7,7},{0,1,3,4,6,7,7,7},{0,1,2,4,5,6,7,7},{0,1,2,3,4,5,6,7}};uint8_t c=activeCount(p);int8_t o=activeOrder(p,slot);return(c==0||o<0)?3:l[c-1][o];}
  static void spriteSet(void* ctx,int x,int y,CRGB c){static_cast<MatrixRenderer*>(ctx)->set(x,y,c);}
  bool drawPresentationCue(const ArcadeGameEngine& g,const ArcadeDirector& d){
    using Cue=ArcadeDirector::VisualCue;
    const uint32_t age=d.cueAgeMs();
    if(g.stage==ArcadeStage::PLATFORM_SELECT||g.stage==ArcadeStage::GAME_SELECT){
      if(d.isAttractRunning()){
        if(d.cue()==Cue::ATTRACT_PAC_CHASE)drawPacChase(age);
        else if(d.cue()==Cue::ATTRACT_PONG_CPU)drawPongCpuDemo(age);
        else if(d.cue()==Cue::ATTRACT_STACK_CPU)drawStackCpuDemo(age);
        else if(d.cue()==Cue::ATTRACT_RAIDER_CPU)drawRaiderCpuDemo(age);
        else if(d.cue()==Cue::ATTRACT_JOIN)drawJoinMessage(age);
        else drawPacChase(age);
      } else drawPacChase(age);
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
  static int triangleWave(uint32_t phase,int span){
    if(span<=0)return 0;
    const uint32_t period=uint32_t(span)*2U;
    const uint32_t value=phase%period;
    return value<=uint32_t(span)?int(value):int(period-value);
  }
  void drawPacChase(uint32_t age){
    const uint32_t outboundMs=6200U;
    const uint32_t emptyStageMs=600U;
    const uint32_t returnStartMs=outboundMs+emptyStageMs;
    const uint32_t returnMs=ATTRACT_PAC_CHASE_MS-returnStartMs;
    const bool mouthOpen=((age/150U)&1U)==0;

    for(uint8_t px=1;px<MATRIX_WIDTH;px+=4)set(px,4,CRGB(50,34,0));

    if(age<outboundMs){
      // First pass: the yellow character leads while the same red and pink
      // monsters chase at fixed spacing. The complete group leaves the stage.
      const int progress=int((age*64U)/outboundMs);
      const int pacX=-7+progress;
      const int redX=pacX-9;
      const int pinkX=pacX-18;
      SpriteRenderer::drawRows(this,spriteSet,pacX,1,
          mouthOpen?CHOMPER_OPEN:CHOMPER_CLOSED,6,CRGB::Yellow);
      SpriteRenderer::drawRows(this,spriteSet,redX,2,GHOST,5,CRGB::Red);
      SpriteRenderer::drawRows(this,spriteSet,pinkX,2,GHOST,5,CRGB::DeepPink);
      return;
    }

    if(age<returnStartMs)return;

    // LIFO return: the last monster to leave (pink) re-enters first, followed
    // by red, then the yellow character. All actors move at one speed and keep
    // their spacing, so the yellow character never overtakes or eats them.
    const uint32_t returnAge=age-returnStartMs;
    const int progress=int((returnAge*64U)/returnMs);
    const int pinkX=MATRIX_WIDTH-progress;
    const int redX=pinkX+9;
    const int pacX=pinkX+18;
    SpriteRenderer::drawRowsMirrored(this,spriteSet,pinkX,2,GHOST,5,CRGB::DeepPink);
    SpriteRenderer::drawRowsMirrored(this,spriteSet,redX,2,GHOST,5,CRGB::Red);
    SpriteRenderer::drawRowsMirrored(this,spriteSet,pacX,1,
        mouthOpen?CHOMPER_OPEN:CHOMPER_CLOSED,6,CRGB::Yellow);
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
  int pongDemoBallY(uint32_t rallyAge,uint8_t point)const{
    return 1+triangleWave(rallyAge/190U+uint32_t(point)*2U,5);
  }
  void drawPongCpuDemo(uint32_t age){
    const uint32_t pointCycle=age%8000U;
    const uint8_t point=uint8_t(age/8000U);
    const bool pointPause=pointCycle>=6600U;
    uint8_t leftScore=uint8_t((point+1U)/2U);
    uint8_t rightScore=uint8_t(point/2U);
    if(pointPause){
      if((point&1U)==0)leftScore++;
      else rightScore++;
    }

    for(uint8_t y=0;y<MATRIX_HEIGHT;y+=2){
      set(15,y,CRGB(12,12,24));
      set(16,y,CRGB(12,12,24));
    }
    for(uint8_t i=0;i<PONG_SCORE_TO_WIN;i++){
      set(5+i,0,i<leftScore?CRGB(0,100,255):CRGB(8,8,14));
      set(26-i,0,i<rightScore?CRGB(255,20,120):CRGB(8,8,14));
    }

    const uint32_t leftSample=pointCycle>220U?pointCycle-220U:0U;
    const uint32_t rightSample=pointCycle>380U?pointCycle-380U:0U;
    const int leftY=constrain(pongDemoBallY(leftSample,point)-1,0,int(MATRIX_HEIGHT-PONG_PADDLE_HEIGHT));
    const int rightY=constrain(pongDemoBallY(rightSample,point)-1,0,int(MATRIX_HEIGHT-PONG_PADDLE_HEIGHT));
    for(uint8_t i=0;i<PONG_PADDLE_HEIGHT;i++){
      set(PONG_LEFT_X,leftY+i,CRGB(0,100,255));
      set(PONG_RIGHT_X,rightY+i,CRGB(255,20,120));
    }

    if(pointPause){
      const int goalX=(point&1U)==0?PONG_RIGHT_X-1:PONG_LEFT_X+1;
      const int goalY=pongDemoBallY(6500U,point);
      set(goalX,goalY,CRGB::Gold);
      set(goalX-1,goalY,CRGB::White);
      set(goalX+1,goalY,CRGB::White);
      if(goalY>0)set(goalX,goalY-1,CRGB::White);
      if(goalY<MATRIX_HEIGHT-1)set(goalX,goalY+1,CRGB::White);
      return;
    }

    const int span=PONG_RIGHT_X-PONG_LEFT_X-3;
    const int ballX=PONG_LEFT_X+2+triangleWave(pointCycle/82U,span);
    const int ballY=pongDemoBallY(pointCycle,point);
    const int previousX=PONG_LEFT_X+2+triangleWave((pointCycle>82U?pointCycle-82U:0U)/82U,span);
    set(previousX,ballY,CRGB(45,45,65));
    set(ballX,ballY,CRGB::White);
  }
  struct StackDemoPlacement {
    uint8_t piece;
    uint8_t rotation;
    int8_t x;
    int8_t y;
    uint8_t clearedLines;
  };
  void drawStackDemoPiece(uint8_t piece,uint8_t rotation,int x,int y,CRGB color){
    for(uint8_t block=0;block<4;block++){
      const StackBlockOffset& offset=STACK_SHAPES[piece][rotation][block];
      setStack(x+offset.x,y+offset.y,color);
    }
  }
  void drawStackDemoBoard(const uint8_t board[STACK_HEIGHT][STACK_WIDTH]){
    for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++){
      const uint8_t cell=board[y][x];
      if(cell) setStack(x,y,stackColor(cell));
    }
  }
  bool stackDemoFits(const uint8_t board[STACK_HEIGHT][STACK_WIDTH],
                     uint8_t piece,uint8_t rotation,int x,int y)const{
    for(uint8_t block=0;block<4;block++){
      const StackBlockOffset& offset=STACK_SHAPES[piece][rotation][block];
      const int bx=x+offset.x;
      const int by=y+offset.y;
      if(bx<0||bx>=STACK_WIDTH||by>=STACK_HEIGHT)return false;
      if(by>=0&&board[by][bx])return false;
    }
    return true;
  }
  int stackDemoLandingY(const uint8_t board[STACK_HEIGHT][STACK_WIDTH],
                        uint8_t piece,uint8_t rotation,int x)const{
    int y=-4;
    if(!stackDemoFits(board,piece,rotation,x,y))return -100;
    while(stackDemoFits(board,piece,rotation,x,y+1))y++;
    for(uint8_t block=0;block<4;block++)
      if(y+STACK_SHAPES[piece][rotation][block].y<0)return -100;
    return y;
  }
  void stackDemoPlace(uint8_t board[STACK_HEIGHT][STACK_WIDTH],
                      const StackDemoPlacement& placement)const{
    for(uint8_t block=0;block<4;block++){
      const StackBlockOffset& offset=
          STACK_SHAPES[placement.piece][placement.rotation][block];
      const int bx=placement.x+offset.x;
      const int by=placement.y+offset.y;
      if(bx>=0&&bx<STACK_WIDTH&&by>=0&&by<STACK_HEIGHT)
        board[by][bx]=placement.piece+1;
    }
  }
  uint8_t stackDemoClearLines(uint8_t board[STACK_HEIGHT][STACK_WIDTH])const{
    uint8_t cleared=0;
    for(int y=STACK_HEIGHT-1;y>=0;y--){
      bool full=true;
      for(uint8_t x=0;x<STACK_WIDTH;x++)if(!board[y][x]){full=false;break;}
      if(!full)continue;
      cleared++;
      for(int pull=y;pull>0;pull--)
        memcpy(board[pull],board[pull-1],STACK_WIDTH);
      memset(board[0],0,STACK_WIDTH);
      y++;
    }
    return cleared;
  }
  int32_t stackDemoBoardScore(const uint8_t board[STACK_HEIGHT][STACK_WIDTH],
                              uint8_t clearedLines)const{
    uint8_t heights[STACK_WIDTH]={};
    uint16_t aggregateHeight=0;
    uint16_t holes=0;
    uint8_t maximumHeight=0;
    for(uint8_t x=0;x<STACK_WIDTH;x++){
      bool occupied=false;
      for(uint8_t y=0;y<STACK_HEIGHT;y++){
        if(board[y][x]){
          if(!occupied){
            heights[x]=STACK_HEIGHT-y;
            aggregateHeight+=heights[x];
            if(heights[x]>maximumHeight)maximumHeight=heights[x];
            occupied=true;
          }
        } else if(occupied) holes++;
      }
    }
    uint16_t bumpiness=0;
    for(uint8_t x=0;x+1<STACK_WIDTH;x++){
      int delta=int(heights[x])-int(heights[x+1]);
      if(delta<0)delta=-delta;
      bumpiness+=uint16_t(delta);
    }
    return int32_t(clearedLines)*10000L-int32_t(holes)*500L-
        int32_t(aggregateHeight)*12L-int32_t(bumpiness)*25L-
        int32_t(maximumHeight)*10L;
  }
  StackDemoPlacement stackDemoChoose(
      const uint8_t board[STACK_HEIGHT][STACK_WIDTH],uint8_t piece)const{
    StackDemoPlacement best={piece,0,2,0,0};
    int32_t bestScore=-2147483647L;
    bool found=false;
    for(uint8_t rotation=0;rotation<4;rotation++)for(int x=-2;x<STACK_WIDTH;x++){
      const int y=stackDemoLandingY(board,piece,rotation,x);
      if(y<-20)continue;
      uint8_t candidate[STACK_HEIGHT][STACK_WIDTH];
      memcpy(candidate,board,sizeof(candidate));
      StackDemoPlacement placement={piece,rotation,int8_t(x),int8_t(y),0};
      stackDemoPlace(candidate,placement);
      placement.clearedLines=stackDemoClearLines(candidate);
      int32_t score=stackDemoBoardScore(candidate,placement.clearedLines);
      int centerPenalty=x-2;
      if(centerPenalty<0)centerPenalty=-centerPenalty;
      score-=centerPenalty;
      if(!found||score>bestScore){
        best=placement;
        bestScore=score;
        found=true;
      }
    }
    return best;
  }
  uint8_t stackDemoFullRows(
      const uint8_t board[STACK_HEIGHT][STACK_WIDTH],uint8_t rows[4])const{
    uint8_t count=0;
    for(uint8_t y=0;y<STACK_HEIGHT&&count<4;y++){
      bool full=true;
      for(uint8_t x=0;x<STACK_WIDTH;x++)if(!board[y][x]){full=false;break;}
      if(full)rows[count++]=y;
    }
    return count;
  }
  void stackDemoBuildPrevious(uint8_t turn,
                              uint8_t board[STACK_HEIGHT][STACK_WIDTH],
                              uint8_t& clearedLines)const{
    static const uint8_t sevenBag[7]={2,6,4,5,3,0,1}; // T L Z J S I O
    memset(board,0,STACK_HEIGHT*STACK_WIDTH);
    clearedLines=0;
    for(uint8_t index=0;index<turn&&index<7;index++){
      StackDemoPlacement placement=stackDemoChoose(board,sevenBag[index]);
      stackDemoPlace(board,placement);
      clearedLines+=stackDemoClearLines(board);
    }
  }
  void drawStackCpuDemo(uint32_t age){
    clearStackPixels();
    static const uint8_t sevenBag[7]={2,6,4,5,3,0,1}; // T L Z J S I O
    static const uint32_t turnMs=2400U;
    static const uint32_t playMs=turnMs*7U;
    const uint32_t sceneAge=age%ATTRACT_STACK_CPU_MS;
    uint8_t board[STACK_HEIGHT][STACK_WIDTH];
    uint8_t clearedLines=0;

    if(sceneAge<playMs){
      const uint8_t turn=uint8_t(sceneAge/turnMs);
      const uint32_t turnAge=sceneAge%turnMs;
      stackDemoBuildPrevious(turn,board,clearedLines);
      drawStackDemoBoard(board);

      const uint8_t piece=sevenBag[turn];
      const StackDemoPlacement target=stackDemoChoose(board,piece);

      // Rotate visibly at the spawn point, then move laterally and fall into
      // the CPU-selected landing. This uses the same seven tetromino shapes
      // and collision geometry as the real game rather than a canned O/I loop.
      uint8_t visibleRotation=target.rotation;
      if(turnAge<840U)visibleRotation=uint8_t((turnAge/210U)%4U);

      int currentX=2;
      if(turnAge>760U){
        const uint32_t moveAge=turnAge>1210U?450U:turnAge-760U;
        currentX=2+int((int32_t(target.x-2)*int32_t(moveAge))/450L);
      }

      int currentY=0;
      if(turnAge>1040U){
        const uint32_t fallAge=turnAge>2050U?1010U:turnAge-1040U;
        currentY=int((int32_t(target.y)*int32_t(fallAge))/1010L);
      }

      if(turnAge>=1210U&&turnAge<2050U)
        drawStackDemoPiece(piece,target.rotation,target.x,target.y,CRGB::White);

      if(turnAge<2050U){
        drawStackDemoPiece(piece,visibleRotation,currentX,currentY,
                           stackColor(piece+1));
        return;
      }

      uint8_t locked[STACK_HEIGHT][STACK_WIDTH];
      memcpy(locked,board,sizeof(locked));
      stackDemoPlace(locked,target);
      drawStackDemoPiece(piece,target.rotation,target.x,target.y,
                         stackColor(piece+1));
      uint8_t rows[4]={};
      const uint8_t rowCount=stackDemoFullRows(locked,rows);
      if(rowCount&&(((turnAge-2050U)/90U)&1U))
        for(uint8_t row=0;row<rowCount;row++)
          for(uint8_t x=0;x<STACK_WIDTH;x++)setStack(x,rows[row],CRGB::White);
      return;
    }

    stackDemoBuildPrevious(7,board,clearedLines);
    drawStackDemoBoard(board);
    const uint8_t sweep=uint8_t(((sceneAge-playMs)/55U)%STACK_HEIGHT);
    for(uint8_t x=0;x<STACK_WIDTH;x++)setStack(x,STACK_HEIGHT-1-sweep,CRGB::Gold);
    for(uint8_t line=0;line<clearedLines&&line<STACK_WIDTH;line++)
      setStack(line,0,CRGB(0,255,70));
  }
  int raiderDemoCorridorCenter(int worldX)const{
    static const uint8_t centers[10]={3,3,4,5,5,4,3,2,2,3};
    const int segment=(worldX/5)%10;
    return centers[segment<0?segment+10:segment];
  }
  void drawRaiderCpuDemo(uint32_t age){
    const int scroll=int(age/105U);
    for(int x=0;x<MATRIX_WIDTH;x++){
      const int center=raiderDemoCorridorCenter(scroll+x);
      for(int y=0;y<MATRIX_HEIGHT;y++){
        if(y<center-2||y>center+2){
          const uint8_t edge=(y==center-3||y==center+3)?95:45;
          set(x,y,CRGB(0,edge/2,edge));
        }
      }
    }

    const int shipY=raiderDemoCorridorCenter(scroll+RAIDER_PLAYER_X);
    const bool shield=(age%9000U)>7600U;
    const CRGB ship=shield?CRGB::White:CRGB::Cyan;
    set(RAIDER_PLAYER_X,shipY,ship);
    set(RAIDER_PLAYER_X-1,shipY,ship);
    if(shipY>0)set(RAIDER_PLAYER_X-1,shipY-1,CRGB(0,120,180));
    if(shipY<MATRIX_HEIGHT-1)set(RAIDER_PLAYER_X-1,shipY+1,CRGB(0,120,180));

    for(uint8_t shot=0;shot<3;shot++){
      const uint32_t shifted=age+uint32_t(shot)*310U;
      const uint32_t local=shifted%930U;
      const int bulletX=RAIDER_PLAYER_X+2+int(local/52U);
      const uint32_t spawnAge=shifted-local;
      const int bulletY=raiderDemoCorridorCenter(int(spawnAge/105U)+RAIDER_PLAYER_X);
      if(bulletX<MATRIX_WIDTH)set(bulletX,bulletY,CRGB::Orange);
    }

    for(uint8_t enemy=0;enemy<3;enemy++){
      const int travel=int((age/88U+uint32_t(enemy)*13U)%42U);
      const int enemyX=MATRIX_WIDTH+5-travel;
      if(enemyX<6||enemyX>=MATRIX_WIDTH)continue;
      const int center=raiderDemoCorridorCenter(scroll+enemyX);
      const int enemyY=constrain(center+int(enemy)-1,1,int(MATRIX_HEIGHT)-2);
      set(enemyX,enemyY,CRGB::Red);
      set(enemyX+1,enemyY,CRGB(120,0,20));
    }

    const int powerX=MATRIX_WIDTH-int((age/120U+9U)%46U);
    if(powerX>=7&&powerX<MATRIX_WIDTH){
      const int powerY=raiderDemoCorridorCenter(scroll+powerX);
      set(powerX,powerY,CRGB::Yellow);
      if((age/150U)&1U)set(powerX,powerY+1,CRGB::White);
    }
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
  void drawPlatformSelect(){drawPacChase(millis()%ATTRACT_PAC_CHASE_MS);}
  void drawGameSelect(const ArcadeGameEngine&g){drawPacChase(millis()%ATTRACT_PAC_CHASE_MS);}
  void drawLobby(const PlayerManager&p,const ArcadeGameEngine&g){for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;CRGB c=playerColor(i);c.nscale8_video(x.ready?180:50);uint8_t row=displayRow(p,i);for(uint8_t xx=0;xx<(x.ready?8:3);xx++)set(xx,row,c);}}
  void drawAnnouncement(const ArcadeGameEngine&g){if(g.announcePhase%2==0)drawTextOriented(g.selectedGame,"BOSS",CRGB::Gold);else if(g.bossSlot>=0)drawTextOriented(g.selectedGame,colorName(g.bossSlot),playerColor(g.bossSlot));}
  void drawCountdown(const ArcadeGameEngine&g){
    const uint8_t v=g.countdownValue;
    if(g.selectedGame==GameId::STACK_SHIFT){
      clearStackPixels();
      if(v==0)drawPortraitGo(millis()-g.countdownChangedAtMs);
      else drawPortraitNumber(v,CRGB::White);
      return;
    }
    if(v==0){
      if(gameOrientation(g.selectedGame)==DisplayOrientation::HORIZONTAL){
        for(uint8_t x=0;x<MATRIX_WIDTH;x++)for(uint8_t y=0;y<MATRIX_HEIGHT;y++)set(x,y,CRGB::Green);
      }else drawTextOriented(g.selectedGame,"GO",CRGB::Green);
      return;
    }
    char t[2]={char('0'+v),0};drawTextOriented(g.selectedGame,t,CRGB::White);
  }
  void drawRace(const PlayerManager&p){for(uint8_t y=0;y<MATRIX_HEIGHT;y++){set(FINISH_X,y,CRGB(45,45,45));set(TURBO_X_1,y,CRGB(55,34,0));set(TURBO_X_2,y,CRGB(55,34,0));}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting)continue;set(x.position,displayRow(p,i),x.turboTaps?CRGB::Gold:playerColor(i));}}
  void drawTron(const PlayerManager&p,const ArcadeGameEngine&g){for(uint16_t idx=0;idx<LED_COUNT;idx++){uint8_t owner=g.tronTrail[idx];if(!owner)continue;uint8_t y=idx/MATRIX_WIDTH,x=idx%MATRIX_WIDTH;CRGB c=playerColor(owner-1);c.nscale8_video(90);set(x,y,c);}for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(x.tronAlive)set(x.tronX,x.tronY,playerColor(i));}}
  void drawClash(const PlayerManager& p,const ArcadeGameEngine& g){
    for(uint8_t y=0;y<MATRIX_HEIGHT;y++)for(uint8_t x=0;x<MATRIX_WIDTH;x++){
      const uint8_t owner=g.clashPaint[y*MATRIX_WIDTH+x];
      if(owner){CRGB c=playerColor(owner-1);c.nscale8_video(75);set(x,y,c);}else set(x,y,CRGB(2,2,4));
    }
    for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto& pl=p.players[i];if(!pl.occupied||!pl.connected||pl.waiting||pl.clashX<0)continue;set(pl.clashX,pl.clashY,CRGB::White);}
    const uint32_t remain=g.clashRemainingMs();
    if(remain>0&&remain<=5000){const uint8_t lights=(remain+999)/1000;for(uint8_t i=0;i<5;i++)set(27+i,0,i<lights?CRGB::White:CRGB(12,12,12));}
  }
  void drawPong(const PlayerManager& p,const ArcadeGameEngine& g){
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
  void drawStackLevelBreak(const StackShiftGame& stack){
    clearStackPixels();
    const uint32_t age=stack.levelBreakAgeMs(millis());
    if(age<STACK_LEVEL_INTRO_MS){
      // A dedicated level-up beat establishes context before the row scanner.
      // The large level number is static while two full-bright chevrons rise.
      drawPortraitNumber(stack.levelBreakLevel,CRGB(255,190,0));
      const uint8_t travel=uint8_t((age/70)%18);
      for(uint8_t band=0;band<2;band++){
        const int base=STACK_HEIGHT-3-int(travel)-int(band)*9;
        for(uint8_t d=0;d<3;d++){
          setStack(3-d/2,base+d,CRGB(255,190,0));
          setStack(4+d/2,base+d,CRGB(255,190,0));
        }
      }
      return;
    }

    drawStackLockedBoard(stack);
    const uint8_t scanned=stack.levelBreakScannedRows(millis());
    const uint8_t emptyRows=stack.levelBreakEmptyRows;
    if(emptyRows==0)return;
    for(uint8_t y=0;y<scanned&&y<emptyRows;y++){
      setStack(0,y,CRGB(0,255,70));
      setStack(STACK_WIDTH-1,y,CRGB(0,255,70));
    }
    const uint32_t scanStart=STACK_LEVEL_INTRO_MS+STACK_LEVEL_SCAN_LEAD_MS;
    uint8_t scanRow=0;
    if(age>=scanStart){
      scanRow=(age-scanStart)/STACK_LEVEL_SCAN_ROW_MS;
      if(scanRow>=emptyRows)scanRow=emptyRows-1;
    }
    for(uint8_t x=0;x<STACK_WIDTH;x++)setStack(x,scanRow,CRGB::White);
  }
  void drawStack(const ArcadeGameEngine&g){
    const StackShiftGame& stack=g.stack;
    // Stack Shift deliberately has no ambient field, halo, trail, or dim grid.
    // Clear its rotated playfield explicitly on every frame before drawing only
    // locked cells, the active piece, and the earned landing ghost.
    clearStackPixels();
    if(stack.perfectClearActive){drawStackPerfectClear(stack);return;}
    if(stack.levelBreakActive){drawStackLevelBreak(stack);return;}
    const uint8_t clearPhase=stack.lineClearPhase(millis());
    for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++){
      if(stack.lineMarked(y)){
        if((clearPhase&1)==0){
          CRGB flash=CRGB::White;
          if(stack.lineClearCount>=4){
            static const CRGB rainbow[7]={CRGB::Red,CRGB::Orange,CRGB::Yellow,CRGB::Green,CRGB::Cyan,CRGB::Blue,CRGB::Purple};
            flash=rainbow[(x+clearPhase)%7];
          }else if(stack.lineClearCount==3)flash=CRGB(255,70,220);
          else if(stack.lineClearCount==2)flash=CRGB::Cyan;
          setStack(x,y,flash);
        }
        continue;
      }
      const uint8_t piece=stack.cell(x,y);
      if(piece)setStack(x,y,stackColor(piece));
    }
    if(stack.running&&!stack.matchFinished){
      const CRGB active=stackColor(stack.activePiece+1);
      if(stack.ghostVisible()){
        // One LED is one cell, so an outline is impossible. Use a static,
        // full-bright neutral projection: distinct from the colored active
        // piece without introducing dim pixels or blinking.
        for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++)
          if(stack.ghostAt(x,y)&&!stack.activeAt(x,y))setStack(x,y,CRGB::White);
      }
      for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++)if(stack.activeAt(x,y))setStack(x,y,active);
      if(stack.paused){
        for(uint8_t y=0;y<STACK_HEIGHT;y++)for(uint8_t x=0;x<STACK_WIDTH;x++){
          const int physicalX=gameOrientation(GameId::STACK_SHIFT)==DisplayOrientation::VERTICAL_CLOCKWISE?y:MATRIX_WIDTH-1-y;
          const int physicalY=gameOrientation(GameId::STACK_SHIFT)==DisplayOrientation::VERTICAL_CLOCKWISE?MATRIX_HEIGHT-1-x:x;
          leds[xy(physicalX,physicalY)].nscale8_video(70);
        }
        const CRGB pauseColor=((millis()/450)%2)==0?CRGB::White:CRGB(90,90,110);
        for(uint8_t y=13;y<=18;y++){setStack(2,y,pauseColor);setStack(5,y,pauseColor);}
      }
    }
  }
  void drawRaider(const ArcadeGameEngine&g){
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
  void drawResult(const PlayerManager&p,const ArcadeGameEngine&g){
    if(g.selectedGame==GameId::PIXEL_RAIDER){
      if(g.raiderNewRecord)drawTextOriented(g.selectedGame,DISPLAY_LANGUAGE_TR?"REKOR":"RECORD",CRGB::Gold);
      else drawTextOriented(g.selectedGame,((millis()/900)%2)==0?"GAME":"OVER",CRGB::Red);
      return;
    }
    if(g.selectedGame==GameId::STACK_SHIFT){
      clearStackPixels();
      if(g.stack.newRecord)drawPortraitRecord(millis());
      else drawPortraitGameOver(millis());
      return;
    }
    if(g.selectedGame==GameId::COLOR_CLASH){
      if(g.winner<0){drawTextOriented(g.selectedGame,DISPLAY_LANGUAGE_TR?"BERABERE":"DRAW",CRGB::White);return;}
      const char* word=DISPLAY_LANGUAGE_TR?"KAZANAN":"WINNER";
      if((millis()/900)%2==0)drawTextOriented(g.selectedGame,word,CRGB::White);else drawTextOriented(g.selectedGame,colorName(g.winner),playerColor(g.winner));
      return;
    }
    if(g.winner<0){drawTextOriented(g.selectedGame,DISPLAY_LANGUAGE_TR?"BERABERE":"DRAW",CRGB::White);return;}
    const char* word=DISPLAY_LANGUAGE_TR?"KAZANAN":"WINNER";
    if((millis()/900)%2==0)drawTextOriented(g.selectedGame,word,CRGB::White);else drawTextOriented(g.selectedGame,colorName(g.winner),playerColor(g.winner));
  }
  void drawBoss(const PlayerManager&p,const ArcadeGameEngine&g){const uint8_t bs=25;if(g.bossSlot>=0){CRGB c=playerColor(g.bossSlot);c.nscale8_video((millis()/110)%2?230:110);for(uint8_t x=bs;x<MATRIX_WIDTH;x++)for(uint8_t y=1;y<7;y++)if(x==bs||x==31||y==1||y==6||((x+y+millis()/140)%3==0))set(x,y,c);set(27,0,CRGB::Gold);set(29,0,CRGB::Gold);set(31,0,CRGB::Gold);}uint8_t hp=g.bossMaxHp?uint8_t((uint32_t(g.bossHp)*23+g.bossMaxHp-1)/g.bossMaxHp):0;for(uint8_t x=0;x<23;x++)set(x,0,x<hp?CRGB::Red:CRGB(18,0,0));for(uint8_t i=0;i<MAX_PLAYERS;i++){const auto&x=p.players[i];if(!x.occupied||!x.connected||x.waiting||i==g.bossSlot)continue;uint8_t row=displayRow(p,i);CRGB c=g.stunRemainingMs(x)?CRGB::White:playerColor(i);set(0,row,c);set(2+((millis()/75+x.bossDamage*3)%21),row,c);}}
  void drawBossResult(const PlayerManager&p,const ArcadeGameEngine&g){if(g.bossDefeated)drawTextOriented(g.selectedGame,DISPLAY_LANGUAGE_TR?"TAKIM":"TEAM",CRGB::Green);else if(g.bossSlot>=0)drawTextOriented(g.selectedGame,colorName(g.bossSlot),playerColor(g.bossSlot));}
};
