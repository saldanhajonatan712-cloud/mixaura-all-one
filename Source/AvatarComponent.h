#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class AvatarComponent : public juce::Component {
public:
  AvatarComponent();
  void setBeat(double bpm, bool hostPlaying, double dt);
  void paint(juce::Graphics&) override; void resized() override;
private:
  juce::TextButton next {"Danca >"}, random {"Aleatoria"}, pause {"Pausar"}, gender {"Masculino"};
  int dance = 0; bool paused = false, female = false, isPlaying = false; double phase = 0, bpm = 120;
};
