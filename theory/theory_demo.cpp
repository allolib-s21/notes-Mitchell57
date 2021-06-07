#include <cstdio> // for printing to stdout

#include "Gamma/Analysis.h"
#include "Gamma/Effects.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/scene/al_PolySynth.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

#include "theoryOne.h"


// using namespace gam;
using namespace al;
using namespace theory;

// This example shows how to use SynthVoice and SynthManagerto create an audio
// visual synthesizer. In a class that inherits from SynthVoice you will
// define the synth's voice parameters and the sound and graphic generation
// processes in the onProcess() functions.

class SquareWave : public SynthVoice
{
public:
  // Unit generators
  gam::Pan<> mPan;
  gam::Sine<> mOsc1;
  gam::Sine<> mOsc3;
  gam::Sine<> mOsc5;
  gam::Sine<> mOsc7;

  gam::Env<3> mAmpEnv;

  // Initialize voice. This function will only be called once per voice when
  // it is created. Voices will be reused if they are idle.
  void init() override
  {
    // Intialize envelope
    mAmpEnv.curve(0); // make segments lines
    mAmpEnv.levels(0, 1, 1, 0);
    mAmpEnv.sustainPoint(2); // Make point 2 sustain until a release is issued

    createInternalTriggerParameter("amplitude", 0.8, 0.0, 1.0);
    createInternalTriggerParameter("frequency", 440, 20, 5000);
    createInternalTriggerParameter("attackTime", 0.1, 0.01, 3.0);
    createInternalTriggerParameter("releaseTime", 0.1, 0.1, 10.0);
    createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
  }

  // The audio processing function
  void onProcess(AudioIOData &io) override
  {
    // Get the values from the parameters and apply them to the corresponding
    // unit generators. You could place these lines in the onTrigger() function,
    // but placing them here allows for realtime prototyping on a running
    // voice, rather than having to trigger a new voice to hear the changes.
    // Parameters will update values once per audio callback because they
    // are outside the sample processing loop.
    float f = getInternalParameterValue("frequency");
    mOsc1.freq(f);
    mOsc3.freq(f * 3);
    mOsc5.freq(f * 5);
    mOsc7.freq(f * 7);

    float a = getInternalParameterValue("amplitude");
    mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
    mAmpEnv.lengths()[2] = getInternalParameterValue("releaseTime");
    mPan.pos(getInternalParameterValue("pan"));
    while (io())
    {
      float s1 = mAmpEnv() * (mOsc1() * a +
                              mOsc3() * (a / 3.0) +
                              mOsc5() * (a / 5.0) +
                              mOsc7() * (a / 7.0));

      float s2;
      mPan(s1, s1, s2);
      io.out(0) += s1;
      io.out(1) += s2;
    }
    // We need to let the synth know that this voice is done
    // by calling the free(). This takes the voice out of the
    // rendering chain
    if (mAmpEnv.done())
      free();
  }

  // The triggering functions just need to tell the envelope to start or release
  // The audio processing function checks when the envelope is done to remove
  // the voice from the processing chain.
  void onTriggerOn() override { mAmpEnv.reset(); }
  void onTriggerOff() override { mAmpEnv.release(); }
};

// We make an app.
class MyApp : public App
{
public:
  // GUI manager for SquareWave voices
  // The name provided determines the name of the directory
  // where the presets and sequences are stored
  SynthGUIManager<SquareWave> synthManager{"SquareWave"};
  
  // This function is called right after the window is created
  // It provides a grphics context to initialize ParameterGUI
  // It's also a good place to put things that should
  // happen once at startup.
  void onCreate() override
  {
    navControl().active(false); // Disable navigation via keyboard, since we
                                // will be using keyboard for note triggering

    // Set sampling rate for Gamma objects from app's audio
    gam::sampleRate(audioIO().framesPerSecond());

    imguiInit();

    

    // Play example sequence. Comment this line to start from scratch
    // synthManager.synthSequencer().playSequence("synth1.synthSequence");
    synthManager.synthRecorder().verbose(true);
  }

