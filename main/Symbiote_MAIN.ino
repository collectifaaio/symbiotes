#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MedianFilter.h>   // https://github.com/daPhoosa/MedianFilter
#include <RunningAverage.h> // https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage

#include "Symbiote.h" //

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A9));  //passer la fonction random en réelle fonction aléatoire.

  AudioMemory(70);             // Audio connections require memory to work.  For more detailed information, see the MemoryAndCpuUsage example
  // setup audio board
  SoundFile = String();
  SoundType = String(".wav");
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.7);                         //REGLAGE
  sgtl5000_1.micGain(50);

  //Setup SD Card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(1000);
    }
  }
  //Compte le nombre de fichiers dans chacun des dossiers d'humeur.

  for (int i = 0; i <= 3; i++) {
    String FileRemove = HumeurFolder[i] + "/" + "DS_STO~1";
    SD.remove(FileRemove.c_str());
    String FolderName = HumeurFolder[i] + "/";
    root = SD.open(FolderName.c_str());
    NbFiles[i] = CountingFile(root);
    //    Serial.printf("Le nombre de fichiers dans le dossier %s est de : %d\n", HumeurFolder[i], NbFiles[i]);
  }

  // Calibration Proximètre et Microphone
  CalibProxiMic();
}

// >>>>> MAIN LOOP <<<<<

void loop() {
  // LECTURE DES DONNEES

  Proxi = random(60, 700);
  MedianProx.in(Proxi);
  ProxiMedian = MedianProx.out();

  //Micro Moyenne

  MicroRms = random(15, 70);
  MicroRA.addValue(MicroRms);
  Micro_Moyenne = MicroRA.getAverage();

  // JAUGES PEAK

  // Jauge du nombre de peak toutes les x secondes
  if (Micro_Moyenne > ThreshPeak) {
    DetectPeak = true;
  }
  else {
    DetectPeak = false;
  }

  jaugePeak = JaugePeak(DetectPeak, TP_STOCKAGE_PEAK);

  // SELECTIONNE LA BONNE  HUMEUR ET LES BONNES PLAGES DE RANDOM RETRIG

  // Serein
  if (jaugePeak < SeuilJaugeMicro && ProxiMedian > SeuilProxi) {
    if (Condition != 1) {
      corpusSampleNumber = NbFiles[0] ;          //Met à jour le nombre total de fichiers disponible dans le dossier serein
      randomMin = 4000;                              // REGLAGE
      randomMin = 15000;                             // REGLAGE
      selecthumeur = 0 ;
      threshTrig = random(randomMin, randomMax);
      Condition = 1;
    }
  }

  // Hostile
  if (jaugePeak < SeuilJaugeMicro && ProxiMedian < SeuilProxi) {
    if (Condition != 2) {
      corpusSampleNumber = NbFiles[3];          //Met à jour le nombre total de fichiers disponible dans le dossier timide
      randomMin = 100;                              // REGLAGE
      randomMin = 800;                              // REGLAGE
      selecthumeur = 3;
      threshTrig = random(randomMin, randomMax);
      Condition = 2;
    }
  }

  // Hilare
  if (jaugePeak > SeuilJaugeMicro && ProxiMedian > SeuilProxi) {
    if (Condition != 3) {
      corpusSampleNumber = NbFiles[2];         //Met à jour le nombre total de fichiers disponible dans le dossier hilare
      randomMin = 800;                              // REGLAGE
      randomMin = 3000;                             // REGLAGE
      selecthumeur = 2;
      threshTrig = random(randomMin, randomMax);
      Condition = 3;
    }
  }

  // Timide
  if (jaugePeak > SeuilJaugeMicro && ProxiMedian < SeuilProxi) {
    if (Condition != 4) {
      corpusSampleNumber = NbFiles[1];                     //Met à jour le nombre total de fichiers disponible dans le dossier hostile
      randomMin = 15000;                              // REGLAGE
      randomMin = 30000;                              // REGLAGE
      selecthumeur = 1;
      threshTrig = random(randomMin, randomMax);
      Condition = 4;
    }
  }

  // PLAYER AUDIO

  // Random Retrig

  if (millis() - resetMillis > threshTrig) {
    resetMillis = millis();
    sample_rand = URN(corpusSampleNumber);             // TIRAGE ALEATOIRE SANS REDECLENCHER LE MEME SON.
    SoundFile = HumeurFolder[selecthumeur] + "/" + sample_rand + SoundType;  //inttochar
    TrigFile(SoundFile.c_str());                                //JOUE LE fichier après un temps tiré aléatoirement définie dans une fourchette qui varie selon la somme des deux capteurs. .c_str() passe un string en char (en gros...)
    threshTrig = random(randomMin, randomMax);
  }
  delay(500);
}

//-------------

//FONCTIONS

//-------------

// CALIBRATION PROXI & MICRO

