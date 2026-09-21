#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>
struct Settings {
 float a=340,b=510,feedback=.42f,cutoff=6500,fold=.16f,space=.35f,motion=.25f,mix=.42f,rate=.22f,output=0;
 bool series=false,bypass=false;
};
class Engine {
 struct Delay {std::vector<float> mem;int pos=0;float tone=0;
  void prepare(int n){mem.assign(n,0);pos=0;tone=0;}
  float read(float samples){float at=float(pos)-samples;while(at<0)at+=float(mem.size());int i=int(at);float f=at-float(i);return mem[size_t(i)]*(1-f)+mem[size_t((i+1)%int(mem.size()))]*f;}
  void put(float x){mem[size_t(pos)]=x;pos=(pos+1)%int(mem.size());}
 } da[2],db[2];
 double sr=48000,phase=0;float lp[2]{},dcX[2]{},dcY[2]{};
 std::array<juce::SmoothedValue<float>,10> smooth;
 juce::Reverb verb;juce::AudioBuffer<float> folded,spaceBuffer;
 juce::dsp::Oversampling<float> oversample{2,1,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true};
 juce::SmoothedValue<float> topology;
public:
 std::array<std::atomic<float>,7> meters{};
 void prepare(double rate){sr=rate;phase=0;for(int ch=0;ch<2;ch++){da[ch].prepare(int(sr*2)+8);db[ch].prepare(int(sr*2)+8);lp[ch]=dcX[ch]=dcY[ch]=0;}
  for(auto& s:smooth)s.reset(sr,.04);topology.reset(sr,.04);topology.setCurrentAndTargetValue(0);
  verb.setSampleRate(sr);verb.reset();folded.setSize(2,512);spaceBuffer.setSize(2,512);oversample.reset();oversample.initProcessing(512);set(Settings{},true);
 }
 void set(const Settings& p,bool immediate=false){const float v[]={p.a,p.b,p.feedback,p.cutoff,p.fold,p.space,p.motion,p.bypass?0.f:p.mix,p.rate,p.bypass?1.f:juce::Decibels::decibelsToGain(p.output)};
  for(size_t i=0;i<smooth.size();i++)if(immediate)smooth[i].setCurrentAndTargetValue(v[i]);else smooth[i].setTargetValue(v[i]);
  if(immediate)topology.setCurrentAndTargetValue(p.series?1.f:0.f);else topology.setTargetValue(p.series?1.f:0.f);
 }
 void process(juce::AudioBuffer<float>& buffer,const Settings& p){juce::ScopedNoDenormals guard;set(p);std::array<float,7> peaks{};
  for(int offset=0;offset<buffer.getNumSamples();offset+=512){int n=std::min(512,buffer.getNumSamples()-offset);
   folded.setSize(2,n,false,false,true);spaceBuffer.setSize(2,n,false,false,true);
   for(int ch=0;ch<2;ch++)for(int i=0;i<n;i++){float x=buffer.getSample(ch,offset+i);folded.setSample(ch,i,std::isfinite(x)?x:0.f);}
   juce::dsp::AudioBlock<float> fb(folded);auto up=oversample.processSamplesUp(fb);
   auto predicted=smooth[4];const float drive=1.f+predicted.getCurrentValue()*3.f,endDrive=1.f+predicted.skip(n)*3.f;
   for(size_t ch=0;ch<2;ch++)for(size_t i=0;i<up.getNumSamples();i++){
    float x=up.getSample(int(ch),int(i));float d=drive+(endDrive-drive)*float(i)/float(up.getNumSamples());float foldedSample=std::asin(std::sin(x*d))*0.63661977f;
    up.setSample(int(ch),int(i),foldedSample);
   }
   oversample.processSamplesDown(fb);
   std::array<float,512> spaceGains{},mixes{};
   for(int i=0;i<n;i++){
    std::array<float,10> v;for(size_t k=0;k<v.size();k++)v[k]=smooth[k].getNextValue();float serial=topology.getNextValue();
    spaceGains[size_t(i)]=v[5]*v[7]*.32f*v[9];mixes[size_t(i)]=v[7];phase+=v[8]/sr;if(phase>=1)phase-=1;
    float movement=std::sin(float(phase)*juce::MathConstants<float>::twoPi);
    float cutoff=juce::jlimit(30.f,float(sr*.4),v[3]*std::pow(2.f,movement*v[6]));
    float alpha=1.f-std::exp(-juce::MathConstants<float>::twoPi*cutoff/float(sr));
    for(int ch=0;ch<2;ch++){
     float input=buffer.getSample(ch,offset+i);if(!std::isfinite(input))input=0;
     float delayA=da[ch].read(juce::jlimit(1.f,float(sr*1.8),v[0]*float(sr)*.001f*(ch?1.013f:1.f)));
     float delayB=db[ch].read(juce::jlimit(1.f,float(sr*1.8),v[1]*float(sr)*.001f*(ch?.991f:1.f)));
     da[ch].tone+=.25f*(delayA-da[ch].tone);db[ch].tone+=.22f*(delayB-db[ch].tone);
     da[ch].put(std::tanh(input+da[ch].tone*v[2]));db[ch].put(std::tanh(input*(1-serial)+delayA*serial+db[ch].tone*v[2]));
     float shape=input*(1-v[4])+folded.getSample(ch,i)*v[4];
     float dc=shape-dcX[ch]+.995f*dcY[ch];dcX[ch]=shape;dcY[ch]=dc;
     lp[ch]+=alpha*(dc-lp[ch]);float mod=1.f-v[6]*.45f*(1.f+std::sin(float(phase)*juce::MathConstants<float>::twoPi+(ch?.7f:0.f)));
     float wet=(delayA*(1-serial)*.3f+delayB*(.3f+.25f*serial)+lp[ch]*.4f)*mod;
     spaceBuffer.setSample(ch,i,wet);
     float out=(input*(1-v[7])+wet*v[7])*v[9];buffer.setSample(ch,offset+i,out);
     peaks[0]=std::max(peaks[0],std::abs(input));peaks[1]=std::max(peaks[1],std::abs(delayA));peaks[2]=std::max(peaks[2],std::abs(delayB));peaks[3]=std::max(peaks[3],std::abs(lp[ch]));peaks[4]=std::max(peaks[4],std::abs(shape));
    }
   }
   juce::Reverb::Parameters rp;rp.roomSize=.86f;rp.damping=.6f;rp.wetLevel=1;rp.dryLevel=0;rp.width=1;rp.freezeMode=0;verb.setParameters(rp);
   verb.processStereo(spaceBuffer.getWritePointer(0),spaceBuffer.getWritePointer(1),n);
   for(int ch=0;ch<2;ch++)for(int i=0;i<n;i++){
    float x=buffer.getSample(ch,offset+i)+spaceBuffer.getSample(ch,i)*spaceGains[size_t(i)];
    // Transparent dry/bypass; bounded wet excursions without hard clipping.
    if(mixes[size_t(i)]>0&&std::abs(x)>.95f)x=std::copysign(.95f+.05f*std::tanh((std::abs(x)-.95f)*20.f),x);
    buffer.setSample(ch,offset+i,x);peaks[5]=std::max(peaks[5],std::abs(spaceBuffer.getSample(ch,i)));peaks[6]=std::max(peaks[6],std::abs(x));
   }
  }
  for(size_t k=0;k<meters.size();k++)meters[k].store(std::max(peaks[k],meters[k].load()*.86f));
 }
};
