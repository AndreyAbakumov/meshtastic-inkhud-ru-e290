#include "graphics/niche/InkHUD/Applets/System/LockScreen/UnreadInbox.h"
#include "mesh/Throttle.h"
#include <cassert>
#include <vector>
#include <iostream>
uint32_t nowMs=0, epoch=120;
uint32_t millis() { return nowMs; }
enum class RTCQuality { RTCQualityDevice };
uint32_t getValidTime(RTCQuality,bool) { return epoch; }
struct meshtastic_MeshPacket { uint32_t id=1, to=UINT32_MAX; };
bool isBroadcast(uint32_t n) { return n==UINT32_MAX; }
struct NodeDB { uint32_t getNodeNum() { return 99; } } db;
auto *nodeDB=&db;
namespace NicheGraphics {
namespace Drivers { struct EInk { enum class UpdateTypes { FAST, FULL }; }; }
unsigned saves=0;
template<class T> struct FlashData { static void save(T*, const char*) { ++saves; } };
namespace InkHUD {
struct MessageStore { struct Message { uint32_t sender=42,timestamp=120; uint8_t channelIndex=0; std::string text="Привет"; }; };
struct SystemApplet {
    std::string name; bool foreground=false,handleInput=false,lockRendering=false,lockRequests=false;
    bool isForeground() const { return foreground; }
    virtual void onForeground() {} virtual void onBackground() {}
    void bringToForeground() { foreground=true;onForeground(); }
    void sendToBackground() { foreground=false;onBackground(); }
};
struct NotificationApplet:SystemApplet { void dismiss() { foreground=false; } };
struct FakeHUD {
    std::vector<SystemApplet*> systemApplets; unsigned updates=0;
    SystemApplet *getSystemApplet(const char *name) { for(auto *a:systemApplets) if(a->name==name) return a; assert(false); return nullptr; }
    void forceUpdate(Drivers::EInk::UpdateTypes) { ++updates; }
};
class LockScreenApplet:public SystemApplet {
public:
    inline static LockScreenApplet *instance=nullptr;
    FakeHUD *inkhud;
    struct Options { uint32_t version=1;uint8_t minutes=0; } options;
    UnreadInbox inbox;
    uint32_t lastInput=0,minute=UINT32_MAX;
    unsigned requests=0;
    Drivers::EInk::UpdateTypes lastType=Drivers::EInk::UpdateTypes::FAST;
    void setIntervalFromNow(uint32_t) {}
    void requestUpdate(Drivers::EInk::UpdateTypes type = Drivers::EInk::UpdateTypes::FAST) { ++requests;lastType=type; }
    static bool input(bool unlock=false);
    bool canOpen(); void open(); void setTimeout(uint8_t); void clearUnread();
    void receive(const meshtastic_MeshPacket*, const MessageStore::Message&);
    static void markRead(const MessageStore::Message&,bool);
    void onForeground() override; void onBackground() override; void onButtonLongPress();
    int32_t runOnce();
};
}}
using namespace NicheGraphics;
#include "lock_runtime.inc"
int main() {
    using namespace InkHUD;
    LockScreenApplet lock; lock.name="LockScreen"; LockScreenApplet::instance=&lock;
    SystemApplet logo,menu; logo.name="Logo";menu.name="Menu";
    NotificationApplet notify;notify.name="Notification";
    FakeHUD hud;hud.systemApplets={&logo,&lock,&menu,&notify};lock.inkhud=&hud;
    assert(lock.options.minutes==0); nowMs=100000;lock.runOnce();assert(!lock.isForeground());
    for(uint8_t delay:{1,5,15}) {
        nowMs=0;lock.setTimeout(delay); nowMs=delay*60000UL-1;lock.runOnce();assert(!lock.isForeground());
        ++nowMs;lock.runOnce();assert(lock.isForeground()&&lock.lockRendering&&lock.handleInput);
        assert(LockScreenApplet::input());assert(lock.isForeground());
        assert(LockScreenApplet::input(true));assert(!lock.isForeground()&&!lock.lockRendering);
    }
    unsigned saved=saves;lock.setTimeout(2);assert(saves==saved);
    lock.setTimeout(1); nowMs=UINT32_MAX-30000;LockScreenApplet::input();nowMs=29998;lock.runOnce();assert(!lock.isForeground());
    nowMs=30000;lock.runOnce();assert(lock.isForeground());LockScreenApplet::input(true);
    menu.foreground=true;nowMs+=90000;lock.runOnce();assert(!lock.isForeground());
    menu.foreground=false;nowMs+=59999;lock.runOnce();assert(!lock.isForeground());++nowMs;lock.runOnce();assert(lock.isForeground());
    logo.foreground=true;logo.handleInput=true;assert(!LockScreenApplet::input(true));assert(lock.isForeground());logo.foreground=false;
    unsigned requests=lock.requests;lock.runOnce();assert(lock.requests==requests);epoch+=60;lock.runOnce();assert(lock.requests==requests+1);assert(lock.lastType==Drivers::EInk::UpdateTypes::FULL);
    meshtastic_MeshPacket p;MessageStore::Message m;lock.receive(&p,m);assert(lock.inbox.entries.size()==1);
    lock.receive(&p,m);assert(lock.inbox.entries.size()==1);
    LockScreenApplet::markRead(m,false);assert(lock.inbox.entries.size()==1);
    LockScreenApplet::input(true);LockScreenApplet::markRead(m,true);assert(lock.inbox.entries.size()==1);
    LockScreenApplet::markRead(m,false);assert(lock.inbox.entries.empty());
    p.id=2;p.to=123;lock.receive(&p,m);assert(lock.inbox.entries.empty());
    p.to=99;lock.receive(&p,m);assert(lock.inbox.entries.front().direct);lock.clearUnread();assert(lock.inbox.entries.empty());
    for(uint32_t id=3;id<=50;++id) { p.id=id;lock.receive(&p,m); }
    assert(lock.inbox.entries.size()==20&&lock.inbox.entries.front().id==50&&lock.inbox.entries.back().id==31);
    lock.clearUnread();lock.receive(&p,m);assert(lock.inbox.entries.empty());
    LockScreenApplet fresh;assert(fresh.inbox.entries.empty());
    std::cout<<"PASS: default off, 1/5/15 min, activity, rollover, modal priority, unlock, minute refresh, DM privacy, dedup, read guards, 20-message cap.\n";
}
