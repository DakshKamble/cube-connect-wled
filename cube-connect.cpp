#include "wled.h"

/*
 * ============================================================
 * CubeConnect Usermod
 * ============================================================
 *
 * Hardware:
 *
 *   XIAO ESP32-C3
 *
 *   GPIO 4 -------- BUTTON -------- GND
 *
 *
 * Behaviour:
 *
 *   Normal WLED animation
 *          |
 *          | Button pressed
 *          v
 *   Entire LED matrix -> RED
 *          |
 *        500 ms
 *          |
 *          v
 *   Previous WLED animation continues
 *
 *
 * IMPORTANT:
 * This usermod does NOT stop the WLED effect.
 *
 * Instead, handleOverlayDraw() temporarily replaces the
 * rendered pixels with red immediately before WLED displays
 * them.
 *
 * Therefore the existing WLED animation continues running
 * underneath the red flash.
 * ============================================================
 */


class CubeConnectUsermod : public Usermod {

private:

  // ==========================================================
  // CONFIGURATION
  // ==========================================================

  // Button connected between GPIO 4 and GND
  static const uint8_t BUTTON_PIN = 4;

  // Red flash duration in milliseconds
  static const uint32_t FLASH_DURATION = 500;


  // ==========================================================
  // BUTTON STATE
  // ==========================================================

  bool lastButtonState = HIGH;

  // Used for simple button debounce
  uint32_t lastButtonChange = 0;

  static const uint32_t DEBOUNCE_TIME = 50;


  // ==========================================================
  // FLASH STATE
  // ==========================================================

  bool flashing = false;

  uint32_t flashStartTime = 0;


  // ==========================================================
  // PRIVATE FUNCTION
  // ==========================================================

  void startRedFlash()
  {
    // Don't restart the timer if the button is pressed
    // repeatedly while a flash is already happening.
    if (flashing) {
      return;
    }

    flashing = true;
    flashStartTime = millis();

    Serial.println(F("CubeConnect: RED FLASH START"));
  }


public:

  // ==========================================================
  // SETUP
  // ==========================================================

  void setup() override
  {
    /*
     * Button is connected to GND.
     *
     * INPUT_PULLUP means:
     *
     * Button released -> HIGH
     * Button pressed  -> LOW
     */
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println(F("CubeConnect Usermod initialized"));
    Serial.print(F("Button GPIO: "));
    Serial.println(BUTTON_PIN);
  }


  // ==========================================================
  // LOOP
  // ==========================================================

  void loop() override
  {
    uint32_t now = millis();

    bool buttonState = digitalRead(BUTTON_PIN);


    // ========================================================
    // BUTTON PRESS DETECTION
    // ========================================================

    /*
     * Detect HIGH -> LOW transition.
     *
     * HIGH = button released
     * LOW  = button pressed
     */
    if (buttonState != lastButtonState) {

      if (now - lastButtonChange >= DEBOUNCE_TIME) {

        lastButtonChange = now;

        if (buttonState == LOW) {
          startRedFlash();
        }

        lastButtonState = buttonState;
      }
    }


    // ========================================================
    // END FLASH
    // ========================================================

    if (flashing) {

      if (now - flashStartTime >= FLASH_DURATION) {

        flashing = false;

        Serial.println(F("CubeConnect: RED FLASH END"));
      }
    }
  }


  // ==========================================================
  // LED OVERLAY
  // ==========================================================

  /*
   * This is the important part.
   *
   * WLED first renders its normal effect.
   *
   * Then handleOverlayDraw() is called.
   *
   * While "flashing" is true, we replace every pixel with RED.
   *
   * Therefore:
   *
   *   WLED effect
   *        ↓
   *   handleOverlayDraw()
   *        ↓
   *   ALL PIXELS = RED
   *        ↓
   *   strip.show()
   *
   * This happens for every frame during the 500 ms flash.
   */
  void handleOverlayDraw() override
  {
    if (!flashing) {
      return;
    }


    // ========================================================
    // MAKE EVERY LED RED
    // ========================================================

    uint32_t red = RGBW32(255, 0, 0, 0);

    uint16_t ledCount = strip.getLength();

    for (uint16_t i = 0; i < ledCount; i++) {
      strip.setPixelColor(i, red);
    }
  }


  // ==========================================================
  // JSON INFO
  // ==========================================================

  /*
   * This is optional.
   *
   * It makes CubeConnect information available through
   * /json/info.
   */
  void addToJsonInfo(JsonObject& root) override
  {
    JsonObject user = root["u"];

    if (user.isNull()) {
      user = root.createNestedObject("u");
    }

    user["CubeConnect"] = "Button Flash";
  }


  // ==========================================================
  // USERMOD CONFIGURATION
  // ==========================================================

  /*
   * These values appear in WLED's Usermods settings.
   *
   * For now the GPIO and flash duration are fixed constants,
   * but this keeps the usermod compatible with WLED's config
   * system.
   */
  void addToConfig(JsonObject& root) override
  {
    JsonObject top = root.createNestedObject("CubeConnect");

    top["buttonPin"] = BUTTON_PIN;
    top["flashDuration"] = FLASH_DURATION;
  }


  // ==========================================================
  // READ CONFIGURATION
  // ==========================================================

  bool readFromConfig(JsonObject& root) override
  {
    JsonObject top = root["CubeConnect"];

    if (top.isNull()) {
      return false;
    }

    return true;
  }


  // ==========================================================
  // USERMOD ID
  // ==========================================================

  uint16_t getId() override
  {
    return USERMOD_ID_UNSPECIFIED;
  }
};


// ============================================================
// REGISTER USERMOD
// ============================================================

static CubeConnectUsermod cubeConnectUsermod;

REGISTER_USERMOD(cubeConnectUsermod);