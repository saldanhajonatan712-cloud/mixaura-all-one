#include "PluginProcessor.h"
#include "PluginEditor.h"
using Coef = juce::dsp::IIR::Coefficients<float>;
struct Preset { const char* name; std::vector<std::pair<const char*, float>> v; };
static const std::vector<Preset> kPresets = {
 {"Init", {}},
 {"Trap Vocal", {{"hpf",90},{"body",2},{"nasal",-3},{"presence",4},{"air",5},{"deEss",55},{"compThresh",-22},{"compRatio",4},{"compAttack",5},{"makeup",6},{"drive",18},{"delayMix",14},{"delayDiv",1},{"revMix",12}}},
 {"Funk", {{"hpf",80},{"bass",2},{"mids",2},{"presence",5},{"air",3},{"deEss",40},{"compThresh",-24},{"compRatio",6},{"compAttack",2},{"makeup",8},{"drive",30},{"revMix",6}}},
 {"Rap", {{"hpf",85},{"body",3},{"nasal",-2},{"presence",3},{"air",2},{"deEss",45},{"compThresh",-20},{"compRatio",4},{"makeup",5},{"drive",12},{"revMix",5}}},
 {"Drill", {{"hpf",100},{"mids",-2},{"presence",5},{"air",4},{"deEss",50},{"compThresh",-26},{"compRatio",6},{"makeup",8},{"drive",22},{"delayMix",18},{"delayDiv",1},{"delayFb",45},{"revMix",15}}},
 {"Voz Grave", {{"hpf",60},{"bass",2},{"body",4},{"mids",-2},{"presence",3},{"air",4},{"deEss",30},{"compThresh",-18},{"compRatio",3},{"makeup",4}}},
 {"Voz Aguda", {{"hpf",120},{"body",3},{"nasal",-4},{"deEss",70},{"compThresh",-20},{"compRatio",3},{"makeup",4},{"revMix",10}}},
 {"Adlibs", {{"hpf",200},{"bass",-6},{"mids",4},{"presence",6},{"air",6},{"compThresh",-28},{"compRatio",8},{"makeup",8},{"drive",35},{"width",160},{"delayMix",30},{"delayDiv",2},{"revMix",25},{"chorus",20}}},
 {"Vocal Profissional", {{"hpf",80},{"body",1.5f},{"nasal",-2.5f},{"presence",2.5f},{"air",3},{"deEss",45},{"compThresh",-18},{"compRatio",3},{"makeup",4},{"drive",8},{"revMix",10}}},
 {"Master Loud", {{"bass",1.5f},{"air",2},{"compThresh",-14},{"compRatio",2},{"compAttack",30},{"makeup",2},{"drive",10},{"width",115},{"limitDrive",8}}},
};
MixAuraProcessor::MixAuraProcessor() : AudioProcessor(BusesProperties()
  .withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}
