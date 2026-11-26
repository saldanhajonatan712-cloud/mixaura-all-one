#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
class MixAuraProcessor : public juce::AudioProcessor {
public:
  MixAuraProcessor();
  void prepareToPlay(double, int) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout&) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }
  const juce::String getName() const override { return "MIX AURA ALL-IN-ONE"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  double getTailLengthSeconds() const override { return 5.0; }
  int getNumPrograms() override;
  int getCurrentProgram() override { return currentProgram; }
  void setCurrentProgram(int) override;
  const juce::String getProgramName(int) override;
  void changeProgramName(int, const juce::String&) override {}
  void getStateInformation(juce::MemoryBlock&) override;
  void setStateInformation(const void*, int) override;

  juce::AudioProcessorValueTreeState apvts { *this, nullptr, "STATE", createLayout() };
  std::atomic<float> peakL{0}, peakR{0}, rmsDb{-100}, lufsM{-100}, truePeak{-100}, gr{0};
  std::atomic<double> bpm{120}; std::atomic<bool> playing{false};
  static constexpr int fftOrder = 11; float fftFifo[1 << fftOrder] {}; std::atomic<int> fifoPos{0}; std::atomic<bool> fftReady{false};
private:
  float p(const char* id) const { return apvts.getRawParameterValue(id)->load(); }
  void updateFilters();
  double sr = 44100; int currentProgram = 0;
  using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
  Filter hpf, bass, body, mids, nasal, presence, air, dsLow, dsHigh, kShelf, kHp;
  juce::dsp::NoiseGate<float> gate; juce::dsp::Compressor<float> comp, deEsser;
  juce::dsp::Chorus<float> chorus; juce::dsp::Phaser<float> phaser;
  juce::dsp::Reverb reverb; juce::dsp::Limiter<float> limiter;
  juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 48000 * 5 };
  juce::dsp::Oversampling<float> os { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
  juce::AudioBuffer<float> hiBand, kBuf; float envFast[2]{}, envSlow[2]{}; double lufsAcc = 0; int lufsN = 0;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixAuraProcessor)
};
