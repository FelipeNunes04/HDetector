/*
 * Programa executado no Arduino Mega 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

const int rele1 = 40; //Led no pino 13 (Relé 1)
const int rele2 = 32;  //Pino ligado ao Buzzer (Relé 2)
const int buzzer = 8; // Buzzer do RFID
const int pinopir = 22; //Pino ligado ao sensor PIR
const int ldrPin = 0; //LDR no pino analógico 0
int ldrValor = 0; //Valor lido do LDR
int acionamento = 0; //Valor do sensor de movimento
boolean movimento = false;
int recByte;

#define SS_PIN 53
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
 
LiquidCrystal lcd(6, 7, 5, 4, 3, 2); 
char st[20];

void mensageminicial()
{
  lcd.clear();
  lcd.print(" Aproxime o seu");  
  lcd.setCursor(0,1);
  lcd.print("cartao do leitor");  
}

void desligarSensores() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();
  if (conteudo.substring(1) == "75 E6 76 50") //UID 1 - Chaveiro
  {
    //Ligando o buzzer com uma frequencia de 1500 hz.
    tone(buzzer,1500);   
    delay(500);
        
    Serial.println("Ola Chaveiro !");
    Serial.println();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ola Chaveiro !");
    lcd.setCursor(0,1);
    lcd.print("Sensores OFF!");
    
    //DESLIGANDO SENSORES PIR
    digitalWrite(pinopir, LOW);    
    movimento = false;
    digitalWrite(rele1,HIGH);    
    digitalWrite(rele2,HIGH);  
    
    //Desligando o buzzer.
    noTone(buzzer);
    
    delay(2000);
    mensageminicial();
  }
 
  if (conteudo.substring(1) == "6B 05 EE 75") //UID 2 - Cartao
  {
    //Ligando o buzzer com uma frequencia de 1500 hz.
    tone(buzzer,1500);   
    delay(500);
    
    Serial.println("Ola Cartao !");
    Serial.println();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ola Cartao !");
    lcd.setCursor(0,1);
    lcd.print("Sensores OFF !");

    //DESLIGANDO SENSORES PIR
    digitalWrite(pinopir, LOW);    
    movimento = false;
    digitalWrite(rele1,HIGH);    
    digitalWrite(rele2,HIGH);  
    
    //Desligando o buzzer.
    noTone(buzzer);
    
    delay(2000);
    mensageminicial();
  }
} 
 
void setup()
{
     pinMode(pinopir, INPUT); //Define pino sensor como entrada     
     pinMode(rele1, OUTPUT); //Led indicador de movimento
     pinMode(rele2,OUTPUT);  // buzzer do PIR   
     Serial.begin(9600);
     //Estado inicial dos reles - desligados
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);    
    pinMode(buzzer,OUTPUT);

    SPI.begin();      // Inicia  SPI bus
    mfrc522.PCD_Init();   // Inicia MFRC522
    lcd.begin(16, 2); 
    mensageminicial(); 
}
 
void loop()
{
  ///ler o valor do LDR
  ldrValor = analogRead(ldrPin); //O valor lido será entre 0 e 1023
  Serial.print("Valor LDR: ");
  Serial.println(ldrValor);

  acionamento = digitalRead(pinopir); //Le o valor do sensor PIR  
    
  if (acionamento == HIGH) // SE Movimento
  {    
    movimento = true;
        
    while(movimento){
      Serial.println("Movimento!!");    
      boolean sms = true;
      //se o valor lido for maior que 500, liga o led e aciona o buzzer
      if (ldrValor>= 500) {
        //Ligando o buzzer e o led
        digitalWrite(rele2,LOW);   
        digitalWrite(rele1,LOW);   
        if(sms){
          Serial.write('1');           
          if(Serial.available() > 0){
            recByte = Serial.read();        
            if(recByte == '1'){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("SMS enviado !");
              Serial.println("SMS enviado !");
              sms = false;              
              delay(2000);
              mensageminicial();
            }                                        
          }
        }        
        delay(2000);
      } else{      
        //Ligando o buzzer
        digitalWrite(rele2,LOW);       
        if(sms){
          Serial.write('1');
            if(Serial.available() > 0){
            recByte = Serial.read();        
            if(recByte == '1'){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("SMS enviado !");
              Serial.println("SMS enviado !");
              sms = false;              
              delay(2000);
              mensageminicial();
            }                                        
          }        
        }        
        delay(2000);
      }  
      desligarSensores();
    }     
  }
  delay(2000);    
}
