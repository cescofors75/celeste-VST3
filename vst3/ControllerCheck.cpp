#include "Plugin.cpp"
#include <iostream>
#include <mutex>
#include <atomic>
#include <functional>

static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static void until(const std::function<bool()>& condition){auto end=juce::Time::getMillisecondCounterHiRes()+3500;while(!condition()){if(juce::Time::getMillisecondCounterHiRes()>end)throw std::runtime_error("test timeout");juce::Thread::sleep(2);}}
struct FakePort {
 inline static std::mutex mutex;
 inline static std::map<int,int> registers;
 inline static std::atomic<int> writeCount{0},openCount{0};
 inline static std::atomic<bool> hold{false},entered{false},corrupt{false},rejectValue{false};
 std::vector<uint8_t> response;
 static void reset(){std::lock_guard<std::mutex> guard(mutex);registers.clear();writeCount=0;openCount=0;hold=false;entered=false;corrupt=false;rejectValue=false;}
 void open(const juce::String&){openCount++;} void close(){}
 void write(const std::vector<uint8_t>& packet){
  const int kind=packet[3];std::vector<uint8_t> payload;
  {std::lock_guard<std::mutex> guard(mutex);
   if(kind==1)payload={'C','E','L','E','S','T','E','/','1'};
   else if(kind==2){payload.resize(32);payload[0]=0x80;payload[1]=0xbb;payload[28]=255;payload[29]=231;}
   else if(kind==9||kind==11){payload.resize(kind==9?110:72);for(auto& p:tang::params){int wire=p.wire;int at=wire==32?72:wire<32?wire*2:(wire-64)*2;if((kind==9&&wire<=32)||(kind==11&&wire>=64)){int raw=registers[wire];payload[size_t(at)]=uint8_t(raw);if(wire!=32)payload[size_t(at+1)]=uint8_t(raw>>8);}}}
   else if(kind==3){int wire=tang::word(packet,10),raw=tang::word(packet,12);if(!rejectValue)registers[wire]=raw;writeCount++;}
  }
  if(kind==3&&hold){entered=true;auto end=juce::Time::getMillisecondCounterHiRes()+2500;while(hold&&juce::Time::getMillisecondCounterHiRes()<end)juce::Thread::sleep(2);}
  response={67,69,1,uint8_t(kind|128),packet[4],packet[5],packet[6],packet[7],uint8_t(payload.size()),0};response.insert(response.end(),payload.begin(),payload.end());auto c=tang::crc(response.data()+2,response.size()-2);response.push_back(uint8_t(c));response.push_back(uint8_t(c>>8));if(corrupt)response.back()^=1;
 }
 int read(uint8_t* bytes,int capacity){int n=std::min({capacity,int(response.size()),7});std::copy_n(response.begin(),n,bytes);response.erase(response.begin(),response.begin()+n);return n;}
};
using TestLink=tang::BasicLink<FakePort>;
static void connect(TestLink& link){link.connect("FAKE");until([&]{return link.connected.load();});std::map<int,int> values;require(link.take(values)&&values.size()==55,"initial snapshot incomplete");}
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI init;
 try{
  require(tang::tempoRaw(0,3,120)==47992&&tang::tempoRaw(1,3,120)==-1&&tang::tempoRaw(1,2,120)==47984&&tang::tempoRaw(0,1,0)==-1,"tempo range/quantization");
  std::cout<<"PASS tempo divisions honor both physical RAM capacities\n";
  require(tang::count==55,"missing controls");
  for(auto& p:tang::params)for(int value:{0,p.maximum/2,p.maximum})require(tang::decode(p.wire,tang::encode(p.wire,value))==value,"mode encoding mismatch");
  require(tang::decode(23,32767)==0&&tang::decode(23,32768)==1,"glitch threshold");
  std::cout<<"PASS all 55 control ranges and discrete encoding\n";
  FakePort::reset();{TestLink link;connect(link);std::map<int,int> target;for(auto& p:tang::params)target[p.wire]=p.maximum;link.apply(target);until([&]{return link.verifiedBatches.load()==1;});std::map<int,int> result;require(link.take(result)&&result==target,"full preset not restored");require(FakePort::writeCount==55,"unexpected write count");}
  std::cout<<"PASS full apply, fragmented packets, CRC and verified readback\n";
  FakePort::reset();{TestLink link;connect(link);FakePort::hold=true;link.apply({{0,1},{1,2},{2,3}});until([]{return FakePort::entered.load();});link.disconnect();FakePort::hold=false;juce::Thread::sleep(150);require(FakePort::writeCount==1,"writes after disconnect");require(!link.connected,"connected after disconnect");}
  std::cout<<"PASS disconnect cancels remaining in-flight batch\n";
  FakePort::reset();{TestLink link;connect(link);FakePort::hold=true;link.apply({{0,1},{1,2},{2,3}});until([]{return FakePort::entered.load();});link.readState();FakePort::hold=false;std::map<int,int> result;until([&]{return link.take(result);});require(FakePort::writeCount==1&&result.at(0)==1&&result.at(1)==0,"read did not cancel stale batch");}
  std::cout<<"PASS READ cancels writes and reads actual partial state\n";
  FakePort::reset();{TestLink link;connect(link);FakePort::hold=true;link.apply({{0,1},{1,2}});until([]{return FakePort::entered.load();});link.disconnect();link.connect("OTHER");FakePort::hold=false;until([&]{return link.connected.load()&&FakePort::openCount==2;});require(FakePort::writeCount==1,"reconnect replayed stale controls");}
  std::cout<<"PASS rapid reconnect reopens the port without stale writes\n";

  FakePort::reset();{TestLink link;connect(link);FakePort::rejectValue=true;link.apply({{3,2345}});until([&]{return !link.connected.load()&&link.status().contains("readback mismatch");});require(link.verifiedBatches==0&&link.status().contains("readback mismatch"),"false successful apply");}
  std::cout<<"PASS acknowledged but unapplied value rejected\n";
  FakePort::reset();{TestLink link;connect(link);FakePort::corrupt=true;link.readState();until([&]{return !link.connected.load()&&link.status().contains("CRC");});require(link.status().contains("CRC"),"corrupt response accepted");}
  std::cout<<"PASS corrupt response disconnects without reconnect\n";
  FakePort::reset();{TestLink link;connect(link);link.followHardware(true);{std::lock_guard<std::mutex> guard(FakePort::mutex);FakePort::registers[4]=41000;}std::map<int,int> result;until([&]{return link.take(result)&&result.at(4)==41000;});require(FakePort::writeCount==0,"hardware follow wrote controls");}
  std::cout<<"PASS hardware-follow snapshots do not write\n";
  {Processor p;juce::ValueTree old("CELESTE");for(auto entry:std::initializer_list<std::pair<const char*,float>>{{"tang_88",5},{"tang_27",3},{"tang_2",37.5f}}){juce::ValueTree v("PARAM");v.setProperty("id",entry.first,nullptr);v.setProperty("value",entry.second,nullptr);old.appendChild(v,nullptr);}juce::MemoryBlock bytes;juce::AudioProcessor::copyXmlToBinary(*old.createXml(),bytes);p.setStateInformation(bytes.getData(),int(bytes.getSize()));require(p.state.getRawParameterValue("tang_bit_88_0")->load()==1&&p.state.getRawParameterValue("tang_bit_88_1")->load()==0&&p.state.getRawParameterValue("tang_bit_88_2")->load()==1,"legacy mask migration");require(p.state.getRawParameterValue("tangRouting")->load()==2,"legacy routing migration");require(p.state.getRawParameterValue("tang_2")->load()==37.5f,"legacy continuous state");require(!p.state.getParameter("tang_88")->isAutomatable()&&p.state.getParameter("tang_bit_88_0")->isBoolean(),"mask still rampable");p.state.getParameter("tang_bit_88_1")->setValueNotifyingHost(1);p.getStateInformation(bytes);Processor restored;restored.setStateInformation(bytes.getData(),int(bytes.getSize()));require(restored.state.getRawParameterValue("tang_bit_88_1")->load()==1,"new bit recall");}
  std::cout<<"PASS old session migration, independent booleans and new state recall\n";
  if(argc>1){Processor p;std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());for(auto* child:editor->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="TANG CONTROL"){button->setToggleState(true,juce::dontSendNotification);button->onClick();}auto screenshot=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.f);auto stream=juce::File(argv[1]).createOutputStream();require(bool(stream),"preview output");stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;require(png.writeImageToStream(screenshot,*stream),"preview render");std::cout<<"PASS hardware editor rendered\n";}
  return 0;
 }catch(const std::exception& ex){FakePort::hold=false;std::cerr<<"FAIL "<<ex.what()<<"\n";return 1;}
}