bool MixAuraProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
  return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() && l.getMainInputChannelSet() == l.getMainOutputChannelSet();
}
int MixAuraProcessor::getNumPrograms() { return (int) kPresets.size(); }
const juce::String MixAuraProcessor::getProgramName(int i)
{
  if (i < 0 || i >= (int) kPresets.size())
    return {};
  return kPresets[(size_t) i].name;
}
void MixAuraProcessor::setCurrentProgram(int i) {
  if (i < 0 || i >= (int) kPresets.size())
    return;
  currentProgram = i;
  for (auto& d : kParams) if (auto* pr = apvts.getParameter(d.id)) pr->setValueNotifyingHost(pr->convertTo0to1(d.def));
  for (auto& [id, v] : kPresets[(size_t) i].v) if (auto* pr = apvts.getParameter(id)) pr->setValueNotifyingHost(pr->convertTo0to1(v));
}
void MixAuraProcessor::prepareToPlay(double s, int block) {
  sr = s; juce::dsp::ProcessSpec spec { s, (juce::uint32) block, 2 };
  for (auto* f : { &hpf,&bass,&body,&mids,&nasal,&presence,&air,&dsLow,&dsHigh,&kShelf,&kHp }) f->prepare(spec);
  gate.prepare(spec); comp.prepare(spec); deEsser.prepare(spec); chorus.prepare(spec); phaser.prepare(spec);
  reverb.prepare(spec); limiter.prepare(spec); delay.prepare(spec); os.initProcessing((size_t) block); os.reset();
  hiBand.setSize(2, block); kBuf.setSize(2, block);
  *dsLow.state = *Coef::makeLowPass(s, 5000); *dsHigh.state = *Coef::makeHighPass(s, 5000);
  *kShelf.state = *Coef::makeHighShelf(s, 1500, 0.707f, juce::Decibels::decibelsToGain(4.0f)); *kHp.state = *Coef::makeHighPass(s, 38, 0.5f);
  setLatencySamples((int) os.getLatencyInSamples()); updateFilters();
}
void MixAuraProcessor::updateFilters() {
  auto g = [](float d){ return juce::Decibels::decibelsToGain(d); };
  *hpf.state = *Coef::makeHighPass(sr, p("hpf"), 0.707f);
  *bass.state = *Coef::makeLowShelf(sr, 100, 0.707f, g(p("bass")));
  *body.state = *Coef::makePeakFilter(sr, 250, 0.9f, g(p("body")));
  *mids.state = *Coef::makePeakFilter(sr, 800, 0.8f, g(p("mids")));
  *nasal.state = *Coef::makePeakFilter(sr, 1200, 2.5f, g(p("nasal")));
  *presence.state = *Coef::makePeakFilter(sr, 3500, 0.9f, g(p("presence")));
  *air.state = *Coef::makeHighShelf(sr, 10000, 0.707f, g(p("air")));
}
void MixAuraProcessor::processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) {
  juce::ScopedNoDenormals nd; const int n = buf.getNumSamples();
  if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) { if (auto b = pos->getBpm()) bpm = *b; playing = pos->getIsPlaying(); }
  buf.applyGain(juce::Decibels::decibelsToGain(p("inGain")));
  updateFilters();
  juce::dsp::AudioBlock<float> blk(buf); juce::dsp::ProcessContextReplacing<float> ctx(blk);
  gate.setThreshold(p("gateThresh")); gate.setRatio(10); gate.setAttack(1); gate.setRelease(100);
  if (p("gateThresh") > -89) gate.process(ctx);
  for (auto* f : { &hpf,&bass,&body,&mids,&nasal,&presence,&air }) f->process(ctx);
  // De-esser: compress >5kHz band only
  const float ds = p("deEss");
  if (ds > 0) {
    hiBand.makeCopyOf(buf, true);
    kBuf.makeCopyOf(buf, true);
    juce::dsp::AudioBlock<float> hb(hiBand), lb(kBuf);
    dsHigh.process(juce::dsp::ProcessContextReplacing<float>(hb));
    dsLow.process(juce::dsp::ProcessContextReplacing<float>(lb));
    deEsser.setThreshold(-ds * 0.45f); deEsser.setRatio(1 + ds * 0.11f); deEsser.setAttack(0.5f); deEsser.setRelease(80);
    deEsser.process(juce::dsp::ProcessContextReplacing<float>(hb));
    buf.makeCopyOf(kBuf, true);
    for (int c = 0; c < 2; ++c) buf.addFrom(c, 0, hiBand, c, 0, n);
  }
  // Transient shaper (envelope difference)
  const float tr = p("transient") / 100.0f;
  if (std::abs(tr) > 0.001f) for (int c = 0; c < 2; ++c) { auto* d = buf.getWritePointer(c);
    for (int i = 0; i < n; ++i) { float a = std::abs(d[i]);
      envFast[c] += (a - envFast[c]) * 0.05f; envSlow[c] += (a - envSlow[c]) * 0.002f;
      d[i] *= juce::jlimit(0.25f, 4.0f, 1.0f + tr * 4.0f * (envFast[c] - envSlow[c]) / (envSlow[c] + 1e-4f)); } }
  // Oversampled analog saturation
  const float drive = p("drive") / 100.0f;
  if (drive > 0) { auto up = os.processSamplesUp(blk); const float k = 1 + drive * 12;
    for (size_t c = 0; c < up.getNumChannels(); ++c) { auto* d = up.getChannelPointer(c);
      for (size_t i = 0; i < up.getNumSamples(); ++i) d[i] = (1 - drive) * d[i] + drive * std::tanh(d[i] * k) / std::tanh(k); }
    os.processSamplesDown(blk); }
  // Compressor
  float before = buf.getRMSLevel(0, 0, n);
  comp.setThreshold(p("compThresh")); comp.setRatio(p("compRatio")); comp.setAttack(p("compAttack")); comp.setRelease(p("compRelease"));
  comp.process(ctx);
  gr = juce::Decibels::gainToDecibels(buf.getRMSLevel(0, 0, n) / (before + 1e-9f));
  buf.applyGain(juce::Decibels::decibelsToGain(p("makeup")));
  // Mid/Side width
  const float w = p("width") / 100.0f; auto* L = buf.getWritePointer(0); auto* R = buf.getWritePointer(1);
  for (int i = 0; i < n; ++i) { float m = (L[i] + R[i]) * 0.5f, s = (L[i] - R[i]) * 0.5f * w; L[i] = m + s; R[i] = m - s; }
  if (p("chorus") > 0) { chorus.setRate(0.8f); chorus.setDepth(0.3f); chorus.setCentreDelay(8); chorus.setMix(p("chorus") / 100); chorus.process(ctx); }
  if (p("phaser") > 0) { phaser.setRate(0.5f); phaser.setDepth(0.7f); phaser.setFeedback(0.5f); phaser.setMix(p("phaser") / 100); phaser.process(ctx); }
  // BPM synced delay
  static const float beats[] = { 0.25f, 0.5f, 1, 2, 4 };
  const float dm = p("delayMix") / 100, fb = p("delayFb") / 100;
  delay.setDelay((float) juce::jmin(sr * 4.9, sr * 60.0 / bpm.load() * beats[(int) p("delayDiv")]));
  if (dm > 0) for (int c = 0; c < 2; ++c) { auto* d = buf.getWritePointer(c);
    for (int i = 0; i < n; ++i) { float y = delay.popSample(c); delay.pushSample(c, d[i] + y * fb); d[i] += y * dm; } }
  if (p("revMix") > 0) { juce::dsp::Reverb::Parameters rp; rp.roomSize = p("revSize") / 100; rp.wetLevel = p("revMix") / 100; rp.dryLevel = 1; rp.width = 1; reverb.setParameters(rp); reverb.process(ctx); }
  // Master limiter
  buf.applyGain(juce::Decibels::decibelsToGain(p("limitDrive")));
  limiter.setThreshold(p("ceiling")); limiter.setRelease(50); limiter.process(ctx);
  buf.applyGain(juce::Decibels::decibelsToGain(p("outGain")));
  // Metering
  float pl = buf.getMagnitude(0, 0, n), pr = buf.getMagnitude(1, 0, n), tp = juce::jmax(pl, pr);
  for (int c = 0; c < 2; ++c) { auto* d = buf.getReadPointer(c); for (int i = 1; i < n; ++i) for (int s = 1; s < 4; ++s) tp = juce::jmax(tp, std::abs(d[i-1] + (d[i]-d[i-1]) * s * 0.25f)); }
  peakL = pl; peakR = pr; truePeak = juce::Decibels::gainToDecibels(tp, -100.0f);
  rmsDb = juce::Decibels::gainToDecibels(0.5f * (buf.getRMSLevel(0,0,n) + buf.getRMSLevel(1,0,n)), -100.0f);
  kBuf.makeCopyOf(buf, true); juce::dsp::AudioBlock<float> kb(kBuf); juce::dsp::ProcessContextReplacing<float> kc(kb); kShelf.process(kc); kHp.process(kc);
  for (int c = 0; c < 2; ++c) { auto* d = kBuf.getReadPointer(c); for (int i = 0; i < n; ++i) lufsAcc += d[i] * d[i]; }
  lufsN += n; if (lufsN >= sr * 0.4) { lufsM = (float) (-0.691 + 10 * std::log10(lufsAcc / lufsN + 1e-12)); lufsAcc = 0; lufsN = 0; }
  for (int i = 0; i < n; ++i) { if (!fftReady) { fftFifo[fifoPos] = 0.5f * (L[i] + R[i]); if (++fifoPos == (1 << fftOrder)) { fifoPos = 0; fftReady = true; } } }
}
juce::AudioProcessorEditor* MixAuraProcessor::createEditor() { return new MixAuraEditor(*this); }
void MixAuraProcessor::getStateInformation(juce::MemoryBlock& d) { if (auto x = apvts.copyState().createXml()) copyXmlToBinary(*x, d); }
void MixAuraProcessor::setStateInformation(const void* d, int s)
{
  if (d != nullptr && s > 0)
    if (auto x = getXmlFromBinary(d, s))
      apvts.replaceState(juce::ValueTree::fromXml(*x));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MixAuraProcessor(); }
