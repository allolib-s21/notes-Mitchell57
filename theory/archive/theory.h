#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <ostream>
#include <assert.h>  
#include <regex>

#include "notehelpers.h"

namespace theory {
    
    class Scale;
    class Chord;
    class Note 
    {
        public:
            
            int index;
            char signPref;

            Note(char note='A', char sign='n', int octave=4, char signPref='b');
            Note(std::string input, char signPref='n');
            Note(int midi, char signPref='b');

            std::string name();
            std::string key();
            int         midi();
            float       frequency(float root=440.0);
            int         octave();
            int         distanceTo(Note* b);

            void set(int midi, char signPref='b');
            void set(std::string input, char signPref='n');
            void set(char key='A', char sign='n', int octave=4, char signPref='b');

            bool setOctave(int octave=4);
            bool setKey(std::string key);
            bool setKey(char key, char sign);

            Note octaveUp(int num=1);
            Note octaveDown(int num=1);

            Note  interval(interval_type::name type, int direction=1);
            Note  interval(int semitones);
            Chord chord(std::string chord_name, int octave=3);
            Scale scale(scale_type::name type);

        private:
            void init(int midi, char signPref);
    };

    typedef std::vector<Note> notelist;

    class Chord
    {
        public:
            notelist notes;
            chord_type::quality quality;

            Chord(std::string name, int octave=3);
            Chord(Note* root, std::string name, int octave=3);
            Chord(std::vector<int> idxs);

            Note root();
            Note third();
            Note fifth();
            Note seventh();
            Note ninth();
            Note eleventh();
            Note thirteenth();

            void invert(int inversion=1);
            void drop(chord_type::degree degree=chord_type::all);
            void raise(chord_type::degree degree=chord_type::all);
            void match(Chord source);

            std::vector<int> indexes();

            float score();
            void sort();
        
        private:
            void init(helper::parsed_chord parsed, int octave=3);
    };

    class Scale{
        public:

            notelist notes;
            scale_type::name type;

            Scale(Note tonic, scale_type::name type);
            Scale(std::string tonic, scale_type::name type);

            Note degree(scale_type::degree degree);
            Note degree(int degree);
            Note index(int idx);

            Chord chord(scale_type::degree degree, int size=3);
            Chord chord(int degree, int size=3);
        
        private:
            void init(Note tonic, scale_type::name type);
            Chord buildChord(int degree, int size);
    };

// ------------------------------------------------------------------
//      Note methods
// ------------------------------------------------------------------ 

        // Constructors
        Note::Note(std::string input, char signPref){ this->set(input, signPref); }

        Note::Note(int midi, char signPref){ init(midi, signPref); }

        Note::Note(char note, char sign, int octave, char signPref){ 
            helper::parsed_str parsed = {note, sign, octave};
            init(helper::parsedToMidi(parsed), signPref);
        }
        
        // Main initializer
        void Note::init(int midi, char signPref){
            if(midi > 127 || midi <0)
            {
                throw std::out_of_range("Note(midi) : midi index ("+std::to_string(midi)+") is out of range");
            }
            
            this->index = midi;
            this->signPref = signPref;
        }

        // returns full note name (e.g. "Db6")
        std::string Note::name(){
            return helper::midiToString(this->index, this->signPref);
        }

        // returns key without octave (e.g. "Db")
        std::string Note::key(){
            return helper::midiToString(this->index, this->signPref, false);
        }

        // returns midi index
        int Note::midi(){
            return this->index;
        }

        // returns frequency (based on root)
        float Note::frequency(float root){
            int distance = this->index - 69;
            double multiplier = pow(2.0, 1.0/12);

            return (float)(root*pow(multiplier, distance));
        }

        // returns octave [-1, 9]
        int Note::octave(){
            return (this->midi()/12)-1;
        }

        // set note to new midi index [0-127]
        void Note::set(int midi, char signPref){ init(midi, signPref); }

        // set note to new string name
        void Note::set(std::string input, char signPref)
        { 
            helper::parsed_str parsed = helper::parseString(input);
            int idx = helper::parsedToMidi(parsed);
            if(signPref == 'n'){
                if(parsed.sign == '#') { init(idx, '#'); }
                else{ init(idx, 'b'); }
            }
            else{ init(idx, signPref); } 
        }
        
        // set note to new key, sign, and octave
        void Note::set(char key, char sign, int octave, char signPref){ 
            helper::parsed_str parsed = {key, sign, octave};
            init(helper::parsedToMidi(parsed), signPref);
        }

