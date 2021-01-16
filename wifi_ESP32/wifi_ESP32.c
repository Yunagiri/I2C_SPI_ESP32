/*
 * wifi_ESP32.c
 *
 * Copyright 2020 Viet Phuong DINH <viet_phuong@doussot>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <stdint.h>

#define SSID  "d206-IOT-AP"
#define PASSPHRASE "d2062019uttSY23"

#define RegTe 0x00
#define RegTh 0x01
#define RegConf 0x02
#define PmodHygroID 0x40

const int LED=LED0;
HTTPClient http;
uint16_t temperature, humidity = 0;

#define cs_alt 9 //definir le chip select

uint8_t statT, statP;
uint16_t TempLSB, TempMSB; //meme taille que la variable finale pour les decalage
uint32_t PLSB, Pinter, PMSB;
uint16_t Rtemp1, Rtemp2, RP1, RP2, RP3, Status;
int16_t temp;
int32_t pression;
float Temperature, Pression;


void AfficheParametres() {
  byte adrMAC[6];
  IPAddress ip,gw,masque;
  WiFi.macAddress(adrMAC);
  // tableau des valeurs de l'adress MAC
  for(int i=0;i<6;i+=1) {
    Serial.print(adrMAC[i],HEX);
    Serial.print(":");
  }
  Serial.println();
  Serial.print("ip : ");
  ip = WiFi.localIP();
  Serial.println(ip);
  Serial.print("passerelle : ");
  gw = WiFi.gatewayIP();
  Serial.println(gw);
  Serial.print("masque : ");
  masque = WiFi.subnetMask();
  Serial.println(masque);
}

void ConnexionWifi(char *ssid, const char *passphrase) {
  int etatled=0;
  Serial.print("connexion ");
  Serial.println(ssid);
  // initialise connexion
  WiFi.begin(ssid, passphrase);
  // attente connexion
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    etatled ^= 1; //etatled XOR 1
    digitalWrite(LED,etatled);
  }
  Serial.println("connexion OK");
  digitalWrite(LED,HIGH);
}

void ConnexionHTTPGET(const char* url) {
  Serial.print("connexion URL: ");
  Serial.println(url);
  http.begin(url);
  int httpCode = http.GET();
  Serial.print("httpCode = ");
  Serial.println(httpCode);
    if (httpCode) {
      if (httpCode == 200) {
        String reponse = http.getString();
        Serial.println(reponse);
      }
      else {
        Serial.print("erreur http ");
        Serial.println(httpCode);
      }
    }

  Serial.println("closing connection");
  http.end();
}

void ConnexionHTTPPOST(const char* url, String datas) {
  Serial.print("connexion URL: ");
  Serial.println(url);
  http.begin(url);
  int httpCode = http.POST(datas);
  Serial.print("httpCode = ");
  Serial.println(httpCode);
    if (httpCode) {
      if (httpCode == 200) {
        String reponse = http.getString();
        Serial.println(reponse);
      }
      else {
        Serial.print("erreur http ");
        Serial.println(httpCode);
      }
    }
  Serial.println("closing connection");
  http.end();
}

float getTemp(uint16_t temperature){
	float celsius;
	celsius = ((float)temperature/0x10000) * 165.0 - 40.0;
	return celsius;
}

float getHumidity(uint16_t humidity){
	float percent;
	percent = ((float) humidity/0x10000) * 100.0;
	return percent;
}

//Request data from the PmodHygro sensor. data can either be temperature or humidity.
uint16_t getData(uint8_t reg){
	Wire.beginTransmission(PmodHygroID);
	Wire.write(reg);
	Wire.endTransmission();
	uint16_t data = 0;
	Wire.requestFrom(PmodHygroID, 2);
	if (Wire.available()){
		data <<= 8;
		data = (uint16_t) Wire.read();  //Read received bytes, cast it into a uint16_t and stock in data
		data <<= 8; //Shift 8 bits to the left, most significant byte first.
		data |= (uint16_t) Wire.read(); //Cast the read byte into uint16_t and add it to the 8-shifted uint16_t already in data
	}
	return data;
}

float getPressionSPI(){
  digitalWrite(cs_alt,LOW);
  SPI.transfer(0x21); //registre de controle 3
  SPI.transfer(1);    //demande de mesure
  digitalWrite(cs_alt,HIGH);

  statT=0; //verifie sur la temperature est disponible
  while(statT!=1){
    digitalWrite(cs_alt,LOW);
    SPI.transfer(Status);
    statT=SPI.transfer(0);
    digitalWrite(cs_alt,HIGH);
    statT=statT & 0x01;
    }

  digitalWrite(cs_alt,LOW);
  SPI.transfer(Rtemp1); //recuperation donnees temperature
  TempLSB=SPI.transfer(0)& 0x00FF; //masquage pour etre sur que les autres bits sont a 0
  digitalWrite(cs_alt,HIGH);

  digitalWrite(cs_alt,LOW);
  SPI.transfer(Rtemp2);
  TempMSB=SPI.transfer(0)& 0x00FF;
  digitalWrite(cs_alt,HIGH);
  statP=0; //verifie sur la pression est disponible

  while(statP!=2){
     digitalWrite(cs_alt,LOW);
    SPI.transfer(Status);
    statP=SPI.transfer(0);
    statP=statP & 0x02;
    digitalWrite(cs_alt,HIGH);
    }
    digitalWrite(cs_alt,LOW);
  SPI.transfer(RP1); //recuperation donnees pression
  PLSB=SPI.transfer(0) & 0x000000FF;
  digitalWrite(cs_alt,HIGH);

     digitalWrite(cs_alt,LOW);
  SPI.transfer(RP2);
  Pinter=SPI.transfer(0)& 0x000000FF;
  digitalWrite(cs_alt,HIGH);

     digitalWrite(cs_alt,LOW);
  SPI.transfer(RP3);
  PMSB=SPI.transfer(0)& 0x000000FF;
  digitalWrite(cs_alt,HIGH);


  temp = TempMSB<<8 | TempLSB; //regroupement sur 16 bits
  pression = PMSB<<16 | Pinter<<8 | PLSB; //regroupement sur 32 bits
  Pression = ((float)pression)/4096.0; //calcul de la pression
  Temperature = ((float)temp/480.0)+42.5; //calcul de la temperature
  Serial.print("Température = ");
  Serial.println(Temperature);
  Serial.print("Pression = ");
  Serial.println(Pression);

  return Pression;

}
//Creates the GET request by concatenating the returns from the i2c function calls with the parameter strings.
String composeRequest(){
  // String surl ="http://10.23.12.199/capteurs/temphygro.php?id=43141";  //Given CGI script
  String surl = "http://10.23.12.187/cgi-bin/uvs/parsing.cgi?id=43141"; //Custom CGI script at individual VM
  String paramTemp = "&temperature=";
  String paramHum = "&hygrometrie=";
  String paramPress = "&pression=";

  float temp =  getTemp(getData(RegTe));   
  float hum =  getHumidity(getData(RegTh));
  float press = getPressionSPI();

  surl += paramTemp;
  surl += temp;
  surl += paramHum;
  surl += hum;
  surl += paramPress;
  surl += press;

  return surl;
}


void setup() {
  Serial.begin(9600);
  Wire.begin();

  //Separate temperature and humidity readings. 0x0000 for separate readings and 0x1000 for grouped
  Wire.beginTransmission(PmodHygroID);
  Wire.write(RegConf);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();

  Rtemp1 = 0x2B | 0x80; //Mode lecture registre temp 1
  Rtemp2 = 0x2C | 0x80; //Mode lecture registre temp 2
  RP1 = 0x28 | 0x80;    //Mode lecture registre pression 1
  RP2 = 0x29 | 0x80;    //Mode lecture registre pression 2
  RP3 = 0x2A | 0x80;    //Mode lecture registre pression 3
  Status = 0x27 | 0x80; //Mode lecture ragistre de status

  pinMode(cs_alt,OUTPUT);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setBitOrder(MSBFIRST);

  digitalWrite(cs_alt,LOW);
  SPI.transfer(0x10); //registre de control 1
  SPI.transfer(0x0A);
  digitalWrite(cs_alt,HIGH);

  digitalWrite(cs_alt,LOW);
  SPI.transfer(0x20); //registre de control 2
  SPI.transfer(0x80);
  digitalWrite(cs_alt,HIGH);

  ConnexionWifi((char*)SSID,PASSPHRASE);
  AfficheParametres();
  delay(1000);
}

void loop() {
  String surl = composeRequest();
  const char* url = surl.c_str(); //String to const char* conversion
  ConnexionHTTPGET(url);
  delay(2000);
}
