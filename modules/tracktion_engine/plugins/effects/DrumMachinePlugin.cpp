/////////////////////////////////////////////////////////////////////////////////////////////
//     ____             _    ____                            _                             //
//    | __ )  ___  __ _| |_ / ___|___  _ __  _ __   ___  ___| |_                           //
//    |  _ \ / _ \/ _` | __| |   / _ \| '_ \| '_ \ / _ \/ __| __|     Copyright 2023       //
//    | |_) |  __/ (_| | |_| |__| (_) | | | | | | |  __/ (__| |_    www.beatconnect.com    //
//    |____/ \___|\__,_|\__|\____\___/|_| |_|_| |_|\___|\___|\__|                          //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

namespace tracktion { inline namespace engine
{
DrumMachinePlugin::DrumMachinePlugin(PluginCreationInfo pluginCreationInfo) : SamplerPlugin(pluginCreationInfo) 
{
    // Effects: Distortion
    for (int i = 0; i < drumMachineTracks; i++)
        distortionOnValues[i] = false;

    for (int i = 0; i < drumMachineTracks; i++)
        distortionValues[i] = 0;
}

DrumMachinePlugin::~DrumMachinePlugin() 
{
    // Effects: Distortion
    for (auto element : distortionPointers)
        if (element)
            element->detachFromCurrentValue();
}

void DrumMachinePlugin::applyEffects(juce::AudioBuffer<float>& buffer, const int channelIndex)
{
    int numSamples = buffer.getNumSamples();

    // Apply Distortion
    if (distortionOnValues[channelIndex])
    {
        float drive = paramValue(distortionPointers[channelIndex]);
        float clip = 1.0f / (2.0f * drive);
        Distortion::distortion(buffer.getWritePointer(0), numSamples, drive, -clip, clip);
        Distortion::distortion(buffer.getWritePointer(1), numSamples, drive, -clip, clip);
    }
}

void DrumMachinePlugin::applyToBuffer(const PluginRenderContext& pluginRenderContext)
{
    SamplerPlugin::applyToBuffer(pluginRenderContext);
    // Get the buffers for each SamplerSound to apply the effects.
}

float DrumMachinePlugin::paramValue(AutomatableParameter::Ptr param)
{
    jassert(param != nullptr);
    if (param == nullptr)
    {
        return 0.0f;
    } else {
        return param->getCurrentValue(); // =8> Temporary debug code.
    }

    // Do the smoothers! =8>
    //auto smoothItr = smoothers.find(param.get());
    //if (smoothItr == smoothers.end())
    //    return param->getCurrentValue();

    //float val = param->getCurrentNormalisedValue();
    //smoothItr->second.setValue(val);
    //return param->valueRange.convertFrom0to1(smoothItr->second.getCurrentValue());
}

const int DrumMachinePlugin::pitchWheelSemitoneRange = 25;
const char* DrumMachinePlugin::uniqueId = "adf30650-4fd8-4cce-933d-fa8aa598c6c9";
const char* DrumMachinePlugin::xmlTypeName = "drum machine";
}} // namespace tracktion { inline namespace engine
