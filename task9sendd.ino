//TASK 9 – SENDER (SPECK32/64 in CTR mode)


#include <stdint.h>

static const uint8_t ROUNDS = 22;

//SPECK32/64 rotate helpers (16-bit words) 
static inline uint16_t ROR16(uint16_t x, uint8_t r) {
  return (uint16_t)((x >> r) | (x << (16 - r)));
}
static inline uint16_t ROL16(uint16_t x, uint8_t r) {
  return (uint16_t)((x << r) | (x >> (16 - r)));
}

//Master key (64-bit) = 4 x 16-bit words 
// MUST match receiver.
static const uint16_t MASTER_KEY[4] = {
  0x0011, 0x2233, 0x4455, 0x6677
};

//Round keys (22 x 16-bit)
static uint16_t RK[ROUNDS];

//Key schedule for SPECK32/64 (word size 16, m=4, alpha=7, beta=2)
static void speck32_64_key_schedule(const uint16_t K[4], uint16_t rk[ROUNDS]) {
  uint16_t l0 = K[0], l1 = K[1], l2 = K[2];
  rk[0] = K[3];

  for (uint8_t i = 0; i < (ROUNDS - 1); i++) {
    uint16_t new_l = (uint16_t)((ROR16(l0, 7) + rk[i]) ^ i);
    rk[i + 1] = (uint16_t)(ROL16(rk[i], 2) ^ new_l);

    l0 = l1;
    l1 = l2;
    l2 = new_l;
  }
}

//Encrypt one 32-bit block (x,y are 16-bit words)
static void speck32_encrypt(uint16_t &x, uint16_t &y, const uint16_t rk[ROUNDS]) {
  for (uint8_t i = 0; i < ROUNDS; i++) {
    x = (uint16_t)((ROR16(x, 7) + y) ^ rk[i]);
    y = (uint16_t)(ROL16(y, 2) ^ x);
  }
}

//CTR
static uint32_t ctr;         //32-bit counter block
static uint8_t  ks[4];       //4 keystream bytes per block
static uint8_t  ks_idx = 4;  //4 means "empty"

//Refill 4-byte keystream by encrypting counter block, then ctr++
static void refill_keystream() {
  uint16_t x = (uint16_t)(ctr >> 16);
  uint16_t y = (uint16_t)(ctr & 0xFFFF);

  speck32_encrypt(x, y, RK);

  uint32_t out = ((uint32_t)x << 16) | y;

  ks[0] = (uint8_t)(out & 0xFF);
  ks[1] = (uint8_t)((out >> 8) & 0xFF);
  ks[2] = (uint8_t)((out >> 16) & 0xFF);
  ks[3] = (uint8_t)((out >> 24) & 0xFF);

  ks_idx = 0;
  ctr++;
}

static uint8_t stream_xor(uint8_t b) {
  if (ks_idx >= 4) refill_keystream();
  return (uint8_t)(b ^ ks[ks_idx++]);
}

//Send SYNC + 32-bit nonce so receiver starts aligned
static void send_sync(uint32_t nonce) {
  Serial.write('S'); Serial.write('Y'); Serial.write('N'); Serial.write('C');
  Serial.write((uint8_t)(nonce & 0xFF));
  Serial.write((uint8_t)((nonce >> 8) & 0xFF));
  Serial.write((uint8_t)((nonce >> 16) & 0xFF));
  Serial.write((uint8_t)((nonce >> 24) & 0xFF));
}

void setup() {
  Serial.begin(9600);

  speck32_64_key_schedule(MASTER_KEY, RK);

  // Make a per-run nonce (leave A0 floating for some noise, or set a fixed value)
  uint32_t nonce = ((uint32_t)micros() << 16) ^ (uint32_t)analogRead(A0);

  ctr = nonce;        //CTR starts from nonce
  ks_idx = 4;         //force refill on first byte

  delay(500);         //give receiver time to boot
  send_sync(nonce);
}

void loop() {
  static uint8_t plain = 0;

  uint8_t cipher = stream_xor(plain);

  Serial.write(cipher);  //one byte per UART frame
  plain++;               //0..255

  delay(100);           
}
