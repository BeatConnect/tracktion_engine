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

StepClip::Pattern::Pattern (StepClip& c, const juce::ValueTree& v) noexcept
    : clip (c), state (v)
{
}

StepClip::Pattern::Pattern (const Pattern& other) noexcept
    : clip (other.clip), state (other.state)
{
}

juce::String StepClip::Pattern::getName() const                 { return state[IDs::name]; }
void StepClip::Pattern::setName (const juce::String& name)      { state.setProperty (IDs::name, name, clip.getUndoManager()); }

int StepClip::Pattern::getNumNotes() const                      { return state[IDs::numNotes]; }
void StepClip::Pattern::setNumNotes (int n)                     { state.setProperty (IDs::numNotes, n, clip.getUndoManager()); }

BeatDuration StepClip::Pattern::getNoteLength() const           { return BeatDuration::fromBeats (static_cast<double> (state[IDs::noteLength])); }
void StepClip::Pattern::setNoteLength (BeatDuration n)          { state.setProperty (IDs::noteLength, n.inBeats(), clip.getUndoManager()); }

juce::BigInteger StepClip::Pattern::getChannel (int channel) const
{
    juce::BigInteger b;
    b.parseString (state.getChild (channel)[IDs::pattern].toString(), 2);
    return b;
}

void StepClip::Pattern::setChannel (int channel, const juce::BigInteger& c)
{
    while (channel >= state.getNumChildren())
    {
        if (c.isZero())
            return;

        insertChannel (-1);
    }

    state.getChild (channel).setProperty (IDs::pattern, c.toString (2), clip.getUndoManager());
}

juce::Array<int> StepClip::Pattern::getVelocities (int channel) const
{
    juce::Array<int> v;
    auto sa = juce::StringArray::fromTokens (state.getChild (channel)[IDs::velocities].toString(), false);
    v.ensureStorageAllocated (sa.size());

    for (auto& s : sa)
        v.add (s.getIntValue());

    return v;
}

void StepClip::Pattern::setVelocities (int channel, const juce::Array<int>& va)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    juce::StringArray sa;
    sa.ensureStorageAllocated (va.size());

    for (int v : va)
        sa.add (juce::String (v));

    state.getChild (channel).setProperty (IDs::velocities, sa.joinIntoString (" "), clip.getUndoManager());
}

static double parseFraction (const juce::String& s)
{
    const int splitIndex = s.indexOfChar ('/');
    const double num    = s.substring (0, splitIndex).getDoubleValue();
    const double denom  = s.substring (splitIndex + 1, s.length()).getDoubleValue();

    if (denom == 0.0)
        return 0.0;

    return num / denom;
}

juce::Array<double> StepClip::Pattern::getGates (int channel) const
{
    juce::Array<double> v;
    auto sa = juce::StringArray::fromTokens (state.getChild (channel)[IDs::gates].toString(), false);
    v.ensureStorageAllocated (sa.size());

    for (auto& s : sa)
    {
        if (s.containsChar ('/'))
            v.add (parseFraction (s));
        else
            v.add (s.getDoubleValue());
    }

    return v;
}

void StepClip::Pattern::setGates (int channel, const juce::Array<double>& ga)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    juce::StringArray sa;
    sa.ensureStorageAllocated (ga.size());

    for (double g : ga)
        sa.add (juce::String (g));

    state.getChild (channel).setProperty (IDs::gates, sa.joinIntoString (" "), clip.getUndoManager());
}

juce::Array<float> StepClip::Pattern::getProbabilities (int channel) const
{
    juce::Array<float> v;
    auto pa = juce::StringArray::fromTokens (state.getChild (channel)[IDs::probabilities].toString(), false);
    v.ensureStorageAllocated (pa.size());

    for (auto& p : pa)
        v.add (p.getFloatValue());

    return v;
}