  // The audio callback function. Called when audio hardware requires data
  void onSound(AudioIOData &io) override
  {
    synthManager.render(io); // Render audio
  }

  void onAnimate(double dt) override
  {
    // The GUI is prepared here
    imguiBeginFrame();
    // Draw a window that contains the synth control panel
    synthManager.drawSynthControlPanel();
    imguiEndFrame();
  }

  // The graphics callback function.
  void onDraw(Graphics &g) override
  {
    g.clear();
    // Render the synth's graphics
    synthManager.render(g);

    // GUI is drawn here
    imguiDraw();
  }

  // Whenever a key is pressed, this function is called
  bool onKeyDown(Keyboard const &k) override
  {
    if (ParameterGUI::usingKeyboard())
    { // Ignore keys if GUI is using
      // keyboard
      return true;
    }

    Scale scale = Scale("C4", theory::Minor);
    Chord chord1 = Chord("Cmaj");
    Chord chord2 = Chord("Emaj");
    Chord chord3 = Chord("G7");
    Chord chord4 = Chord("Bm");

    switch (k.key())
    {
    case 'a':
      noteDemo();
      return false;
    case 'm':
      
      playProgression();
      return false;
    case '1':
      playChord(0, chord1, 1.0);
      return false;
    case '2':
      playChord(0, chord2, 1.0);
      return false;
    case '3':
      playChord(0, chord3, 1.0);
      return false;
    case '4':
      playChord(0, chord4, 1.0);
      return false;
    case '5':
      chord2.match(chord1);
      chord3.match(chord1);
      chord4.match(chord1);
      playChord(0, chord1, 1.0);
      playChord(1, chord2, 1.0);
      playChord(2, chord3, 1.0);
      playChord(3, chord4, 1.0);
      return false;
    
    
    }

    return true;
  }

  // Whenever a key is released this function is called
  bool
  onKeyUp(Keyboard const &k) override
  {
    int midiNote = asciiToMIDI(k.key());
    if (midiNote > 0)
    {
      synthManager.triggerOff(midiNote);
    }
    return true;
  }

  void onExit() override { imguiShutdown(); }

  // New code: a function to play a note A

  float playNote(float time, Note note, float duration = 0.5, float amp = 0.1, bool fullDuration=false)
  {
    auto *voice = synthManager.synth().getVoice<SquareWave>();
    
    // unless specified, note plays for 90% of given duration
    // to allow separation of successive notes
    float dur;
    if(fullDuration){
      dur = duration;
    }
    else{
      dur = duration*0.9;
    }
    // amp, freq, attack, release, pan
    voice->setTriggerParams({amp, note.frequency(), 0.1, 0.1, 0.0});
    synthManager.synthSequencer().addVoiceFromNow(voice, time, dur); 

    return time+duration;
  }

  float playChord(float time, Chord chord, float duration, float roll=0){
      float localTime = 0;
      for(int i=0; i<chord.notes.size(); i++){
          playNote(time+localTime, chord.notes[i], duration, 0.05);
          
          // if passed, chord tones will play in sequence instead of simultaneously
          localTime += roll;
      }
      return time+duration;
  }

  void playProgression(){
    Chord c1 = Chord("Dmin11");
    Chord c2 = Chord("Gmin7");
    Chord c3 = Chord("Ebmin11");
    Chord c4 = Chord("C#dim7");
    c4.raise();

    theory::Tempo tpo(80, (Tempo::timeSignature){4,4});
    float dottedHalf = tpo.duration(Tempo::half, true);
    float sixteenth = tpo.duration(Tempo::sixteenth);
    float quarter = tpo.duration(Tempo::quarter);

    float time=0;

    time = playChord(time, c1, dottedHalf);
    time += quarter; // One beat rest

    time = playChord(time, c2, dottedHalf);
    time += quarter; 

    time = playChord(time, c1, dottedHalf);
    time += quarter; // One beat rest

    time = playChord(time, c3, dottedHalf/2);

    time = playChord(time, c4, dottedHalf/2);
    time += quarter;


  }


