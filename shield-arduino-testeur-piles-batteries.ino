// Mesure de la tension et/ou de la capacité d'un accumulteur
// https://tutoduino.fr/
// Copyleft 2020

// Librairie pour l'afficheur OLED
// https://github.com/greiman/SSD1306Ascii
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define I2C_ADDRESS 0x3C

SSD1306AsciiAvrI2c oled;

// Boolean qui indique si la configuration 
// par le menu est finie ou non
bool configOk = false;

const uint8_t brocheBouton = 2;

bool etatBouton = false;

#define MESURE_TENSION 0
#define MESURE_CAPACITE 1

#define NIMH  0
#define LIPO  1
#define LIION 2
#define ALCA  3

uint8_t mesure;
uint8_t accu;

// Valeur de la resistance de decharge
#define R 26.3

// Seuil bas de la tension des accumulateurs
// (piles et batteries)
#define SEUIL_BAS_TENSION_ACCU_NIMH     0.80
#define SEUIL_BAS_TENSION_ACCU_ALCA     0.00
#define SEUIL_BAS_TENSION_ACCU_LIION    2.00
#define SEUIL_BAS_TENSION_ACCU_LIPO     2.00

// Détection de seuil bas de la tension de l'accumulateur
bool seuilBasAccuAtteint = false;
float seuilBasAccu = 0;

// Heure de début de la mesure
unsigned long initTime;

// Quantité d'électricité générée par l'accumulateur
// lors de la dernière seconde
float quantiteElectricite = 0;

// Quantité d'électricité totale générée par l'accumulateur 
// depuis le lancement du programme
float quantiteElectriciteTotale = 0;


// Verifie si le bouton est appuye
// pendant le temps d'attente specifie
// Le temps d'attente est un multiple de 
// 50 ms
bool appuiBouton() {
  uint8_t cpt=40;
  
  while (cpt != 0) {
    // verifie si le bouton est appuyé pendant 
    // au moins 50 millisecondes afin d'éviter
    // un etat instable
    if (digitalRead(brocheBouton) == 1) {
      delay(50);
      if (digitalRead(brocheBouton) == 1) {
        // le bouton a bien ete appuye pendant 
        // le temps d'attente
        // on attend que le bouton soit relache
        while (digitalRead(brocheBouton) == 1) {
          delay(100);
        }
        return true;
      }
    }
    // si le bouton n'a pas ete appuye
    // attend 50 ms et teste le bouton
    // de nouveau tant que le temps total
    // d'attente n'est pas ecoule
    delay(50);
    cpt = cpt - 1;  
  }

  // le bouton n'a pas ete appuye pendant
  // le temps d'attente
  return false;
}

bool menuTypeMesure() {
  oled.clear();
  oled.set2X();
  oled.println("Config");
  oled.set1X();
  oled.println("");
  oled.println("Mesure tension ?");
  oled.println();
  oled.println("oui -> Appuyez");  
  if (appuiBouton() == true) {
    mesure = MESURE_TENSION;
    return true;
  }

  oled.clear();
  oled.set2X();
  oled.println("Config");
  oled.set1X();
  oled.println("");  
  oled.println("Mesure capacite ?");
  oled.println();
  oled.println("oui -> Appuyez");  
  if (appuiBouton() == true) {
    mesure = MESURE_CAPACITE;
    return true;
  }
  
  return false;

}

bool menuAccu() {

  oled.clear();
  oled.set2X();
  oled.println("Config");
  oled.set1X();
  oled.println("");  
  oled.println("Pile Alcaline ?");
  oled.println();
  oled.println("oui -> Appuyez");   
  if (appuiBouton() == true) {
    accu = ALCA;
    seuilBasAccu = SEUIL_BAS_TENSION_ACCU_ALCA;    
    return true;
  }
  
  oled.clear();
  oled.set2X();  
  oled.println("Config");
  oled.set1X();
  oled.println("");  
  oled.println("Accu Ni-MH ?");
  oled.println();
  oled.println("oui -> Appuyez");   
  if (appuiBouton() == true) {
    accu = NIMH;
    seuilBasAccu = SEUIL_BAS_TENSION_ACCU_NIMH;    
    return true;
  }

  oled.clear();
  oled.set2X();
  oled.println("Config");
  oled.set1X();
  oled.println("");  
  oled.println("Accu Li-Ion ?");
  oled.println();
  oled.println("oui -> Appuyez");   
  if (appuiBouton() == true) {
    accu = LIION;
    seuilBasAccu = SEUIL_BAS_TENSION_ACCU_LIION;    
    return true;
  }

  oled.clear();
  oled.set2X();
  oled.println("Config");
  oled.set1X();
  oled.println("");  
  oled.println("Accu Li-Po ?");
  oled.println();
  oled.println("oui -> Appuyez");  
  if (appuiBouton() == true) {
    accu = LIPO;
    seuilBasAccu = SEUIL_BAS_TENSION_ACCU_LIPO;
    return true;
  }
  
  return false;

}


void configMenu() {

  seuilBasAccuAtteint = false;

  while (menuTypeMesure() != true) {
    delay(100);
  } 

  while (menuAccu() != true) {
    delay(100);
  } 
 
  oled.clear();
  oled.set2X();
  oled.println("Validez !");
  oled.set1X();
  if (mesure == MESURE_TENSION) {
    oled.println("Mesure tension");
  } else {
    oled.println("Mesure capacite");
    // La pile doit debiter du courant
    digitalWrite(7, HIGH);
  }
 switch (accu) {
   case NIMH:
      oled.println("Ni-MH");
      break;  
   case LIPO:
      oled.println("Li-Po");
      break;
   case LIION:
      oled.println("Li-Ion");
      break;  
   case ALCA:
      oled.println("Alcaline");
      break;        
    default:
      break;
   }   

  oled.println("");
  oled.println("ok -> Appuyez");  

  // attente de l'appui sur le bouton
  // pour pouvoir lancer la mesure
  while (appuiBouton() == false) {
  }
  
  // La configuration est finie
  configOk = true;
}


