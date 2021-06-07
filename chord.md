# Chord

Stores a list of notes 

### Chord Names

- Key *required
    - (A-G)
- Sign
    - (#, b, n)
- Quality
    - Major (maj, M)
    - Minor (min, m, -)
    - Diminished (dim, o)
    - Augmented (aug, +, +5)
    - Suspended (sus2, sus4)
- Extensions
    - (7, 9, 11, 13)
- Alterations
    - (#5, #9, b5, b9)
- Added Tones
    - add + (2, 4, 6, 8, 9)
- Slash Chords
    - / + (A-G)
    - Specifies inversion, e.g. "F/C"

### Constructors

`Chord(string name, int octave=3)`

Input: Chord name as string, e.g. Chord("CM7")

`Chord(Note* root, string name, int octave=3)`

    e.g. Chord(&note, "dim7")

`Chord(vector<int> idxs)` 

    Used by Scale for building scale chords

### Accessors

`chord.getNotes()`

    returns vector of Notes

`chord.degree(theory::chord_degree)`

    returns Note at specified degree

    degrees: root, third, fifth, seventh, ninth, eleventh, thirteenth

### Modifiers

`chord.invert(int inversion=1)`

    moves the bottom note up an octave

`chord.drop(chord_degree)`

    moves the specified degree down an octave

    default: moves whole chord down an octave

`chord.raise(chord_degree)`

    moves the specified degree up an octave

    default: moves whole chord up an octave

`chord.match(Chord source)`

    moves chord tones to get average pitch closer to source average
    
