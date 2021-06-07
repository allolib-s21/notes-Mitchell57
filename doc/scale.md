# Scale

Stores a list of notes according to a specified scale type

### Scale Types

Aeolian / Minor, Ionian / Major,

Dorian, Phrygian, Locrian,

Lydian, Mixolydian

Pentatonic Major, Pentatonic Minor,

Harmonic Major, Harmonic Minor,

Melodic Minor (asc), Melodic Minor (desc),

Bebop Dominant, Bebop Major,

Major Minor, Half Diminished,

Algerian, Augmented, Blues,

Chromatic, Prometheus, Tritone,

Locrian Major, Altered / Super Locrian,

Phrygian Dominant, Lydian Augmented / Acoustic,

Enigmatic, Double Harmonic / Flamenco,

Neapolitan Major, Neapolitan Minor,

Hungarian Minor, Hungarian Major

Hirajoshi, In, Insen, Iwato, Persian

### Constructors

`Scale(Note, theory::scale_type)`

e.g. Scale(note, theory::Major)

`Scale(string note, theory::scale_type)`

e.g. Scale("C", theory::Lydian)

### Accessors

degree() - returns Note at specified scale degree

`scale.degree(theory::scale_degree)`

e.g. scale.degree(theory::Mediant) or scale.degree(theory::III)

`scale.degree(int degree)`

e.g. scale.degree(1) returns tonic note

`scale.index(int)` - returns Note at specified index

e.g. scale.index(0)

chord() - returns Chord built with scale tones

- `scale.chord(int degree, int size=3)`
- `scale.chord(scale_degree, int size=3)`
    - degrees:
        - Tonic, Supertonic, Mediant, Subdominant, Dominant, Submediant, Subtonic / Leading, Tonic2
        - I, II, III, IV, V, VI, VII, VIII
- example - Play the V chord of the C Major scale:

    `Scale cMajScale = Scale('C', theory::Major)`

    `Chord cMajV = cMajScale.chord(5)`