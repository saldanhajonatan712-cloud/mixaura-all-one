#pragma once
#include "PluginProcessor.h"
#include "AvatarComponent.h"
class AuraLook : public juce::LookAndFeel_V4 {
public:
  AuraLook();
  void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};
class MixAuraEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit MixAuraEditor(MixAuraProcessor&);
  ~MixAuraEditor() override;
  void paint(juce::Graphics&) override; void resized() override;
private:
  void timerCallback() override;
  MixAuraProcessor& proc; AuraLook look; AvatarComponent avatar;
  juce::ComboBox presets;
  juce::OwnedArray<juce::Slider> sliders; juce::OwnedArray<juce::Label> labels;
  juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;
  juce::dsp::FFT fft { MixAuraProcessor::fftOrder }; std::array<float, 2 << MixAuraProcessor::fftOrder> fftData {}; std::array<float, 256> spectrum {};
  juce::Rectangle<int> meterArea, specArea; float dispL = 0, dispR = 0;
};
