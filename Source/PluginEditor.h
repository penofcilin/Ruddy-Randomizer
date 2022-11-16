/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CFinalV2AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CFinalV2AudioProcessorEditor (CFinalV2AudioProcessor&);
    ~CFinalV2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider reverbSlider;
    juce::Slider reverbWetSlider;
    juce::Slider gainSlider;
    juce::Slider distortionSlider;
    juce::Slider filterSlider;
   

    juce::Label reverbLabel;
    juce::Label reverbWetLabel;
    juce::Label gainLabel;
    juce::Label distortionLabel;

    juce::Label hertzLabelTemp;

    juce::ToggleButton reverbRandomize;
    juce::ToggleButton gainRandomize;
    juce::ToggleButton distortionRandomize;
    juce::ToggleButton filterRandomize;

    juce::TextButton randomizeButton;
    juce::TextButton resetButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttachment;
    

    void randomize();
    void resetSliders();
    void createSlider(juce::Slider& slider);
    void createSliderStylistic(juce::Slider& slider, juce::Slider::SliderStyle style);
    void createLabel(const juce::String& name, juce::Label& label);
    void createButton(const juce::String& text, juce::Button& button);
    void createToggleButton(const juce::String& text, juce::ToggleButton& button);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CFinalV2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CFinalV2AudioProcessorEditor)
};