        // set octave of note without changing key
        bool Note::setOctave(int octave){
            if(octave < -1 || octave > 9) return false;
            int noteIdx = this->index%12;
            int offset = (octave+1)*12;
            this->index = noteIdx + offset;

            return true;
        }

        // set key of note without changing octave
        bool Note::setKey(std::string key){
            int octave = this->octave();
            this->set(key);
            return this->setOctave(octave);
        }

        // set key of note without changing octave
        bool Note::setKey(char key, char sign){
            int octave = this->octave();
            this->set(key, sign, octave);
            return true;
        }

        // returns note at octave intervals above
        Note Note::octaveUp(int num){
            return Note(this->index+(12*num));
        }

        // returns note at octave intervals above
        Note Note::octaveDown(int num){
            return Note((this->index)-(12*num));
        }

        // returns distance (in semitones) to another note
        int Note::distanceTo(Note* b){
            return b->midi() - this->index;
        }

        // returns note at specified interval above/below current note
        // direction = 1 for up
        // direction = -1 for down
        Note Note::interval(interval_type::name type, int direction){
            int interval = interval_type::table[type] * direction;
            Note n = Note(index + interval);
            return n;
        }

        // returns note at specified interval above/below current note
        Note Note::interval(int semitones){
            Note n = Note(index + semitones);
            return n;
        }

// ------------------------------------------------------------------
//      Chord methods
// ------------------------------------------------------------------ 

        // constructors
        Chord::Chord(std::string name, int octave){
            helper::parsed_chord parsed = helper::parseChord(name);
            init(parsed, octave);
        }

        Chord::Chord(Note* root, std::string name, int octave){
            helper::parsed_chord parsed = helper::parseChord(name);
            parsed.key = root->key();
            init(parsed, octave);
        }

        Chord::Chord(std::vector<int> idxs){
            for(int idx: idxs){
                notes.push_back(Note(idx));
            }
        }

        void Chord::init(helper::parsed_chord parsed, int octave){
            std::cout << "--- Chord init ---" << std::endl;
            std::cout << "key: " << parsed.key << std::endl;
            std::cout << "quality: " << parsed.quality << std::endl;
            std::cout << "intervals: ";
            for(int x : parsed.intervals) std::cout << x << ", "; 
            
            
            std::cout << std::endl << std::endl;
            Note root = Note(parsed.key);
            quality = parsed.quality;
            
            root.setOctave(octave);
            int rootIdx = root.midi();

            for(int interval:parsed.intervals){
                notes.push_back(Note(rootIdx + interval));
            }

            if(parsed.bass != parsed.key && parsed.bass.length() > 0){
                int bassNote = helper::noteIndex(parsed.bass);
                int bassIdx = -1;
                for(int i=0; i<notes.size(); i++){
                    int noteLoc = helper::noteIndex(notes[i].key());
                    if(noteLoc == bassNote){
                        bassIdx = i;
                    }
                }
                if(bassIdx == -1){
                    throw std::out_of_range("Chord(string) : Figured bass ("+parsed.bass+") is not in chord");
                }
                else{ this->invert(bassIdx); }
            }
        }

        // Returns chord with root (this) and type (name)
        Chord Note::chord(std::string name, int octave){
            return Chord(this, name, octave);
        }

        // returns root note
        Note Chord::root(){ return notes[0]; }

        // returns third note
        Note Chord::third(){ return notes[1]; }

        // returns fifth note
        Note Chord::fifth(){ return notes[2]; }

        // returns seventh note
        Note Chord::seventh(){ 
            if(notes.size() > 3){
                return notes[3];
            }
            else{
                int interval = chord_type::table[quality][3];
                if(interval < 0){
                    throw std::out_of_range("Chord(string) : Sus chords cannot be extended");
                }
                return Note(root().index + interval);
            }
        }

        // returns ninth note
        Note Chord::ninth(){ 
            if(notes.size() > 4){
                return notes[4];
            }
            else{
                int interval = chord_type::table[quality][4];
                if(interval < 0){
                    throw std::out_of_range("Chord(string) : Sus chords cannot be extended");
                }
                return Note(root().index + interval);
            }
        }   

        // returns eleventh note
        Note Chord::eleventh(){ 
            if(notes.size() > 5){
                return notes[5];
            }
            else{
                int interval = chord_type::table[quality][5];
                if(interval < 0){
                    throw std::out_of_range("Chord(string) : Sus chords cannot be extended");
                }
                return Note(root().index + interval);
            }
        }

