#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
using namespace juce;
static const char* ids[]={"timeA","timeB","feedback","cutoff","fold","space","motion","mix","rate","output"};
static const char* labels[]={"DELAY A","DELAY B","FEEDBACK","FILTER","WAVEFOLDER","SPACE","MOTION","MASTER MIX"};
static const uint32_t accents[]={0xff35a8ff,0xff5bcfff,0xff35a8ff,0xffffa53d,0xfffa66bf,0xffaa71ff,0xff00dfa2,0xffb9d3df};
class Processor:public AudioProcessor {
public:
 AudioProcessorValueTreeState state;
 Engine engine;
 Processor():AudioProcessor(BusesProperties().withInput("Stereo input",AudioChannelSet::stereo(),true).withOutput("Stereo output",AudioChannelSet::stereo(),true)),state(*this,nullptr,"CELESTE",layout()){}
 static AudioProcessorValueTreeState::ParameterLayout layout(){
  AudioProcessorValueTreeState::ParameterLayout l;
  const float lo[]={20,20,0,80,0,0,0,0,.05f,-18},hi[]={1200,1600,.85f,18000,1,1,1,1,8,6},defaults[]={340,510,.42f,6500,.16f,.35f,.25f,.42f,.22f,0};
  for(int i=0;i<10;i++){NormalisableRange<float> r(lo[i],hi[i],i<2?1.f:i==3?1.f:.001f);if(i==3)r.setSkewForCentre(2500);if(i==8)r.setSkewForCentre(.8f);
   l.add(std::make_unique<AudioParameterFloat>(ParameterID(ids[i],1),i<8?labels[i]:i==8?"LFO Rate":"Output",r,defaults[i]));}
  l.add(std::make_unique<AudioParameterBool>(ParameterID("series",1),"Delays in series",false));
  l.add(std::make_unique<AudioParameterBool>(ParameterID("bypass",1),"Bypass",false));return l;
 }
 Settings settings()const{Settings s;float* fields[]={&s.a,&s.b,&s.feedback,&s.cutoff,&s.fold,&s.space,&s.motion,&s.mix,&s.rate,&s.output};for(int i=0;i<10;i++)*fields[i]=state.getRawParameterValue(ids[i])->load();s.series=state.getRawParameterValue("series")->load()>.5f;s.bypass=state.getRawParameterValue("bypass")->load()>.5f;return s;}
 const String getName()const override{return "CELESTE Parallel";}
 void prepareToPlay(double sr,int)override{engine.prepare(sr);engine.set(settings(),true);}
 void releaseResources()override{}
 void reset()override{engine.reset();engine.set(settings(),true);}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{return b.getMainInputChannelSet()==AudioChannelSet::stereo()&&b.getMainOutputChannelSet()==AudioChannelSet::stereo();}
 void processBlock(AudioBuffer<float>& b,MidiBuffer&)override{engine.process(b,settings());}
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 20;}
 AudioProcessorParameter* getBypassParameter()const override{return state.getParameter("bypass");}
 bool hasEditor()const override{return true;}AudioProcessorEditor* createEditor()override;
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
 const String getProgramName(int)override{return "Custom";}void changeProgramName(int,const String&)override{}
 void getStateInformation(MemoryBlock& b)override{auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* d,int n)override{auto xml=getXmlFromBinary(d,n);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(ValueTree::fromXml(*xml));}
 void preset(int which){
  const float values[5][10]={{340,510,.42f,6500,.16f,.35f,.25f,.42f,.22f,0},{480,720,.62f,3400,.08f,.72f,.5f,.64f,.13f,0},{125,375,.48f,9500,.32f,.2f,.18f,.55f,.7f,0},{230,610,.7f,1800,.42f,.6f,.65f,.7f,.09f,0},{73,147,.3f,12000,.72f,.15f,.38f,.4f,2.4f,0}};
  for(int i=0;i<10;i++){auto* p=state.getParameter(ids[i]);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(values[which][i]));p->endChangeGesture();}
  auto* p=state.getParameter("series");p->beginChangeGesture();p->setValueNotifyingHost(which==2?1.f:0.f);p->endChangeGesture();
 }
};
class NeonLook:public LookAndFeel_V4 {
public:
 NeonLook(){setColour(Slider::textBoxTextColourId,Colour(0xffdbedf5));setColour(Slider::textBoxOutlineColourId,Colours::transparentBlack);setColour(Slider::textBoxBackgroundColourId,Colour(0xff0a1821));setColour(ComboBox::backgroundColourId,Colour(0xff0c202c));setColour(ComboBox::outlineColourId,Colour(0xff275163));setColour(TextButton::buttonColourId,Colour(0xff102733));setColour(TextButton::buttonOnColourId,Colour(0xff3d2432));setColour(TextButton::textColourOffId,Colour(0xffa9d9e9));}
 void drawRotarySlider(Graphics& g,int x,int y,int w,int h,float pos,float start,float end,Slider& s)override{
  auto r=Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(9);float radius=jmin(r.getWidth(),r.getHeight())*.5f,cx=r.getCentreX(),cy=r.getCentreY();auto color=s.findColour(Slider::rotarySliderFillColourId);
  g.setColour(Colours::black.withAlpha(.5f));g.fillEllipse(cx-radius+2,cy-radius+5,radius*2,radius*2);
  g.setGradientFill(ColourGradient(Colour(0xff29404a),cx-radius,cy-radius,Colour(0xff040a10),cx+radius,cy+radius,false));g.fillEllipse(cx-radius,cy-radius,radius*2,radius*2);
  Path track;track.addCentredArc(cx,cy,radius-3,radius-3,0,start,end,true);g.setColour(Colour(0xff304451));g.strokePath(track,PathStrokeType(3));
  Path arc;arc.addCentredArc(cx,cy,radius-3,radius-3,0,start,start+pos*(end-start),true);g.setColour(color.withAlpha(.15f));g.strokePath(arc,PathStrokeType(9));g.setColour(color);g.strokePath(arc,PathStrokeType(3));
  float angle=start+pos*(end-start);g.setColour(Colour(0xffe3f9ff));g.drawLine(cx+std::sin(angle)*(radius*.37f),cy-std::cos(angle)*(radius*.37f),cx+std::sin(angle)*(radius*.7f),cy-std::cos(angle)*(radius*.7f),2);
 }
};
class Editor:public AudioProcessorEditor,private Timer {
 Processor& p;NeonLook look;TooltipWindow tips{this,650};
 std::array<Slider,10> knobs;
 std::array<std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>,10> links;
 TextButton bypass{"BYPASS"},series{"SERIES A -> B"};ComboBox presets;
 std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> bypassLink,seriesLink;
 float phase=0;
 void text(Graphics& g,String s,Rectangle<float> r,float size,Colour color,int alignment=Justification::centredLeft){g.setColour(color);g.setFont(Font(size));g.drawText(s,r,alignment);}
 void cable(Graphics& g,Point<float> a,Point<float>b,Colour colour,float level){Path path;path.startNewSubPath(a);path.cubicTo(a.x+65,a.y,b.x-65,b.y,b.x,b.y);g.setColour(colour.withAlpha(.24f));g.strokePath(path,PathStrokeType(2));if(level>.0001f){g.setColour(colour.withAlpha(.85f));g.strokePath(path,PathStrokeType(1.6f));for(int i=0;i<3;i++){auto dot=path.getPointAlongPath(path.getLength()*std::fmod(phase*.18f+i/3.f,1.f));g.fillEllipse(dot.x-2.5f,dot.y-2.5f,5,5);}}}
 void node(Graphics& g,String name,Rectangle<float> r,Colour col,int meter,int shape=0){g.setColour(Colour(0xff0b1924));g.fillRoundedRectangle(r,7);g.setColour(col.withAlpha(.8f));g.drawRoundedRectangle(r,7,1.3f);text(g,name,r.withHeight(26).reduced(11,0),13,col);Path wave;for(int i=0;i<100;i++){float x=float(i)/99,angle=x*12.f-phase*2,y=std::sin(angle);if(shape==1)y=std::asin(std::sin(angle*2))*.65f;if(shape==2)y=std::sin(angle*2)*std::exp(-std::fmod(x*4,1.f)*3);float xx=r.getX()+12+x*(r.getWidth()-24),yy=r.getY()+43+y*9;if(i==0)wave.startNewSubPath(xx,yy);else wave.lineTo(xx,yy);}g.setColour(col.withAlpha(.8f));g.strokePath(wave,PathStrokeType(1.4f));float level=p.engine.meters[size_t(meter)].load();g.setColour(Colour(0xff21313e));g.fillRect(r.getX()+12,r.getBottom()-10,r.getWidth()-24,3.f);g.setColour(col);g.fillRect(r.getX()+12,r.getBottom()-10,(r.getWidth()-24)*jlimit(0.f,1.f,level*1.6f),3.f);}
public:
 Editor(Processor& proc):AudioProcessorEditor(proc),p(proc){setLookAndFeel(&look);setSize(1180,760);
  presets.addItemList({"01  Parallel Dreams","02  Celestial Bloom","03  Prism Cascade","04  Midnight Drift","05  Neon Dust"},1);presets.setTextWhenNothingSelected("PRESETS / choose a texture");presets.onChange=[this]{if(presets.getSelectedId()>0)p.preset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
  for(int i=0;i<10;i++){auto& k=knobs[size_t(i)];k.setSliderStyle(i<8?Slider::RotaryHorizontalVerticalDrag:Slider::LinearHorizontal);k.setTextBoxStyle(Slider::TextBoxBelow,false,104,23);k.setColour(Slider::rotarySliderFillColourId,Colour(accents[i%8]));k.setDoubleClickReturnValue(true,p.state.getParameter(ids[i])->convertFrom0to1(p.state.getParameter(ids[i])->getDefaultValue()));
   const char* help[]={"First stereo delay time. Double-click restores the default.","Second stereo delay time. SERIES feeds A into B.","Shared feedback; softened and damped for stable echoes.","Low-pass cutoff on the Wavefolder branch. MOTION modulates it.","Fold amount and drive. 2x oversampling softens aliasing.","Stereo reverb fed by the wet bus.","LFO depth: animates filter cutoff and wet amplitude.","Dry / wet blend. 0% is the original input.","LFO speed in cycles per second.","Master output gain. BYPASS restores unity gain."};k.setTooltip(help[i]);
   addAndMakeVisible(k);links[size_t(i)]=std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k);
   k.textFromValueFunction=[i](double v){return i<2?String(v,0)+" ms":i==3?(v>=1000?String(v/1000,2)+" kHz":String(v,0)+" Hz"):i==8?String(v,2)+" Hz":i==9?String(v,1)+" dB":String(v*100,0)+" %";};
   k.valueFromTextFunction=[i](const String& s){double v=s.getDoubleValue();return i==3&&s.containsIgnoreCase("k")?v*1000:i>=2&&i<=7&&i!=3?v/100:v;};k.updateText();
  }
  for(auto* b:{&bypass,&series}){b->setClickingTogglesState(true);addAndMakeVisible(b);}
  bypassLink=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass);seriesLink=std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(p.state,"series",series);startTimerHz(30);
 }
 ~Editor()override{stopTimer();setLookAndFeel(nullptr);}
 void timerCallback()override{if(isShowing()){phase+=1.f/30;repaint();}}
 void resized()override{presets.setBounds(630,24,285,34);series.setBounds(932,24,140,34);bypass.setBounds(1082,24,80,34);for(int i=0;i<8;i++)knobs[size_t(i)].setBounds(20+i*144,549,136,142);knobs[8].setBounds(260,704,205,45);knobs[9].setBounds(545,704,205,45);}
 void paint(Graphics& g)override{
  g.fillAll(Colour(0xff071018));g.setColour(Colour(0xff10232e));for(int x=20;x<1170;x+=20)g.drawVerticalLine(x,112,480);for(int y=112;y<480;y+=20)g.drawHorizontalLine(y,20,1160);
  text(g,"C E L E S T E",{24,14,300,42},30,Colour(0xff8beaff));text(g,"PARALLEL FABRIC  /  NATIVE AUDIO",{27,58,450,20},12,Colour(0xff7ea7b9));
  text(g,"ONE SOURCE / MULTIPLE TEXTURES",{24,94,500,24},15,Colour(0xffc7e6f3));
  const auto s=p.settings();text(g,s.bypass?"BYPASS / DRY SIGNAL":"STEREO DSP  /  2x WAVEFOLDER",{795,94,365,24},13,s.bypass?Colour(0xffffc276):Colour(0xff00dfa2),Justification::centredRight);
  Colour blue(0xff35a8ff),cyan(0xff5bcfff),orange(0xffffa53d),pink(0xfffa66bf),purple(0xffaa71ff),green(0xff00dfa2),neutral(0xffb9d3df);
  auto m=[this](int n){return p.engine.meters[size_t(n)].load();};
  cable(g,{178,299},{300,210},neutral,m(0));if(!s.series)cable(g,{178,299},{550,210},neutral,m(0));
  cable(g,{178,299},{865,299},neutral.withAlpha(.4f),m(0)*(s.bypass?1.f:1.f-s.mix));
  if(s.series)cable(g,{480,210},{550,210},blue,m(1));else cable(g,{480,210},{865,299},blue,m(1));cable(g,{730,210},{865,299},cyan,m(2));
  cable(g,{178,299},{300,340},neutral,m(0));cable(g,{480,340},{550,340},pink,m(4));cable(g,{730,340},{865,299},orange,m(3));
  cable(g,{865,299},{955,299},neutral,m(6));cable(g,{730,432},{865,299},purple,m(5));
  node(g,"AUDIO IN",{30,260,148,78},neutral,0);node(g,"DELAY A",{300,170,180,80},blue,1,2);node(g,"DELAY B",{550,170,180,80},cyan,2,2);
  node(g,"WAVEFOLDER",{300,300,180,80},pink,4,1);node(g,"FILTER + VCA",{550,300,180,80},orange,3);node(g,"WET BUS / SPACE",{550,400,180,70},purple,5);node(g,"AUDIO OUT",{955,260,180,78},neutral,6);
  g.setColour(neutral);g.fillEllipse(855,289,20,20);text(g,"MIX",{834,317,70,25},12,neutral,Justification::centred);
  text(g,"LFO",{310,417,80,25},15,green);text(g,String(s.rate,2)+" Hz / "+String(s.motion*100,0)+" %",{310,444,225,20},13,green);
  text(g,"Waves: visual signatures / Bars: measured signal",{24,493,650,20},12,Colour(0xff6d91a3));
  g.setColour(Colour(0xff17313e));g.drawHorizontalLine(526,20,1160);
  for(int i=0;i<8;i++){auto col=Colour(accents[i]);text(g,labels[i],{float(20+i*144),532,136,22},12,col,Justification::centred);}
  text(g,"LFO RATE",{177,707,82,30},11,green);text(g,"OUTPUT",{483,707,70,30},11,neutral);text(g,"CELESTE  /  v0.1",{914,705,235,35},13,Colour(0xff80bbcd),Justification::centredRight);
 }
};
AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}

