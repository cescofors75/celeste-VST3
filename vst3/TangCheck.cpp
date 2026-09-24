#include "TangLink.h"
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2){std::cerr<<"Usage: CelesteTangCheck COM14\n";return 2;}
 tang::Link link;link.connect(argv[1]);
 auto until=juce::Time::getMillisecondCounterHiRes()+5000;
 std::map<int,int> saved;
 while(juce::Time::getMillisecondCounterHiRes()<until&&!link.take(saved))juce::Thread::sleep(20);
 if(saved.size()!=tang::count||!link.connected){std::cerr<<link.status();return 1;}
 std::cout<<"PASS identity, firmware and all "<<saved.size()<<" controls read\n";
 tang::Link competitor;competitor.connect(argv[1]);juce::Thread::sleep(500);
 if(competitor.connected){std::cerr<<"Exclusive ownership failed";return 1;}
 std::cout<<"PASS exclusive serial ownership\n";
 // One LSB on a continuous parameter: verify real write/read without an audible jump.
 int original=saved[3],changed=original==65535?65534:original+1;
 link.set(3,changed);juce::Thread::sleep(300);link.readState();
 std::map<int,int> actual;until=juce::Time::getMillisecondCounterHiRes()+3000;
 while(juce::Time::getMillisecondCounterHiRes()<until&&!link.take(actual))juce::Thread::sleep(20);
 bool passed=actual.count(3)&&actual[3]==changed;
 link.set(3,original);juce::Thread::sleep(300);link.readState();actual.clear();until=juce::Time::getMillisecondCounterHiRes()+3000;
 while(juce::Time::getMillisecondCounterHiRes()<until&&!link.take(actual))juce::Thread::sleep(20);
 if(!passed||actual[3]!=original){std::cerr<<"Write/read/restore failed";return 1;}
 std::cout<<"PASS real parameter write, readback and restoration\n"<<link.status()<<"\n";
 link.disconnect();return 0;
}
