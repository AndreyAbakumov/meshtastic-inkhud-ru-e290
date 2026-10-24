// Exercises production decoding, font metrics, wrapping and rasterization without board/network stubs.
// Applet methods are extracted verbatim at test time; only the pixel sink and Arduino APIs are replaced.
#include "graphics/niche/InkHUD/AppletFont.h"
#include "graphics/niche/InkHUD/Localization.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <deque>
#include <cmath>
#include <ctime>
#include "graphics/niche/InkHUD/Applets/System/LockScreen/UnreadInbox.h"
enum class RTCQuality { RTCQualityDevice };
uint32_t getValidTime(RTCQuality, bool) { return 1792830840; }
#include "graphics/niche/InkHUD/Applets/System/Menu/MenuItem.h"
#define LOG_ERROR(...) ((void)0)
#define IS_ONE_OF(value, a, b) ((value) == (a) || (value) == (b))
using NodeNum = uint32_t;
struct meshtastic_NodeInfoLite {
    bool has_user = true;
    NodeNum num = 0x12345678;
    struct { std::string short_name = "ЁЖИК"; } user;
};
struct TestNodeDB {
    meshtastic_NodeInfoLite node;
    void saveToDisk(int) {}
    meshtastic_NodeInfoLite *getMeshNode(uint32_t) { return &node; }
} testNodeDB;
constexpr int meshtastic_Config_LoRaConfig_RegionCode_UNSET = 0;
constexpr int SEGMENT_DEVICESTATE = 0;
struct { struct { int region = 0; } lora; struct { char tzdef[64] = ""; } device;
    struct { bool flip_screen = true; } display; } config;
struct { struct { bool enabled = false; } joystick; } testSettings;
TestNodeDB *nodeDB = &testNodeDB;

