/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace tracktion { inline namespace engine
{

/** */
class TextPlugin   : public Plugin
{
public:
    TextPlugin (PluginCreationInfo);
    ~TextPlugin() override;

    static const char* getPluginName()                  { return NEEDS_TRANS("Text"); }
    static juce::ValueTree create();

    //==============================================================================
    static const char* xmlTypeName;
    // BEATCONNECT MODIFICATION START
    static const char* uniqueId;
    // BEATCONNECT MODIFICATION END

    bool canBeAddedToFolderTrack() override             { return true; }
    juce::String getName() override                     { return textTitle.get().isNotEmpty() ? textTitle : TRANS("Text Plugin"); }
    juce::String getPluginType() override               { return xmlTypeName; }
    // BEATCONNECT MODIFICATION START
    juce::String getUniqueId() override                 { return uniqueId; }
    // BEATCONNECT MODIFICATION END
    void initialise (const PluginInitialisationInfo&) override {}
    void deinitialise() override                        {}
    void applyToBuffer (const PluginRenderContext&) override {}
    int getNumOutputChannelsGivenInputs (int numInputChannels) override     { return numInputChannels; }
    bool producesAudioWhenNoAudioInput() override       { return false; }
    bool needsConstantBufferSize() override             { return false; }
    juce::String getSelectableDescription() override    { return TRANS("Text Plugin"); }

    juce::CachedValue<juce::String> textTitle, textBody;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextPlugin)
};

}} // namespace tracktion { inline namespace engine
