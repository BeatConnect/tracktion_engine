/////////////////////////////////////////////////////////////////////////////////////////////
//     ____             _    ____                            _                             //
//    | __ )  ___  __ _| |_ / ___|___  _ __  _ __   ___  ___| |_                           //
//    |  _ \ / _ \/ _` | __| |   / _ \| '_ \| '_ \ / _ \/ __| __|     Copyright 2023       //
//    | |_) |  __/ (_| | |_| |__| (_) | | | | | | |  __/ (__| |_    www.beatconnect.com    //
//    |____/ \___|\__,_|\__|\____\___/|_| |_|_| |_|\___|\___|\__|                          //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

namespace tracktion { inline namespace engine
{
// A Drum Machine is a specialised Sampler.
// Whenever general sampler operations are required please defer to the base class SamplerPlugin
struct DrumMachinePlugin : public SamplerPlugin {
public:
	DrumMachinePlugin(PluginCreationInfo pluginCreationInfo);
	~DrumMachinePlugin();

	static const int pitchWheelSemitoneRange;
	static const char* uniqueId;
	static const char* xmlTypeName;

	static const char* getPluginName() { return NEEDS_TRANS("Drum Machine"); }

	juce::String getName() override { return NEEDS_TRANS("Drum Machine"); }
	juce::String getPluginType() override { return xmlTypeName; }
	juce::String getShortName(int) override { return "DrmMchn"; }
	juce::String getSelectableDescription() override { return TRANS("Drum Machine"); }
	juce::String getUniqueId() override { return uniqueId; }
	virtual juce::String getVendor() override { return "BeatConnect"; }

	static const int drumMachineTracks = 16;
	juce::CachedValue<bool> distortionOnValues[drumMachineTracks];
	juce::CachedValue<float> distortionValues[drumMachineTracks];
	AutomatableParameter::Ptr distortionPointers[drumMachineTracks];
private:
	std::unordered_map<AutomatableParameter*, ValueSmoother<float>> smoothers;

	void applyEffects(juce::AudioBuffer<float>& buffer, const int channelIndex);
	void applyToBuffer(const PluginRenderContext& pluginRenderContext);
	float paramValue(AutomatableParameter::Ptr param);
};

}} // namespace tracktion { inline namespace engine
