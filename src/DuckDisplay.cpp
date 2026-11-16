/**
 * @file DuckDisplay.cpp
 * @brief Implementation of the DuckDisplay status screen.
 *
 * @author Brenton Poke
 * @license Apache 2.0
 */

#include "DuckDisplay.h"

#include <algorithm>
#include <cstdio>
#include "utils/DuckLogger.h"
#include "utils/DuckUtils.h"

namespace {

// u8g2_font_6x10_tr: fixed width, printable ASCII only. A 128x64 display fits
// 21 characters on each of 6 lines.
constexpr uint8_t kCharWidth = 6;
constexpr uint8_t kLineHeight = 10;
// Lines 0 and 1 (ID, status) are separated from the packet lines by a
// horizontal rule when the display has room for 6 lines.
constexpr uint8_t kRuleGap = 2;

#if DUCKDISPLAY_HAS_BOARD_OLED && defined(DUCKDISPLAY_OLED_CLASS)

#define DUCKDISPLAY_STRINGIFY_(x) #x
#define DUCKDISPLAY_STRINGIFY(x) DUCKDISPLAY_STRINGIFY_(x)

constexpr bool contains(const char* text, const char* part) {
    for (; *text; ++text) {
        const char* a = text;
        const char* b = part;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (!*b) {
            return true;
        }
    }
    return false;
}

// The _SW_I2C and _HW_I2C constructors take the same pins in a different
// order, so the class name decides which one to call.
constexpr const char* kOledClassName = DUCKDISPLAY_STRINGIFY(DUCKDISPLAY_OLED_CLASS);
constexpr bool kOledHardwareI2C = contains(kOledClassName, "_HW_I2C");
static_assert(!contains(kOledClassName, "_2ND_HW_I2C") &&
                  (kOledHardwareI2C || contains(kOledClassName, "_SW_I2C")),
              "DUCKDISPLAY_OLED_CLASS must be a U8g2 _SW_I2C or _HW_I2C class. For any "
              "other driver, construct it in the sketch and pass it to DuckDisplay(U8G2&).");

/**
 * @brief The board's OLED, constructed on first use so that a sketch that
 * passes its own driver doesn't get a second one.
 */
U8G2* boardOled() {
    const uint8_t clock = static_cast<uint8_t>(CDPCFG_PIN_OLED_CLOCK);
    const uint8_t data = static_cast<uint8_t>(CDPCFG_PIN_OLED_DATA);
    const uint8_t reset = static_cast<uint8_t>(DUCKDISPLAY_OLED_RESET);  // -1 becomes U8X8_PIN_NONE
    if constexpr (kOledHardwareI2C) {
        static DUCKDISPLAY_OLED_CLASS oled(DUCKDISPLAY_OLED_ROTATION, reset, clock, data);
        return &oled;
    } else {
        static DUCKDISPLAY_OLED_CLASS oled(DUCKDISPLAY_OLED_ROTATION, clock, data, reset);
        return &oled;
    }
}

#else

U8G2* boardOled() { return nullptr; }

#endif

/// Payload bytes as printable text, at most DUCKDISPLAY_PREVIEW_LEN characters.
std::string preview(const uint8_t* data, size_t length) {
    const size_t take = std::min(length, static_cast<size_t>(DUCKDISPLAY_PREVIEW_LEN));
    std::string text = duckutils::toString(std::vector<uint8_t>(data, data + take));
    if (text.size() > DUCKDISPLAY_PREVIEW_LEN) {
        text.resize(DUCKDISPLAY_PREVIEW_LEN);
    }
    return text;
}

/// Duck::getSignalScore() scales RSSI and SNR to 0..1; this undoes it.
float rssiDbm(const SignalScore& signal) { return signal.rssi * (RSSI_MAX - RSSI_MIN) + RSSI_MIN; }
float snrDb(const SignalScore& signal) { return signal.snr * (SNR_MAX - SNR_MIN) + SNR_MIN; }

}  // namespace

// ---------------------------------------------------------------------------
// duckdisplay helpers
// ---------------------------------------------------------------------------

