#include "HX711.h"
#include <LCDWIKI_GUI.h> // Core graphics library
#include <LCDWIKI_KBV.h> // Hardware-specific library
#include <Adafruit_GFX.h>

// HX711 circuit wiring
const int broche_DT = 12;
const int broche_SCK = 10;

HX711 balance;

// Variables pour l'échelle
float facteur_echelle = 382; // Ajustez selon la calibration
float poids = 0.0;


// Configuration de l'écran LCD
LCDWIKI_KBV mylcd(ILI9486, A3, A2, A1, A0, A4); // Modèle, cs, cd, wr, rd, reset


// Pin du potentiomètre
#define POTENTIOMETER_PIN A5

// couleur
#define BLACK   0x0000  // Noir
#define WHITE   0xFFFF  // Blanc
#define RED     0xF800  // Rouge
#define GREEN   0x07E0  // Vert
#define BLUE    0x001F  // Bleu
#define YELLOW  0xFFE0  // Jaune
#define CYAN    0x07FF  // Cyan
#define MAGENTA 0xF81F  // Magenta
// Variables pour stocker les valeurs de l'énergie et du facteur CO2
int energie_maison = 6;
float facteur_CO2 = 1.0;
float CO2;
float CO2_emb=5.0;
float CO2_ope=3.0;
float CO2_elec=3;
float CO2_temp;
float gain;

int Qh_init=160;
int Qh_reno=5;
String systemName ="System ";
String isol ="- ";
String CECB="E";
// Variables pour stocker les valeurs du potentiomètre
int potValue;
int potZone;

  // Taille de l'écran
int screenWidth = mylcd.Get_Display_Width();
int screenHeight = mylcd.Get_Display_Height();

// Position des lignes centrales
int midX = screenWidth / 2;
int midY = screenHeight / 2;
int X0=6;
int Y0=8;
int dist=40;
  


void setup() {
  Serial.begin(9600);
  balance.begin(broche_DT, broche_SCK);
  balance.set_scale(-441); //calibration: le paramètre dépend de votre cellule de charge.
  delay(2400);
  balance.tare(); //ajustement du zéro
//  


  mylcd.Init_LCD();
  mylcd.Set_Rotation(1);
  mylcd.Fill_Screen(BLACK);
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_Size(3);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Back_colour(BLACK);

  mylcd.Print_String("La balance est prete!", 20, 50);
//  delay(9600);
  mylcd.Fill_Screen(BLACK); // Effacer l'écran
  screenWidth = mylcd.Get_Display_Width();
  screenHeight = mylcd.Get_Display_Height()-30;

// Position des lignes centrales
  midX = screenWidth / 2;
  midY = screenHeight / 2;
  draw_quadrants();

}

void loop() {

  calcul_value(); 

  if (CO2_ope!=CO2_temp){
    delay(400);
    calcul_value(); 
    mylcd.Fill_Screen(BLACK); // Effacer l'écran
    draw_quadrants();

//    mylcd.Print_String("Energie:", 20, 20);
    mylcd.Print_Number_Float(Qh_reno, 0, X0, Y0+dist, '.', 2, ' '); // Afficher avec 2 décimales
    mylcd.Print_String("kWh/m2", X0+60, Y0+dist);
    mylcd.Print_String(CECB, X0, Y0+dist+dist);


    mylcd.Print_Number_Float(gain, 2, midX+X0, Y0+dist, '.', 3, ' '); // Afficher avec 2 décimales
    mylcd.Print_String("kgCO2/m2.an",midX+X0, Y0+dist+dist);


//    mylcd.Print_String("CO2 oper.:", X0, 60);
    mylcd.Print_String(systemName, X0, midY+Y0+dist);
    mylcd.Print_Number_Float(CO2_ope, 2, X0, midY+Y0+dist+dist, '.', 3, ' '); // Afficher avec 2 décimales
    mylcd.Print_String("+", X0+90, midY+Y0+dist+dist);

    mylcd.Print_Number_Float(CO2_elec, 0, X0+115, midY+Y0+dist+dist, '.', 0, ' '); // Afficher avec 2 décimales

    mylcd.Print_String("kgCO2/m2.an", X0, midY+Y0+dist+dist+dist);

//    mylcd.Print_String("CO2 gris:", 20, 100);
    mylcd.Print_String(isol,  midX+X0, midY+Y0+dist);
    mylcd.Print_Number_Float(CO2_emb, 2, midX+X0, midY+Y0+dist+dist, '.', 3, ' '); // Afficher avec 2 décimales

    mylcd.Print_String("kgCO2/m2.an", midX+X0, midY+Y0+dist+dist+dist);


//    mylcd.Print_String("CO2 total:", 20, 140);
//    mylcd.Print_Number_Float(CO2_emb+CO2_ope, 2, 200, 140, '.', 3, ' '); // Afficher avec 2 décimales
//    mylcd.Print_String("kgCO2/m2.an", 350, 140);



//    mylcd.Print_String(systemName, 20, 220);

    CO2_temp=CO2_ope;}

  delay(1500); // Délai pour actualisation

}

