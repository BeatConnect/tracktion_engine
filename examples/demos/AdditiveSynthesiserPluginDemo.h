/////////////////////////////////////////////////////////////////////////////////////////////////////////
//      ____             __  ______                            __ ___   ____ ___  __ __                //
//     / __ )___  ____ _/ /_/ ____/___  ____  ____  ___  _____/ /|__ \ / __ \__ \/ // /  www.          //
//    / __  / _ \/ __ `/ __/ /   / __ \/ __ \/ __ \/ _ \/ ___/ __/_/ // / / /_/ / // /_   BeatConnect  //
//   / /_/ /  __/ /_/ / /_/ /___/ /_/ / / / / / / /  __/ /__/ /_/ __// /_/ / __/__  __/    .com        //
//  /_____/\___/\__,_/\__/\____/\____/_/ /_/_/ /_/\___/\___/\__/____/\____/____/ /_/         (C)2024   //
//																									   //
/////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                   

#pragma once

#include <JuceHeader.h>

#include "../common/Utilities.h"
#include "../common/Components.h"

using namespace tracktion_engine;

// Include Slider Parameter binding classes and functions
#include "DistortionEffectDemo.h"

#include "../../../Source/Plugin/AdditiveSynthesiserPlugin/bc_AdditiveSynthesiserPlugin.h"

class AdditiveSynthesiserPluginDemo : public Component
{
public:
    AdditiveSynthesiserPluginDemo(Engine& p_engine) : engine(p_engine)
    {
        auto track = EngineHelpers::getOrInsertAudioTrackAt(edit, 0);
        jassert(track != nullptr);
        track->state.setProperty(te::IDs::type, "midi", nullptr);

        // Register our custom plugin with the engine so it can be found using PluginCache::createNewPlugin
        engine.getPluginManager().createBuiltInType<BeatConnect::AdditiveSynthesiserPlugin>();

        // All to do with inputs
        auto& dm = engine.getDeviceManager();

        for (int i = 0; i < dm.getNumMidiInDevices(); i++)
        {
            if (auto mip = dm.getMidiInDevice(i))
            {
                mip->setEndToEndEnabled(true);
                mip->setEnabled(true);
            }
        }

        for (int i = 0; i < dm.getNumWaveInDevices(); i++)
        {
            if (auto wip = dm.getWaveInDevice(i))
                wip->setStereoPair(false);
        }

        for (int i = 0; i < dm.getNumWaveInDevices(); i++)
        {
            if (auto wip = dm.getWaveInDevice(i))
            {
                wip->setEndToEnd(true);
                wip->setEnabled(true);
            }
        }

        edit.getTransport().ensureContextAllocated();

        // Select the MPK mini 3 as an input.
        te::InputDeviceInstance* targetInput = nullptr;
        for (auto instance : edit.getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice ||
                instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice ||
                instance->getInputDevice().getDeviceType() == te::InputDevice::virtualMidiDevice)
            {
                if (instance->getInputDevice().getName() == "MPK mini 3")
                {
                    targetInput = instance;
                    break;
                }
            }
        }

