// Fonction playSong(): les mélodies sont stockées ici.
// Elles sont constituées des notes (NOTE_C3, etc.),
// et des rythmes (1 pour ronde, 2 pour blanche, 4 pour noire, etc.).
// Les deux listes doivent avoir le même nombre d'items.
// Les silences sont notés "0" dans la mélodie.

int melody_1[] = {
  NOTE_A3, 0, NOTE_A3, 0,
  NOTE_A3, 0, NOTE_A3, 0,
  NOTE_A3, 0, NOTE_A3, 0,
  NOTE_A3, 0, NOTE_A3, 0
};

int rythme_1[] = {
  16, 16, 16, 16,
  16, 16, 16, 16,
  16, 16, 16, 16,
  16, 16, 16, 16
};


int melody_2[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0
};
//Mario main them tempo
int rythme_2[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12
};