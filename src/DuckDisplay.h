/**
 * @file DuckDisplay.h
 * @brief OLED display QuackPack for ClusterDuck Protocol ducks.
 *
 * DuckDisplay draws a status screen on a U8g2 display: the duck's ID and
 * type, a status line set by the sketch, and the last packet the duck heard
 * or sent (topic, origin or destination, a payload preview and, for packets
 * it heard, RSSI and SNR). The sketch reports what happens
 * (showReceived(), showSent(), setStatus()); update() draws it.
 *
 * update() never sends a whole frame at once. It draws the screen into the
 * U8g2 buffer, which takes no bus time, then sends one row of tiles (8 pixel
 * rows) per call, so the duck's loop() keeps running while a frame goes out.
 *
 * The default constructor uses the board's OLED as the CDP board header
 * describes it (CDPCFG_PIN_OLED_* and CDPCFG_OLED_CLASS). Any other U8g2
 * driver can be passed to DuckDisplay(U8G2&).
 *
 * The library has no dependency on a particular CDP version: the Duck project
 * that includes it pins ClusterDuck Protocol itself.
 *
 * @author Brenton Poke
 * @license Apache 2.0
 */

#ifndef QUACKPACK_DUCKDISPLAY_H
#define QUACKPACK_DUCKDISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "include/cdpcfg.h"
#include "CdpPacket.h"
#include "Ducks/DuckTypes.h"

#define DUCKDISPLAY_VERSION "1.1.0"

// ---------------------------------------------------------------------------
// Compile-time options. Set them with -D in build_flags.
// ---------------------------------------------------------------------------

// Shortest time between two frames, in milliseconds. Packets that arrive
// faster are combined into the next frame. setRefreshInterval() changes it at
// run time.
#ifndef DUCKDISPLAY_REFRESH_MS
#define DUCKDISPLAY_REFRESH_MS 250
#endif

// Rows of tiles (8 pixel rows each) that one update() call sends. A 128x64
// display has 8. Each row is 128 bytes on the bus: roughly 3 ms on hardware
// I2C at 400 kHz, and in the order of 15 ms on U8g2's software I2C.
#ifndef DUCKDISPLAY_TILE_ROWS_PER_UPDATE
#define DUCKDISPLAY_TILE_ROWS_PER_UPDATE 1
#endif

// Characters of payload kept in PacketInfo::preview. The screen shows as many
// as fit on one line (21 on a 128-pixel-wide display).
#ifndef DUCKDISPLAY_PREVIEW_LEN
#define DUCKDISPLAY_PREVIEW_LEN 40
#endif

// Pin that switches the external 3.3 V rail (Vext) that powers the OLED on
// Heltec boards. begin() drives it LOW (rail on) first. -1: no such pin.
#ifndef DUCKDISPLAY_PIN_VEXT
#if defined(CDPCFG_PIN_VEXT)
#define DUCKDISPLAY_PIN_VEXT CDPCFG_PIN_VEXT
#elif defined(ARDUINO_HELTEC_WIFI_LORA_32_V3) || defined(ARDUINO_heltec_wifi_lora_32_V3) || \
    defined(ARDUINO_heltec_wifi_32_lora_V3)
#define DUCKDISPLAY_PIN_VEXT 36  // Vext in the Heltec V3 variant; CDP's header doesn't name it
#else
#define DUCKDISPLAY_PIN_VEXT -1
#endif
#endif

// Time begin() waits after switching Vext on, in milliseconds.
#ifndef DUCKDISPLAY_VEXT_SETTLE_MS
#define DUCKDISPLAY_VEXT_SETTLE_MS 100
#endif

// ---------------------------------------------------------------------------
// The board's OLED, for the default constructor.
//
// Pins come from the CDP board header. Boards without an OLED define none,
// and neither does the LilyGO T-Beam unless it is built with -D ENABLE_DISPLAY
// (its OLED is optional). The default DuckDisplay then has no display and
// begin() returns false.
// ---------------------------------------------------------------------------

