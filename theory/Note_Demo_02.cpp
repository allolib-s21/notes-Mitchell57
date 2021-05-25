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

#include "theory.h"
#include "tempo.h"

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

    Scale scale = Scale("C4", scale_type::Minor);
    Chord chord1 = Chord("Cmaj");
    Chord chord2 = Chord("Emaj");
    Chord chord3 = Chord("G7");
    Chord chord4 = Chord("Bm");

    switch (k.key())
    {
    case 'a':
      playHappyBirthday(Note("C4"), 90);
      return false;
    case 'm':
      
      chord3.match(chord1);
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

  float playNote(float time, Note note, float duration = 0.5, float amp = 0.2, float attack = 0.1, float decay = 0.5)
  {
    auto *voice = synthManager.synth().getVoice<SquareWave>();
    // amp, freq, attack, release, pan
    voice->setTriggerParams({amp, note.frequency(), 0.1, 0.1, 0.0});
    synthManager.synthSequencer().addVoiceFromNow(voice, time, duration*0.9);

    return time+duration;
  }

  float playChord(float time, Chord chord, float duration, bool roll=false){
      float localTime = 0;
      for(int i=0; i<chord.notes.size(); i++){
          playNote(time+localTime, chord.notes[i], duration, 0.05, 0.2, 0.75);
          if(roll) localTime += duration / 32;
      }
      return time+duration;
  }

  void playHappyBirthday(Note root, float bpm)
  {
    
    // Happy birthday uses: P1(C), M2(D), M3(E), P4(F), P5(G), M6(A), and m7(Bb)
    Scale majScale = Scale(root, scale_type::Major);
    Note M2 = majScale.degree(scale_type::II);
    Note M3 = majScale.degree(scale_type::III);
    Note P4 = majScale.degree(scale_type::IV);
    Note P5 = majScale.degree(scale_type::V);
    Note M6 = majScale.degree(scale_type::VI);
    Note m7 = root.interval(interval_type::m7);
    Note P8 = majScale.degree(scale_type::VIII);

    // For chords it needs: F Maj, C Dom7, Bb Maj, F/C (2nd inversion)
    // (Also dropping the root of the chord down an octave or Perfect Eighth)

    Chord chord1 = P4.octaveDown().chord("maj");

    Chord chord2 = root.chord("7");

    Chord chord3 = m7.octaveDown().chord("maj");

    Chord chord4 = P4.octaveDown().chord("maj"); // 2nd inversion
    chord4.invert(2);

    chord2.match(chord1);
    chord3.match(chord1);
    chord4.match(chord1);


    // Now lets set up a tempo
    //  syntax: Tempo(bpm, timeSig top, timeSig bottom)
    float time = 0;
    Tempo tpo(bpm, 3, 4); 
    // this allows us to say get exact durations for common note types

    time = playNote(time, root, tpo.duration(Tempo::eighth, true)); // true = dotted note
    time = playNote(time, root, tpo.duration(Tempo::sixteenth)); 

    playChord(time, chord1, tpo.duration(Tempo::half));
    time = playNote(time, M2, tpo.duration(Tempo::quarter)); 
    time = playNote(time, root, tpo.duration(Tempo::quarter)); 
    time = playNote(time, P4, tpo.duration(Tempo::quarter)); 

    playChord(time, chord2, tpo.duration(Tempo::half));
    time = playNote(time, M3, tpo.duration(Tempo::half)); 
    time = playNote(time, root, tpo.duration(Tempo::eighth, true)); // true = dotted note
    time = playNote(time, root, tpo.duration(Tempo::sixteenth));

    time = playNote(time, M2, tpo.duration(Tempo::q)); 
    time = playNote(time, root, tpo.duration(Tempo::q)); 
    time = playNote(time, P5, tpo.duration(Tempo::q)); 

    playChord(time, chord1, tpo.duration(Tempo::h));
    time = playNote(time, P4, tpo.duration(Tempo::h)); 
    time = playNote(time, root, tpo.duration(Tempo::e, true)); 
    time = playNote(time, root, tpo.duration(Tempo::s));

    time = playNote(time, P8, tpo.duration(Tempo::q)); 
    time = playNote(time, M6, tpo.duration(Tempo::q)); 
    time = playNote(time, P4, tpo.duration(Tempo::q)); 

    playChord(time, chord3, tpo.duration(Tempo::h));
    time = playNote(time, M3, tpo.duration(Tempo::q)); 
    time = playNote(time, M2, tpo.duration(Tempo::q)); 
    time = playNote(time, m7, tpo.duration(Tempo::e, true));
    time = playNote(time, m7, tpo.duration(Tempo::s));

    playChord(time, chord4, tpo.duration(Tempo::h));
    time = playNote(time, M6, tpo.duration(Tempo::q)); 
    time = playNote(time, P4, tpo.duration(Tempo::q)); 
    playChord(time, chord2, tpo.duration(Tempo::q));   
    time = playNote(time, P5, tpo.duration(Tempo::q));  

    playChord(time, chord1, tpo.duration(Tempo::h));
    time = playNote(time, P4, tpo.duration(Tempo::h)); 

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
