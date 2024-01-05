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
    distortionOnValue.referTo(state, IDs::distortionOn, nullptr);
    distortionValue.referTo(state, IDs::distortion, nullptr, 0.5f);

    distortion = addParam("distortion", TRANS("Distortion"), { 0.0f, 1.0f });

    distortion->attachToCurrentValue(distortionValue);

    if (!state.hasProperty(IDs::distortionOn))
        state.setProperty(IDs::distortionOn, "0", nullptr);
}

DrumMachinePlugin::~DrumMachinePlugin() 
{
    // Effects: Distortion
    distortion->detachFromCurrentValue();
}

void DrumMachinePlugin::applyToBuffer(const PluginRenderContext& pluginRenderContext)
{
    SamplerPlugin::applyToBuffer(pluginRenderContext);
}

const int DrumMachinePlugin::pitchWheelSemitoneRange = 25;
const char* DrumMachinePlugin::uniqueId = "adf30650-4fd8-4cce-933d-fa8aa598c6c9";
const char* DrumMachinePlugin::xmlTypeName = "drum machine";
}} // namespace tracktion { inline namespace engine