        // returns thirteenth note
        Note Chord::thirteenth(){ 
            if(notes.size() > 6){
                return notes[6];
            }
            else{
                int interval = chord_type::table[quality][6];
                if(interval < 0){ throw std::out_of_range("Chord(string) : Sus chords cannot be extended"); }
                return Note(root().index + interval);
            }
        }   

        void Chord::drop(chord_type::degree degree){
            if(degree == chord_type::all){
                for(Note n: notes){
                    n.octaveDown();
                }
            }
            else{
                if(degree < notes.size()){
                    notes[degree].octaveDown();
                }
                else{
                    int interval = chord_type::table[quality][degree];
                    Note newNote = Note(root().midi() + interval - 12);
                }
            }
        }

        void Chord::raise(chord_type::degree degree){
            if(degree == chord_type::all){
                for(Note n: notes){
                    n.octaveUp();
                }
            }
            else{
                if(degree < notes.size()){
                    notes[degree].octaveUp();
                }
                else{
                    int interval = chord_type::table[quality][degree];
                    Note newNote = Note(root().midi() + interval + 12);
                }
            }
        }

        void Chord::invert(int inversion){
            for(int i=0; i<inversion; i++){
                int newIdx = notes[0].index+12;
                if(newIdx > 127){
                    this->drop();
                    newIdx = notes[0].index+12;
                }
                std::rotate(notes.begin(), notes.begin()+1, notes.end());
            }
        }

        //helper funcs for chord match
        bool compareNote(Note a, Note b){ return (a.midi() < b.midi()); }


        void Chord::sort(){
            std::sort(notes.begin(), notes.end(), compareNote);
        }


        std::vector<int> Chord::indexes(){
            std::vector<int> ret;
            for(Note n : notes){
                ret.push_back(n.midi());
            }
            return ret;
        }

        int inner_distance(std::vector<int> idxs){
            int sum = 0;
            for(int i=0; i<idxs.size()-2; i++){
                sum += abs(idxs[i+1]-idxs[i]);
            }
            return sum;
        }

        float Chord::score(){
            int sum = 0.0;
            for(Note n : notes){
                sum += n.midi();
            }
            return sum / notes.size();
        }

        // Attempts to match average pitch of this chord to source chord 
        void Chord::match(Chord dest){
            float destScore = dest.score();
            float score = this->score();
            float dist = score - destScore;
            float newScore, newDist;
            float threshold = (12.0/notes.size())+2;
            while(abs(dist) > threshold){
                if(dist > threshold) {
                    int i= notes.size()-1;
                    Note dropped = notes[i].octaveDown();
                    newScore = (this->score() - notes[i].index + dropped.index) / notes.size();
                    newDist = newScore - destScore;
                    if(newDist > dist) break;
                    else notes[i] = dropped;
                    this->sort();
                }
                else if(dist < -1*threshold){
                    Note raised = notes[0].octaveUp();
                    newScore = (this->score() - notes[0].index + raised.index) / notes.size();
                    newDist = newScore - destScore;
                    if(newDist > dist) break;
                    else notes[0] = raised;
                    this->sort();
                }
                score = this->score();
                dist = score - destScore;
            }
        }

        

// ------------------------------------------------------------------
//      Scale methods
// ------------------------------------------------------------------ 
        void Scale::init(Note tonic, scale_type::name type){
            this->type = type;
            
            // loop through chord intervals to build list of notes
            for(int i=0; i<scale_type::maxLength; i++){
                int interval = scale_type::table[type][i];
                if(interval >= 0 ){ // variable length scales, fixed length array, filled space with -1s
                    int idx = tonic.index + interval;
                    Note cnote = Note(idx);
                    notes.push_back(cnote);
                }
            } 
        }

        Scale::Scale(Note tonic, scale_type::name type){
            init(tonic, type);
        }

        Scale::Scale(std::string tonic, scale_type::name type){
            init(Note(tonic), type);
        }

        Note Scale::degree(scale_type::degree degree){
            return notes[degree];
        }

        Note Scale::degree(int degree){
            return notes[degree-1];
        }

        Note Scale::index(int idx){
            return notes[idx];
        }

        Chord Scale::chord(scale_type::degree degree, int size){
            return this->buildChord(degree, int size);
        }

        Chord Scale::chord(int degree, int size){
            return this->buildChord(degree-1, size);
        }

        Chord Scale::buildChord(int degree, int size){
            int idx = degree;
            std::vector<int> chordNotes;
            for(int i=0; i<size; i++){
                chordNotes.push_back(notes[idx].midi());
                idx = (idx+2)%notes.size();
            }

            return Chord(chordNotes);
        }

}