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

AuxReturnPlugin::AuxReturnPlugin (PluginCreationInfo info)  : Plugin (info)
{
    busNumber.referTo (state, IDs::busNum, getUndoManager());
}

AuxReturnPlugin::~AuxReturnPlugin()
{
    notifyListenersOfDeletion();
}

const char* AuxReturnPlugin::xmlTypeName = "auxreturn";
// BEATCONNECT MODIFICATION START
const char* AuxReturnPlugin::uniqueId = "1dd50556-5b26-4280-bf24-049cd94c174b";
// BEATCONNECT MODIFICATION END

juce::String AuxReturnPlugin::getName()
{
    auto nm = edit.getAuxBusName (busNumber);

    if (nm.isNotEmpty())
        return "R:" + nm;

    return TRANS("Aux Return") + " #" + juce::String (busNumber + 1);
}

juce::String AuxReturnPlugin::getShortName (int)
{
    auto nm = edit.getAuxBusName (busNumber);

    if (nm.isNotEmpty())
        return "R:" + nm;

    return "Ret:" + juce::String (busNumber + 1);
}

void AuxReturnPlugin::initialise (const PluginInitialisationInfo&)
{
}

void AuxReturnPlugin::deinitialise()
{
}

void AuxReturnPlugin::applyToBuffer (const PluginRenderContext&)
{
}

void AuxReturnPlugin::restorePluginStateFromValueTree (const juce::ValueTree& v)
{
    juce::CachedValue<int>* cvsInt[] = { &busNumber, nullptr };
    copyPropertiesToNullTerminatedCachedValues (v, cvsInt);

    for (auto p : getAutomatableParameters())
        p->updateFromAttachedValue();
}

}} // namespace tracktion { inline namespace engine