void StepClip::Pattern::setProbabilities (int channel, const juce::Array<float>& pa)
{
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    juce::StringArray sa;
    sa.ensureStorageAllocated (pa.size());

    for (double p : pa)
        sa.add (juce::String (p));

    state.getChild (channel).setProperty (IDs::probabilities, sa.joinIntoString (" "), clip.getUndoManager());
}

bool StepClip::Pattern::getNote (int channel, int index) const noexcept
{
    return getChannel (channel) [index];
}

void StepClip::Pattern::setNote (int channel, int index, bool value)
{
    if (getNote (channel, index) != value
          && juce::isPositiveAndBelow (index, getNumNotes())
          && juce::isPositiveAndBelow (channel, (int) maxNumChannels))
    {
        juce::BigInteger b (getChannel (channel));
        b.setBit (index, value);
        setChannel (channel, b);

        if (value)
        {
            if (getVelocity (channel, index) == 0)
                setVelocity (channel, index, defaultNoteValue);

            if (getGate (channel, index) == 0.0)
                setGate (channel, index, 1.0);

            if (getTremolo(channel, index) == 0)
                setTremolo(channel, index, defaultTremoloAttacks);
        }
    }
}

// BEATCONNECT MODIFICATION START
int StepClip::Pattern::getKeyNoteOffset(int channel, int index) const {
    if (!getNote(channel, index))
        return noKeyNoteOffset;

    auto keyNoteOffsets = getKeyNoteOffsets(channel);

    if (juce::isPositiveAndBelow(index, keyNoteOffsets.size()))
        return keyNoteOffsets[index];

    if (clip.getChannels()[channel] != nullptr)
        return defaultKeyNoteOffset;

    return defaultKeyNoteOffset;
}

juce::Array<int> StepClip::Pattern::getKeyNoteOffsets(int channel) const {
    juce::Array<int> keyNoteOffSets;
    auto stringArray = juce::StringArray::fromTokens(state.getChild(channel)[IDs::keyNoteOffsets].toString(), false);
    keyNoteOffSets.ensureStorageAllocated(stringArray.size());

    for (auto element : stringArray)
        keyNoteOffSets.add(element.getIntValue());

    return keyNoteOffSets;
}

int StepClip::Pattern::getTremolo(int channel, int index) const {
    if (!getNote (channel, index))
        return errorTremoloAttacks;

    auto tremolos = getTremolos (channel);

    if (juce::isPositiveAndBelow(index, tremolos.size()))
        return tremolos[index];

    if (clip.getChannels()[channel] != nullptr)
        return defaultTremoloAttacks;

    return errorTremoloAttacks;
}

juce::Array<int> StepClip::Pattern::getTremolos(int channel) const {
    juce::Array<int> tremolos;
    auto stringArray = juce::StringArray::fromTokens(state.getChild(channel)[IDs::tremolos].toString(), false);
    tremolos.ensureStorageAllocated(stringArray.size());

    for (auto element : stringArray)
        tremolos.add(element.getIntValue());

    return tremolos;
}

void StepClip::Pattern::setKeyNoteOffset(int channel, int index, int value) {
    if (!juce::isPositiveAndBelow(channel, (int)maxNumChannels))
        return; // Out of range

    setNote(channel, index, true);

    auto keyNoteOffsets = getKeyNoteOffsets(channel);
    const int size = keyNoteOffsets.size();

    if (!juce::isPositiveAndNotGreaterThan(index, size))
        keyNoteOffsets.insertMultiple(size, defaultKeyNoteOffset, index - size);

    keyNoteOffsets.set(index, value);
    setKeyNoteOffsets(channel, keyNoteOffsets);

    return;
}

void StepClip::Pattern::setKeyNoteOffsets(int channel, const juce::Array<int>& values) {
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    juce::StringArray stringArray;
    stringArray.ensureStorageAllocated(values.size());

    for (int element : values)
        stringArray.add(juce::String(element));

    state.getChild(channel).setProperty(IDs::keyNoteOffsets, stringArray.joinIntoString(" "), clip.getUndoManager());
}