namespace duckdisplay {

std::string topicName(uint8_t topic) {
    CdpPacket packet;  // topicToString() is a (non-const) member of CdpPacket
    packet.topic = topic;
    std::string name = packet.topicToString();
    if (name == "unknown") {
        char hex[5];
        snprintf(hex, sizeof(hex), "0x%02X", topic);
        name = hex;
    }
    return name;
}

const char* duckTypeName(DuckType type) {
    switch (type) {
        case DuckType::PAPA:
            return "Papa";
        case DuckType::MAMA:
            return "Mama";
        case DuckType::LINK:
            return "Link";
        case DuckType::DETECTOR:
            return "Detector";
        default:
            return "Duck";
    }
}

std::string duidLabel(const Duid& duid) {
    if (duid == PAPADUCK_DUID) {
        return "papa";
    }
    if (duid == BROADCAST_DUID) {
        return "all";
    }
    // A duck ID shorter than 8 characters is padded with zero bytes.
    size_t length = duid.size();
    while (length > 0 && duid[length - 1] == 0) {
        --length;
    }
    return duckutils::toString(std::vector<uint8_t>(duid.begin(), duid.begin() + length));
}

}  // namespace duckdisplay

// ---------------------------------------------------------------------------
// DuckDisplay
// ---------------------------------------------------------------------------

DuckDisplay::DuckDisplay() : oled_(boardOled()) {}

DuckDisplay::DuckDisplay(U8G2& oled) : oled_(&oled) {}

bool DuckDisplay::begin() {
    if (!oled_) {
        logwarn_ln("[DISPLAY] No display: the CDP board header defines no OLED pins "
                   "(on a T-Beam, build with -D ENABLE_DISPLAY)");
        return false;
    }

    constexpr int vext = DUCKDISPLAY_PIN_VEXT;
    if (vext >= 0) {
        pinMode(static_cast<uint8_t>(vext), OUTPUT);
        digitalWrite(static_cast<uint8_t>(vext), LOW);  // Vext on
        delay(DUCKDISPLAY_VEXT_SETTLE_MS);
    }

    // Before begin(): U8g2 only picks its default clock when none is set.
    if (DUCKDISPLAY_I2C_BUS_CLOCK > 0) {
        oled_->setBusClock(DUCKDISPLAY_I2C_BUS_CLOCK);
    }
    // With a hardware-I2C driver, begin() calls Wire.begin(data, clock). If the
    // sketch already started Wire (for example on GPIO21/22 for the T-Beam's
    // PMU), that call changes nothing and the display shares the bus.
    if (!oled_->begin()) {
        logerr_ln("[DISPLAY] U8g2 begin() failed");
        return false;
    }
    oled_->setFont(u8g2_font_6x10_tr);
    oled_->setFontPosTop();
    oled_->setDrawColor(1);

    const uint16_t width = oled_->getDisplayWidth();
    const uint16_t height = oled_->getDisplayHeight();
    textCols_ = static_cast<uint8_t>(std::min<uint16_t>(width / kCharWidth, 255));
    textRows_ = static_cast<uint8_t>(std::min<uint16_t>(height / kLineHeight, 255));
    tileCols_ = oled_->getBufferTileWidth();
    tileRows_ = oled_->getRows();
    fullBuffer_ = oled_->getBufferTileHeight() == tileRows_;

    ready_ = true;
    asleep_ = false;
    dirty_ = true;
    rowsLeft_ = 0;
    framed_ = false;
    loginfo_ln("[DISPLAY] %ux%u, %u lines of %u characters, %s buffer", unsigned(width),
               unsigned(height), unsigned(textRows_), unsigned(textCols_),
               fullBuffer_ ? "full" : "page");
    return true;
}

void DuckDisplay::setDuckId(const std::string& id) {
    if (id != duckId_) {
        duckId_ = id;
        markDirty();
    }
}

