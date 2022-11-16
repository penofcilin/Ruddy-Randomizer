/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CFinalV2AudioProcessorEditor::CFinalV2AudioProcessorEditor(CFinalV2AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
    //
{
    //Create reverb slider
    createSlider(reverbSlider);

    //Creating the wet slider for the reverb
    createSlider(reverbWetSlider);

    //create gainSlider
    createSlider(gainSlider);

    //Create distortion slider
    createSlider(distortionSlider);

    //Create filter slider
    createSliderStylistic(filterSlider, juce::Slider::SliderStyle::LinearVertical);

    //Create reverb label
    createLabel("Reverb", reverbLabel);

    //Creating reverbWetLabel
    createLabel("Wet", reverbWetLabel);

    //Create gain label
    createLabel("Gain", gainLabel);

    //Creating distortion label
    createLabel("Dist", distortionLabel);



    createLabel("Hertz", hertzLabelTemp);

    filterSlider.onValueChange = [&]() {
        juce::String newText = std::to_string((int)audioProcessor.getHertz(filterSlider.getValue()));
            hertzLabelTemp.setText(("Filter(Hz) \n" + newText), juce::dontSendNotification);
    };

    //Creating the reverb randomize button
    createToggleButton("Don't Randomize", reverbRandomize);

    //Creating the gainrandomizer
    createToggleButton("Don't Randomize", gainRandomize);

    //Creating distortion randomize button
    createToggleButton("Don't Randomize", distortionRandomize);

    createToggleButton("Don't Randomize", filterRandomize);

    //create randomizer button
    createButton("RANDOMIZE!", randomizeButton);
    //Attach button to function
    randomizeButton.onClick = [&]() {randomize(); };

    //Creating reset button
    createButton("Reset", resetButton);
    resetButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkorange);
    resetButton.onClick = [&]() {resetSliders(); };

    //Create the attachments for the sliders
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "ROOMSIZE", reverbSlider);
    reverbWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DW", reverbWetSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);
    distortionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DISTORTION", distortionSlider);
    filterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "FILTER", filterSlider);




    setSize (300, 300);
}

void CFinalV2AudioProcessorEditor::createButton(const juce::String& text, juce::Button& button)
{
    button.setClickingTogglesState(false);
    button.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    button.setButtonText(text);
    addAndMakeVisible(button);
}

void CFinalV2AudioProcessorEditor::createToggleButton(const juce::String& text, juce::ToggleButton& button)
{
    button.setButtonText(text);
    button.setClickingTogglesState(true);
    button.setColour(juce::ToggleButton::ColourIds::textColourId, juce::Colours::black);
    button.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    button.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::black);
    addAndMakeVisible(button);
}

void CFinalV2AudioProcessorEditor::createSliderStylistic(juce::Slider& slider, juce::Slider::SliderStyle style)
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    slider.setRange(0.f, 1.f, 0.01f);
   slider.setValue(0.9f);
   addAndMakeVisible(slider);
}

void CFinalV2AudioProcessorEditor::createSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setDoubleClickReturnValue(true, 0.5);
    slider.setRange(0.f, 1.f, 0.01f);
    slider.setValue(0.5);
    addAndMakeVisible(slider);
}

void CFinalV2AudioProcessorEditor::createLabel(const juce::String& name, juce::Label& label)
{
    label.setText(name, juce::NotificationType::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(label);
}

void CFinalV2AudioProcessorEditor::randomize()
{
    
    juce::Random random;

    if (!reverbRandomize.getToggleState())
    {
        reverbSlider.setValue(random.nextFloat());
    }

    if (!gainRandomize.getToggleState())
    {
        gainSlider.setValue(random.nextFloat());
    }

    if (!distortionRandomize.getToggleState())
    {
        distortionSlider.setValue(random.nextFloat());
    }

    if (!filterRandomize.getToggleState())
    {
        filterSlider.setValue(random.nextFloat());
    }

    
}

void CFinalV2AudioProcessorEditor::resetSliders()
{
    gainSlider.setValue(0.5f);
    reverbSlider.setValue(0.5f);
    reverbWetSlider.setValue(0.5f);
    distortionSlider.setValue(0.5f);
    filterSlider.setValue(18000);

    reverbRandomize.setToggleState(false, false);
    gainRandomize.setToggleState(false, false);
    distortionRandomize.setToggleState(false, false);
    filterRandomize.setToggleState(false, false);
}

CFinalV2AudioProcessorEditor::~CFinalV2AudioProcessorEditor()
{
}

//==============================================================================
void CFinalV2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::seashell);
    getTopLevelComponent()->setName("Ruddy Randomizer");

}

void CFinalV2AudioProcessorEditor::resized()
{
    reverbSlider.setBounds(0, 0, 100, 100);
    reverbWetSlider.setBounds(69, 67, 50, 50);
    gainSlider.setBounds(100, 0, 100, 100);
    distortionSlider.setBounds(200, 0, 100, 100);
    filterSlider.setBounds(210, 150, 35, 100);

    reverbLabel.setBounds(25, 75, 50, 50);
    reverbWetLabel.setBounds(77, 94, 35, 35);
    gainLabel.setBounds(128, 75, 50, 50);
    distortionLabel.setBounds(229, 75, 50, 50);

    hertzLabelTemp.setBounds(235, 175, 75, 50);

    reverbRandomize.setBounds(25, 115, 75, 25);
    gainRandomize.setBounds(125, 115, 75, 25);
    distortionRandomize.setBounds(225, 115, 75, 25);
    filterRandomize.setBounds(215, 245, 75, 25);

    randomizeButton.setBounds(100, 150, 100, 100);
    resetButton.setBounds(125, 260, 50, 30);
}
