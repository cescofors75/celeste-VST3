#include "Engine.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <iostream>
double difference(const juce::AudioBuffer<float>& a,const juce::AudioBuffer<float>& b){double sum=0;for(int c=0;c<2;c++)for(int i=0;i<a.getNumSamples();i++)sum+=std::abs(a.getSample(c,i)-b.getSample(c,i));return sum;}
juce::AudioBuffer<float> impulse(double sr,Settings s){Engine e;e.prepare(sr);e.set(s,true);juce::AudioBuffer<float> b(2,int(sr));b.clear();b.setSample(0,0,.7f);b.setSample(1,0,.5f);e.process(b,s);return b;}
int main(int argc,char** argv){
 Engine reused;
 for(double sr:{44100.,48000.,96000.}){
  reused.prepare(sr);Settings p;p.mix=1;p.space=1;p.fold=1;p.motion=1;p.feedback=.85f;reused.set(p,true);
  juce::AudioBuffer<float> block(2,257);for(int c=0;c<2;c++)for(int i=0;i<257;i++)block.setSample(c,i,.3f*std::sin(float(i)*.07f));
  for(int k=0;k<200;k++)reused.process(block,p);
  reused.reset();reused.set(p,true);
  for(int n:{64,257,512,1024,17}){block.setSize(2,n);block.clear();reused.process(block,p);
   for(int c=0;c<2;c++)for(int i=0;i<n;i++)if(block.getSample(c,i)!=0){std::cerr<<"Reset silence failed "<<sr<<" block "<<n<<" sample "<<i<<" value "<<block.getSample(c,i)<<"\n";return 14;}
  }
  std::cout<<"PASS reset after wet audio and variable blocks at "<<sr<<" Hz\n";
  p.a=110.00001f;p.b=270.00003f;reused.prepare(sr);reused.set(p,true);
  block.setSize(2,64);
  for(int k=0;k<int(sr*3/64)+1;k++){block.clear();reused.process(block,p);
   for(int c=0;c<2;c++)for(int i=0;i<64;i++)if(block.getSample(c,i)!=0){std::cerr<<"Delay wrap silence failed "<<sr<<" at "<<k*64+i<<"\n";return 15;}
  }
  std::cout<<"PASS fractional delay wraparound stays silent at "<<sr<<" Hz\n";
 }
 for(double sr:{44100.,48000.,96000.}){
  Engine e;e.prepare(sr);Settings s;s.mix=0;s.space=0;e.set(s,true);juce::AudioBuffer<float>b(2,1024);
  for(int c=0;c<2;c++)for(int i=0;i<1024;i++)b.setSample(c,i,.3f*std::sin(float(i)*.07f));auto original=b;e.process(b,s);
  for(int c=0;c<2;c++)for(int i=0;i<1024;i++)if(b.getSample(c,i)!=original.getSample(c,i))return 2;
  s.mix=1;s.feedback=.85f;s.fold=1;s.space=1;s.motion=1;
  for(int k=0;k<300;k++){b.clear();if(k==0){b.setSample(0,0,.8f);b.setSample(1,0,.5f);}s.series=(k/50)%2;e.process(b,s);for(int c=0;c<2;c++)for(int i=0;i<1024;i++)if(!std::isfinite(b.getSample(c,i))||std::abs(b.getSample(c,i))>1.001f)return 3;}
  std::cout<<"PASS dry identity / 1024-frame blocks / feedback extrema / routing transitions at "<<sr<<" Hz\n";
  Settings test;test.mix=1;test.space=0;test.motion=0;test.fold=0;test.a=110;test.b=270;
  auto parallel=impulse(sr,test);test.series=true;auto series=impulse(sr,test);if(difference(parallel,series)<.1)return 6;
  test.series=false;test.fold=1;auto fold=impulse(sr,test);if(difference(parallel,fold)<.01)return 7;
  test.fold=0;test.cutoff=120;auto filter=impulse(sr,test);if(difference(parallel,filter)<.01)return 8;
  test.cutoff=6500;test.motion=1;test.rate=3;auto motion=impulse(sr,test);if(difference(parallel,motion)<.01)return 9;
  test.motion=0;test.space=1;auto space=impulse(sr,test);if(difference(parallel,space)<.01)return 10;
  test.bypass=true;test.output=6;auto dry=impulse(sr,test);if(dry.getSample(0,0)!=.7f||dry.getSample(1,0)!=.5f)return 11;
  double tail=0;for(int i=1;i<dry.getNumSamples();i++)tail+=std::abs(dry.getSample(0,i));if(tail!=0)return 12;
  e.prepare(sr);test=Settings{};test.bypass=true;e.set(test,true);b.clear();e.process(b,test);for(int c=0;c<2;c++)for(int i=0;i<b.getNumSamples();i++)if(b.getSample(c,i)!=0)return 13;
  std::cout<<"PASS audible route / fold / filter / motion / space differences; exact bypass; silent input at "<<sr<<" Hz\n";
 }
 if(argc==3){juce::AudioFormatManager fm;fm.registerBasicFormats();auto reader=std::unique_ptr<juce::AudioFormatReader>(fm.createReaderFor(juce::File(argv[1])));if(!reader)return 4;
  auto stream=juce::File(argv[2]).createOutputStream();if(!stream)return 5;stream->setPosition(0);stream->truncate();juce::WavAudioFormat wav;auto writer=std::unique_ptr<juce::AudioFormatWriter>(wav.createWriterFor(stream.release(),reader->sampleRate,2,24,{},0));if(!writer)return 5;
  Engine engine;engine.prepare(reader->sampleRate);Settings s;s.feedback=.62f;s.space=.72f;s.mix=.64f;s.motion=.5f;s.cutoff=3400;s.fold=.08f;engine.set(s,true);juce::AudioBuffer<float>b(2,512);
  for(juce::int64 at=0;at<reader->lengthInSamples;at+=512){int n=int(std::min<juce::int64>(512,reader->lengthInSamples-at));b.setSize(2,n,false,false,true);reader->read(&b,0,n,at,true,true);engine.process(b,s);writer->writeFromAudioSampleBuffer(b,0,n);}
  std::cout<<"Rendered actual CELESTE native DSP audio\n";
 }
 return 0;
}
