#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <map>
#include <vector>
#include <stdexcept>
#include <cerrno>
#if JUCE_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#if JUCE_MAC
#include <IOKit/serial/ioss.h>
#endif
#endif
namespace tang {
struct Param {int wire;const char* name;int maximum;};
inline const Param params[]={
 {0,"Glitch amount",65535},{1,"Glitch probability",65535},{2,"Delay time",65535},{3,"Delay feedback",65535},
 {4,"Filter cutoff",65535},{5,"Filter resonance",65535},{13,"Wavefolder drive",65535},{14,"VCA level",65535},
 {11,"LFO rate",65535},{12,"LFO depth",65535},{15,"Chaos amount",65535},{24,"Delay 2 time",65535},
 {25,"Delay 2 feedback",65535},{67,"Chorus depth",65535},{70,"Flanger depth",65535},{72,"Crusher bits",65535},
 {75,"Freeze hold",65535},{79,"Tremolo depth",65535},{82,"Auto-pan depth",65535},{84,"Envelope depth",65535},
 {6,"Dry mix",65535},{7,"Glitch mix",65535},{8,"Delay mix",65535},{9,"Filter mix",65535},
 {16,"Wavefolder mix",65535},{17,"VCA mix",65535},{26,"Delay 2 mix",65535},{68,"Chorus mix",65535},
 {71,"Flanger mix",65535},{74,"Crusher mix",65535},{77,"Freeze mix",65535},{80,"Tremolo mix",65535},
 {83,"Auto-pan mix",65535},{10,"Master",65535},{20,"Global bypass",1},{32,"Routes mask",63},
 {27,"Delay 2 routing",3},{64,"Expanded routes",127},{18,"LFO targets",3},{19,"Chaos targets",3},
 {22,"Bypass mask",255},{65,"Expanded bypass",127},{88,"Mute mask",8191},{89,"Solo mask",8191}};
inline juce::String id(int i){return "tang_"+juce::String(params[i].wire);}
inline constexpr int count=sizeof(params)/sizeof(params[0]);
inline uint16_t word(const std::vector<uint8_t>& b,size_t i){if(i+2>b.size())throw std::runtime_error("Short response");return uint16_t(b[i]|b[i+1]<<8);}
inline uint32_t dword(const std::vector<uint8_t>& b,size_t i){return word(b,i)|(uint32_t(word(b,i+2))<<16);}
inline uint16_t crc(const uint8_t* b,size_t n){uint16_t c=65535;while(n--){c^=uint16_t(*b++)<<8;for(int j=0;j<8;j++)c=uint16_t((c<<1)^((c&32768)?0x1021:0));}return c;}
class Port {
#if JUCE_WINDOWS
 HANDLE handle=INVALID_HANDLE_VALUE;
#else
 int handle=-1;
#endif
public:
 ~Port(){close();}
 void close(){
#if JUCE_WINDOWS
 if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);handle=INVALID_HANDLE_VALUE;
#else
 if(handle>=0)::close(handle);handle=-1;
#endif
 }
 void open(const juce::String& path){close();
#if JUCE_WINDOWS
 handle=CreateFileW(("\\\\.\\"+path).toWideCharPointer(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
 if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("Port busy/unavailable. Disconnect web or other plugin instance.");
 DCB d{};d.DCBlength=sizeof(d);if(!GetCommState(handle,&d))throw std::runtime_error("Serial configuration failed");
 d.BaudRate=3000000;d.ByteSize=8;d.Parity=NOPARITY;d.StopBits=ONESTOPBIT;d.fBinary=TRUE;d.fParity=FALSE;d.fOutxCtsFlow=FALSE;d.fOutxDsrFlow=FALSE;d.fDtrControl=DTR_CONTROL_DISABLE;d.fRtsControl=RTS_CONTROL_DISABLE;d.fOutX=FALSE;d.fInX=FALSE;d.fAbortOnError=FALSE;
 COMMTIMEOUTS t{};t.ReadIntervalTimeout=MAXDWORD;t.ReadTotalTimeoutConstant=50;t.WriteTotalTimeoutConstant=300;
 if(!SetCommState(handle,&d)||!SetCommTimeouts(handle,&t))throw std::runtime_error("Cannot configure 3 Mbaud");PurgeComm(handle,PURGE_RXCLEAR|PURGE_TXCLEAR);
#else
 handle=::open(path.toRawUTF8(),O_RDWR|O_NOCTTY|O_NONBLOCK);if(handle<0)throw std::runtime_error("Port busy/unavailable");
 if(ioctl(handle,TIOCEXCL)<0)throw std::runtime_error("Cannot lock port");termios t{};if(tcgetattr(handle,&t))throw std::runtime_error("Serial settings failed");cfmakeraw(&t);t.c_cflag|=CLOCAL|CREAD;t.c_cflag&=~CRTSCTS;
#if JUCE_MAC
 cfsetspeed(&t,B9600);if(tcsetattr(handle,TCSANOW,&t))throw std::runtime_error("Serial settings failed");speed_t baud=3000000;if(ioctl(handle,IOSSIOSPEED,&baud))throw std::runtime_error("3 Mbaud unavailable");
#else
 cfsetspeed(&t,B3000000);if(tcsetattr(handle,TCSANOW,&t))throw std::runtime_error("3 Mbaud unavailable");
#endif
 tcflush(handle,TCIOFLUSH);
#endif
 }
 int read(uint8_t* b,int n){
#if JUCE_WINDOWS
 DWORD got=0;if(!ReadFile(handle,b,DWORD(n),&got,nullptr))throw std::runtime_error("Serial read failed");return int(got);
#else
 auto r=::read(handle,b,size_t(n));if(r<0&&errno!=EAGAIN&&errno!=EINTR)throw std::runtime_error("Serial read failed");return r<0?0:int(r);
#endif
 }
 void write(const std::vector<uint8_t>& b){size_t sent=0;auto end=juce::Time::getMillisecondCounterHiRes()+500;while(sent<b.size()){
#if JUCE_WINDOWS
 DWORD n=0;if(!WriteFile(handle,b.data()+sent,DWORD(b.size()-sent),&n,nullptr))throw std::runtime_error("Serial write failed");
#else
 auto n=::write(handle,b.data()+sent,b.size()-sent);if(n<0){if(errno!=EAGAIN&&errno!=EINTR)throw std::runtime_error("Serial write failed");n=0;}
#endif
 sent+=size_t(n);if(juce::Time::getMillisecondCounterHiRes()>end)throw std::runtime_error("Serial write timeout");if(!n)juce::Thread::sleep(1);
 }}
};
class Link:private juce::Thread {
 Port port;uint32_t sequence=0;juce::CriticalSection lock;juce::String path,message="Disconnected";bool desired=false,readPending=false;std::map<int,int> writes,received;bool snapshot=false;
 std::vector<uint8_t> request(int kind,const std::vector<uint8_t>& payload={}){
 auto seq=sequence++;std::vector<uint8_t> b={67,69,1,uint8_t(kind),uint8_t(seq),uint8_t(seq>>8),uint8_t(seq>>16),uint8_t(seq>>24),uint8_t(payload.size()),uint8_t(payload.size()>>8)};b.insert(b.end(),payload.begin(),payload.end());auto c=crc(b.data()+2,b.size()-2);b.push_back(uint8_t(c));b.push_back(uint8_t(c>>8));port.write(b);
 std::vector<uint8_t> r;auto until=juce::Time::getMillisecondCounterHiRes()+1000;
 while(!threadShouldExit()&&juce::Time::getMillisecondCounterHiRes()<until){uint8_t tmp[1100];int n=port.read(tmp,1100);r.insert(r.end(),tmp,tmp+n);if(r.size()>=10){if(r[0]!=67||r[1]!=69||r[2]!=1)throw std::runtime_error("Invalid response header");auto size=word(r,8);if(size>1024)throw std::runtime_error("Invalid response length");if(r.size()>=size_t(size+12)){if(r.size()!=size_t(size+12)||word(r,r.size()-2)!=crc(r.data()+2,r.size()-4)||dword(r,4)!=seq||r[3]!=(kind|128))throw std::runtime_error("Response CRC/sequence/command rejected");return {r.begin()+10,r.end()-2};}}if(!n)juce::Thread::sleep(2);}
 throw std::runtime_error("Tang response timeout");
 }
 void readControls(){auto a=request(9),e=request(11);if(a.size()!=110||e.size()<64)throw std::runtime_error("Requires expanded CELESTE firmware");std::map<int,int> values;for(auto& p:params)values[p.wire]=p.wire==32?a[72]:p.wire<32?word(a,size_t(p.wire*2)):word(e,size_t((p.wire-64)*2));const juce::ScopedLock g(lock);received=values;snapshot=true;}
 void run()override{bool active=false;while(!threadShouldExit()){
 bool want,read;juce::String selected;std::map<int,int> pending;{const juce::ScopedLock g(lock);want=desired;selected=path;read=readPending;readPending=false;pending.swap(writes);}
 try{if(!want){if(active){port.close();active=false;connected=false;const juce::ScopedLock g(lock);message="Disconnected";}wait(250);continue;}
 if(!active){port.open(selected);sequence=0;auto ping=request(1);if(std::string(ping.begin(),ping.end())!="CELESTE/1")throw std::runtime_error("Not a CELESTE device");auto s=request(2);if(dword(s,0)!=48000||word(s,28)!=59391)throw std::runtime_error("Requires tested expanded Line-In firmware (59391)");active=true;readControls();connected=true;{const juce::ScopedLock g(lock);message="Connected / Line-In 48 kHz / USB control";}pending.clear();read=false;}
 if(read)readControls();for(auto& item:pending){int v=item.second;if(item.first==20)v=v?65535:0;request(3,{uint8_t(item.first),uint8_t(item.first>>8),uint8_t(v),uint8_t(v>>8)});}
 auto s=request(2);{const juce::ScopedLock g(lock);message="Connected / 48 kHz / losses "+juce::String(dword(s,8))+" / errors "+juce::String(dword(s,20));}wait(50);
 }catch(const std::exception& ex){port.close();active=false;connected=false;const juce::ScopedLock g(lock);desired=false;writes.clear();message=ex.what();}}
 port.close();connected=false;
 }
public:
 std::atomic<bool> connected{false};
 Link():Thread("CELESTE Tang USB"){startThread();}~Link()override{signalThreadShouldExit();notify();stopThread(4000);}
 void connect(juce::String p){const juce::ScopedLock g(lock);if(desired||connected)return;path=p.trim();desired=true;message="Connecting...";notify();}
 void disconnect(){const juce::ScopedLock g(lock);desired=false;writes.clear();message="Disconnected";notify();}
 void readState(){const juce::ScopedLock g(lock);readPending=true;writes.clear();notify();}
 void set(int id,int value){const juce::ScopedLock g(lock);if(connected)writes[id]=value;}
 bool take(std::map<int,int>& result){const juce::ScopedLock g(lock);if(!snapshot)return false;result=received;snapshot=false;return true;}
 juce::String status(){const juce::ScopedLock g(lock);return message;}
};
}