void DuckDisplay::setDuckId(const std::vector<uint8_t>& id) {
    size_t length = id.size();
    while (length > 0 && id[length - 1] == 0) {
        --length;
    }
    setDuckId(duckutils::toString(std::vector<uint8_t>(id.begin(), id.begin() + length)));
}

void DuckDisplay::setDuckType(DuckType type) {
    if (type != duckType_) {
        duckType_ = type;
        markDirty();
    }
}

void DuckDisplay::setStatus(const std::string& status) {
    if (status != status_) {
        status_ = status;
        markDirty();
    }
}

void DuckDisplay::showReceived(const CdpPacket& packet) {
    ++received_;
    PacketInfo info;
    info.valid = true;
    info.received = true;
    info.topic = packet.topic;
    info.topicName = duckdisplay::topicName(packet.topic);
    info.peer = duckdisplay::duidLabel(packet.sduid);
    info.preview = preview(packet.data.data(), packet.data.size());
    info.hopCount = packet.hopCount;
    if (packet.receivedSignal) {
        info.hasSignal = true;
        info.rssiDbm = rssiDbm(*packet.receivedSignal);
        info.snrDb = snrDb(*packet.receivedSignal);
    }
    info.number = received_;
    info.atMs = millis();
    last_ = std::move(info);
    markDirty();
}

void DuckDisplay::showSent(uint8_t topic, const std::string& payload, const Duid& destination) {
    showSent(topic, reinterpret_cast<const uint8_t*>(payload.data()), payload.size(), destination);
}

void DuckDisplay::showSent(uint8_t topic, const uint8_t* data, size_t length, const Duid& destination) {
    ++sent_;
    PacketInfo info;
    info.valid = true;
    info.received = false;
    info.topic = topic;
    info.topicName = duckdisplay::topicName(topic);
    info.peer = duckdisplay::duidLabel(destination);
    info.preview = data ? preview(data, length) : std::string();
    info.number = sent_;
    info.atMs = millis();
    last_ = std::move(info);
    markDirty();
}

void DuckDisplay::showLogo(uint32_t durationMs) {
    showLogo(duckdisplay::DUCK_LOGO_XBM, duckdisplay::DUCK_LOGO_WIDTH, duckdisplay::DUCK_LOGO_HEIGHT,
             durationMs);
}

void DuckDisplay::showLogo(const uint8_t* xbm, uint16_t width, uint16_t height, uint32_t durationMs) {
    if (!ready_ || !xbm) {
        return;
    }
    if (width > oled_->getDisplayWidth() || height > oled_->getDisplayHeight()) {
        logwarn_ln("[DISPLAY] Logo %ux%u is larger than the display, not shown", unsigned(width),
                   unsigned(height));
        return;
    }
    logo_ = xbm;
    logoWidth_ = width;
    logoHeight_ = height;
    logoMs_ = durationMs;
    logoDrawn_ = false;
    markDirty();
}

void DuckDisplay::update() {
    if (!ready_ || asleep_) {
        return;
    }
    if (rowsLeft_ > 0) {
        sendRows(DUCKDISPLAY_TILE_ROWS_PER_UPDATE);
        return;
    }
    const unsigned long now = millis();
    if (logo_ && logoDrawn_) {
        if (now - logoAtMs_ < logoMs_) {
            return;  // the logo stays up; changes wait for the status screen
        }
        logo_ = nullptr;
        markDirty();
    }
    if (!dirty_ || (framed_ && now - frameAtMs_ < refreshMs_)) {
        return;
    }
    startFrame();
    if (rowsLeft_ > 0) {
        sendRows(DUCKDISPLAY_TILE_ROWS_PER_UPDATE);
    }
}

void DuckDisplay::refresh() {
    if (!ready_ || asleep_) {
        return;
    }
    if (logo_ && logoDrawn_ && millis() - logoAtMs_ >= logoMs_) {
        logo_ = nullptr;
    }
    startFrame();
    if (rowsLeft_ > 0) {
        sendRows(rowsLeft_);
    }
}

