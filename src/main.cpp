#include <Arduino.h>

/*
 * ╔══════════════════════════════════════════════════════════════╗
 *   CLAWD MOCHI — USB MODE — Claude Status Companion
 *
 *   Wiring (ESP32-C3 Super Mini → ST7789 1.54" 240×240):
 *     SDA → GPIO 10  (hardware SPI MOSI)
 *     SCL → GPIO 8   (hardware SPI SCK)
 *     RST → GPIO 5
 *     DC  → GPIO 1
 *     CS  → GPIO 4
 *     BL  → GPIO 3
 *     VCC → 3V3, GND → GND
 *
 *   USB: built-in USB Serial/JTAG → COM port on PC
 *   Protocol: text commands, one per line
 *     IDLE  | THINK | CODE | RUN | ERROR | PING
 *   Response: "OK\n" or "PONG\n"
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
// ── Pins ──────────────────────────────────────────────────────
#define TFT_CS  4
#define TFT_DC  1
#define TFT_RST 5
#define TFT_BLK 3

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ── Display ───────────────────────────────────────────────────
#define DISP_W 240
#define DISP_H 240

// ── Eye geometry (for normal-eye idle animation) ──────────────
#define EYE_W   30
#define EYE_H   60
#define EYE_GAP 120
#define EYE_OX  0
#define EYE_OY  40

// ── Colours ───────────────────────────────────────────────────
uint16_t C_ORANGE, C_DARKBG, C_MUTED, C_GREEN;
uint16_t animBgColor = 0;  // used by face_code.h functions
bool busy = false;             // used by legacy anim_* functions in face_code.h
inline int speedMs(int ms) { return ms; }  // legacy animation speed scaler
#define C_WHITE ST77XX_WHITE
#define C_BLACK ST77XX_BLACK
#define C_RED   ST77XX_RED

// ── Claude states ─────────────────────────────────────────────
enum ClaudeState : uint8_t {
  STATE_IDLE     = 0,  // waiting for user
  STATE_THINKING = 1,  // reasoning / reading / planning
  STATE_CODING   = 2,  // writing files
  STATE_RUNNING  = 3,  // executing shell commands
  STATE_ERROR    = 4,  // something went wrong
  STATE_BLUSH    = 5,  // face: blush
  STATE_AWE      = 6,  // face: awe
  STATE_WORRY    = 7,  // face: worry
  STATE_PROUD    = 8,  // face: proud
  STATE_HEART    = 9,  // face: heart eyes
  STATE_SLEEP    = 10, // face: sleep
  STATE_WINK     = 11, // face: wink
  STATE_THK      = 12, // face: thinking cat
};

ClaudeState  currentState     = STATE_IDLE;
ClaudeState  drawnState       = STATE_IDLE;  // used as "dirty" sentinel
uint8_t      animFrame        = 0;
uint8_t      animSubFrame     = 0;
unsigned long lastFrameTime   = 0;
unsigned long lastHeartbeat   = 0;
uint8_t       lastDrawnFrame  = 255;  // invalid frame forces first draw
#define ANIM_SPEED 1.0               // >1 = slower; 2.0 = 2X slower
#define HEARTBEAT_TIMEOUT 15000   // 15 s without PING → IDLE
#define ANIM_FRAME_DEFAULT 250

// ── Serial command buffer ─────────────────────────────────────
String cmdBuf;

// ═════════════════════════════════════════════════════════════
//  HELPERS
// ═════════════════════════════════════════════════════════════

void setBacklight(bool on) {
  digitalWrite(TFT_BLK, on ? HIGH : LOW);
}

void initColours() {
  C_ORANGE = tft.color565(218, 17, 0);
  C_DARKBG = tft.color565(10,  12,  16);
  C_MUTED  = tft.color565(90,  88,  86);
  C_GREEN  = tft.color565(80, 220, 130);
  animBgColor = C_ORANGE;
}

// ── Eye position helpers ──────────────────────────────────────
inline int16_t eyeLX(int16_t ox) {
  return (DISP_W - (EYE_W * 2 + EYE_GAP)) / 2 + EYE_OX + ox;
}
inline int16_t eyeRX(int16_t ox) { return eyeLX(ox) + EYE_W + EYE_GAP; }
inline int16_t eyeY()            { return (DISP_H - EYE_H) / 2 - EYE_OY; }

// ── Normal-eye drawing ────────────────────────────────────────
void drawNormalEyes(int16_t ox = 0, bool blink = false) {
  tft.fillScreen(animBgColor);
  const int16_t lx = eyeLX(ox), rx = eyeRX(ox), ey = eyeY();
  if (!blink) {
    tft.fillRect(lx, ey, EYE_W, EYE_H, C_BLACK);
    tft.fillRect(rx, ey, EYE_W, EYE_H, C_BLACK);
  } else {
    tft.fillRect(lx, ey + EYE_H / 2 - 3, EYE_W, 6, C_BLACK);
    tft.fillRect(rx, ey + EYE_H / 2 - 3, EYE_W, 6, C_BLACK);
  }
}

// ── Squish-eye drawing ────────────────────────────────────────
void drawChevron(int16_t cx, int16_t cy, int16_t arm, int16_t reach,
                 uint8_t thk, bool rightFacing, uint16_t col) {
  for (int8_t t = -(int8_t)thk; t <= (int8_t)thk; t++) {
    if (rightFacing) {
      tft.drawLine(cx - reach/2, cy - arm + t, cx + reach/2, cy + t,      col);
      tft.drawLine(cx + reach/2, cy + t,       cx - reach/2, cy + arm + t, col);
    } else {
      tft.drawLine(cx + reach/2, cy - arm + t, cx - reach/2, cy + t,      col);
      tft.drawLine(cx - reach/2, cy + t,       cx + reach/2, cy + arm + t, col);
    }
  }
}

// ═════════════════════════════════════════════════════════════
//  FACE DRAWING FUNCTIONS  (generated, from face_code.h)
// ═════════════════════════════════════════════════════════════

#include "face_code.h"
#include "face_code2.h"

// ═════════════════════════════════════════════════════════════
//  NON-BLOCKING ANIMATION SYSTEM
// ═════════════════════════════════════════════════════════════

// Each animation is a sequence of {drawFunc, duration_ms} pairs.
// The loop() calls runAnimation() each tick; it advances to the
// next frame when enough time has passed, non-blocking.

typedef void (*DrawFunc)();

struct AnimFrame {
  DrawFunc  draw;
  uint16_t  duration;
};

// ── IDLE: wandering eyes + blinks ─────────────────────────────
void _idle_center() { drawNormalEyes( 0, false); }
void _idle_left()   { drawNormalEyes(-16, false); }
void _idle_right()  { drawNormalEyes( 16, false); }
void _idle_blink()  { drawNormalEyes(  0, true);  }

static const AnimFrame idleAnim[] = {
  { _idle_center, 1200 },
  { _idle_left,    600 },
  { _idle_center,  900 },
  { _idle_blink,   300 },
  { _idle_center,  800 },
  { _idle_blink,   300 },
  { _idle_center, 1000 },
  { _idle_right,   600 },
  { _idle_center,  900 },
  { _idle_blink,   300 },
};
#define IDLE_LEN 10

// ── THINKING: dots appearing ───────────────────────────────────
static const AnimFrame thinkAnim[] = {
  { drawFace_dian1, 500 },
  { drawFace_dian2, 500 },
  { drawFace_dian3, 500 },
};
#define THINK_LEN 3

// ── CODING: smile alternating ──────────────────────────────────
static const AnimFrame codeAnim[] = {
  { drawFace_smile1, 400 },
  { drawFace_smile2, 400 },
};
#define CODE_LEN 2

// ── RUNNING: heartbeat ────────────────────────────────────────
static const AnimFrame runAnim[] = {
  { drawFace_hart1, 300 },
  { drawFace_hart2, 300 },
};
#define RUN_LEN 2

// ── ERROR: angry → X flash → angry ────────────────────────────
static const AnimFrame errorAnim[] = {
  { drawFace_angry, 2000 },
  { drawFace_X,     300 },
  { drawFace_angry, 1500 },
  { drawFace_X,     300 },
};
#define ERROR_LEN 4

// ── BLUSH ──────────────────────────────────────────────────────
static const AnimFrame blushAnim[] = {
  { drawFace_blush1, 500 },
  { drawFace_blush2, 500 },
};
#define BLUSH_LEN 2

// ── AWE ────────────────────────────────────────────────────────
static const AnimFrame aweAnim[] = {
  { drawFace_awe1, 600 },
  { drawFace_awe2, 600 },
};
#define AWE_LEN 2

// ── WORRY ──────────────────────────────────────────────────────
static const AnimFrame worryAnim[] = {
  { drawFace_worry1, 500 },
  { drawFace_worry2, 500 },
};
#define WORRY_LEN 2

// ── PROUD ──────────────────────────────────────────────────────
static const AnimFrame proudAnim[] = {
  { drawFace_proud1, 700 },
  { drawFace_proud2, 700 },
};
#define PROUD_LEN 2

// ── HEART ──────────────────────────────────────────────────────
static const AnimFrame heartAnim[] = {
  { drawFace_heart1, 450 },
  { drawFace_heart2, 450 },
};
#define HEART_LEN 2

// ── SLEEP ──────────────────────────────────────────────────────
static const AnimFrame sleepAnim[] = {
  { drawFace_sleep1, 800 },
  { drawFace_sleep2, 800 },
};
#define SLEEP_LEN 2

// ── WINK ───────────────────────────────────────────────────────
static const AnimFrame winkAnim[] = {
  { drawFace_wink1, 500 },
  { drawFace_wink2, 500 },
};
#define WINK_LEN 2

// ── THINKING CAT ───────────────────────────────────────────────
static const AnimFrame thkAnim[] = {
  { drawFace_thinking1, 600 },
  { drawFace_thinking2, 600 },
};
#define THK_LEN 2

// Meta-table
static const AnimFrame* const animTables[] = {
  idleAnim, thinkAnim, codeAnim, runAnim, errorAnim,
  blushAnim, aweAnim, worryAnim, proudAnim,
  heartAnim, sleepAnim, winkAnim, thkAnim
};
static const uint8_t animLengths[] = {
  IDLE_LEN, THINK_LEN, CODE_LEN, RUN_LEN, ERROR_LEN,
  BLUSH_LEN, AWE_LEN, WORRY_LEN, PROUD_LEN,
  HEART_LEN, SLEEP_LEN, WINK_LEN, THK_LEN
};

// ═════════════════════════════════════════════════════════════
//  STATE MACHINE
// ═════════════════════════════════════════════════════════════

void switchState(ClaudeState s) {
  if (s >= 13) return;
  currentState = s;
  animFrame    = 0;
  animSubFrame = 0;
  drawnState   = (ClaudeState)0xFF;  // force redraw this tick
  lastFrameTime = 0;
}

void runAnimation() {
  unsigned long now = millis();

  // Force redraw on state change
  if (drawnState != currentState) {
    drawnState    = currentState;
    animFrame     = 0;
    lastFrameTime = now;   // give first frame its full duration
    lastDrawnFrame = 255;  // force redraw this tick
  }

  uint8_t len = animLengths[currentState];
  if (len == 0) return;

  // Advance frame if enough time has passed
  if (now - lastFrameTime >= (unsigned long)(animTables[currentState][animFrame].duration * ANIM_SPEED)) {
    animFrame++;
    if (animFrame >= len) animFrame = 0;
    lastFrameTime = now;
  }

  // Only draw when frame changed (not every loop iteration)
  if (animFrame != lastDrawnFrame) {
    animTables[currentState][animFrame].draw();
    lastDrawnFrame = animFrame;
  }
}

// ═════════════════════════════════════════════════════════════
//  SERIAL COMMAND PARSER
// ═════════════════════════════════════════════════════════════

void processCommand(const String& raw) {
  String cmd = raw;
  cmd.toUpperCase();
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd == "IDLE")       switchState(STATE_IDLE);
  else if (cmd == "THINK") switchState(STATE_THINKING);
  else if (cmd == "CODE")  switchState(STATE_CODING);
  else if (cmd == "RUN")   switchState(STATE_RUNNING);
  else if (cmd == "ERROR") switchState(STATE_ERROR);
  // ── Manual face expressions ──
  else if (cmd == "BLUSH")  switchState(STATE_BLUSH);
  else if (cmd == "AWE")    switchState(STATE_AWE);
  else if (cmd == "WORRY")  switchState(STATE_WORRY);
  else if (cmd == "PROUD")  switchState(STATE_PROUD);
  else if (cmd == "HEART")  switchState(STATE_HEART);
  else if (cmd == "SLEEP")  switchState(STATE_SLEEP);
  else if (cmd == "WINK")   switchState(STATE_WINK);
  else if (cmd == "THK")    switchState(STATE_THK);
  else if (cmd == "RANDOM") {
    static const uint8_t randStates[] = {
      STATE_BLUSH, STATE_AWE, STATE_WORRY, STATE_PROUD,
      STATE_HEART, STATE_SLEEP, STATE_WINK, STATE_THK
    };
    switchState((ClaudeState)randStates[(uint8_t)esp_random() % 8]);
  }
  else if (cmd == "PING") {
    lastHeartbeat = millis();
    Serial.println("PONG");
    return;
  }
  else {
    Serial.println("ERR");
    return;
  }

  lastHeartbeat = millis();
  Serial.println("OK");
}

void checkSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmdBuf.length() > 0) {
        processCommand(cmdBuf);
        cmdBuf = "";
      }
    } else if (c >= 32 && c < 127) {
      if (cmdBuf.length() < 32) cmdBuf += c;  // safety cap
    }
  }
}

// ═════════════════════════════════════════════════════════════
//  LOGO (kept for boot splash)
// ═════════════════════════════════════════════════════════════

#define LOGO_CX 120
#define LOGO_CY 105

#define LOGO_TRI_COUNT 162
static const int16_t LOGO_TRIS[][6] PROGMEM = {
  {120,105,65,134,100,114},{120,105,100,114,101,113},{120,105,101,113,100,112},
  {120,105,100,112,99,112},{120,105,99,112,93,111},{120,105,93,111,73,111},
  {120,105,73,111,55,110},{120,105,55,110,38,109},{120,105,38,109,34,108},
  {120,105,34,108,30,103},{120,105,30,103,30,100},{120,105,30,100,34,98},
  {120,105,34,98,39,98},{120,105,39,98,50,99},{120,105,50,99,67,100},
  {120,105,67,100,80,101},{120,105,80,101,98,103},{120,105,98,103,101,103},
  {120,105,101,103,101,102},{120,105,101,102,100,101},{120,105,100,101,100,100},
  {120,105,100,100,82,88},{120,105,82,88,63,76},{120,105,63,76,53,69},
  {120,105,53,69,48,65},{120,105,48,65,45,61},{120,105,45,61,44,54},
  {120,105,44,54,49,49},{120,105,49,49,55,49},{120,105,55,49,57,49},
  {120,105,57,49,64,55},{120,105,64,55,78,66},{120,105,78,66,96,79},
  {120,105,96,79,99,81},{120,105,99,81,100,81},{120,105,100,81,100,80},
  {120,105,100,80,99,78},{120,105,99,78,89,60},{120,105,89,60,78,41},
  {120,105,78,41,73,34},{120,105,73,34,72,29},{120,105,72,29,72,28},
  {120,105,72,28,72,27},{120,105,72,27,71,26},{120,105,71,26,71,25},
  {120,105,71,25,71,24},{120,105,71,24,77,16},{120,105,77,16,80,15},
  {120,105,80,15,87,16},{120,105,87,16,91,19},{120,105,91,19,95,29},
  {120,105,95,29,103,46},{120,105,103,46,114,68},{120,105,114,68,118,75},
  {120,105,118,75,119,81},{120,105,119,81,120,83},{120,105,120,83,121,83},
  {120,105,121,83,121,82},{120,105,121,82,122,69},{120,105,122,69,124,54},
  {120,105,124,54,126,34},{120,105,126,34,126,28},{120,105,126,28,129,21},
  {120,105,129,21,135,18},{120,105,135,18,139,20},{120,105,139,20,143,25},
  {120,105,143,25,142,28},{120,105,142,28,140,42},{120,105,140,42,136,64},
  {120,105,136,64,133,78},{120,105,133,78,135,78},{120,105,135,78,136,76},
  {120,105,136,76,144,67},{120,105,144,67,156,51},{120,105,156,51,162,45},
  {120,105,162,45,168,38},{120,105,168,38,172,35},{120,105,172,35,180,35},
  {120,105,180,35,185,43},{120,105,185,43,183,52},{120,105,183,52,175,62},
  {120,105,175,62,168,71},{120,105,168,71,159,83},{120,105,159,83,153,94},
  {120,105,153,94,154,94},{120,105,154,94,155,94},{120,105,155,94,176,90},
  {120,105,176,90,188,88},{120,105,188,88,201,85},{120,105,201,85,208,88},
  {120,105,208,88,208,91},{120,105,208,91,206,97},{120,105,206,97,191,101},
  {120,105,191,101,174,104},{120,105,174,104,148,110},{120,105,148,110,148,111},
  {120,105,148,111,148,111},{120,105,148,111,160,112},{120,105,160,112,165,112},
  {120,105,165,112,177,112},{120,105,177,112,200,114},{120,105,200,114,205,118},
  {120,105,205,118,209,123},{120,105,209,123,208,126},{120,105,208,126,199,131},
  {120,105,199,131,187,128},{120,105,187,128,159,121},{120,105,159,121,149,119},
  {120,105,149,119,147,119},{120,105,147,119,147,120},{120,105,147,120,156,128},
  {120,105,156,128,170,141},{120,105,170,141,189,158},{120,105,189,158,190,163},
  {120,105,190,163,188,166},{120,105,188,166,185,166},{120,105,185,166,169,153},
  {120,105,169,153,162,148},{120,105,162,148,148,136},{120,105,148,136,147,136},
  {120,105,147,136,147,137},{120,105,147,137,150,142},{120,105,150,142,168,168},
  {120,105,168,168,169,176},{120,105,169,176,168,179},{120,105,168,179,163,180},
  {120,105,163,180,158,179},{120,105,158,179,148,165},{120,105,148,165,137,149},
  {120,105,137,149,129,134},{120,105,129,134,128,135},{120,105,128,135,123,189},
  {120,105,123,189,120,192},{120,105,120,192,115,194},{120,105,115,194,110,191},
  {120,105,110,191,108,185},{120,105,108,185,110,174},{120,105,110,174,113,160},
  {120,105,113,160,116,148},{120,105,116,148,118,134},{120,105,118,134,119,129},
  {120,105,119,129,119,129},{120,105,119,129,118,129},{120,105,118,129,107,144},
  {120,105,107,144,91,166},{120,105,91,166,78,180},{120,105,78,180,75,181},
  {120,105,75,181,70,178},{120,105,70,178,70,173},{120,105,70,173,73,169},
  {120,105,73,169,91,146},{120,105,91,146,102,132},{120,105,102,132,109,124},
  {120,105,109,124,109,123},{120,105,109,123,108,123},{120,105,108,123,61,153},
  {120,105,61,153,52,155},{120,105,52,155,49,151},{120,105,49,151,49,146},
  {120,105,49,146,51,144},{120,105,51,144,65,134},{120,105,65,134,65,134},
};

void drawLogoFilled(uint16_t bg, uint16_t fg) {
  tft.fillScreen(bg);
  for (uint16_t i = 0; i < LOGO_TRI_COUNT; i++) {
    tft.fillTriangle(
      pgm_read_word(&LOGO_TRIS[i][0]), pgm_read_word(&LOGO_TRIS[i][1]),
      pgm_read_word(&LOGO_TRIS[i][2]), pgm_read_word(&LOGO_TRIS[i][3]),
      pgm_read_word(&LOGO_TRIS[i][4]), pgm_read_word(&LOGO_TRIS[i][5]),
      fg);
  }
  tft.setTextColor(fg); tft.setTextSize(2);
  tft.setCursor(LOGO_CX - 54, 210); tft.print("Anthropic");
  tft.setCursor(LOGO_CX - 53, 210); tft.print("Anthropic");
}

void showBootLogo() {
  drawLogoFilled(C_ORANGE, C_WHITE);
  delay(1500);
}

// ═════════════════════════════════════════════════════════════
//  USB INFO SCREEN
// ═════════════════════════════════════════════════════════════

void showUsbInfo() {
  tft.fillScreen(C_DARKBG);
  tft.fillRect(0, 0, DISP_W, 4, C_ORANGE);
  tft.setTextColor(C_WHITE);  tft.setTextSize(2);
  tft.setCursor(12, 16);  tft.print("USB Mode Ready");
  tft.setTextColor(C_MUTED);  tft.setTextSize(1);
  tft.setCursor(12, 44);  tft.print("Waiting for Claude...");
  tft.setTextColor(C_GREEN); tft.setTextSize(1);
  tft.setCursor(12, 64);  tft.print("Commands: IDLE THINK CODE RUN ERROR");
  tft.fillRect(0, DISP_H - 4, DISP_W, 4, C_ORANGE);
  delay(2000);
}

// ═════════════════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════════════════

void setup() {
  // USB CDC — built-in USB Serial/JTAG
  Serial.begin(115200);
  delay(500);

  pinMode(TFT_BLK, OUTPUT);
  setBacklight(true);

  // Manual reset sequence
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH); delay(10);
  digitalWrite(TFT_RST, LOW);  delay(10);
  digitalWrite(TFT_RST, HIGH); delay(150);

  // Init display
  SPI.begin(8, -1, 10, TFT_CS);
  tft.setSPISpeed(8000000);
  tft.init(240, 240, SPI_MODE0);
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.setRotation(1);
  initColours();

  // Boot splash
  tft.fillScreen(C_ORANGE);
  tft.setTextColor(C_WHITE); tft.setTextSize(3);
  tft.setCursor(DISP_W / 2 - 54, DISP_H / 2 - 22); tft.print("Clawd");
  tft.setCursor(DISP_W / 2 - 54, DISP_H / 2 + 14); tft.print("Mochi");
  delay(1000);

  // Logo
  showBootLogo();

  // USB info screen
  showUsbInfo();

  // Enter IDLE state
  switchState(STATE_IDLE);

  Serial.println("CLAWD READY");
  lastFrameTime = millis();   // prevent boot frame skip
}

// ═════════════════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════════════════

void loop() {
  // 1. Check for incoming serial commands
  checkSerial();

  // 2. Heartbeat timeout — no PING for too long → back to IDLE
  if (currentState != STATE_IDLE &&
      millis() - lastHeartbeat > HEARTBEAT_TIMEOUT) {
    switchState(STATE_IDLE);
  }

  // 3. Run animation (non-blocking, advances frame when timer expires)
  runAnimation();

}