namespace NicheGraphics::InkHUD {
struct MessageStore {
    struct Message { NodeNum sender = 0x12345678; std::string text = "Привет, мир!"; };
};
struct TestPersistence { struct { MessageStore::Message broadcast, dm; } latestMessage; } testPersistence;
struct TestInkHUD { bool twoWayRocker = false; TestPersistence *persistence = &testPersistence; } testInkHUD;
struct Notification { enum class Type { NOTIFICATION_MESSAGE_DIRECT, NOTIFICATION_MESSAGE_BROADCAST }; };
class Applet : public GFX {
public:
    enum HorizontalAlignment { LEFT, CENTER, RIGHT };
    enum VerticalAlignment { TOP, MIDDLE, BOTTOM };
    Applet(int w=296, int h=128) : GFX(w, h), pixels(w * h, 255) { setTextWrap(false); }
    inline static AppletFont fontSmall = FREESANS_6PT_WIN1251;
    inline static AppletFont fontMedium = FREESANS_9PT_WIN1251;
    inline static AppletFont fontLarge = FREESANS_12PT_WIN1251;
    int X(float x) { return x * width(); }
    int Y(float y) { return y * height(); }
    AppletFont currentFont;
    TestInkHUD *inkhud = &testInkHUD;
    int16_t cropLeft = 0, cropTop = 0;
    uint16_t cropWidth = 296, cropHeight = 128;
    std::vector<uint8_t> pixels;
    int minX = 32767, maxX = -1, minY = 32767, maxY = -1;
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (cropWidth && cropHeight) {
            minX = std::min(minX, int(x)); maxX = std::max(maxX, int(x));
            minY = std::min(minY, int(y)); maxY = std::max(maxY, int(y));
        }
        if (x >= cropLeft && x < cropLeft + cropWidth && y >= cropTop && y < cropTop + cropHeight &&
            x >= 0 && x < width() && y >= 0 && y < height()) pixels[y * width() + x] = color ? 0 : 255;
    }
    void setCrop(int16_t l, int16_t t, uint16_t w, uint16_t h) {
        cropLeft=l; cropTop=t; cropWidth=w; cropHeight=h;
    }
    void save(const std::string &path) {
        std::ofstream f(path, std::ios::binary);
        f << "P5\n" << width() << " " << height() << "\n255\n";
        f.write(reinterpret_cast<const char *>(pixels.data()), pixels.size());
    }
    void setFont(AppletFont f);
    AppletFont getFont();
    std::string parse(const std::string &text);
    uint16_t getTextWidth(const char *text);
    uint16_t getTextWidth(const std::string &text);
    void printWrapped(int16_t left, int16_t top, uint16_t width, const std::string &text);
    uint32_t getWrappedTextHeight(int16_t left, uint16_t width, const std::string &text);
    void printAt(int16_t x, int16_t y, const char *text, HorizontalAlignment ha=LEFT, VerticalAlignment va=TOP);
    void printAt(int16_t x, int16_t y, const std::string &text, HorizontalAlignment ha=LEFT, VerticalAlignment va=TOP);
    std::string ellipsizeToWidth(std::string value, uint16_t width);
    std::string parseShortName(meshtastic_NodeInfoLite *node);
    bool isPrintable(const std::string &text);
    std::string hexifyNodeNum(NodeNum num);
};
class MenuApplet : public Applet {
public:
    using Applet::Applet;
    std::vector<MenuItem> items;
    bool freeTextMode = false, cursorShown = true;
    uint8_t cursor = 0;
    uint16_t systemInfoPanelHeight = 58;
    MenuPage currentPage = ROOT;
    struct { struct { std::string rawText; } freeTextItem; } cm;
    void onRender(bool full);
    void drawInputField(int,int,int,int,const std::string &) {}
    void drawSystemInfoPanel(int16_t, int16_t, uint16_t, uint16_t * = nullptr);
    std::string getTimeString() { return "12:34"; }
};
class LockScreenApplet : public Applet {
public:
    using Applet::Applet;
    UnreadInbox inbox;
    std::string clockValue = "12:34";
    std::string getTimeString() { return clockValue; }
    void onRender(bool full);
};
class TipsApplet : public Applet {
public:
    using Applet::Applet;
    enum class Tip { WELCOME, FINISH_SETUP, PICK_REGION, SAFE_SHUTDOWN, CUSTOMIZATION, BUTTONS, ROTATION };
    std::deque<Tip> tipQueue;
    decltype(testSettings) *settings = &testSettings;
    void onRender(bool full);
    void renderWelcome();
    uint16_t getLogoWidth(uint16_t w, uint16_t h) { return std::min<uint16_t>(w, h * 2); }
    uint16_t getLogoHeight(uint16_t w, uint16_t h) { return std::min<uint16_t>(w / 2, h); }
    void drawLogo(int, int, int, int) {} // Logo artwork is outside text-layout tests.
};
class NotificationApplet : public Applet {
public:
    bool hasNotification = true;
    struct { Notification::Type type = Notification::Type::NOTIFICATION_MESSAGE_BROADCAST; } currentNotification;
    std::string getNotificationText(uint16_t widthAvailable);
};
}
using namespace NicheGraphics;
#include "applet_methods.inc"
#include "notification_method.inc"
#include "tips_methods.inc"
constexpr uint16_t BLACK = 1, WHITE = 0;
struct {
    int charge=85; bool present=true;
    int getBatteryVoltageMv() { return 4200; }
    int getBatteryChargePercent() { return charge; }
    bool getHasBattery() { return present; }
} testPower;
auto *powerStatus = &testPower;
struct { float channelUtilizationPercent() { return 100; } float utilizationTXPercent() { return 100; } } testAir;
auto *airTime = &testAir;
#include "menu_methods.inc"
using std::to_string;
#include "lock_render.inc"
using InkHUD::Applet;
using InkHUD::AppletFont;

const std::vector<std::string> samples = {
    "Привет, мир!", "Ёжик в тумане", "Съешь ещё этих мягких французских булок",
    "Проверка связи", "Сообщение получено", "Привет, Mesh!", "Node Воронеж 01",
    "Температура: −12°C", "Заряд батареи: 85%", "Русский + English 123"
};