void CalibProxiMic() {

  uint32_t InitTimerCalib = millis();
  Serial.println(" >>>>>>>>>>> Début de Calibration <<<<<<<<<< ");

  while (millis() - InitTimerCalib < TEMPS_CALIB) {

    MicroRms = random(15, 70);
    MicroRA.addValue(MicroRms);
    Micro_Moyenne = MicroRA.getAverage();

    Proxi = random(60, 700);

    // record the minimum Proxi Value
    if (Proxi < ProxiMin) {
      ProxiMin = Proxi;
    }
    // record the maximum Proxi value
    if (Proxi > ProxiMax) {
      ProxiMax = Proxi;
    }
    // record the minimum Micro Value
    if (Micro_Moyenne < MicroMin) {
      MicroMin = Micro_Moyenne;
    }
    delay(10);
  }

  if (ProxiMin > (int)(ProxiMax / 2)) {
    ProxiMin = ProxiMax / 2 ;
  }

  SeuilProxi = (int)(ProxiMax / 2);
  ThreshPeak = MicroMin * 3;

  Serial.print("ProxiMin & ProxiMax : ");
  Serial.print(ProxiMin);
  Serial.print(" & ");
  Serial.println(ProxiMax);
  Serial.print("MicroMin : ");
  Serial.print(MicroMin);
  Serial.print("  !!  Seuil Proxi : ");
  Serial.print(SeuilProxi);
  Serial.print("  !!  ThresPeak : ");
  Serial.println(ThreshPeak);
  Serial.println(" >>>>>>>>>>> Fin de Calibration <<<<<<<<<< ");
}

//-------------

// COMPTAGE DU NOMBRE DE FICHIERS PAR DOSSIER

int CountingFile (File DirName) {
  int FileNumber = 0;

  while (true) {
    File entry =  DirName.openNextFile();
    Serial.println(entry.name());
    if (!entry) {
      break;
    } else {
      FileNumber++;
      entry.close();
    }
  }
  return FileNumber;
}

//-------------

// FONCTIONS JAUGE : COMPTE NB D'EVENEMENTS DANS UN INTERVALLE DE TEMPS DONNE ET L'INTERPOLE LINEAIREMENT PAR RAPPORT A LA VALEUR PRECEDENTE ET PENDANT L'INTERVALLE DE TEMPS DETERMINE.

float JaugePeak (bool DetectEvent, const int TimeWindow) {

  static int nbEvent = 0;
  static int arrivee = 0;
  static int depart = 0;
  static uint32_t Temps = 0;
  static bool LastDetectEvent = 0;
  static bool LastStockageEvent = 0;
  static bool StockageEvent = 0;

  if (DetectEvent != LastDetectEvent) { //Detection d'un front montant
    if (DetectEvent) {
      nbEvent++;  // Incrémentation du nombre de peak.
      LastDetectEvent = true;
    }
    else {
      LastDetectEvent = false;
    }
  }

  if ((millis() - Temps) > TimeWindow) {
    StockageEvent = true;
  } else {
    StockageEvent = false;
  }

  if (StockageEvent != LastStockageEvent) {
    if (StockageEvent) {
      depart = arrivee;
      arrivee = nbEvent; // On retourne le nombre d'évènements.
      Temps = millis();  //On remet le compteur de temps à 0 pour la prochaine boucle et avoir un snapshot régulier.
      nbEvent = 0; // On remet le nombre d'évènements à 0.
      LastStockageEvent = true;
    }
    else {
      LastStockageEvent = false;
    }
  }

  float coef = (arrivee - depart) / TimeWindow ;
  float Line = (coef * (millis() - Temps) + depart); // Interpolation linéaire (= Line pure data)
  return Line;
}

//-------------
//-------------

// FONCTION URN Thomas (tire aléatoirement un son qui n'a pas encore été joué)

int index_tab = 0; // nombre de samples deja jouées;
int sampleDejaJoues[100]; //Défini la taille du tableau et initialise toutes les valeurs à 0
int unplayedSample = 0;
int lastPlayedSample = -1;

int URN(int nbtotfile) {
  boolean etat = true;

  while (etat) {
    etat = false;
    unplayedSample = int(random(nbtotfile));
    for (int i = 0; i < index_tab; i++) {
      if (sampleDejaJoues[i] == unplayedSample) {
        etat = true;
        break;
      }
    }
    if (index_tab == 0 && unplayedSample == lastPlayedSample) {
      etat = true;
    }
  }

  if (index_tab < nbtotfile) {
    sampleDejaJoues[index_tab] = unplayedSample;
    index_tab ++;
  }

  if (index_tab == nbtotfile) {
    index_tab = 0;
    lastPlayedSample = unplayedSample;
    initSampleSelecteur(nbtotfile);
  }
  return unplayedSample + 1;
}

//Init Sample

void initSampleSelecteur(int nbtotfile) {
  for (int i = 0; i < nbtotfile; i++) {
    sampleDejaJoues[i] = -1;
  }
}

//-------------
//-------------

// TRIG UN FICHIER SON EN COUPANT LE PRECEDENT

void TrigFile(const char *filename) {
  Serial.println("");
  Serial.print(" >>>>>>>>>>>>>>> Playing file : ");
  Serial.print(filename);
  Serial.println(" <<<<<<<<<<<<<<< ");
  Serial.println("");
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playSdWav1.play(filename);
  // A brief delay for the library read WAV info
  delay(10);
}