void draw_quadrants() 
{

  // Dessiner les lignes pour diviser l'écran
  mylcd.Set_Draw_color(WHITE);
  mylcd.Draw_Fast_VLine(midX, 0, screenHeight+50); // Ligne verticale
  mylcd.Draw_Fast_HLine(0, midY, screenWidth); // Ligne horizontale
  

  // Écrire les titres dans chaque quadrant
  mylcd.Print_String("Eff. envel.", X0, Y0); // Quadrant en haut à gauche

  mylcd.Print_String("Gain", midX + X0, Y0); // Quadrant en haut à droite

  mylcd.Print_String("Em. directes", X0, midY + Y0); // Quadrant en bas à gauche

  mylcd.Print_String("Carbone gris", midX + X0, midY + Y0); // Quadrant en bas à droite
}

void calcul_value() 
{

  // Lecture de la valeur du potentiomètre
  potValue = analogRead(POTENTIOMETER_PIN);
  
  // Détermination de la zone du potentiomètre
  if (potValue < 55) {
    potZone = 1; 
    systemName="Pellet ";
    facteur_CO2 = 0.038;
  } else if (potValue < 258) { 
    potZone = 2;
    systemName="Bois";
    facteur_CO2 = 0.033;
  } else if (potValue < 410) { 
    potZone = 3;
    systemName="Mazout";
    facteur_CO2 = 0.343;  
  } else if (potValue < 669) { 
    potZone = 4;
    systemName="Gaz";
    facteur_CO2 = 0.234;
  } else if (potValue < 862) { 
    potZone = 5; // 
    systemName="Electrique";
    facteur_CO2 = 0.125;
  } else if (potValue < 1023) { 
    potZone = 6; // 
    systemName="Pac geo";
    facteur_CO2 = 0.041;
  } else {
    potZone = 7; //
    systemName="Pac air/eau";
    facteur_CO2 = 0.054;
  }


  poids = balance.get_units(5); // Lire la moyenne de 5 mesures
  if (poids < 4) {
    CO2_emb=0;
    Qh_reno=160;
    CECB="cat. F";
    isol="-";
  } else if (poids < 7.8) {
    CO2_emb=0.16; // 5cm d'EPS
    Qh_reno=107;
    CECB="cat. D";
    isol="5cm d'EPS";
  } else if (poids < 14.1) {
    CO2_emb=0.32; // 10cm d'EPS
    Qh_reno=96;
    CECB="cat. C";
    isol="10cm d'EPS";
  } else if (poids < 21) {
    CO2_emb=0.64; // 20cm d'EPS
    Qh_reno=90;
    CECB="cat. C";
    isol="20cm d'EPS";
  } else if (poids < 27) {
    CO2_emb=0.96; // 30cm d'EPS
    Qh_reno=87;
    CECB="cat. C";
    isol="30cm d'EPS";
  } else if (poids < 32.5) {
    CO2_emb=0.35; //20 de laine de roche
    Qh_reno=91;
    CECB="cat. C";
    isol="20cm l. roche";
  } else if (poids < 41.7) {
    CO2_emb=2.7; // 20cm d'XPS
    Qh_reno=89;
    CECB="cat. C";
    isol="20cm d'XPS";  
  } else {
    CO2_emb=4.0; // 20cm d'XPS
    Qh_reno=86;
    CECB="cat. C";
    isol="30cm d'XPS";
  }

  // Calcul de l'empreinte carbone
  CO2_ope = Qh_reno * facteur_CO2;
  gain=facteur_CO2*(160-Qh_reno)-CO2_emb;
}