void DuckDisplay::sleep() {
    if (!ready_ || asleep_) {
        return;
    }
    oled_->setPowerSave(1);
    asleep_ = true;
    if (rowsLeft_ > 0) {
        rowsLeft_ = 0;  // the frame was cut short; wake() sends a whole one
        markDirty();
    }
}

void DuckDisplay::wake() {
    if (!ready_ || !asleep_) {
        return;
    }
    oled_->setPowerSave(0);
    asleep_ = false;
    markDirty();
}

void DuckDisplay::startFrame() {
    const bool logo = logo_ != nullptr;
    if (fullBuffer_) {
        oled_->clearBuffer();
        render();
        nextRow_ = 0;
        rowsLeft_ = tileRows_;
    } else {
        // Page buffer: U8g2 draws and sends one page at a time, so the frame
        // goes out here, at once.
        oled_->firstPage();
        do {
            render();
        } while (oled_->nextPage());
        rowsLeft_ = 0;
    }
    dirty_ = false;
    framed_ = true;
    frameAtMs_ = millis();
    if (logo && !logoDrawn_) {
        logoDrawn_ = true;
        logoAtMs_ = frameAtMs_;
    }
}

void DuckDisplay::sendRows(uint8_t count) {
    const uint8_t rows = std::min(count, rowsLeft_);
    oled_->updateDisplayArea(0, nextRow_, tileCols_, rows);
    nextRow_ += rows;
    rowsLeft_ -= rows;
    if (rowsLeft_ == 0) {
        oled_->refreshDisplay();  // nothing on an OLED; e-paper needs it
    }
}

void DuckDisplay::render() {
    if (logo_) {
        const uint16_t x = (oled_->getDisplayWidth() - logoWidth_) / 2;
        const uint16_t y = (oled_->getDisplayHeight() - logoHeight_) / 2;
        oled_->drawXBM(x, y, logoWidth_, logoHeight_, logo_);
        return;
    }
    drawStatusScreen();
}

void DuckDisplay::drawStatusScreen() {
    // 128x64:
    //   MAMA0001         Mama
    //   Running
    //   ---------------------
    //   RX sensor         #12
    //   from DUCK0002
    //   {"count":12,"uptimeS
    //   -87dBm 9.5dB    hop 1
    drawLine(0, duckId_.empty() ? std::string("ClusterDuck") : duckId_,
             duckType_ == DuckType::UNKNOWN ? std::string() : std::string(duckdisplay::duckTypeName(duckType_)));
    drawLine(1, status_.empty() ? "CDP " + duckutils::getCDPVersion() : status_);
    if (textRows_ >= 6) {
        oled_->drawHLine(0, 2 * kLineHeight, oled_->getDisplayWidth());
    }

    if (!last_.valid) {
        drawLine(2, "No packets yet");
        return;
    }
    const PacketInfo& p = last_;
    drawLine(2, (p.received ? "RX " : "TX ") + p.topicName, "#" + std::to_string(p.number));
    drawLine(3, (p.received ? "from " : "to ") + p.peer);
    drawLine(4, p.preview);
    if (p.received) {
        char signal[24] = "";
        if (p.hasSignal) {
            snprintf(signal, sizeof(signal), "%.0fdBm %.1fdB", p.rssiDbm, p.snrDb);
        }
        drawLine(5, signal, "hop " + std::to_string(p.hopCount));
    }
}

void DuckDisplay::drawLine(uint8_t row, const std::string& left, const std::string& right) {
    if (row >= textRows_) {
        return;
    }
    const uint16_t y = row * kLineHeight + ((textRows_ >= 6 && row >= 2) ? kRuleGap : 0);
    // The right-hand text is optional: it is left out when it doesn't fit
    // next to the whole left-hand text.
    const bool withRight = !right.empty() && left.size() + 1 + right.size() <= textCols_;
    const std::string text = left.substr(0, textCols_);
    if (!text.empty()) {
        oled_->drawStr(0, y, text.c_str());
    }
    if (withRight) {
        oled_->drawStr(oled_->getDisplayWidth() - right.size() * kCharWidth, y, right.c_str());
    }
}
