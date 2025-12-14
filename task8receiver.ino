//TASK 8 – FULL SPN DECRYPTION (RECEIVER)

#include <stdint.h>

const uint8_t SECRET_KEY = 0x54;

//Inverse S-box
const uint8_t INV_SBOX[16] = {
  4, 8, 6,10,
  1, 3, 0, 5,
 12,14,13,15,
   2,11, 7, 9
};

//Inverse of the bit permutation
uint8_t invPermute(uint8_t x) {
  uint8_t y = 0;

  y |= ((x >> 7) & 0x01) << 3; //S3 = C7
  y |= ((x >> 6) & 0x01) << 5; //S5 = C6
  y |= ((x >> 5) & 0x01) << 1; //S1 = C5
  y |= ((x >> 4) & 0x01) << 0; //S0 = C4
  y |= ((x >> 3) & 0x01) << 7; //S7 = C3
  y |= ((x >> 2) & 0x01) << 2; //S2 = C2
  y |= ((x >> 1) & 0x01) << 4; //S4 = C1
  y |= ((x >> 0) & 0x01) << 6; //S6 = C0

  return y;
}

uint8_t decryptByte(uint8_t c) {
  // undo permutation
  uint8_t s = invPermute(c);

  // undo S-box substitution
  uint8_t shi = (s >> 4) & 0x0F;
  uint8_t slo =  s       & 0x0F;

  uint8_t hi = INV_SBOX[shi];
  uint8_t lo = INV_SBOX[slo];

  uint8_t x = (hi << 4) | lo;

  // undo XOR key mixing
  return x ^ SECRET_KEY;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    uint8_t cipher = Serial.read();
    uint8_t plain  = decryptByte(cipher);

    Serial.print("Cipher: ");
    Serial.print(cipher);
    Serial.print("  Plain: ");
    Serial.println(plain);
  }
}
