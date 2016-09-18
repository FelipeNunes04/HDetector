#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Ultrasonic.h>
#include <MFRC522.h>
#include <SPI.h>
#include "SIM900.h"
#include "sms.h"

SMSGSM sms;

const int rele_led = 24; //Led simbolizando a lampada (Relé 1)
const int rele_buzzer = 25;  //Pino ligado ao Buzzer (Relé 2)
const int rele_lcd = 13;  //Pino ligado ao LCD (Relé 3)
const int pinopir = 22; //Pino ligado ao sensor PIR
const int buzzer = 23; // Buzzer do RFID
const int ldrPin = 0; //LDR no pino analógico 0
int ldrValor = 0; //Valor lido do LDR
int acionamento = 0; //Valor do sensor de movimento
boolean movimento = false; //Flag que controla o loop do sensor de movimento
boolean smsFlag = false; //Flag que controla o loop do SMS conforme a presença de movimento     

SoftwareSerial mySerial(18, 19); // RX, TX

//Define os pinos para o trigger e echo (Sensor Ultrassonico)
#define pino_trigger 6
#define pino_echo 7
//Inicializa o sensor nos pinos definidos acima
Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Configuração de pinos do RFID
#define SS_PIN 53
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// Configuração de pinos do LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
char st[20];

void liga_desliga_GSMShield()
{
  Serial.print(F("Aguarde..."));
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  delay(1000);
  digitalWrite(6, HIGH);
  delay(1000);
  Serial.println(F("OK!"));
  digitalWrite(6, LOW);
  delay(500);
}

void mensageminicial()
{
  lcd.clear();
  lcd.print("Monitorando.....");    
  lcd.setCursor(0,1);
  lcd.print("Nada Detectado");
}

void lerRFID() 
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
    noTone(buzzer);
        
    Serial.println("Ola Chaveiro !");     
    desligarAlarme();    
  }
 
  if (conteudo.substring(1) == "6B 05 EE 75") //UID 2 - Cartao
  {
    //Ligando o buzzer com uma frequencia de 1500 hz.
    tone(buzzer,1500);   
    delay(500);
    noTone(buzzer);
    
    Serial.println("Ola Cartao !");  
    desligarAlarme();  
  }
} 

void desligarAlarme(){
  //Altera o estado do sensor de movimento
  digitalWrite(pinopir, LOW); 
  movimento = false;

  //Desliga os Relés
  digitalWrite(rele_led,HIGH);    
  digitalWrite(rele_buzzer,HIGH);  

  //Altera a mensagem do LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alarme Desligado");  

  delay(1000);
  mensageminicial();
}

void enviarSMS(){
  Serial.println("Enviando SMS...");
  lcd.clear();  
  lcd.print("Movimento Detec.");    
  lcd.setCursor(0,1);
  lcd.print("Enviando SMS...");
            
  //Envia um SMS para o numero selecionado. Formato sms.SendSMS(<numero>,<mensagem>)
  if (sms.SendSMS("02189999722963", "ALERTA!!! Movimento Detectado..."))
    Serial.println("SMS enviado!");          
  
  smsFlag = false;                          
}

// Função que desliga o LCD quando fechar a tampa da caixa
void monitoraLCD(){
  //Le as informacoes do sensor em cm
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  
  //Exibe informacoes no serial monitor
  Serial.print("Distancia em cm: ");
  Serial.print(cmMsec);
  Serial.print(" ");
  
  if(cmMsec <= 5.0){
    // Turn off the display:
    digitalWrite(rele_lcd, HIGH);    
  }else{
    digitalWrite(rele_lcd, LOW);        
  }  
}

void setup()
{
    Serial.begin(9600);
    mySerial.begin(9600);
    Serial.print("Ligando shield GSM SIM900. ");
    liga_desliga_GSMShield();
    Serial.println("Testando GSM Shield...");
    //Comunicacao com o Shield GSM a 2400 bauds
    if (gsm.begin(9600)){            
      Serial.println("\nstatus=READY");            
    }else Serial.println("\nstatus=IDLE");
  
    pinMode(pinopir, INPUT); //Define pino sensor como entrada     
    pinMode(rele_led, OUTPUT); //Led indicador de movimento
    pinMode(rele_buzzer,OUTPUT);  // buzzer do PIR   
    pinMode(rele_lcd,OUTPUT);  // Vcc do LCD   
    pinMode(buzzer,OUTPUT);
        
    //Estado inicial dos reles - desligados
    digitalWrite(rele_led, HIGH);
    digitalWrite(rele_buzzer, HIGH);    
    digitalWrite(rele_lcd, HIGH);    
    
    SPI.begin();      // Inicia  SPI bus
    mfrc522.PCD_Init();   // Inicia MFRC522
    lcd.begin(16, 2); // Inicia display LCD
    mensageminicial(); 
}
 
void loop(){
  monitoraLCD();
  ///ler o valor do LDR
  ldrValor = analogRead(ldrPin); //O valor lido será entre 0 e 1023
  Serial.print("Valor LDR: ");
  Serial.println(ldrValor);

  acionamento = digitalRead(pinopir); //Le o valor do sensor PIR  

  // Se ocorrer Movimento 
  if (acionamento == HIGH) {    
    movimento = true;
    smsFlag = true;      
    while(movimento){
      monitoraLCD();
      Serial.println("Movimento!!");          
      lcd.clear();
      lcd.print("Movimento Detec.");    
      lcd.setCursor(0,1);  
      lcd.print("Alarme Ativado!");
      //se o valor lido for maior que 500, liga o led e aciona o buzzer
      if (ldrValor>= 500) {
        //Ligando o buzzer e o led
        digitalWrite(rele_led,LOW);   
        digitalWrite(rele_buzzer,LOW);
               
        if(smsFlag){
          enviarSMS();
        }
      } else{      
        //Ligando o buzzer
        digitalWrite(rele_buzzer,LOW);       
        if(smsFlag){
          enviarSMS();                    
        }                                        
      } 
      delay(1000);
      lerRFID();          
    }       
  }         
  delay(3000);    
}