int main(int argc, char **argv) {
    const bool baseline = argc > 1 && std::string(argv[1]) == "--baseline";
    std::vector<AppletFont> fonts = {FREESANS_6PT_WIN1251, FREESANS_9PT_WIN1251, FREESANS_12PT_WIN1251};
    for (size_t i=0; i<fonts.size(); ++i) {
        auto f = fonts[i];
#include "locale_expected.inc"
        std::cout << "font " << i << " height=" << int(f.lineHeight()) << " ascent=" << int(f.heightAboveCursor())
                  << " descent=" << int(f.heightBelowCursor()) << '\n';
        for (unsigned b=0xC0; b<=0xFF; ++b) {
            unsigned cp=0x410+b-0xC0;
            std::string u{char(0xC0 | cp>>6), char(0x80 | (cp & 63))};
            assert(f.decodeUTF8(u) == std::string(1, char(b)));
            auto g=f.gfxFont->glyph[b-f.gfxFont->first];
            assert(g.width && g.height && g.xAdvance);
        }
        assert(f.decodeUTF8("ЁёІіЇїЄєҐґЎў") == std::string("\xA8\xB8\xB2\xB3\xAF\xBF\xAA\xBA\xA5\xB4\xA1\xA2"));
        assert(f.decodeUTF8("").empty());
        assert(f.decodeUTF8("ASCII 0123!?,.-+%") == "ASCII 0123!?,.-+%");
        for (size_t j=0; j<samples.size(); ++j) {
            Applet a; a.setFont(f);
            auto text=a.parse(samples[j]);
            if (!baseline) assert(text.find('\x1A') == std::string::npos);
            a.printWrapped(2, 2, 292, text);
            a.save("tests/build/sample-"+std::to_string(i)+"-"+std::to_string(j)+".pgm");
        }
    }
    if (baseline) {
        Applet a; a.setFont(FREESANS_9PT_WIN1252);
        a.printWrapped(2, 2, 292, a.parse("Привет, мир!\nNode Воронеж 01"));
        a.save("tests/build/baseline.pgm");
        std::cout << "Baseline: Cyrillic maps to SUB; malformed input tests deferred to patched version.\n";
        return 0;
    }
    for (const auto &entry : InkHUD::translations) {
        assert(std::string(INKHUD_TEXT(entry.english)) == entry.russian);
        for (auto font : fonts) {
            for (const unsigned char b : std::string(entry.russian)) {
                if (b == '\n' || b == ' ') continue;
                assert(b >= font.gfxFont->first && b <= font.gfxFont->last);
                auto g = font.gfxFont->glyph[b - font.gfxFont->first];
                assert(g.width && g.height && g.xAdvance);
            }
        }
    }
    for (int rotation = 0; rotation < 2; ++rotation) {
        int w = rotation ? 128 : 296, h = rotation ? 296 : 128;
        for (int tip = 0; tip < 7; ++tip) {
            InkHUD::TipsApplet a(w,h); a.setCrop(0,0,w,h);
            a.tipQueue.push_back(static_cast<InkHUD::TipsApplet::Tip>(tip));
            a.onRender(true);
            a.save("tests/build/tip-" + std::to_string(rotation) + "-" + std::to_string(tip) + ".pgm");
            if (!(a.minX >= 0 && a.maxX < w && a.minY >= 0 && a.maxY < h)) {
                std::cerr << "Tip bounds: " << rotation << ":" << tip << " x=" << a.minX << "," << a.maxX << std::endl;
                assert(false);
            }
        }
    }
    for (int rotation = 0; rotation < 2; ++rotation) {
        int w = rotation ? 128 : 296, h = rotation ? 296 : 128;
        InkHUD::MenuApplet menu(w,h); menu.setCrop(0,0,w,h); menu.setFont(fonts[0]);
        for (auto key : {"Send", "Options", "Node Config", "Save & Shut Down", "Exit"})
            menu.items.emplace_back(INKHUD_TEXT(key), InkHUD::EXIT);
        menu.cursorShown = false;
        menu.onRender(true);
        menu.save("tests/build/menu-root-" + std::to_string(rotation) + ".pgm");
        menu.currentPage = InkHUD::OPTIONS; menu.items.clear(); menu.cursorShown = true;
        for (auto key : {"Back", "Applets", "Auto-show", "Recents Duration", "Layout", "Rotate", "Notifications", "Battery Icon", "Invert Color", "Exit"})
            menu.items.emplace_back(INKHUD_TEXT(key), InkHUD::EXIT);
        for (uint8_t cursor = 0; cursor < menu.items.size(); ++cursor) {
            std::fill(menu.pixels.begin(), menu.pixels.end(), 255); menu.cursor = cursor;
            menu.onRender(true);
            menu.save("tests/build/menu-options-" + std::to_string(rotation) + "-" + std::to_string(cursor) + ".pgm");
        }
    }
    std::cout << "PASS: UI translations, actual Tips renderer in both orientations, menu scrolling and 100% indicators.\n";
    for (int rotation=0; rotation<2; ++rotation) {
        const int w=rotation?128:296, h=rotation?296:128;
        for (int count : {0,1,3,20}) {
            InkHUD::LockScreenApplet lock(w,h); lock.setCrop(0,0,w,h);
            for (int i=0; i<count; ++i)
                lock.inbox.add({uint32_t(i+1), 12, 0, 0, i%2==0, "Привет! Встречаемся у станции в 18:00."});
            lock.onRender(true);
            assert(lock.minX>=0 && lock.maxX<w && lock.minY>=0 && lock.maxY<h);
            lock.save("tests/build/lock-"+std::to_string(rotation)+"-"+std::to_string(count)+".pgm");
        }
        InkHUD::LockScreenApplet lock(w,h); lock.setCrop(0,0,w,h); lock.clockValue="";
        lock.onRender(true);
        assert(lock.minX>=0 && lock.maxX<w && lock.minY>=0 && lock.maxY<h);
        lock.save("tests/build/lock-no-time-"+std::to_string(rotation)+".pgm");
    }
    for (int rotation=0; rotation<2; ++rotation) {
        const int w=rotation?128:296,h=rotation?296:128;
        InkHUD::LockScreenApplet reused(w,h), fresh(w,h);
        reused.setCrop(0,0,w,h); fresh.setCrop(0,0,w,h);
        reused.clockValue="11:59";
        reused.inbox.add({1,12,0,0,true,"Длинное сообщение"});
        reused.onRender(true);
        reused.clockValue="12:00";reused.inbox.clear();reused.onRender(false);
        fresh.clockValue="12:00";fresh.onRender(true);
        assert(reused.pixels==fresh.pixels);
    }
    std::cout<<"PASS: repeated lockscreen redraw clears previous digits and messages in both orientations.\n";

    for (int rotation=0; rotation<2; ++rotation) {
        const int w=rotation?128:296, h=rotation?296:128;
        for (int charge : {0,1,50,99,100,101,255}) {
            testPower.charge=charge;
            InkHUD::LockScreenApplet lock(w,h); lock.setCrop(0,0,w,h); lock.onRender(true);
            assert(lock.minX>=0 && lock.maxX<w && lock.minY>=0 && lock.maxY<h);
            lock.save("tests/build/lock-battery-"+std::to_string(rotation)+"-"+std::to_string(charge)+".pgm");
        }
        InkHUD::LockScreenApplet reused(w,h),fresh(w,h);
        reused.setCrop(0,0,w,h);fresh.setCrop(0,0,w,h);
        testPower.charge=100;reused.onRender(true);
        testPower.charge=0;reused.onRender(false);fresh.onRender(true);
        assert(reused.pixels==fresh.pixels);
        testPower.present=false;reused.onRender(false);
        std::fill(fresh.pixels.begin(),fresh.pixels.end(),255);fresh.onRender(true);
        assert(reused.pixels==fresh.pixels);
        testPower.present=true;
        auto *savedPower=powerStatus;powerStatus=nullptr;reused.onRender(false);powerStatus=savedPower;
    }
    testPower.charge=85;
    std::cout<<"PASS: lockscreen battery 0..100%, invalid, absent, null status and 100% -> 0% redraw.\n";
    auto f=fonts[0];
    InkHUD::NotificationApplet notification;
    notification.setFont(f);
    assert(notification.getNotificationText(290) == notification.parse("Сообщ. от ЁЖИК: Привет, мир!"));
    notification.currentNotification.type = InkHUD::Notification::Type::NOTIFICATION_MESSAGE_DIRECT;
    assert(notification.getNotificationText(290) == notification.parse("ЛС от ЁЖИК: Привет, мир!"));
    for (unsigned width=0; width<296; ++width) {
        auto text=notification.getNotificationText(width);
        assert(text.find('\x1A') == std::string::npos);
        assert(notification.getTextWidth(text) <= width);
    }
    notification.printAt(2,2,notification.getNotificationText(290));
    notification.save("tests/build/notification.pgm");
    assert(f.decodeUTF8("\xD0") == "\x1A");
    assert(f.decodeUTF8("\xD0" "A") == "\x1A" "A");
    assert(f.decodeUTF8("\x80" "B") == "\x1A" "B");
    for (const auto &bad : {"\xC0\xAF", "\xE0\x80\xAF", "\xED\xA0\x80", "\xF4\x90\x80\x80", "\xF8", "\xFF"}) {
        auto out=f.decodeUTF8(bad);
        assert(!out.empty());
        assert(out.find_first_not_of('\x1A') == std::string::npos);
    }
    assert(f.decodeUTF8("🦄") == "\x1A");
    assert(f.decodeUTF8("a\r\nb\rc\td") == "a\nb\nc d");
    std::mt19937 random(290);
    for (unsigned i=0;i<50000;++i) {
        std::string bytes;
        for (unsigned n=random()%256; n; --n) bytes += char(random());
        auto out=f.decodeUTF8(bytes);
        assert(out.size() <= bytes.size());
        assert(out.find('\0') == std::string::npos);
    }
    for (auto font : fonts) {
        Applet a; a.setFont(font);
        assert(a.getWrappedTextHeight(0, 292, "") == 0);
        for (unsigned width=0; width<296; ++width) {
            auto name=a.ellipsizeToWidth(a.parse("ОченьДлинноеИмяУзлаВоронежEnglish01"), width);
            assert(a.getTextWidth(name) <= width);
        }
        auto text=a.parse("ДлинноесловобезпробеловЁжикВоронежРусскийEnglish123");
        a.printWrapped(5, 0, 60, text);
        assert(a.minX >= 5 && a.maxX < 65);
        assert(a.getWrappedTextHeight(0, 292, a.parse("Ёжик\nдруг")) >= 2*font.lineHeight());
        Applet exact; exact.setFont(font);
        auto one=exact.parse("Привет");
        auto w=exact.getTextWidth(one);
        exact.printWrapped(0, 0, w, one);
        assert(exact.getCursorY() == font.heightAboveCursor());
        Applet zero; zero.setFont(font); zero.printWrapped(0,0,0,text);
        assert(zero.maxX == -1);
        Applet multiline; multiline.setFont(font);
        auto h=multiline.getWrappedTextHeight(0, 292, multiline.parse("Ёжик\nдруг"));
        multiline.printWrapped(0, 0, 292, multiline.parse("Ёжик\nдруг"));
        assert(multiline.minY >= 0 && multiline.maxY < int(h));
        assert(multiline.getWrappedTextHeight(0,292,"a\n\nb") == 3*font.lineHeight());
        Applet bottom; bottom.setFont(font);
        bottom.printWrapped(0, 128-font.lineHeight()+1, 292, bottom.parse("Ёжик"));
        assert(bottom.maxY == -1);
        for (unsigned byte=1; byte<=255; ++byte) {
            auto g=font.gfxFont->glyph[byte-1];
            assert(-g.yOffset <= font.heightAboveCursor());
            assert(g.height+g.yOffset < font.heightBelowCursor());
        }
    }
    for (size_t j=0; j<samples.size(); ++j) {
        Applet screen; screen.setFont(fonts[0]);
        screen.printAt(2, 2, screen.parse("ВОРО (Node Воронеж 01)"));
        screen.drawLine(0,18,295,18,0);
        const auto body=screen.parse(samples[j]);
        for (int size=2; size>=0; --size) {
            screen.setFont(fonts[size]);
            if (screen.getWrappedTextHeight(2,292,body)+20 <= 128 || size==0) {
                screen.printWrapped(2,20,292,body);
                break;
            }
        }
        screen.save("tests/build/screen-"+std::to_string(j)+".pgm");
    }
    Applet alphabet; alphabet.setFont(fonts[0]);
    alphabet.printWrapped(2,2,292,alphabet.parse(
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ\n"
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюя\nІіЇїЄєҐґЎў\n"
        "English 0123456789 −12°C №1 «тест»"));
    alphabet.save("tests/build/alphabet.pgm");
    Applet names; names.setFont(fonts[0]);
    for (int row=0; row<5; ++row) {
        auto name=names.parse("ВоронежОченьДлинноеИмяУзла English 01");
        names.printAt(2,row*24+2,names.ellipsizeToWidth(name, 200));
        names.printAt(294,row*24+2,"1 hop",Applet::RIGHT);
    }
    names.save("tests/build/names.pgm");
    std::cout << "PASS: alphabets, samples, ASCII, invalid UTF-8, 50000 fuzz cases, wrap and bounds.\n";
}
