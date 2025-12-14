#include <stdint.h>
const uint8_t SECRET_KEY = 0x54;  
// 4-bit S-box
const uint8_t SBOX[16] = {
  6, 4,12, 5,
  0, 7, 2,14,
  1,15, 3,13,
  8,10, 9,11
};


uint8_t permute(uint8_t x) {
  uint8_t y = 0;

  y |= ((x >> 3) & 0x01) << 7; 
  y |= ((x >> 5) & 0x01) << 6; 
  y |= ((x >> 1) & 0x01) << 5; 
  y |= ((x >> 0) & 0x01) << 4; 
  y |= ((x >> 7) & 0x01) << 3; 
  y |= ((x >> 2) & 0x01) << 2; 
  y |= ((x >> 4) & 0x01) << 1; 
  y |= ((x >> 6) & 0x01) << 0; 

  return y;
}

uint8_t encryptByte(uint8_t p) {
  uint8_t x = p ^ SECRET_KEY;
  uint8_t hi = (x >> 4) & 0x0F;
  uint8_t lo =  x       & 0x0F;

  uint8_t shi = SBOX[hi];
  uint8_t slo = SBOX[lo];

  uint8_t s = (shi << 4) | slo;

  return permute(s);
}

void setup() {
  Serial.begin(9600);
}

const uint8_t TEST_VALUE = 42;

void loop() {
  //send the plaintext byte without encryption
  Serial.write(TEST_VALUE);      
  delay(100);
}