#if defined(CDPCFG_PIN_OLED_CLOCK) && defined(CDPCFG_PIN_OLED_DATA) && !defined(CDPCFG_OLED_NONE)
#define DUCKDISPLAY_HAS_BOARD_OLED 1
#else
#define DUCKDISPLAY_HAS_BOARD_OLED 0
#endif

#if DUCKDISPLAY_HAS_BOARD_OLED

#ifndef DUCKDISPLAY_OLED_ROTATION
#ifdef CDPCFG_PIN_OLED_ROTATION
#define DUCKDISPLAY_OLED_ROTATION CDPCFG_PIN_OLED_ROTATION
#else
#define DUCKDISPLAY_OLED_ROTATION U8G2_R0
#endif
#endif

#ifndef DUCKDISPLAY_OLED_RESET
#ifdef CDPCFG_PIN_OLED_RESET
#define DUCKDISPLAY_OLED_RESET CDPCFG_PIN_OLED_RESET
#else
#define DUCKDISPLAY_OLED_RESET U8X8_PIN_NONE
#endif
#endif

// The U8g2 class of the board's OLED. It must be an _SW_I2C or _HW_I2C class.
//
// It is CDPCFG_OLED_CLASS, which uses software I2C, except on boards whose OLED
// is on GPIO21/22, the ESP32's Wire pins (the T-Beam, and TTGO LoRa32 V1 boards
// built without TTGO_1_3). Those boards also have their PMU (AXP192/AXP2101) on
// GPIO21/22, and XPowersLib drives it through Wire. Software I2C on the same
// pins would take them away from Wire, so there the display uses hardware I2C
// (Wire) too.
#ifndef DUCKDISPLAY_OLED_CLASS
#if (CDPCFG_PIN_OLED_DATA == 21) && (CDPCFG_PIN_OLED_CLOCK == 22)
#if defined(CDPCFG_OLED_64x32)
#define DUCKDISPLAY_OLED_CLASS U8G2_SSD1306_64X32_NONAME_F_HW_I2C
#else
#define DUCKDISPLAY_OLED_CLASS U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#endif
#elif defined(CDPCFG_OLED_CLASS)
#define DUCKDISPLAY_OLED_CLASS CDPCFG_OLED_CLASS
#endif
#endif

#endif  // DUCKDISPLAY_HAS_BOARD_OLED

namespace duckdisplay {

/// Width and height of DUCK_LOGO_XBM, in pixels.
constexpr uint16_t DUCK_LOGO_WIDTH = 128;
constexpr uint16_t DUCK_LOGO_HEIGHT = 64;
/// The ClusterDuck duck, 128x64 XBM. showLogo() without arguments draws it.
extern const uint8_t DUCK_LOGO_XBM[];

/**
 * @brief Name of a CDP topic, as CdpPacket::topicToString() gives it
 * ("sensor", "gps", "health", ...), or "0x" and two hex digits when CDP
 * doesn't name it.
 */
std::string topicName(uint8_t topic);

/// "Papa", "Mama", "Link", "Detector" or "Duck".
const char* duckTypeName(DuckType type);

/**
 * @brief A DUID as text for a small screen: "papa" for PAPADUCK_DUID, "all"
 * for BROADCAST_DUID, otherwise its bytes with trailing zero bytes removed
 * and any other non-printable byte as two hex digits.
 */
std::string duidLabel(const Duid& duid);

}  // namespace duckdisplay

/**
 * @brief Status screen for a Duck's OLED. See the file comment.
 *
 * Typical use, in a MamaDuck sketch:
 * @code
 * DuckDisplay display;                        // the board's OLED
 *
 * void onPacket(CdpPacket packet) { display.showReceived(packet); }
 *
 * void setup() {
 *   display.begin();
 *   display.setDuck(duck);
 *   duck.setupWithDefaults();
 *   duck.onReceiveDuckData(onPacket);
 *   display.setStatus("Running");
 * }
 *
 * void loop() {
 *   duck.run();
 *   display.update();
 * }
 * @endcode
 */
