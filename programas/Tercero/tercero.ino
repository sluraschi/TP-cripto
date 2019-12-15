#include <Crypto.h>
#include <CryptoLW.h>
#include <Curve25519.h>
#include <Ascon128.h>
#include <RNG.h>
#include <string.h>


struct cipherBlock
{
  uint8_t key[32];
  uint8_t plaintext[32];
  uint8_t ciphertext[32];
  uint8_t authdata[17] = {0x48, 0x6f, 0x77, 0x20, 0x6e, 0x6f, 0x77, 0x20,
                          0x62, 0x72, 0x6f, 0x77, 0x6e, 0x20, 0x63, 0x6f,
                          0x77
                         };
  uint8_t iv[16] = {0xbc, 0x52, 0x27, 0xa5, 0x72, 0x58, 0xfe, 0x00,
                    0xcb, 0x7b, 0x0f, 0x31, 0xa4, 0xb6, 0xff, 0xda
                   };
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

void printNumber(const uint8_t *x, size_t size)
{
  static const char hexchars[] = "0123456789ABCDEF";
  for (uint8_t posn = 0; posn < size; ++posn) {
    Serial.print(hexchars[(x[posn] >> 4) & 0x0F]);
    Serial.print(hexchars[x[posn] & 0x0F]);
  }
  Serial.println();
}


void DiffieSharedKey(uint8_t *shared)
{
  static uint8_t alice_k[32];
  static uint8_t alice_f[32];
  static uint8_t bob_k[32];
  static uint8_t bob_f[32];

  Serial.println("Se generan las claves publicas y privadas");
  Serial.flush();
  Curve25519::dh1(alice_k, alice_f);
  Serial.print("Pu1 = ");
  printNumber(alice_k, sizeof(alice_k));
  Serial.print("Pr1 = ");
  printNumber(alice_f, sizeof(alice_f));
  Serial.println();
  Curve25519::dh1(bob_k, bob_f);
  Serial.print("Pu2 = ");
  printNumber(bob_k, sizeof(alice_k));
  Serial.print("Pr2 = ");
  printNumber(bob_f, sizeof(alice_k));
  Serial.println();
  Serial.println("Se genera la clave compartida de ambos metodos. Tienen que ser iguales");
  Serial.flush();
  Curve25519::dh2(bob_k, alice_f);
  Curve25519::dh2(alice_k, bob_f);
  Serial.print("Shared1 = ");
  printNumber(bob_k, sizeof(alice_k));
  Serial.print("Shared2 = ");
  printNumber(alice_k, sizeof(alice_k));
  
  Serial.println();
  Serial.println("Se utiliza la clave compartida en algun cifrador. En este caso se utilizo Ascorn128");
  Serial.println();
}


static uint8_t sharedKey[32];

ISR(TIMER1_OVF_vect) {


  //simulacion de tiempo
  sec++;
  if (sec == 60) {
    sec = 0;
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
      if (hour == 24) {
        hour = 0;
      }
    }
  }

  //generar buffer
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
  Serial.println();

  //Pasaje de buffer a bloque del cifrador
  for (int i = 0; i < sizeof(block.plaintext); i++) {
    if (i < buff.length()) {
      block.plaintext[i] = buff[i];
    }
    else {
      block.plaintext[i] = 0;
    }
  }





  // Codificacion
  Serial.print("Codificacion de: ");
  printNumber(block.plaintext, sizeof(block.plaintext));

  cifrador.setKey(block.key, 16);
  cifrador.setIV(block.iv, 16);

  // se agregan los datos a autenticar
  cifrador.addAuthData(block.authdata, sizeof(block.authdata));

  cifrador.encrypt(block.ciphertext, block.plaintext, 32);

  //computo del tag
  cifrador.computeTag(block.tag, sizeof(block.tag));
  Serial.println();
  // se imprime el resultado
  Serial.println("Resultado: ");
  Serial.print("   Texto cifrado:");
  printNumber(block.ciphertext, sizeof(block.ciphertext));
  Serial.print("   Tag:");
  printNumber(block.tag, sizeof(block.tag));
  Serial.println();


  cifrador.clear();

  // Decodificacion
  Serial.print("Decodificion: ");
  cifrador.setKey(block.key, 16);
  cifrador.setIV(block.iv, 16);

  // se agregan los datos a autenticar
  cifrador.addAuthData(block.authdata, sizeof(block.authdata));

  cifrador.decrypt(block.plaintext, block.ciphertext, 32);
  printNumber(block.plaintext, sizeof(block.plaintext));
  Serial.println();

  // Checkeo del tag
  Serial.print("Tag autenticado: ");
  if (cifrador.checkTag(block.tag, sizeof(block.tag))) {
    Serial.println("Bien ");
  } else {
    Serial.println("Mal ");
  }



  Serial.println();
  Serial.println("--------------------------------");
  Serial.println();
  delay(500);
  TCNT1 = timer1; // resetea el timer
}

void setup()
{
  Serial.begin(9600);

  RNG.begin("Semilla de prueba");

  // Generacion de la llave del canal

  DiffieSharedKey(sharedKey);

  // Se fija la clave compartida
  for (int i = 0; i < sizeof(block.key); i++) {
    block.key[i] = sharedKey[i];
  }

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


}

void loop()
{

}