void StepClip::Pattern::setTremolo(int channel, int index, int value) {
    if (!juce::isPositiveAndBelow(channel, (int)maxNumChannels))
        return; // Out of range

    setNote(channel, index, true);

    auto tremolos = getTremolos(channel);
    const int size = tremolos.size();

    if (!juce::isPositiveAndNotGreaterThan(index, size))
        tremolos.insertMultiple(size, defaultTremoloAttacks, index - size);

    tremolos.set(index, value);
    setTremolos(channel, tremolos);

    return;
}

void StepClip::Pattern::setTremolos(int channel, const juce::Array<int>& values) {
    if (channel >= state.getNumChildren())
    {
        jassertfalse;
        return;
    }

    juce::StringArray stringArray;
    stringArray.ensureStorageAllocated(values.size());

    for (int element : values)
        stringArray.add(juce::String(element));

    state.getChild(channel).setProperty(IDs::tremolos, stringArray.joinIntoString(" "), clip.getUndoManager());
}
// BEATCONNECT MODIFICATION END

int StepClip::Pattern::getVelocity (int channel, int index) const
{
    if (! getNote (channel, index))
        return -1;

    auto velocities = getVelocities (channel);

    if (juce::isPositiveAndBelow (index, velocities.size()))
        return velocities[index];

    if (clip.getChannels()[channel] != nullptr)
        return 127;

    return -1;
}

void StepClip::Pattern::setVelocity (int channel, int index, int value)
{
    if (! juce::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, true);

    auto velocities = getVelocities (channel);
    const int size = velocities.size();

    if (! juce::isPositiveAndNotGreaterThan (index, size))
        velocities.insertMultiple (size, 127, index - size);

    velocities.set (index, value);
    setVelocities (channel, velocities);
}

double StepClip::Pattern::getGate (int channel, int index) const
{
    if (! getNote (channel, index))
        return 0.0;

    auto gates = getGates (channel);

    if (juce::isPositiveAndBelow (index, gates.size()))
        return gates[index];

    return 1.0;
}

float StepClip::Pattern::getProbability (int channel, int index) const
{
    if (! getNote (channel, index))
        return 1.0;

    auto p = getProbabilities (channel);

    if (juce::isPositiveAndBelow (index, p.size()))
        return p[index];

    return 1.0;
}

void StepClip::Pattern::setGate (int channel, int index, double value)
{
    if (! juce::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, value != 0.0);

    auto gates = getGates (channel);
    const int size = gates.size();

    if (! juce::isPositiveAndNotGreaterThan (index, size))
        gates.insertMultiple (size, 1.0, index - size);

    gates.set (index, value);
    setGates (channel, gates);
}

void StepClip::Pattern::setProbability (int channel, int index, float value)
{
    if (! juce::isPositiveAndBelow (channel, (int) maxNumChannels))
        return;

    setNote (channel, index, value != 0.0);

    auto p = getProbabilities (channel);
    const int size = p.size();

    if (! juce::isPositiveAndNotGreaterThan (index, size))
        p.insertMultiple (size, 1.0f, index - size);

    p.set (index, value);
    setProbabilities (channel, p);
}

void StepClip::Pattern::clear()
{
    state.removeAllChildren (clip.getUndoManager());
}

void StepClip::Pattern::clearChannel (int channel)
{
    setChannel (channel, juce::BigInteger());
}

void StepClip::Pattern::insertChannel (int channel)
{
    // BEATCONNECT MODIFICATION START
    state.addChild (juce::ValueTree (IDs::PatternChannel), channel, clip.getUndoManager());
    // BEATCONNECT MODIFICATION END
}

void StepClip::Pattern::removeChannel (int channel)
{
    state.removeChild (channel, clip.getUndoManager());
}

void StepClip::Pattern::randomiseChannel (int channel)
{
    clearChannel (channel);

    juce::Random r;
    for (int i = 0; i < getNumNotes(); ++i)
        setNote (channel, i, r.nextBool());
}