class DuckDisplay {
public:
    /// Default shortest time between two frames, in milliseconds.
    static constexpr uint32_t DEFAULT_REFRESH_MS = DUCKDISPLAY_REFRESH_MS;
    /// Default time showLogo() shows the logo, in milliseconds.
    static constexpr uint32_t DEFAULT_LOGO_MS = 2000;

    /// What the display shows about the last packet.
    struct PacketInfo {
        bool valid = false;      ///< false until the first showReceived() or showSent()
        bool received = false;   ///< true: the duck heard it; false: the duck sent it
        uint8_t topic = 0;       ///< CDP topic
        std::string topicName;   ///< duckdisplay::topicName(topic)
        std::string peer;        ///< heard: origin (sduid); sent: destination (duckdisplay::duidLabel())
        std::string preview;     ///< payload as text, at most DUCKDISPLAY_PREVIEW_LEN characters
        uint8_t hopCount = 0;    ///< heard: times it was relayed before it reached this duck
        bool hasSignal = false;  ///< heard, with RSSI and SNR from the radio
        float rssiDbm = 0;       ///< heard: RSSI in dBm
        float snrDb = 0;         ///< heard: SNR in dB
        uint32_t number = 0;     ///< heard: receivedCount(); sent: sentCount()
        unsigned long atMs = 0;  ///< millis() when it was reported
    };

    /**
     * @brief Uses the board's OLED, as the CDP board header describes it.
     * Does not touch the hardware. On a board without one, begin() returns
     * false and every other call does nothing.
     */
    DuckDisplay();

    /**
     * @brief Uses a U8g2 driver the sketch constructed, for example an
     * external display. Does not touch the hardware. A full-buffer driver
     * (_F_ in its class name) lets update() send a frame in parts; with a
     * page-buffer driver (_1_ or _2_) update() sends each frame at once.
     *
     * @param oled the driver; it must outlive the DuckDisplay
     */
    explicit DuckDisplay(U8G2& oled);

    /**
     * @brief Starts the display. Call once from setup(), before the other
     * calls show anything. Switches Vext on where the board has it
     * (DUCKDISPLAY_PIN_VEXT), resets and clears the display, which blocks for
     * about 0.1 s, plus DUCKDISPLAY_VEXT_SETTLE_MS on Heltec boards.
     *
     * @return false if there is no display to start (see DuckDisplay())
     */
    bool begin();

    /// begin() succeeded.
    bool ready() const { return ready_; }

    /// Shows the duck's ID, as text (at most 8 characters fit with the type).
    void setDuckId(const std::string& id);
    /// Shows the duck's ID, as Duck::getDuckId() returns it.
    void setDuckId(const std::vector<uint8_t>& id);
    /// Shows the duck's type next to its ID.
    void setDuckType(DuckType type);

    /**
     * @brief Shows a duck's ID and type: setDuckId(duck.getDuckId()) and
     * setDuckType(duck.getType()).
     */
    template <typename DuckT>
    void setDuck(DuckT& duck) {
        setDuckId(duck.getDuckId());
        setDuckType(duck.getType());
    }

    /**
     * @brief Sets the status line, under the ID. Empty shows the CDP version.
     * Setting the text it already shows does not redraw.
     */
    void setStatus(const std::string& status);

    /**
     * @brief Shows a packet the duck heard. Call it from the
     * onReceiveDuckData() callback. Counts it in receivedCount().
     */
    void showReceived(const CdpPacket& packet);

    /**
     * @brief Shows a packet the duck sent. Call it after sendData() returned
     * DUCK_ERR_NONE (the packet is queued). Counts it in sentCount().
     *
     * @param topic the topic passed to sendData()
     * @param payload the data passed to sendData()
     * @param destination the target passed to sendData() (PAPADUCK_DUID by default)
     */
    void showSent(uint8_t topic, const std::string& payload,
                  const Duid& destination = PAPADUCK_DUID);
    /// showSent() for a byte payload.
    void showSent(uint8_t topic, const uint8_t* data, size_t length,
                  const Duid& destination = PAPADUCK_DUID);

