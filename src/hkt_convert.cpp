#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>
#include <unistd.h> //getopt
#include "utf8_v2_3/source/utf8.h"

using namespace std;

string getSyllable(string::iterator& it, string::iterator end);
int noteToPitch(const string& note);
void writeHeader(ostream& out);

class OptionData {
public:
    OptionData(const int argc, char** argv) {
        output = &cout;
        lyricsSpecified = false;
        pitchesSpecified = false;
        timesSpecified = false;
        
        // Process args
        extern char *optarg;
        extern int optopt;
        int errflg = 0;
        
        while (1) {
            int c = getopt(argc, argv, ":l:p:t:o:");
            if (c == -1) break;
            
            switch(c) {
            case 'l':
                lyrics.open(optarg);
                if (!lyrics.good()) {
                    cerr<<"Error opening lyrics file: "<<optarg<<endl;
                    errflg++;
                }
                lyricsSpecified = true;
                break;
            case 'p':
                pitches.open(optarg);
                if(!pitches.good()) {
                    cerr<<"Error opening pitches file: "<<optarg<<endl;
                    errflg++;
                }
                pitchesSpecified = true;
                break;
            case 't':
                times.open(optarg);
                if (!times.good()) {
                    cerr<<"Error opening times file: "<<optarg<<endl;
                    errflg++;
                }
                timesSpecified = true;
                break;
            case 'o':
                output = new ofstream(optarg);
                if (!output->good()) {
                    cerr<<"Error opening output file: "<<optarg<<endl;
                    errflg++;
                }
                outputSpecified = true;
                break;
            case ':':
                cerr<<"Option -"<<static_cast<char>(optopt)<<" requires an operand"<<endl;
                errflg++;
                break;
            case '?':
                cerr<<"Unrecognized option: -"<<static_cast<char>(optopt)<<endl;
                errflg++;
            }
        }
        if (errflg) {
            cerr<<"Options:\n"
                "-l Lyrics file\n"
                "-p Pitches file\n"
                "-t Times file\n"
                "-o Output file (default is stdout)"<<endl;
            exit(2);
        }
    }
    
    ~OptionData() {
        pitches.close();
        lyrics.close();
        times.close();
        
        if (outputSpecified) {
            ofstream *ofs = static_cast<ofstream *>(output);
            ofs->close();
            delete ofs;
        }
    }

//private:
    ifstream lyrics;
    ifstream pitches;
    ifstream times;
    ostream* output;
    bool lyricsSpecified;
    bool pitchesSpecified;
    bool timesSpecified;
    bool outputSpecified;
};

int main(int argc, char** argv) {
    OptionData options(argc, argv);
    ostream& output = *(options.output);
    
    writeHeader(output);
    int time = 0;
    int prevTime = 0;
    bool newLine = false;
    
    while (options.lyrics.good() || options.pitches.good() || options.times.good()) {
        string lyricsLine;
        getline(options.lyrics, lyricsLine);
        string::iterator it = lyricsLine.begin();
        string::iterator end = lyricsLine.end();
        
        string pitchesLine;
        getline(options.pitches, pitchesLine);
        istringstream pitchStream(pitchesLine);
        
        string timesLine;
        getline(options.times, timesLine);
        istringstream timeStream(timesLine);
        
        if (lyricsLine.length() == 0 && !pitchStream && !timeStream) break;
        
        if (newLine) {
            output<<"- "<<prevTime<<endl;
            newLine = false;
        }
        
        while (1) {
            string syllable = getSyllable(it, end);
            string pitch;
            pitchStream >> pitch;
            
            // Get note duration
            int duration = 2;
            string timeString;
            timeStream >> timeString;
            if (timeStream) {
                const char* timeCString = timeString.c_str();
                
                // Rest
                if (timeString[0] == 'r') {
                    time += atoi(timeCString+1);
                    timeString = "";
                    timeStream >> timeString;
                }
                
                if (timeStream) duration = atoi(timeCString);
            }

            if (syllable.length() == 0 && !pitchStream && !timeStream) break;
            
            if ((options.lyricsSpecified && syllable.length() == 0)
                || (options.pitchesSpecified && !pitchStream)
                || (options.timesSpecified && !timeStream))
            {
                string errorMessage;
                if (options.lyricsSpecified && syllable.length() == 0) errorMessage += "No syllable. ";
                if (options.pitchesSpecified && !pitchStream) errorMessage += "No pitch. ";
                if (options.timesSpecified && !timeStream) errorMessage += "No time. ";
                if (errorMessage.length() > 0) cerr<<time<<" "<<errorMessage<<endl;
            }

            output<<": "<<time<<" "<<duration<<' '<<noteToPitch(pitch)<<' '<<syllable<<endl;
            
            if (timeStream) time += duration;
            else time += 2;
        }
        prevTime = time;
        newLine = true;
    }
    
    output<<"E";
}

void writeHeader(ostream& out) {
    out<<"#ENCODING:UTF8\n"
        "#TITLE:\n"
        "#ARTIST:\n"
        "#EDITION:\n"
        "#GENRE:Anime\n"
        "#LANGUAGE:Japanese\n"
        "#MP3:.mp3\n"
        "#COVER:cover.jpg\n"
        "#BACKGROUND:background.jpg\n"
        "#BPM:150\n"
        "#GAP:1000"<<endl;
}

/*
C2 is octave starting at C4 (middle C)
Only sharps (#) are accepted.
Asssumes valid note is given. TODO: error checking.
*/
int noteToPitch(const string& note) {
    if (note.length() < 2) return 0;

    map<char, int> m;
    m['c'] = 0;
    m['d'] = 2;
    m['e'] = 4;
    m['f'] = 5;
    m['g'] = 7;
    m['a'] = 9;
    m['b'] = 11;
    
    int pitch = m[note[0]];
    if (note[1] == '#') pitch++;
    
    string octave(1, note[note.length() - 1]);
    pitch += 12*(atoi(octave.c_str()) - 2); 
    
    return pitch;
}

string getSyllable(string::iterator& it, string::iterator end) {
    string syllable;
   
    int c;
        
    // Ignore whitespace
    while(1) {
        if (it == end) return syllable;
        c = utf8::next(it, end);
        if (!isspace(c)) break;
    }

    // When '-' is a separate syllable
    if (c == '-') {
        if (it == end) return "~";
       
        c = utf8::next(it, end);

        if (isspace(c)) return "~";
        
        if (c == '-') {
            // Put back so we can test when it is a separate syllable
            it--;
            return "~";
        }
     }
    
    while (1) {
        switch(c) {
        case ' ': // Word end
            syllable += ' ';
            return syllable;
        case '-': // Syllable start
            // Put back so we can test when it is a separate syllable
            it--;
            return syllable;
        case '\\': // Escape character
            if (it == end) return syllable;

            c = utf8::next(it, end);
            utf8::append(c, back_inserter(syllable));
            
            if (it == end) return syllable;
            c = utf8::next(it, end);
            break;
        default:
            utf8::append(c, back_inserter(syllable));
            if (it == end) return syllable;
            c = utf8::next(it, end);
        }
    }
    
    return syllable;
}
