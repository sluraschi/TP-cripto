
#include <Crypto.h>
#include <CryptoLW.h>
#include <Ascon128.h>
#include <string.h>


struct cipherBlock
{
    uint8_t key[16] = {0x8a, 0xa5, 0xed, 0xc5, 0x88, 0x49, 0x75, 0xc8,
                    0xd1, 0xa1, 0xb8, 0x44, 0xd0, 0x15, 0x50, 0x5a};
    uint8_t plaintext[32];
    uint8_t ciphertext[32];
    uint8_t authdata[17];
    uint8_t iv[16] = {0xbc, 0x52, 0x27, 0xa5, 0x72, 0x58, 0xfe, 0x00,
                    0xcb, 0x7b, 0x0f, 0x31, 0xa4, 0xb6, 0xff, 0xda};
    uint8_t tag[16];
    size_t authsize;
    size_t datasize;
};

Ascon128 cifrador;
cipherBlock block;
int timer1;
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
  for(int i = 0; i<sizeof(block.plaintext);i++){
    if(i<buff.length()){
      block.plaintext[i] = buff[i];
    }
    else{
      block.plaintext[i] = 0;
    }
  }
  
// Codificacion

  Serial.print("Codificacion de: ");
  for(int i = 0; i<sizeof(block.plaintext);i++){
    Serial.print(block.plaintext[i],HEX);
  }
  Serial.println(",HEX");

  cifrador.setKey(block.key, 16);
  cifrador.setIV(block.iv, 16);
  cifrador.encrypt(block.ciphertext, block.plaintext, 32);
  
  Serial.print("Resultado: ");
  for(int i = 0; i<sizeof(block.ciphertext);i++){
    Serial.print(block.ciphertext[i],HEX);
  }
  Serial.println(",HEX");

// Decodificacion
  Serial.print("Decodificion: ");
  cifrador.setKey(block.key, 16);
  cifrador.setIV(block.iv, 16);
  cifrador.decrypt(block.plaintext, block.ciphertext, 32);
  
  for(int i = 0; i<sizeof(block.plaintext);i++){
    Serial.print(block.plaintext[i],HEX);
  }
  Serial.println(",HEX");
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
