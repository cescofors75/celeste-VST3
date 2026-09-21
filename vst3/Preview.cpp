#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI init;
 std::unique_ptr<juce::AudioProcessor> processor(createPluginFilter());
 processor->prepareToPlay(48000,512);
 juce::AudioBuffer<float> audio(2,512);juce::MidiBuffer midi;
 for(int k=0;k<100;k++){for(int c=0;c<2;c++)for(int i=0;i<512;i++)audio.setSample(c,i,.35f*std::sin(float(k*512+i)*.0288f));processor->processBlock(audio,midi);}
 std::unique_ptr<juce::AudioProcessorEditor> editor(processor->createEditor());
 auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f);
 juce::File file(argc>1?argv[1]:"celeste-preview.png");auto stream=file.createOutputStream();
 if(!stream)return 1;stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;if(!png.writeImageToStream(image,*stream))return 2;
 editor.reset();processor->releaseResources();std::cout<<"Editor rendered successfully\n";
}
