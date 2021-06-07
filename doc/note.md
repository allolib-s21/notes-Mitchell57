# Note

The Note object stores pitch values as Midi index, and provides methods that allow for construction and extrapolation from 'verbal' and relational declaration 

### Constructors

`Note(int midi)`

- input: midi index between 0 and 127

- e.g `Note a4 = Note(69)`



`Note(string name)`

- input: string name of note formatted as [key][sign][octave]

    - key: A-G

    - sign: #, n, or b (optional, default=n)

    - octave: -1 to 9 (optional, default=4)

- e.g. `Note("A4"), Note("Db"), Note("C#6")`



`Note(char key, char sign, int octave)`

- input: 

    - key: A-G

    - sign: #, n, or b (optional, default=n)

    - octave: -1 to 9 (optional, default=4)

- e.g. `Note('A',4), Note('D', 'b'), Note('C', '#', 6)`

***

### Descriptors


`note.midi()`

- returns midi index [0-127]

- e.g. `Note("A4").midi() = 69` 



`note.frequency(float root=440.0)`

- returns (float) frequency based on root (default 440)

- e.g. `Note("A4").frequency() = 440.0` 



`note.name()`

- returns note as string

- e.g. `Note(69).name() = "A4"`



`note.key()`

- returns note as string (no octave)

- e.g. `Note(69).key() = "A"`



`note.octave()`

- returns (int) octave of note [-1,9]

- e.g. `Note(69).octave() = 4`



`note.distanceTo(Note b)`

- returns (int) distance between notes in semitones

- e.g. `Note("A4").distanceTo("C4") = 3`


***

### Modifiers

`note.set(string name)`

`note.set(int midi)`

`note.set(char key, [char sign, int octave])`

- set note to new location



`note.setOctave(int octave)`

- set note to new octave without changing key



`note.setKey(string name)`

`note.setKey(char key, [char sign])`

- set note to new key without changing octave

***

### Extrapolators

`note.octaveUp(int num)`

`note.octaveDown(int num)`

- returns Note at [num] octaves above/below source



`note.interval(type)`

`note.interval(int num)`

- returns Note at [interval] from source, or [num] semitones from source

- Interval types:

    - Perfect - P1, P4, P5, P8,

    - Minor - m2, m3, m6, m7,

    - Major - M2, M3, M6, M7,

    - Diminished - d2, d3, d4, d5, d6, d7, d8,

    - Augmented - A1, A2, A3, A4, A5, A6, A7
    
    

`note.chord(string chordType, int octave)`

- returns [chordType] Chord with source as root note

- see "Chord"



`note.scale(scale_type type)`

- returns [scaleType] Scale with source as tonic

- see "Scale"

[Scale](https://www.notion.so/Scale-d87c19e4857a43f6900abe69428f8bcf)

[Chord](https://www.notion.so/Chord-5907b736d2a2423e8fba5028ab57096b)