void StepClip::Pattern::randomiseSteps()
{
    juce::Random r;
    const int numChannels = clip.getChannels().size();
    const int numSteps = getNumNotes();

    juce::Array<juce::BigInteger> chans;
    chans.insertMultiple (0, juce::BigInteger(), numChannels);

    for (int i = 0; i < numSteps; ++i)
        chans.getReference (r.nextInt (numChannels)).setBit (i);

    for (int i = 0; i < numChannels; ++i)
        setChannel (i, chans.getReference (i));
}

void StepClip::Pattern::shiftChannel (int channel, bool toTheRight)
{
    juce::BigInteger c (getChannel (channel));

    // NB: Notes are added in reverse order
    if (toTheRight)
        c <<= 1;
    else
        c >>= 1;

    setChannel (channel, c);
}

void StepClip::Pattern::toggleAtInterval (int channel, int interval)
{
    juce::BigInteger c = getChannel (channel);

    for (int i = 0; i < getNumNotes(); ++i)
        c.setBit (i, (i % interval) == 0);

    setChannel (channel, c);
}

StepClip::Pattern::CachedPattern::CachedPattern (const Pattern& p, int c)
    : notes (p.getChannel (c)),
      velocities (p.getVelocities (c)),
      gates (p.getGates (c)),
      // BEATCONNECT MODIFICATION START
      keyNoteOffsets (p.getKeyNoteOffsets(c)),
      // BEATCONNECT MODIFICATION END
      probabilities (p.getProbabilities (c))
      // BEATCONNECT MODIFICATION START
      , tremolos (p.getTremolos(c))
      // BEATCONNECT MODIFICATION END
{
}

bool StepClip::Pattern::CachedPattern::getNote (int index) const noexcept
{
    return notes[index];
}

int StepClip::Pattern::CachedPattern::getVelocity (int index) const noexcept
{
    if (! getNote (index))
        return -1;

    if (juce::isPositiveAndBelow (index, velocities.size()))
        return velocities[index];
    
    return 127;
}

double StepClip::Pattern::CachedPattern::getGate (int index) const noexcept
{
    if (! getNote (index))
        return 0.0;

    if (juce::isPositiveAndBelow (index, gates.size()))
        return gates[index];

    return 1.0;
}

// BEATCONNECT MODIFICATION START
int StepClip::Pattern::CachedPattern::getKeyNoteOffset(int index) const noexcept {
    if (!getNote(index))
        return -1;

    if (juce::isPositiveAndBelow(index, keyNoteOffsets.size()))
        return keyNoteOffsets[index];

    return defaultKeyNoteOffset;
}
// BEATCONNECT MODIFICATION END

float StepClip::Pattern::CachedPattern::getProbability (int index) const noexcept
{
    if (! getNote (index))
        return 0.0;

    if (juce::isPositiveAndBelow (index, probabilities.size()))
        return probabilities[index];

    return 1.0;
}

// BEATCONNECT MODIFICATION START
int StepClip::Pattern::CachedPattern::getTremolo(int index) const noexcept
{
    if (!getNote(index))
        return -1;

    if (juce::isPositiveAndBelow(index, tremolos.size()))
        return tremolos[index];

    return defaultTremoloAttacks;
}
// BEATCONNECT MODIFICATION END

//==============================================================================
StepClip::Pattern StepClip::PatternInstance::getPattern() const
{
    return clip.getPattern (patternIndex);
}

int StepClip::PatternInstance::getSequenceIndex() const
{
    return clip.getPatternSequence().indexOf (this);
}

juce::String StepClip::PatternInstance::getSelectableDescription()
{
    return clip.getName() + "  -  "
         + TRANS("Variation 123").replace ("123", juce::String (getSequenceIndex() + 1)) + " ("
         + TRANS("Variation 123").replace ("123", juce::String (patternIndex + 1)) + ")";
}

}} // namespace tracktion { inline namespace engine
