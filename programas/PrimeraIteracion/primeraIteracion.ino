
#include <Crypto.h>
#include <Speck.h>
#include <SpeckSmall.h>
#include <string.h>

SpeckSmall cifrador;
int timer1;
struct cipherBlock
{
    byte key[32]= {0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
    byte plaintext[32];
    byte ciphertext[32];
};

cipherBlock block;
String buff;
int temperature = 25;
int humidity = 81;

int hour = 16;
int minute = 25;
int sec = 12;


ISR(TIMER1_OVF_vect){
  TCNT1 = timer1; // preload timer

  //time count
  sec++;
  if (sec == 60){
    sec = 0;
    minute++;
    if(minute == 60){
       minute = 0;
       hour++;
        if(hour == 24){
          hour = 0;
        }
      }
    }
    
  buff = String(hour);
  buff.concat(":");
  buff.concat(String(minute));
  buff.concat(":");
  buff.concat(String(sec));
  buff.concat(",S1.T=");
  buff.concat(String(temperature));
  buff.concat(",S2.H=");
  buff.concat(String(humidity));

  Serial.print("Buffer: ");
  Serial.println(buff);
  //Pasaje de buffer a bloque del cifrador
  for(int i = 0; i<buff.length();i++){
    block.plaintext[i] = buff[i];    
  }
  
// Codificacion
  Serial.print("Codificacion de: ");
  for(int i = 0; i<sizeof(block.plaintext);i++){
    Serial.print(block.plaintext[i]);
  }
  Serial.println();

  cifrador.setKey(block.key, sizeof(block.key));
  cifrador.encryptBlock(block.ciphertext, block.plaintext);
  
  Serial.print("Resultado: ");
  for(int i = 0; i<sizeof(block.ciphertext);i++){
    Serial.print(block.ciphertext[i],HEX);
  }
  Serial.println();

// Decodificacion
  Serial.print("Decodificion: ");
  cifrador.decryptBlock(block.plaintext, block.ciphertext);
  
  for(int i = 0; i<sizeof(block.plaintext);i++){
    Serial.print(block.plaintext[i],HEX);
  }
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
}

void setup(){
  
// initialize timer1
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0; 
  // Set timer1_counter to the correct value for our interrupt interval
  timer1 = 65535;   // preload timer 65536-16MHz/256/2Hz
  TCNT1 = timer1;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts(); // enable all interrupts

// Initialize serial port
	Serial.begin(9600);
	Serial.println();


}

void loop()
{
  
}
