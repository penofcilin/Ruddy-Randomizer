/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CFinalV2AudioProcessor::CFinalV2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
     ), apvts(*this, nullptr, "apvts", createParameters())
#endif
{
    apvts.state = juce::ValueTree("SavedParams");
}

CFinalV2AudioProcessor::~CFinalV2AudioProcessor()
{
}

//==============================================================================
const juce::String CFinalV2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CFinalV2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CFinalV2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CFinalV2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CFinalV2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CFinalV2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CFinalV2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void CFinalV2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CFinalV2AudioProcessor::getProgramName (int index)
{
    return {};
}

void CFinalV2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CFinalV2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    

    juce::dsp::ProcessSpec spec;


    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    gainModule.prepare(spec);
    reverbLeft.prepare(spec);
    reverbRight.prepare(spec);

    spec.numChannels = 2;
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    
    
}

void CFinalV2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CFinalV2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CFinalV2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //Getting our distortion slider value
    auto& distortionValue = *apvts.getRawParameterValue("DISTORTION");
    float distVal = distortionValue.load();

    //These next lines iterate through both channels, and every sample within that channel, effectively allowing us to manipulate each sample within the for loop.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int samples = 0; samples < buffer.getNumSamples(); ++samples)
        {
            //If the distortion value isn't 0, the value will be limited based on how high the distortion knob is turned up, which flattens out the signal, thus introducing distortion.
            if (distVal != 0)
            {
                if (distVal != 1.f)
                {
                    channelData[samples] = juce::jlimit(float(-(1 - distVal)), float(1 - distVal), channelData[samples]);
                }
                else
                {
                    channelData[samples] = juce::jlimit(float(-(1 - 0.99f)), float(1 - 0.99f), channelData[samples]);
                }
            }
        }
    }


    //This next part uses the JUCE dsp modules to do the processing for me.
    juce::dsp::AudioBlock<float> block(buffer);

    //Assigning our reverb parameters to the roomsize parameter in our apvts so they all go up at once
    //Except dry wet, because this is an important parameter
    reverbParams.roomSize = *apvts.getRawParameterValue("ROOMSIZE");
    reverbParams.damping = 1 - *apvts.getRawParameterValue("ROOMSIZE");
    reverbParams.wetLevel = *apvts.getRawParameterValue("DW");
    reverbParams.dryLevel = 1 - reverbParams.wetLevel;
    reverbParams.width = *apvts.getRawParameterValue("ROOMSIZE");
    reverbParams.freezeMode = *apvts.getRawParameterValue("FREEZE");

    //Setting the gain value within the DSP module to our gain parameter that we get from the slider
    gainModule.setGainLinear(*apvts.getRawParameterValue("GAIN"));


    auto& rawFilterParam = *apvts.getRawParameterValue("FILTER");

    auto filterParam = getHertz(rawFilterParam.load());


    filter.setCutoffFrequency(filterParam);
    filter.setResonance(1 / sqrt(2));


    //Setting parameters for the reverb module

    reverbLeft.setParameters(reverbParams);
    reverbRight.setParameters(reverbParams);

    //Getting some processblocks 
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    //I don't really know what these do but you need a context to process
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    juce::dsp::ProcessContextReplacing<float> regularContext(block);
    
    
    //Processing both channels with our reverb modules.
    reverbLeft.process(leftContext);
    reverbRight.process(rightContext);
    filter.process(regularContext);

   gainModule.process(regularContext);

    
}


//==============================================================================
bool CFinalV2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CFinalV2AudioProcessor::createEditor()
{
    return new CFinalV2AudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void CFinalV2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    std::unique_ptr<juce::XmlElement> xml (apvts.state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CFinalV2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlParams(getXmlFromBinary(data, sizeInBytes));
    if (xmlParams != nullptr)
    {
        if (xmlParams->hasTagName(apvts.state.getType()))
        {
            apvts.state = juce::ValueTree::fromXml(*xmlParams);
        }
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CFinalV2AudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CFinalV2AudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //Adding the gain parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("FILTER", "Filter", 0.f, 1.0f, 0.9f));


    //Adding distortion parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>("DISTORTION", "Distortion", 0.0f, 1.0f, 0.5f));

    //Adding the reverb parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("ROOMSIZE", "Room Size", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DAMPING", "Damping", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DW", "Dry Wet", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("WIDTH", "Width", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterBool>("FREEZE", "Freeze Mode", false));


    return layout;

}

void CFinalV2AudioProcessor::reset()
{
    filter.reset();
}

//first self-implemented algorithm :3 maps out a 0 to 1 value to its corresponding number within the range of 20-20,000
float CFinalV2AudioProcessor::getHertz(float s)
{
    //Float to return
    float t = 20 + (s * 19980);
    return t;
}

