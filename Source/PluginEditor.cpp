#include "PluginEditor.h"
static const juce::Colour kBg(0xff0d0b17), kPanel(0xff16132a), kPurple(0xff9b5cf0), kBlue(0xff3b8cf0), kGold(0xfff2c14e);
AuraLook::AuraLook() { setColour(juce::Slider::textBoxTextColourId, kGold); setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
  setColour(juce::ComboBox::backgroundColourId, kPanel); setColour(juce::TextButton::buttonColourId, kPanel); }
void AuraLook::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) {
  auto r = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(4); auto sz = juce::jmin(r.getWidth(), r.getHeight());
  r = r.withSizeKeepingCentre(sz, sz); const float ang = a0 + pos * (a1 - a0);
  g.setColour(kPanel.brighter(0.15f)); g.fillEllipse(r.reduced(5));
  juce::Path bg, val; bg.addCentredArc(r.getCentreX(), r.getCentreY(), sz / 2 - 2, sz / 2 - 2, 0, a0, a1, true);
  val.addCentredArc(r.getCentreX(), r.getCentreY(), sz / 2 - 2, sz / 2 - 2, 0, a0, ang, true);
  g.setColour(juce::Colours::white.withAlpha(0.1f)); g.strokePath(bg, juce::PathStrokeType(3));
  g.setColour(kPurple.withAlpha(0.35f)); g.strokePath(val, juce::PathStrokeType(7)); g.setColour(kPurple); g.strokePath(val, juce::PathStrokeType(3, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  juce::Path ptr; ptr.addRoundedRectangle(-1.5f, -sz / 2 + 7, 3, sz * 0.22f, 1.5f);
  g.setColour(kGold); g.fillPath(ptr, juce::AffineTransform::rotation(ang).translated(r.getCentreX(), r.getCentreY()));
}
MixAuraEditor::MixAuraEditor(MixAuraProcessor& p) : AudioProcessorEditor(p), proc(p) {
  setLookAndFeel(&look);
  for (auto& d : kParams) {
    auto* s = sliders.add(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16); addAndMakeVisible(s);
    attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(proc.apvts, d.id, *s));
    auto* l = labels.add(new juce::Label({}, d.name)); l->setJustificationType(juce::Justification::centred); l->setFont(12.0f); addAndMakeVisible(l);
  }
  for (int i = 0; i < proc.getNumPrograms(); ++i) presets.addItem(proc.getProgramName(i), i + 1);
  presets.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
  presets.onChange = [this] { proc.setCurrentProgram(presets.getSelectedId() - 1); };
  addAndMakeVisible(presets); addAndMakeVisible(avatar);
  setResizable(true, true); setResizeLimits(900, 600, 2200, 1400); setSize(1200, 760); startTimerHz(30);
}
MixAuraEditor::~MixAuraEditor() { setLookAndFeel(nullptr); }
void MixAuraEditor::resized() {
  auto r = getLocalBounds().reduced(12); auto top = r.removeFromTop(40); presets.setBounds(top.removeFromRight(220).reduced(4));
  avatar.setBounds(r.removeFromRight(juce::jmax(260, r.getWidth() / 4)).reduced(6));
  auto viz = r.removeFromTop(170); meterArea = viz.removeFromRight(200).reduced(6); specArea = viz.reduced(6);
  const int cols = juce::jmax(6, r.getWidth() / 90), n = sliders.size(), rows = (n + cols - 1) / cols;
  const int cw = r.getWidth() / cols, ch = r.getHeight() / rows;
  for (int i = 0; i < n; ++i) { auto cell = juce::Rectangle<int>(r.getX() + (i % cols) * cw, r.getY() + (i / cols) * ch, cw, ch).reduced(4);
    labels[i]->setBounds(cell.removeFromTop(16)); sliders[i]->setBounds(cell); }
}
void MixAuraEditor::timerCallback() {
  if (proc.fftReady) { std::fill(fftData.begin(), fftData.end(), 0.0f); std::copy(std::begin(proc.fftFifo), std::end(proc.fftFifo), fftData.begin());
    juce::dsp::WindowingFunction<float>(1 << MixAuraProcessor::fftOrder, juce::dsp::WindowingFunction<float>::hann).multiplyWithWindowingTable(fftData.data(), 1 << MixAuraProcessor::fftOrder);
    fft.performFrequencyOnlyForwardTransform(fftData.data()); const int half = 1 << (MixAuraProcessor::fftOrder - 1);
    for (size_t i = 0; i < spectrum.size(); ++i) { int bin = juce::jlimit(0, half - 1, (int) (std::pow(1000.0, i / 256.0) * 20.0 / (proc.getSampleRate() / 2) * half));
      float db = juce::jmap(juce::Decibels::gainToDecibels(fftData[(size_t) bin] / half, -100.0f), -100.0f, 0.0f, 0.0f, 1.0f);
      spectrum[i] = juce::jmax(db, spectrum[i] * 0.9f); }
    proc.fftReady = false; }
  dispL = juce::jmax(proc.peakL.load(), dispL * 0.9f); dispR = juce::jmax(proc.peakR.load(), dispR * 0.9f);
  avatar.setBeat(proc.bpm.load(), proc.playing.load(), 1.0 / 30.0); repaint();
}
void MixAuraEditor::paint(juce::Graphics& g) {
  g.fillAll(kBg); g.setGradientFill(juce::ColourGradient(kPurple.withAlpha(0.25f), 0, 0, kBlue.withAlpha(0.0f), (float) getWidth() * 0.6f, (float) getHeight(), true)); g.fillAll();
  g.setColour(kGold); g.setFont(juce::FontOptions(24.0f, juce::Font::bold)); g.drawText(juce::CharPointer_UTF8("MIX AURA ALL-IN-ONE"), 20, 12, 500, 34, juce::Justification::left);
  g.setColour(kPanel); g.fillRoundedRectangle(specArea.toFloat(), 8); g.fillRoundedRectangle(meterArea.toFloat(), 8);
  juce::Path sp; auto sa = specArea.toFloat().reduced(4); sp.startNewSubPath(sa.getX(), sa.getBottom());
  for (size_t i = 0; i < spectrum.size(); ++i) sp.lineTo(sa.getX() + sa.getWidth() * i / 255.0f, sa.getBottom() - spectrum[i] * sa.getHeight());
  sp.lineTo(sa.getRight(), sa.getBottom()); sp.closeSubPath();
  g.setGradientFill(juce::ColourGradient(kPurple, sa.getX(), 0, kGold, sa.getRight(), 0, false)); g.setOpacity(0.5f); g.fillPath(sp); g.setOpacity(1); g.strokePath(sp, juce::PathStrokeType(1.5f));
  auto m = meterArea.reduced(8); auto bars = m.removeFromLeft(30);
  for (int c = 0; c < 2; ++c) { auto b = bars.removeFromLeft(14).reduced(2, 0).toFloat(); g.setColour(juce::Colours::white.withAlpha(0.08f)); g.fillRect(b);
    float lv = juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(c ? dispR : dispL, -60.0f) + 60) / 60);
    g.setGradientFill(juce::ColourGradient(kBlue, 0, b.getBottom(), kGold, 0, b.getY(), false)); g.fillRect(b.withTop(b.getBottom() - b.getHeight() * lv)); }
  g.setFont(juce::FontOptions(13.0f)); int y = m.getY();
  auto row = [&](const char* k, float v) { g.setColour(juce::Colours::grey); g.drawText(k, m.getX() + 6, y, 80, 18, juce::Justification::left);
    g.setColour(kGold); g.drawText(juce::String(v, 1), m.getX() + 86, y, 60, 18, juce::Justification::right); y += 22; };
  row("LUFS M", proc.lufsM); row("RMS", proc.rmsDb); row("Peak", juce::Decibels::gainToDecibels(juce::jmax(dispL, dispR), -100.0f)); row("True Peak", proc.truePeak); row("Comp GR", proc.gr);
}