// Fonction qui doit être appellée toutes les secondes
// La fonction mesure la tension aux bornes de la résistance
// et en déduit le courant qui y circule. 
// Elle mesure la capacité de la pile en additionnant
// le courant de décharges toutes les secondes jusqu'à ce
// que la pile soit totalement déchargée
void mesureQuantiteElectricite() {
  float U0,U1;
  float tensionResistance;
  float courant;
  
  // Lit la tension aux bornes de la pile et aux bornes 
  // de la résistance
  U0 = 2*analogRead(A0)*2.5/1023.0;
  U1 = 2*analogRead(A1)*2.5/1023.0;
  
  tensionResistance = U0-U1;
  Serial.println("tensionResistance="+String(tensionResistance));
  
  // Verifie que la tension de la pile n'est
  // pas inferieure à son seuil bas afin de ne pas 
  // l'endommager
  if (U0 < seuilBasAccu) {
    seuilBasAccuAtteint = true;
  }

  // Calcul du courant qui circule dans la résistance
  courant = (U0-U1)/R;

  // La quantité d'électricité est la quantité de charges électriques
  // déplacées par les électrons. Elle est égale à l'intensité
  // multipliée par le temps (1 seconde ici).
  // Afin d'avoir la quantité d'électricité en mAh, il faut
  // utiliser le facteur 1000/3600
  quantiteElectricite = courant/3.6;

  // On additionne cette quantité d'électricité aux précédentes
  // afin de connaître la quantité totale d'électricité
  // à la fin de la mesure
  quantiteElectriciteTotale = quantiteElectriciteTotale+quantiteElectricite;

  // Affichage sur l'afficheur OLED de la tension de la pile, 
  // du courant débité ainsi que de la quantité d'électricité débitée 
  // par la pile depuis le lancement du programme
  oled.clear();
  oled.set2X();
  oled.println("Capacite");
  oled.set1X();
  oled.println("U = "+String(U0)+" V");
  oled.println("I = "+String(courant)+" A");
  oled.println("Q = "+String(quantiteElectriciteTotale)+" mAh");

  delay(1000);
 
}

void mesureCapacite() {
  // Mesure de la quantité d'électricité débitée toutes les 
  // secondes jusqu'à ce que la tension de l'accu soit inférieure
  // à son seuil bas afin de ne pas l'endommager.
  if (seuilBasAccuAtteint == false) {
    mesureQuantiteElectricite();
  } else {
    // Une fois que le seuil bas de l'accu est atteint, 
    // on positionne le transistor en mode bloqué afin
    // d'arrêter la décharge de l'accu et on affiche
    // sa capacite
    digitalWrite(7, LOW);
    oled.println("Mesure terminee");
    oled.println("");
    oled.println("reset -> Appuyez");  
    
    // attente de l'appui sur le bouton
    // pour pouvoir relancer le programme
    while (appuiBouton() == false) {
    }
    configOk = false;
    }
}

void mesureTension() {
  float tensionVide;
  float tensionCharge;
 
  // Lecture de la tension à vide de la pile
  // Bloque le transistor en positionnant la sortie D7 à LOW
  digitalWrite(7, LOW);
  delay(1000);
  
  tensionVide = 2*analogRead(A0)*2.5/1023;
  Serial.println("tensionVide="+String(tensionVide));
  delay(500);

  // Lecture de la tension en charge de la pile
  // Sature le transistor en positionnant la sortie D7 à HIGH
  digitalWrite(7, HIGH);
  delay(1000);

  tensionCharge = 2*analogRead(A0)*2.5/1023;
  Serial.println("tensionCharge="+String(tensionCharge));
  delay(500);

  // Arrete la decharge de la pile
  digitalWrite(7, LOW);

  oled.clear();
  oled.set2X();
  oled.println("Tension");
  oled.set1X();  
  oled.println("U_vide = "+String(tensionVide)+" V");
  oled.println("U_charge = "+String(tensionCharge)+" V");
  oled.println("");
  oled.println("reset -> Appuyez");  
  
  // attente de l'appui sur le bouton
  // pour pouvoir relancer le programme
  while (appuiBouton() == false) {
  }
  configOk = false;
}

void setup() {
  
  Serial.begin(9600);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);  
  oled.clear();
  oled.set2X();
  oled.println("Tutoduino");
  oled.set1X();
  oled.println("Apprendre");
  oled.println("l'electronique");
  oled.println("avec un Arduino");

  delay(2000);
  
  // Positionne la référence de tension 
  // sur la référence externe à 2,5 V
  analogReference(EXTERNAL);
  
  pinMode(brocheBouton, INPUT_PULLUP);

  // La sortie D7 sera utilisé pour contrôler 
  // l'état du transistor
  pinMode(7, OUTPUT);
  delay(100);

  // Passe le transistor dans son état bloqué
  // afin de ne pas décharger l'accumulateur
  digitalWrite(7, LOW);  

}
void loop() {

  // Affiche le menu de configuration lors de la première 
  // boucle (une fois la configuration terminee le flag
  // configOk passe à true 
  if (configOk == false) {
    configMenu();
  }
  
  if (mesure == MESURE_CAPACITE) {
    mesureCapacite();
  } else {
    mesureTension();
  }

}
