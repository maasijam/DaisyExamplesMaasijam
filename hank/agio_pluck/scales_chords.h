#ifndef SCALES_CHORDS_H_
#define SCALES_CHORDS_H_

const int numChords = 12;

enum scale {
  MAJOR,
  MINOR,
  DORIAN,
  LAST_SCALE
};

struct Chord {
  int num_notes;
  int notes[numChords];  
};

const Chord scaleChords[LAST_SCALE][numChords] = {
  // MAJOR
  {
  // I
  { 3, {0,4,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,4,7,11}},
  // IV
  { 3, {0,5,9}},
  // IV7
  { 4, {0,4,5,9}},
  // V
  { 3, {2,7,11}},
  // V7
  { 4, {2,5,7,11}},
  // VI
  { 3, {0,4,9}},
  // VI7
  { 4, {0,4,7,9}},
  // II
  { 3, {2,5,9}},
  // III
  { 3, {4,7,11}}
},
//MINOR
{
  // I
  { 3, {0,3,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,3,7,10}},
  // IV
  { 3, {0,5,8}},
  // IV7
  { 4, {0,3,5,8}},
  // V
  { 3, {2,7,10}},
  // V7
  { 4, {2,5,7,10}},
  // VI
  { 3, {0,3,8}},
  // VI7
  { 4, {0,3,7,8}},
  // II
  { 3, {2,5,8}},
  // III
  { 3, {3,7,10}}
},
//DORIAN
{
  // I
  { 3, {0,3,7}},
  // IS2
  { 3, {0,2,7}},
  // IS4
  { 3, {0,5,7}},
  // I7
  { 4, {0,3,7,10}},
  // IV
  { 3, {0,5,9}},
  // IV7
  { 4, {0,3,5,9}},
  // V
  { 3, {2,7,10}},
  // V7
  { 4, {2,5,7,10}},
  // VI
  { 3, {0,3,9}},
  // VI7
  { 4, {0,3,7,9}},
  // II
  { 3, {2,5,9}},
  // III
  { 3, {3,7,10}}
}
};

#endif  // SCALES_CHORDS_H_