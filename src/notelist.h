#ifndef NOTELIST_H
#define NOTELIST_H


#define NB_MIDI_NOTES 129

class NoteList
{
  bool hasNote(int);
  int nbNotesPressed;
  int first, last;

  bool notesPressed[NB_MIDI_NOTES];
  int next[NB_MIDI_NOTES];
  int prev[NB_MIDI_NOTES];
      
public: 
  NoteList();
  void pushNote(int);
  int lastNote() {
    return last;
  }
  void deleteNote(int);
  bool anyNotesPressed() {
    return last != -1;
  }
  void reset();
};

#endif
