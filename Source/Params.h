#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
struct PDef { const char* id; const char* name; float min, max, def, step; const char* section; };
static const PDef kParams[] = {
 {"inGain","Entrada",-24,24,0,0.1f,"IO"},{"outGain","Saida",-24,12,0,0.1f,"IO"},
 {"hpf","Corte Grave",20,400,20,1,"EQ"},{"bass","Graves",-12,12,0,0.1f,"EQ"},{"body","Corpo",-12,12,0,0.1f,"EQ"},
 {"mids","Medios",-12,12,0,0.1f,"EQ"},{"nasal","Nasalidade",-12,6,0,0.1f,"EQ"},{"presence","Presenca",-12,12,0,0.1f,"EQ"},
 {"air","Brilho",-12,12,0,0.1f,"EQ"},
 {"gateThresh","Gate",-90,-20,-90,0.5f,"DYN"},{"deEss","De-Esser",0,100,0,1,"DYN"},
 {"compThresh","Threshold",-60,0,0,0.1f,"DYN"},{"compRatio","Ratio",1,20,2,0.1f,"DYN"},
 {"compAttack","Attack",0.1f,100,10,0.1f,"DYN"},{"compRelease","Release",10,1000,150,1,"DYN"},{"makeup","Makeup",0,24,0,0.1f,"DYN"},
 {"transient","Transiente",-100,100,0,1,"DYN"},
 {"drive","Saturacao",0,100,0,1,"COLOR"},{"width","Stereo Width",0,200,100,1,"COLOR"},
 {"chorus","Chorus",0,100,0,1,"FX"},{"phaser","Phaser",0,100,0,1,"FX"},
 {"delayMix","Delay Mix",0,100,0,1,"FX"},{"delayDiv","Divisao",0,4,2,1,"FX"},{"delayFb","Feedback",0,90,35,1,"FX"},
 {"revMix","Reverb Mix",0,100,0,1,"FX"},{"revSize","Tamanho",0,100,50,1,"FX"},
 {"ceiling","Ceiling",-12,0,-1,0.1f,"MASTER"},{"limitDrive","Loudness",0,18,0,0.1f,"MASTER"},
};
inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout l;
  for (auto& p : kParams)
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{p.id,1}, p.name, juce::NormalisableRange<float>(p.min,p.max,p.step), p.def));
  return l;
}
