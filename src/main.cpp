#include <Arduino.h>
#include <BALISE_LED.h>
#include <BALISE_BPandPOT.h>
#include <BALISE_PHOTODIODE.h>
#include <BALISE_10V.h>
#include <ESP32Time.h>
#include <BALISE_DISPLAY.h>
#include <Wire.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;


int pot1 = 39;
int pot_val1;
int pot2 = 36;
int pot_val2;

int current_time_B;
int previsous_time_B = 0;
int previous_time_BT = 0;

int BP_PRESS = 0;

char tab[150];

TaskHandle_t Task1; // "CORE"
TaskHandle_t Task2; // "CORE"

ESP32Time rtc(0); // "RTC" - offset in seconds

char trame[150];
char A;
int i = 0;
char valide[1];
int heure_GPS_measure, heures_GPS, minutes_GPS, secondes_GPS;
int date_GPS_measure, jour_GPS, mois_GPS, annee_GPS;
int heures_RTC, minutes_RTC, secondes_RTC;
int jour_RTC, mois_RTC, annee_RTC;
int verif_valide = 0;
int verif_trame = 0;
int current_time;
int previous_time = 0;
int GR;

void fonction_temps(void)
{

  current_time = millis();

  if (Serial1.available())
  {
    previous_time = current_time;
    verif_trame = 0;
    GR = 0;

    A = Serial1.read(); // recupere dans A les trames lues sur le Serial1

    if (A == '\n') // si fin de la trame
    {
      i++;
      trame[i] = '\0';

      if (trame[5] == 'C')
      {
        // valide fonction
        sscanf(trame, "%*s %*d.%*d %s", &valide);
        if (valide[0] == 'A')
        {
          verif_valide = 0;
        } // donnees valides
        else if (valide[0] == 'V')
        {
          verif_valide = 1;
        } // donnees invalides

        // heure fonction
        sscanf(trame, "%*s %d.%*d", &heure_GPS_measure);
        heures_GPS = (heure_GPS_measure / 10000) + 2;
        minutes_GPS = (heure_GPS_measure % 10000) / 100;
        secondes_GPS = (heure_GPS_measure % 100);
        // Serial.printf("GPS: %02dh %02dm %02ds\n", heures_GPS, minutes_GPS, secondes_GPS);

        // date fonction
        sscanf(trame, "%*s %*d.%*d %*s %*d.%*d %*s %*d.%*d %*s %*d.%*d %*d.%*d %d", &date_GPS_measure);
        jour_GPS = date_GPS_measure / 10000;
        mois_GPS = (date_GPS_measure % 10000) / 100;
        annee_GPS = (date_GPS_measure % 100);
        // Serial.printf("     %02d/%02d/20%02d\n", jour_GPS, mois_GPS, annee_GPS);
      }

      i = 0;
    }
    else // rempli trame et remplace ',' par ' '
    {
      trame[i] = A;
      if (trame[i] == ',')
      {
        trame[i] = ' ';
      }
      i++;
    }
  }

  if (current_time - previous_time > 3000)
  {
    verif_trame = 1;
  }

  if (verif_valide == 1 || verif_trame == 1)
  {
    GR = 1;
    struct tm timeinfo = rtc.getTimeStruct();

    heures_RTC = timeinfo.tm_hour;
    minutes_RTC = timeinfo.tm_min;
    secondes_RTC = timeinfo.tm_sec;

    jour_RTC = timeinfo.tm_mday;
    mois_RTC = timeinfo.tm_mon + 1;
    annee_RTC = timeinfo.tm_year + 1900;

    // Serial.printf("RTC: %02dh %02dm %02ds\n", heures_RTC, minutes_RTC, secondes_RTC);
    // Serial.printf("     %02d/%02d/%02d\n", jour_RTC, mois_RTC, annee_RTC);
  }
}

int cptr = 1;
int a = 1;

unsigned char text1[20];
unsigned char text2[20];
unsigned char text3[20];
unsigned char text4[20];
unsigned char text5[20];
unsigned char text6[20];
unsigned char text7[20];
unsigned char text8[20];
unsigned char text9[20];

int photodiode_val;
double ampli_val = 5.21; // en V (X.XX)

