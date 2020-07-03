/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once


namespace tracktion_engine
{

ModifierNode::ModifierNode (std::unique_ptr<Node> inputNode,
                            tracktion_engine::Modifier::Ptr modifierToProcess,
                            double sampleRateToUse, int blockSizeToUse,
                            const TrackMuteState* trackMuteStateToUse,
                            tracktion_graph::PlayHeadState& playHeadStateToUse, bool rendering)
    : input (std::move (inputNode)),
      modifier (std::move (modifierToProcess)),
      trackMuteState (trackMuteStateToUse),
      playHeadState (playHeadStateToUse),
      isRendering (rendering)
{
    jassert (input != nullptr);
    jassert (modifier != nullptr);
    initialiseModifier (sampleRateToUse, blockSizeToUse);
}

ModifierNode::~ModifierNode()
{
    if (isInitialised && ! modifier->baseClassNeedsInitialising())
        modifier->baseClassDeinitialise();
}

//==============================================================================
tracktion_graph::NodeProperties ModifierNode::getNodeProperties()
{
    auto props = input->getNodeProperties();

    props.numberOfChannels = juce::jmax (props.numberOfChannels, modifier->getAudioInputNames().size());
    props.hasAudio = props.hasAudio || modifier->getAudioInputNames().size() > 0;
    props.hasMidi  = props.hasMidi || modifier->getMidiInputNames().size() > 0;
    props.nodeID = (size_t) modifier->itemID.getRawID();

    return props;
}

void ModifierNode::prepareToPlay (const tracktion_graph::PlaybackInitialisationInfo& info)
{
    juce::ignoreUnused (info);
    jassert (sampleRate == info.sampleRate);
}

void ModifierNode::process (const ProcessContext& pc)
{
    auto inputBuffers = input->getProcessedOutput();
    auto& inputAudioBlock = inputBuffers.audio;
    
    auto& outputBuffers = pc.buffers;
    auto& outputAudioBlock = outputBuffers.audio;
    jassert (inputAudioBlock.getNumSamples() == outputAudioBlock.getNumSamples());

    // Copy the inputs to the outputs, then process using the
    // output buffers as that will be the correct size
    {
        const size_t numInputChannelsToCopy = std::min (inputAudioBlock.getNumChannels(), outputAudioBlock.getNumChannels());
        
        if (numInputChannelsToCopy > 0)
        {
            jassert (inputAudioBlock.getNumSamples() == outputAudioBlock.getNumSamples());
            outputAudioBlock.copyFrom (inputAudioBlock.getSubsetChannelBlock (0, numInputChannelsToCopy));
        }
    }

    // Setup audio buffers
    auto outputAudioBuffer = tracktion_graph::test_utilities::createAudioBuffer (outputAudioBlock);

    // Then MIDI buffers
    midiMessageArray.copyFrom (inputBuffers.midi);
    bool shouldProcess = getBoolParamValue (*modifier->enabledParam);
    
    if (playHeadState.didPlayheadJump())
        midiMessageArray.isAllNotesOff = true;
    
    if (trackMuteState != nullptr)
    {
        if (! trackMuteState->shouldTrackContentsBeProcessed())
        {
            shouldProcess = shouldProcess && trackMuteState->shouldTrackBeAudible();
        
            if (trackMuteState->wasJustMuted())
                midiMessageArray.isAllNotesOff = true;
        }
    }

    // Process the plugin
    if (shouldProcess)
        modifier->applyToBuffer (getPluginRenderContext (pc.referenceSampleRange.getStart(), outputAudioBuffer));
    
    // Then copy the buffers to the outputs
    outputBuffers.midi.copyFrom (midiMessageArray);
}

//==============================================================================
void ModifierNode::initialiseModifier (double sampleRateToUse, int blockSizeToUse)
{
    sampleRate = sampleRateToUse;
    modifier->baseClassInitialise (sampleRate, blockSizeToUse);
    isInitialised = true;
}

PluginRenderContext ModifierNode::getPluginRenderContext (int64_t referenceSamplePosition, juce::AudioBuffer<float>& destBuffer)
{
    auto& playHead = playHeadState.playHead;
    
    return { &destBuffer,
             juce::AudioChannelSet::canonicalChannelSet (destBuffer.getNumChannels()),
             0, destBuffer.getNumSamples(),
             &midiMessageArray, 0.0,
             tracktion_graph::sampleToTime (playHead.referenceSamplePositionToTimelinePosition (referenceSamplePosition), sampleRate),
             playHead.isPlaying(), playHead.isUserDragging(), isRendering };
}

}