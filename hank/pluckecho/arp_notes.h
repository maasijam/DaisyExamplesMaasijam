namespace arps {
    
// direction - to be implemented later
enum direction {
    up = 0,
    down,
    upUp,
    downDown,
    upDown,
    downUp,
    random,
    LAST_DIR
};
// 0 scale
const float noMajTri[3] = {0, 4, 7};
const float noMinTri[3] = {0, 3, 7};
const float noMajPen[5] = {0, 2, 4, 5, 7};
const float noMinPen[5] = {0, 2, 3, 5, 7};

enum chordname {
    I,
    IS2,
    IS4,
    I7,
    IV,
    IV7,
    V,
    V7,
    VI,
    VI7,
    II,
    III
};

struct Chord {
  int num_notes;
  int notes[12];  
};

const Chord chords[] = {
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
  { 3, {4,7,11}},
};

// triples
//const float majTri[3] = {48.0f, 52.0f, 55.0f};
//const float minTri[3] = {48.0f, 51.0f, 55.0f};
//
// fifths
//const float majFif[5] = {48.0f, 50.0f, 52.0f, 53.0f, 55.0f};
//const float minFif[5] = {48.0f, 50.0f, 51.0f, 53.0f, 55.0f};

} // namespace arps
