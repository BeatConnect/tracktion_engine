/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

#pragma once

#include "common/Utilities.h"
#include "common/Components.h"

//==============================================================================
class RecordingDemo  : public Component,
                       private ChangeListener,
                       private ValueTree::Listener,
                       private InterprocessConnection
{
public:
    //==============================================================================
    RecordingDemo (te::Engine& e)
        : engine (e)
    {
        newEditButton.onClick = [this] { createOrLoadEdit(); };
        
        updatePlayButtonText();
        updateRecordButtonText();
        editNameLabel.setJustificationType (Justification::centred);
        Helpers::addAndMakeVisible (*this, { &newEditButton, &playPauseButton, &recordButton, &showEditButton,
                                             &newTrackButton, &clearTracksButton, &deleteButton, &editNameLabel, &showWaveformButton, &undoButton, &redoButton });

        deleteButton.setEnabled (false);
        
        auto d = File::getSpecialLocation (File::tempDirectory).getChildFile ("RecordingDemo");
        d.createDirectory();
        
        auto f = Helpers::findRecentEdit (d);
        if (f.existsAsFile())
            createOrLoadEdit (f);
        else
            createOrLoadEdit (d.getNonexistentChildFile ("Test", ".tracktionedit", false));
        
        selectionManager.addChangeListener (this);

        setupButtons();

        tryToConnectToPipe();
        
        setSize (700, 500);
    }

    ~RecordingDemo() override
    {
        disconnect();
        te::EditFileOperations (*edit).save (true, true, false);
        engine.getTemporaryFileManager().getTempDirectory().deleteRecursively();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        int w = r.getWidth() / 9;
        auto topR = r.removeFromTop (30);
        newEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
        playPauseButton.setBounds (topR.removeFromLeft (w).reduced (2));
        recordButton.setBounds (topR.removeFromLeft (w).reduced (2));
        showEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
        newTrackButton.setBounds (topR.removeFromLeft (w).reduced (2));
        clearTracksButton.setBounds (topR.removeFromLeft (w).reduced (2));
        deleteButton.setBounds (topR.removeFromLeft (w).reduced (2));
        undoButton.setBounds(topR.removeFromLeft(w).reduced(2));
        redoButton.setBounds(topR.removeFromLeft(w).reduced(2));

        topR = r.removeFromTop (30);
        showWaveformButton.setBounds (topR.removeFromLeft (w * 2).reduced (2));
        editNameLabel.setBounds (topR);
        
        if (editComponent != nullptr)
            editComponent->setBounds (r);
    }

private:
    //==============================================================================
    te::Engine& engine;
    te::SelectionManager selectionManager { engine };
    std::unique_ptr<te::Edit> edit;
    std::unique_ptr<EditComponent> editComponent;

    TextButton newEditButton { "New" }, playPauseButton { "Play" }, recordButton { "Record" },
               showEditButton { "Show Edit" }, newTrackButton { "New Track" }, clearTracksButton { "Clear Tracks" }, deleteButton { "Delete" },
               undoButton {"Undo"}, redoButton {"Redo"};
    Label editNameLabel { "No Edit Loaded" };
    ToggleButton showWaveformButton { "Show Waveforms" };

    bool valueTreeChangeFromBackend = false;

    //==============================================================================
    void setupButtons()
    {
        playPauseButton.onClick = [this]
        {
            bool wasRecording = edit->getTransport().isRecording();
            EngineHelpers::togglePlay (*edit);
            if (wasRecording)
            {
                DBG(edit->state.toXmlString());
                te::EditFileOperations(*edit).save(true, true, false);
            }
               
        };
        recordButton.onClick = [this]
        {
            bool wasRecording = edit->getTransport().isRecording();
            EngineHelpers::toggleRecord (*edit);
            if (wasRecording)
            {
                te::EditFileOperations(*edit).save(true, true, false);
            }
                
        };
        newTrackButton.onClick = [this]
        {
            edit->ensureNumberOfAudioTracks (getAudioTracks (*edit).size() + 1);
        };
        clearTracksButton.onClick = [this]
        {
            for (auto t : te::getAudioTracks (*edit))
                edit->deleteTrack (t);
                
        };
        deleteButton.onClick = [this]
        {
            auto sel = selectionManager.getSelectedObject (0);
            if (auto clip = dynamic_cast<te::Clip*> (sel))
            {
                clip->removeFromParentTrack();
            }
            else if (auto track = dynamic_cast<te::Track*> (sel))
            {
                if (! (track->isMasterTrack() || track->isMarkerTrack() || track->isTempoTrack() || track->isChordTrack()))
                    edit->deleteTrack (track);
            }
        };
        showWaveformButton.onClick = [this]
        {
            auto& evs = editComponent->getEditViewState();
            evs.drawWaveforms = ! evs.drawWaveforms.get();
            showWaveformButton.setToggleState (evs.drawWaveforms, dontSendNotification);
        };
        undoButton.onClick = [this]
        {
            edit->getUndoManager().undo();
        };
        redoButton.onClick = [this]
        {
            edit->getUndoManager().redo();
        };
    }
    
    void updatePlayButtonText()
    {
        if (edit != nullptr)
            playPauseButton.setButtonText (edit->getTransport().isPlaying() ? "Stop" : "Play");
    }
    
    void updateRecordButtonText()
    {
        if (edit != nullptr)
            recordButton.setButtonText (edit->getTransport().isRecording() ? "Abort" : "Record");
    }

    void createOrLoadEdit (File editFile = {})
    {
        if (editFile == File())
        {
            FileChooser fc ("New Edit", File::getSpecialLocation (File::userDocumentsDirectory), "*.tracktionedit");
            if (fc.browseForFileToSave (true))
                editFile = fc.getResult();
            else
                return;
        }
        
        selectionManager.deselectAll();
        editComponent = nullptr;
        
        if (editFile.existsAsFile())
            edit = te::loadEditFromFile (engine, editFile);
        else
            edit = te::createEmptyEdit (engine, editFile);
        
        edit->playInStopEnabled = true;
        
        auto& transport = edit->getTransport();
        transport.addChangeListener (this);
        
        editNameLabel.setText (editFile.getFileNameWithoutExtension(), dontSendNotification);
        showEditButton.onClick = [this, editFile]
        {
            te::EditFileOperations (*edit).save (true, true, false);
            editFile.revealToUser();
        };
        
        createTracksAndAssignInputs();
        
        te::EditFileOperations (*edit).save (true, true, false);

        edit->state.addListener(this);
        
        editComponent = std::make_unique<EditComponent> (*edit, selectionManager);
        addAndMakeVisible (*editComponent);
        resized();
    }
    
    void createTracksAndAssignInputs()
    {
        auto& dm = engine.getDeviceManager();

        for (int i = 0; i < dm.getNumWaveInDevices(); i++)
            if (auto wip = dm.getWaveInDevice (i))
                wip->setStereoPair (false);
        
        for (int i = 0; i < dm.getNumWaveInDevices(); i++)
        {
            if (auto wip = dm.getWaveInDevice (i))
            {
                wip->setEndToEnd (true);
                wip->setEnabled (true);
            }
        }
        
        edit->getTransport().ensureContextAllocated();
        
        int trackNum = 0;
        for (auto instance : edit->getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
            {
                if (auto t = EngineHelpers::getOrInsertAudioTrackAt (*edit, trackNum))
                {
                    instance->setTargetTrack (*t, 0, true);
                    instance->setRecordingEnabled (*t, true);
                    
                    trackNum++;
                }
            }
        }
        
        edit->restartPlayback();
    }
    
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (edit != nullptr && source == &edit->getTransport())
        {
            updatePlayButtonText();
            updateRecordButtonText();
        }
        else if (source == &selectionManager)
        {
            auto sel = selectionManager.getSelectedObject (0);
            deleteButton.setEnabled (dynamic_cast<te::Clip*> (sel) != nullptr || dynamic_cast<te::Track*> (sel) != nullptr);
        }
    }

    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override
    {
    }

    void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override 
    {
        static juce::Identifier audioClipType("AUDIOCLIP");
        static juce::Identifier audioClipSource("source");
        static juce::Identifier newClipRecorded("NEWCLIPRECORDED");
        static juce::Identifier payloadName("PAYLOAD");
        static juce::Identifier clipAudioIdentifier("CLIPAUDIO");
        static juce::Identifier trackInfoIdentifier("trackInfo");

        if (childWhichHasBeenAdded.getType() == audioClipType)
        {
            // Prevent backend-to-valuetree change loop
            if (valueTreeChangeFromBackend)
            {
                valueTreeChangeFromBackend = false;
                return;
            }

            DBG(childWhichHasBeenAdded.toXmlString());

            //get raw audio file name
            String rawAudioFileName = childWhichHasBeenAdded.getProperty(audioClipSource);
            //get raw audio of clip 
            if (rawAudioFileName.isNotEmpty())
            {   
                //set the id of the clip to be a uuid
                File clipAudio(edit->filePathResolver(rawAudioFileName));
                MemoryBlock mb;
                clipAudio.loadFileAsData(mb);

                ValueTree newClipMessage(newClipRecorded);
                ValueTree clipVtCopy = childWhichHasBeenAdded.createCopy();
                newClipMessage.addChild(clipVtCopy, -1, nullptr);
                ValueTree trackInfoVt(trackInfoIdentifier);
                trackInfoVt.setProperty("TrackID", parentTree.getProperty("id"), nullptr);
                newClipMessage.addChild(trackInfoVt, -1, nullptr);
                ValueTree clipRawAudio(payloadName);
                clipRawAudio.setProperty(clipAudioIdentifier, mb, nullptr);
                newClipMessage.addChild(clipRawAudio, -1, nullptr);

                sendMessageOverPipe(newClipMessage);
            }
            
        }
        
    }

    //InterprocessConnection helper
    void tryToConnectToPipe()
    {
        if (!isConnected())
        {
            auto pipeAlreadyExists =
                connectToPipe("Test", -1);

            if (!pipeAlreadyExists)
                createPipe("Test", -1, true);
        }
    }

    void sendMessageOverPipe(ValueTree& messageToSend)
    {
        MemoryOutputStream os;
        messageToSend.writeToStream(os);
        sendMessage(os.getMemoryBlock());
    }

    // InterprocessConnection callbacks
    void connectionMade() override
    {
        DBG("Connection made");
    }

    void connectionLost() override
    {
        DBG("Connection lost");
    }

    void messageReceived(const MemoryBlock& message) override
    {
        DBG(edit->state.toXmlString());
        valueTreeChangeFromBackend = true;

        auto messageVt = ValueTree::readFromData(message.getData(), message.getSize());

        // handle audio file
        String rawAudioFileName = messageVt.getChildWithName("AUDIOCLIP").getProperty("source");
        File clipAudio(edit->filePathResolver(rawAudioFileName));
        auto audioClip = messageVt.getChildWithName("PAYLOAD").getProperty("CLIPAUDIO");
        clipAudio.create();
        clipAudio.replaceWithData(audioClip.getBinaryData()->getData(), audioClip.getBinaryData()->getSize());

        // handle edit valuetree
        auto trackInfoVt = messageVt.getChildWithName("trackInfo");
        auto clipInfoFromMessageVt = messageVt.getChildWithName("AUDIOCLIP").createCopy();
        // set te id to 0 so te can handle interal id increment
        clipInfoFromMessageVt.setProperty("id", 0, nullptr);
        auto editTrackToAddClip = edit->state.getChildWithProperty("id", trackInfoVt.getProperty("TrackID"));
        editTrackToAddClip.addChild(clipInfoFromMessageVt, -1, nullptr);
        te::EditFileOperations(*edit).save(true, true, false);

        DBG(edit->state.toXmlString());      
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingDemo)
};

//==============================================================================
static DemoTypeBase<RecordingDemo> recordingDemo ("Audio Recording");