    /**
     * @brief Shows the ClusterDuck logo for durationMs, then the status screen.
     * The time starts when the logo is drawn. Call it after begin(); before,
     * it does nothing.
     */
    void showLogo(uint32_t durationMs = DEFAULT_LOGO_MS);

    /**
     * @brief Shows an XBM image, centred, for durationMs, then the status
     * screen. Skipped if it is larger than the display.
     *
     * @param xbm image data; must stay valid while the image shows
     */
    void showLogo(const uint8_t* xbm, uint16_t width, uint16_t height,
                  uint32_t durationMs = DEFAULT_LOGO_MS);

    /**
     * @brief Call on every loop() pass. When something changed and at least
     * the refresh interval has passed, draws the screen into the buffer; then
     * sends DUCKDISPLAY_TILE_ROWS_PER_UPDATE rows of it per call until the
     * frame is out.
     */
    void update();

    /**
     * @brief Draws and sends the current screen at once. This blocks (about
     * 25 ms on hardware I2C, about 0.1 s on software I2C). Meant for setup(),
     * to show the logo or a status while other parts start; in loop(), use
     * update().
     */
    void refresh();

    /**
     * @brief Turns the display off (controller power save, a few µA; the
     * picture is kept). Packets are still recorded while it is off. Call it
     * before deep sleep.
     */
    void sleep();

    /// Turns the display back on and redraws the current screen.
    void wake();

    /// sleep() was called and wake() was not.
    bool asleep() const { return asleep_; }

    /// Changes the shortest time between two frames (DUCKDISPLAY_REFRESH_MS).
    void setRefreshInterval(uint32_t ms) { refreshMs_ = ms; }

    /// The last packet shown.
    const PacketInfo& lastPacket() const { return last_; }
    /// Packets passed to showReceived() since start-up.
    uint32_t receivedCount() const { return received_; }
    /// Packets passed to showSent() since start-up.
    uint32_t sentCount() const { return sent_; }

    /**
     * @brief The U8g2 driver, for settings such as setContrast() or
     * setI2CAddress(), or nullptr when there is no display. Drawing on it
     * directly is overwritten by the next frame.
     */
    U8G2* oled() { return oled_; }

private:
    void markDirty() { dirty_ = true; }
    void render();
    void drawStatusScreen();
    void drawLine(uint8_t row, const std::string& left, const std::string& right = std::string());
    void startFrame();
    void sendRows(uint8_t count);

    U8G2* oled_ = nullptr;
    bool ready_ = false;
    bool asleep_ = false;
    bool dirty_ = true;
    bool fullBuffer_ = false;

    // geometry, set by begin()
    uint8_t tileCols_ = 0;   // buffer width in tiles
    uint8_t tileRows_ = 0;   // display height in tiles
    uint8_t textCols_ = 0;   // characters per line
    uint8_t textRows_ = 0;   // lines per screen

    // frame being sent
    uint8_t nextRow_ = 0;
    uint8_t rowsLeft_ = 0;
    bool framed_ = false;    // at least one frame was drawn
    unsigned long frameAtMs_ = 0;
    uint32_t refreshMs_ = DEFAULT_REFRESH_MS;

    // logo
    const uint8_t* logo_ = nullptr;
    uint16_t logoWidth_ = 0;
    uint16_t logoHeight_ = 0;
    uint32_t logoMs_ = 0;
    bool logoDrawn_ = false;
    unsigned long logoAtMs_ = 0;

    // content
    std::string duckId_;
    DuckType duckType_ = DuckType::UNKNOWN;
    std::string status_;
    PacketInfo last_;
    uint32_t received_ = 0;
    uint32_t sent_ = 0;
};

#endif  // QUACKPACK_DUCKDISPLAY_H
