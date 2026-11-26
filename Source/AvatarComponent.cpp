#include "AvatarComponent.h"
static const char* kDances[] = { "Trap Bounce", "Funk Passinho", "Shuffle Viral", "Hip-Hop Groove", "Drill Slide" };
AvatarComponent::AvatarComponent() {
  for (auto* b : { &next, &random, &pause, &gender }) addAndMakeVisible(*b);
  next.onClick = [this] { dance = (dance + 1) % 5; };
  random.onClick = [this] { dance = juce::Random::getSystemRandom().nextInt(5); };
  pause.onClick = [this] { paused = !paused; pause.setButtonText(paused ? "Iniciar" : "Pausar"); };
  gender.onClick = [this] { female = !female; gender.setButtonText(female ? "Feminino" : "Masculino"); };
}
void AvatarComponent::setBeat(double b, bool p, double dt) { bpm = b; isPlaying = p; if (p && !paused) phase += dt * bpm / 60.0; repaint(); }
void AvatarComponent::resized() { auto r = getLocalBounds().removeFromBottom(28); int w = r.getWidth() / 4;
  for (auto* b : { &next, &random, &pause, &gender }) b->setBounds(r.removeFromLeft(w).reduced(2)); }
void AvatarComponent::paint(juce::Graphics& g) {
  auto area = getLocalBounds().withTrimmedBottom(32).toFloat();
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3a1d6e), area.getCentreX(), area.getY(), juce::Colour(0xff0b0a14), area.getCentreX(), area.getBottom(), false));
  g.fillRoundedRectangle(area, 10);
  const float t = (float) (phase * juce::MathConstants<double>::twoPi), s = area.getHeight() / 260.0f;
  const float cx = area.getCentreX(), base = area.getY();
  auto sw = std::sin(t), sw2 = std::sin(t * 0.5f);
  float bounce = 0, sway = 0, slide = 0, aL = 0, aR = 0, lL = 0, lR = 0;
  switch (dance) {
    case 0: bounce = std::abs(sw) * 6; aL = 0.4f * sw; aR = -0.4f * sw; lL = 0.2f * sw; lR = -0.2f * sw; break;
    case 1: sway = 0.08f * sw2; aL = 1.5f + 0.8f * sw2; aR = -1.5f - 0.8f * sw2; lL = std::max(0.0f, sw) * 0.6f; lR = -std::max(0.0f, -sw) * 0.6f; break;
    case 2: bounce = std::abs(sw) * 5; aL = 2.3f + 0.3f * sw; aR = -2.3f - 0.3f * sw; lL = std::max(0.0f, sw) * 0.7f; lR = -0.2f * sw; break;
    case 3: sway = 0.08f * sw2; aL = 0.5f * sw2; aR = -2.4f + 0.3f * sw; lL = 0.2f * sw; lR = -0.2f * sw; break;
    default: slide = 12 * sw2; aL = 2.3f + 0.3f * sw; aR = 0.4f * sw; lL = 0.2f * sw; lR = std::min(0.0f, sw) * 0.6f; break;
  }
  g.setColour(juce::Colour(0xfff2c14e)); g.setFont(12.0f);
  g.drawText(juce::String(kDances[dance]) + "  |  " + juce::String(bpm, 0) + " BPM" + (isPlaying ? "" : "  (play no DAW)"), area.reduced(8).toNearestInt(), juce::Justification::topLeft);
  juce::Graphics::ScopedSaveState ss(g);
  g.addTransform(juce::AffineTransform::translation(-100, 0).scaled(s).rotated(sway, 0, 200 * s).translated(cx + slide * s, base + bounce * s));
  auto limb = [&](float px, float py, float w, float h, float ang, juce::Colour c) {
    juce::Path p; p.addRoundedRectangle(px - w / 2, py, w, h, w / 2); p.applyTransform(juce::AffineTransform::rotation(ang, px, py)); g.setColour(c); g.fillPath(p); };
  const juce::Colour skin(0xff8a5a36), pants(0xff2a2d36), gold(0xfff2c14e);
  limb(88, 150, 16, 70, lL, pants); limb(112, 150, 16, 70, lR, pants);
  g.setColour(juce::Colours::white); g.fillRoundedRectangle(74 + lL * 30, 214, 26, 12, 5); g.fillRoundedRectangle(100 + lR * 30, 214, 26, 12, 5);
  g.setColour(female ? juce::Colour(0xffd6408f) : juce::Colour(0xff6a2fa8)); g.fillRoundedRectangle(74, 80, 52, 76, 14);
  g.setColour(gold); juce::Path chain; chain.startNewSubPath(84, 84); chain.quadraticTo(100, 112, 116, 84); g.strokePath(chain, juce::PathStrokeType(3)); g.fillEllipse(96, 102, 8, 8);
  g.setFont(11.0f); g.drawText("AURA", 74, 116, 52, 14, juce::Justification::centred);
  limb(77, 88, 13, 58, aL, skin); limb(123, 88, 13, 58, aR, skin);
  g.setColour(skin); g.fillRect(94.0f, 70.0f, 12.0f, 14.0f); g.fillEllipse(80, 36, 40, 40);
  if (female) { g.setColour(juce::Colour(0xff2a1a12)); g.fillRoundedRectangle(76, 50, 8, 44, 4); g.fillRoundedRectangle(116, 50, 8, 44, 4); }
  g.setColour(juce::Colour(0xff141414)); juce::Path cap; cap.addPieSegment(79, 30, 42, 40, -juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi, 0); g.fillPath(cap);
  g.fillRoundedRectangle(96, 44, 38, 7, 3); g.setColour(juce::Colours::black); g.fillRoundedRectangle(86, 52, 28, 7, 3);

}
