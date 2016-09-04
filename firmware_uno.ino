//Progama : Envio SMS com o GSM Shield
//Alteracoes e adaptacoes : FILIPEFLOP
 
#include "SIM900.h"
#include <SoftwareSerial.h>
//Carrega a biblioteca SMS
#include "sms.h"
 
SMSGSM sms;
 
int numdata;
boolean started=false;
char smsbuffer[160];
char n[20];

int recByte;
int led = 13;

void setup()
{  
     //Inicializa a serial 
     Serial.begin(9600);
     pinMode(led,OUTPUT);
     
     Serial.println("Testando GSM shield...");
     //Inicia a configuracao do Shield
     if (gsm.begin(2400)) 
     {
       Serial.println("nstatus=READY");
       started=true;
     } 
     else Serial.println("nstatus=IDLE"); 
}
 
void loop(){
  if(Serial.available() > 0){
    recByte = Serial.read();  
  
    if(recByte == 0){
     digitalWrite(led,LOW);
     delay(100);
    }else if(recByte == 1){
      digitalWrite(led,HIGH);
      if(started) {
        //Envia um SMS para o numero selecionado. Formato sms.SendSMS(<numero>,<mensagem>)
        sms.SendSMS("04189994348161", "Arduino SMS");
        Serial.println(1);
        Serial.println("SMS enviado!");
      }    
      delay(100);
    }
  } 
}
