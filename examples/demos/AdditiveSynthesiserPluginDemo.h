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

// Include Slider Parameter binding classes and functions
#include "DistortionEffectDemo.h"

#include "../../../Source/Plugin/AdditiveSynthesiserPlugin/bc_AdditiveSynthesiserPlugin.h"
#include "../../../Source/Plugin/AdditiveSynthesiserPlugin/bc_AdditiveSynthesiserPlugin.cpp"

class AdditiveSynthesiserPluginDemo : public Component, private ChangeListener
{
public:
    AdditiveSynthesiserPluginDemo(Engine& p_engine) : engine(p_engine)
    {
        // Register our custom plugin with the engine so it can be found using PluginCache::createNewPlugin
        engine.getPluginManager().createBuiltInType<BeatConnect::AdditiveSynthesiserPlugin>();

        Helpers::addAndMakeVisible(*this, { &noiseSlider, &noiseLabel });

        //// Load demo audio file
        //oggTempFile = std::make_unique<TemporaryFile>(".ogg");
        //auto demoFile = oggTempFile->getFile();
        //demoFile.replaceWithData(PlaybackDemoAudio::guitar_loop_ogg, PlaybackDemoAudio::guitar_loop_oggSize);

        //// Creates clip. Loads clip from file f
        //// Creates track. Loads clip into track
        auto track = EngineHelpers::getOrInsertAudioTrackAt(edit, 0);
        jassert(track != nullptr);

        //// Add a new clip to this track
        //te::AudioFile audioFile(edit.engine, demoFile);

        //auto clip = track->insertWaveClip(demoFile.getFileNameWithoutExtension(), demoFile,
        //    { { {}, tracktion::TimePosition::fromSeconds(audioFile.getLength()) }, {} }, false);
        //jassert(clip != nullptr);

        // Creates new instance of AdditiveSynthesiserPlugin and inserts to track 1
        auto plugin = edit.getPluginCache().createNewPlugin(BeatConnect::AdditiveSynthesiserPlugin::xmlTypeName, {});
        track->pluginList.insertPlugin(plugin, 0, nullptr);

        //// Set the loop points to the start/end of the clip, enable looping and start playback
        //edit.getTransport().addChangeListener(this);
        //EngineHelpers::loopAroundClip(*clip);

        // Setup button callbacks
        playPauseButton.onClick = [this] { EngineHelpers::togglePlay(edit); };

        // Setup sliders and labels
        //auto noiseParam = plugin->getAutomatableParameterByID("noise");
        //bindSliderToParameter(noiseSlider, *noiseParam);
        //noiseSlider.setSkewFactorFromMidPoint(1.0);
        //noiseLabel.attachToComponent(&noiseSlider, true);
        //noiseLabel.setText("Noise", sendNotification);

        updatePlayButtonText();

        setSize(600, 400);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto topR = r.removeFromTop(30);
        playPauseButton.setBounds(topR.reduced(2));

        noiseSlider.setBounds(50, 50, 500, 50);
    }

private:
    te::Engine& engine;
    te::Edit edit{ Edit::Options { engine, te::createEmptyEdit(engine), ProjectItemID::createNewID(0) } };
    std::unique_ptr<TemporaryFile> oggTempFile;

    TextButton playPauseButton{ "Play" };

    Slider noiseSlider;
    Label noiseLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    void updatePlayButtonText()
    {
        playPauseButton.setButtonText(edit.getTransport().isPlaying() ? "Pause" : "Play");
    }

    void changeListenerCallback(ChangeBroadcaster*) override
    {
        updatePlayButtonText();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditiveSynthesiserPluginDemo)
};

static DemoTypeBase<AdditiveSynthesiserPluginDemo> AdditiveSynthesiserPluginDemo("AdditiveSynthesiserPlugin Demo");