#include "SIM900.h"
#include <SoftwareSerial.h>
#include "call.h"
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

CallGSM call;

const int rele1 = 24; //Led simbolizando a lampada (Relé 1)
const int rele2 = 25;  //Pino ligado ao Buzzer (Relé 2)
const int pinopir = 22; //Pino ligado ao sensor PIR
const int buzzer = 23; // Buzzer do RFID
const int ldrPin = 0; //LDR no pino analógico 0
int ldrValor = 0; //Valor lido do LDR
int acionamento = 0; //Valor do sensor de movimento
boolean movimento = false; //Flag que controla o loop do sensor de movimento
boolean chamada = false; //Flag que controla o loop da chamada telefonica conforme a presença de movimento     

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
  digitalWrite(rele1,HIGH);    
  digitalWrite(rele2,HIGH);  

  //Altera a mensagem do LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alarme Desligado");  

  delay(2000);
  mensageminicial();
}

void realizarChamada(){
  Serial.println("Discando...");
  lcd.clear();  
  lcd.print("Movimento Detec.");    
  lcd.setCursor(0,1);
  lcd.print("Discando...");
            
  //Efetua a chamada formato call.Call(<numero a ser chamado>)
  call.Call("03189999722963");
  delay(2000);
          
  Serial.println("Chamada Efetuada!");
            
  call.HangUp();                        
  chamada = false;                          
}

void setup()
{
    Serial.begin(9600);
    Serial.print("Ligando shield GSM SIM900. ");
    liga_desliga_GSMShield();
    Serial.println("Testando GSM Shield...");
    //Comunicacao com o Shield GSM a 2400 bauds
    if (gsm.begin(2400))
      Serial.println("nstatus=READY");
    else Serial.println("nstatus=IDLE");
  
    pinMode(pinopir, INPUT); //Define pino sensor como entrada     
    pinMode(rele1, OUTPUT); //Led indicador de movimento
    pinMode(rele2,OUTPUT);  // buzzer do PIR   
    pinMode(buzzer,OUTPUT);
        
    //Estado inicial dos reles - desligados
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);    
    
    SPI.begin();      // Inicia  SPI bus
    mfrc522.PCD_Init();   // Inicia MFRC522
    lcd.begin(16, 2); 
    mensageminicial(); 
}
 
void loop(){
  ///ler o valor do LDR
  ldrValor = analogRead(ldrPin); //O valor lido será entre 0 e 1023
  Serial.print("Valor LDR: ");
  Serial.println(ldrValor);

  acionamento = digitalRead(pinopir); //Le o valor do sensor PIR  

  // Se ocorrer Movimento 
  if (acionamento == HIGH) {    
    movimento = true;
    chamada = true;      
    while(movimento){
      Serial.println("Movimento!!");          
      lcd.clear();
      lcd.print("Movimento Detec.");    
      lcd.setCursor(0,1);  
      lcd.print("Alarme Ativado!");
      //se o valor lido for maior que 500, liga o led e aciona o buzzer
      if (ldrValor>= 500) {
        //Ligando o buzzer e o led
        digitalWrite(rele1,LOW);   
        digitalWrite(rele2,LOW);
               
        if(chamada){
          realizarChamada();
        }
      } else{      
        //Ligando o buzzer
        digitalWrite(rele2,LOW);       
        if(chamada){
          realizarChamada();                    
        }                                        
      } 
      delay(1000);
      lerRFID();          
    }       
  }         
  delay(3000);    
}