        // Is the MPK mini 3 plugged in?
        if (targetInput != nullptr)
        {
            targetInput->setTargetTrack(*track, 0, true);
            if(!targetInput->getInputDevice().isEndToEndEnabled())
                targetInput->getInputDevice().flipEndToEnd();

            // Creates new instance of AdditiveSynthesiserPlugin and inserts to track 1
            auto plugin = edit.getPluginCache().createNewPlugin(BeatConnect::AdditiveSynthesiserPlugin::xmlTypeName, {});
            track->pluginList.insertPlugin(plugin, 0, nullptr);

            m_BtnNoise = std::make_unique<ToggleButton>();
            m_BtnPulse = std::make_unique<ToggleButton>();
            m_BtnSawtooth = std::make_unique<ToggleButton>();
            m_BtnSine = std::make_unique<ToggleButton>();
            m_BtnTriangle = std::make_unique<ToggleButton>();
            Helpers::addAndMakeVisible(*this, 
                { 
                    m_BtnNoise.get(),
                    m_BtnPulse.get(),
                    m_BtnSawtooth.get(),
                    m_BtnSine.get(),
                    m_BtnTriangle.get(),
                });

            int shift = 1;
            m_BtnNoise->setBounds(100 * shift, 5, 200 * shift, 20);
            m_BtnNoise->setButtonText("Noise");
            shift++;
            m_BtnPulse->setBounds(100 * shift, 5, 200 * shift, 20);
            m_BtnPulse->setButtonText("Pulse");
            shift++;
            m_BtnSawtooth->setBounds(100 * shift, 5, 200 * shift, 20);
            m_BtnSawtooth->setButtonText("Sawtooth");
            shift++;
            m_BtnSine->setBounds(100 * shift, 5, 200 * shift, 20);
            m_BtnSine->setButtonText("Sine");
            shift++;
            m_BtnTriangle->setBounds(100 * shift, 5, 200 * shift, 20);
            m_BtnTriangle->setButtonText("Triangle");

            // Create all the slider that control the plugin parameters.
            ValueTree paramsNode = plugin->state.getChildWithName("PluginParameters");
            assert(paramsNode.isValid());

            auto voiceParam = plugin->getAutomatableParameterByID("voiceTypeNoise");
            m_BtnNoise->getToggleStateValue().referTo(juce::Value(new ParameterValueSource(voiceParam)));
            voiceParam = plugin->getAutomatableParameterByID("voiceTypePulse");
            m_BtnPulse->getToggleStateValue().referTo(juce::Value(new ParameterValueSource(voiceParam)));
            voiceParam = plugin->getAutomatableParameterByID("voiceTypeSawtooth");
            m_BtnSawtooth->getToggleStateValue().referTo(juce::Value(new ParameterValueSource(voiceParam)));
            voiceParam = plugin->getAutomatableParameterByID("voiceTypeSine");
            m_BtnSine->getToggleStateValue().referTo(juce::Value(new ParameterValueSource(voiceParam)));
            voiceParam = plugin->getAutomatableParameterByID("voiceTypeTriangle");
            m_BtnTriangle->getToggleStateValue().referTo(juce::Value(new ParameterValueSource(voiceParam)));

            for (auto param : paramsNode)
            {
                const String paramId = param.getProperty(te::IDs::paramId).toString();

                if (paramId == "voiceTypeNoise" ||
                    paramId == "voiceTypePulse" ||
                    paramId == "voiceTypeSawtooth" ||
                    paramId == "voiceTypeSine" ||
                    paramId == "voiceTypeTriangle")
                    continue;

                m_Sliders.push_back(std::make_unique<Slider>());
                m_Labels.push_back(std::make_unique<Label>());
                Helpers::addAndMakeVisible(*this, { m_Sliders.back().get(), m_Labels.back().get() });
                auto noiseParam = plugin->getAutomatableParameterByID(paramId);
                bindSliderToParameter(*m_Sliders.back().get(), *noiseParam);
                m_Labels.back()->attachToComponent(m_Sliders.back().get(), true);
                m_Labels.back()->setText(paramId, sendNotification);
            }
        }

        setSize(600, 400);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        int i = 1;
        for (auto& slider : m_Sliders)
        {
            slider->setBounds(100, 30 * i, 500, 30);
            i++;
        }
    }

private:

    te::Engine& engine;
    te::Edit edit{ Edit::Options { engine, te::createEmptyEdit(engine), ProjectItemID::createNewID(0) } };

    std::unique_ptr<ToggleButton> m_BtnNoise;
    std::unique_ptr<ToggleButton> m_BtnPulse;
    std::unique_ptr<ToggleButton> m_BtnSawtooth;
    std::unique_ptr<ToggleButton> m_BtnSine;
    std::unique_ptr<ToggleButton> m_BtnTriangle;
    std::vector<std::unique_ptr<Slider>> m_Sliders;
    std::vector<std::unique_ptr<Label>> m_Labels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesiserPluginDemo)
};

static DemoTypeBase<AdditiveSynthesiserPluginDemo> AdditiveSynthesiserPluginDemo("AdditiveSynthesiserPlugin Demo");