  void noteDemo(){
    // Tempo constructor takes bpm and time signature, or bpm, timesig top, timesig bottom
    theory::Tempo tpo(80, (Tempo::timeSignature){4,4}); 

    // Then we can define a couple common note durations
    float quarter = tpo.duration(Tempo::quarter);
    float half = tpo.duration(Tempo::half);
    float dottedHalf = tpo.duration(Tempo::half, true);
    float eighth = tpo.duration(Tempo::eighth);
    float sixteenth = tpo.duration(Tempo::sixteenth);

    // Notes can be declared several ways
    //    default sign = natural, default octave = 4
    Note note1 = Note("A5");         // as a string (with or without octave)
    // Note note2 = Note('A', 'n', 4);  // as char key, char sign (#, b, or n), and int octave
    // Note note3 = Note(69);           // as a midi index  (A4=69)

    // Then with playNote (above), they can be played
    // Time is passed and returned by playNote() so that it can increment automatically
    float time = 0;
    time = playNote(time, note1, quarter);
    
    std::cout << "press enter to continue" << std::endl;
    system("read");
    time=0;

    // To build on this note, you can grab notes that are "musically related" with scales and intervals
    // For example, we could play the pentatonic major scale
    // ( Scale types should pop up as autocomplete on VS Code )
    Scale scale = Scale(note1, theory::PentMajor); 

    for(Note n : scale.notes)
    {
      time = playNote(time, n, quarter); 
    }

    system("read"); // waits for user
    time=0;

    // We can move notes by semitone, interval, or octave
    // For example, we can play the scale again but play the perfect fourth of each note
    for(Note n : scale.notes)
    {
      time = playNote(time, n.interval(theory::P4), quarter);
    }
    
    system("read");
    time=0;

    // Or drop the note by an octave
    for(Note n : scale.notes)
    {
      time = playNote(time, n.octaveDown(), quarter);
    }
    
    system("read");
    time=0;

    // We can also use note objects to build chords
    Chord chord1 = note1.chord("maj");

    // Or build a chord using only scale tones
    // scale chord parameters: (scale_degree) degree, (int) length
    // length can be 3 (triad), 4 (seventh)
    Chord chord2 = scale.chord(theory::Tonic);

    // Or build the chord directly
    Chord chord3 = Chord("Bbm7b5"); // B flat, minor seventh, flat five

    // Then we can play those with playChord
    time = playChord(time, chord1, dottedHalf, sixteenth);
    time += quarter; // One beat rest

    time = playChord(time, chord2, dottedHalf, sixteenth);
    time += quarter; 

    time = playChord(time, chord3, dottedHalf, sixteenth);
    time += quarter; 

    system("read");
    time=0;

    // That second one is quite a bit higher than the other two,
    // so we can 'roll' the pitch down to try and match the first
    //    i.e. invert the chord by shifting the highest note down an octave
    //          until the average pitch is as close to the destination chord
    Chord chord2copy = Chord(chord2);
    chord2copy.match(chord1);

    // First the original
    time = playChord(time, chord1, half);
    time = playChord(time, chord2, half);
    
    time += quarter;
    system("read");
    time=0;

    // Then matched
    time = playChord(time, chord1, half);
    time = playChord(time, chord2copy, half);
    system("read");
    time=0;

    // We can also invert the chord (move the bottom note to the top)
    //  (parameter = # of inversions  / # of chord notes to raise)
    Chord chord4 = Chord("F");
    chord4.invert(2);
    time = playChord(time, chord4, half);
    system("read");
    time=0;

    // When translating sheet music, chords are often specified with "figured bass"
    // e.g. F/C means play F major but make C the bottom chord
    // For someone without extensive chord knowledge, figuring out which inversion it is can be time consuming
    // So instead:
    Chord chord5 = Chord("F/C");
    time = playChord(time, chord5, half);
  }


  };

  

int main()
{
  // Create app instance
  MyApp app;

  // Set up audio
  app.configureAudio(48000., 512, 2, 0);

  app.start();

  return 0;
}