void fonction_affichage(void)
{

  if (VABP3() == true)
  {
    cptr = cptr + 1;
  }
  if (cptr == 4)
  {
    cptr = 1;
  }

  switch (a)
  {
  case 1:
    if (GR == 0)
    {
      setCursor(1, 1);
      sprintf((char *)text1, "GPS:%02dh%02dm%02ds", heures_GPS, minutes_GPS, secondes_GPS);
      show(text1);

      setCursor(2, 1);
      sprintf((char *)text2, "     %02d/%02d/20%02d\n", jour_GPS, mois_GPS, annee_GPS);
      show(text2);
    }
    else if (GR == 1)
    {
      setCursor(1, 1);
      sprintf((char *)text3, "RTC:%02dh%02dm%02ds", heures_RTC, minutes_RTC, secondes_RTC);
      show(text3);

      setCursor(2, 1);
      sprintf((char *)text4, "     %02d/%02d/%02d\n", jour_RTC, mois_RTC, annee_RTC);
      show(text4);
    }

    if (cptr == 2)
    {
      cleardisplay();
      a = 2;
    }
    break;

  case 2:

    if (photodiode_val <= 20)
    {
      setCursor(1, 1);
      sprintf((char *)text5, "PHOTO: %02dLUX nuit", photodiode_val);
      show(text5);
    }
    else if (photodiode_val > 20 && photodiode_val <= 88)
    {
      setCursor(1, 1);
      sprintf((char *)text6, "PHOTO: %02dLUX jour", photodiode_val);
      show(text6);
    }
    else if (photodiode_val > 88)
    {
      setCursor(1, 1);
      sprintf((char *)text6, "PHOTO: SATU_JOUR");
      show(text6);
    }

    setCursor(2, 1);
    sprintf((char *)text7, "AMPLI: %.2f V", ampli_val);
    show(text7);

    if (cptr == 3)
    {
      cleardisplay();
      a = 3;
    }
    break;

  case 3:
    setCursor(1, 1);
    sprintf((char *)text8, "ATTENTE: APP");
    show(text8);
    setCursor(2, 1);
    sprintf((char *)text9, "ATTENTE: WEB");
    show(text9);
    if (cptr == 1)
    {
      cleardisplay();
      a = 1;
    }
    break;
  }

  delay(20);
}

void task1Code(void *parameter)
{ // loop coeur0
  for (;;)
  {
    // Code de la premiere tache
    photodiode_val = Val_Photodiode(); // no delay
    AMPLI(ampli_val);                  // delay : 10ms
    fonction_affichage();              // delay : 100 ms (clear)



    current_time_B = millis();



    sprintf(tab, "%d,%d", pot_val1, pot_val2); // pot_val1 et pot_val2 dans le tableau

    if (Val_BP1() == 1 && BP_PRESS == 0)
    {
      previous_time_BT = current_time;
      BP_PRESS = 1;
    }
    else if (Val_BP1 == 0 && BP_PRESS == 1)
    {
      previous_time_BT = 0;
      BP_PRESS = 0;
    }

    if (current_time - previous_time_BT > 3000 && BP_PRESS == 1)
    {
      SerialBT.begin("Balise");
      BP_PRESS = 3;
    }

    if (current_time - previsous_time_B > 200)
    { // envoie les donees toutes les 200ms (clock BT 100ms)

      SerialBT.print(tab);

      previsous_time_B = current_time;
    }

    if (SerialBT.available())
    {
      // Lit la donnée reçue
      char commande = SerialBT.read();

      if (commande == '1')
      {
        Allumer_LED(); // Allume la LED
      }
      if (commande == '0')
      {
        Eteindre_LED(); // Éteint la LED
      }

      if (commande == '9')
      {
        Allumer_RGB_ROUGE();
      }
      if (commande == 'a')
      {
        Allumer_RGB_VERT();
      }
      if (commande == 'b')
      {
        Allumer_RGB_BLEU();
      }
      if (commande == '5')
      {
        Eteindre_LED();
      }
    }

    vTaskDelay(5 / portTICK_PERIOD_MS); // Pause 5 ms
  }
}

void task2Code(void *parameter)
{ // loop coeur1
  for (;;)
  {
    // Code de la deuxieme tache
    fonction_temps();                   // no delay
    vTaskDelay(5 / portTICK_PERIOD_MS); // Pause 5 ms
  }
}

void setup()
{
  Serial.begin(115200);
  Init_3BP();
  Init_LED();
  Init_AMPLI();
  Serial1.begin(9600, SERIAL_8N1, 19, 18); // GPS
  rtc.setTime(00, 23, 16, 29, 11, 2023);   // RTC initialisation date : s/m/h/j/m/a
  Init_DISPLAY();
  Wire.begin();
  delay(10);
  CiZ_init();
  delay(5);
  cleardisplay();
  delay(10);
  Eteindre_LED();
  Eteindre_RGB();

  xTaskCreatePinnedToCore(
      task1Code, // Fonction pour le loop du coeur 0
      "Task1",   // Nom de la tâche
      10000,     // Taille de la pile
      NULL,      // Paramètres de tâche
      1,         // Priorité (1er cœur)
      &Task1,    // Poignée de tâche
      0);        // Numéro du cœur (0 ou 1)

  xTaskCreatePinnedToCore(
      task2Code, // Fonction pour le loop du coeur 1
      "Task2",   // Nom de la tâche
      10000,     // Taille de la pile
      NULL,      // Paramètres de tâche
      1,         // Priorité (1er cœur)
      &Task2,    // Poignée de tâche
      1);        // Numéro du cœur (0 ou 1)
}

void loop()
{
  // VIDE
}
