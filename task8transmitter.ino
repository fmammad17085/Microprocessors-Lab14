//TASK 8 SENDER


#include <stdint.h>

const uint8_t SECRET_KEY = 0x54;  //8-bit key

//4-bit S-box
const uint8_t SBOX[16] = {
  6, 4,12, 5,
  0, 7, 2,14,
  1,15, 3,13,
  8,10, 9,11
};

//Bit permutation used after S-box layer
uint8_t permute(uint8_t x) {
  uint8_t y = 0;

  y |= ((x >> 3) & 0x01) << 7; //C7 = S3
  y |= ((x >> 5) & 0x01) << 6; //C6 = S5
  y |= ((x >> 1) & 0x01) << 5; //C5 = S1
  y |= ((x >> 0) & 0x01) << 4; //C4 = S0
  y |= ((x >> 7) & 0x01) << 3; //C3 = S7
  y |= ((x >> 2) & 0x01) << 2; //C2 = S2
  y |= ((x >> 4) & 0x01) << 1; //C1 = S4
  y |= ((x >> 6) & 0x01) << 0; //C0 = S6

  return y;
}

uint8_t encryptByte(uint8_t p) {
  //XOR with key
  uint8_t x = p ^ SECRET_KEY;

  //S-box substitution
  uint8_t hi = (x >> 4) & 0x0F;
  uint8_t lo =  x       & 0x0F;

  uint8_t shi = SBOX[hi];
  uint8_t slo = SBOX[lo];

  uint8_t s = (shi << 4) | slo;

  //bit permutation layer
  return permute(s);
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  static uint8_t counter = 0;

  uint8_t cipher = encryptByte(counter);

  Serial.write(cipher);  //send encrypted byte
  counter++;

  delay(100); //0.1 sec
}
