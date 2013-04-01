#include "notelist.h"

NoteList::NoteList()
{
  nbNotesPressed = 0;
  for (int i = 0; i < NB_MIDI_NOTES; ++i){
    notesPressed[i] = false;
    next[i] = -1;
    prev[i] = -1;
  }
  first = -1;
  last = -1;
}

bool NoteList::hasNote(int note)
{
  return note >= 0 && note < NB_MIDI_NOTES && notesPressed[note];
}

void NoteList::pushNote(int note)
{
  if (!hasNote(note)) {
    notesPressed[note] = true;
    if (first == -1)
      first = note;
    if (last == -1)
      last = note;
    else {
      next[last] = note;
      prev[note] = last;
      last = note;
    }
    nbNotesPressed++;
  }
}

void NoteList::deleteNote(int note)
{
  if (hasNote(note)) {
    notesPressed[note] = false;
    if (note != last && note != first) {
      prev[next[note]] = prev[note];
      next[prev[note]] = next[note];
      prev[note] = -1;
      next[note] = -1;
    } else if(note == first && note == last) {
      first = -1;
      last = -1;
    } else if (note == last) {
      last = prev[last];
      prev[next[note]] = prev[note];
      next[prev[note]] = next[note];
      prev[note] = -1;
    } else if (note == first) {
      first = next[first];
      prev[next[note]] = prev[note];
      next[prev[note]] = next[note];
      next[note] = -1;
    }
  }
}

void NoteList::reset()
{
  while (anyNotesPressed())
    deleteNote(lastNote());
}
