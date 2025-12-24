//task 9 – receiver (speck32/64 in ctr mode)


#include <stdint.h>

static const uint8_t ROUNDS = 22;   //number of rounds for speck32/64

//rotate-right 16-bit helper
static inline uint16_t ROR16(uint16_t x, uint8_t r) {
  return (uint16_t)((x >> r) | (x << (16 - r)));
}

//rotate-left 16-bit helper
static inline uint16_t ROL16(uint16_t x, uint8_t r) {
  return (uint16_t)((x << r) | (x >> (16 - r)));
}

//must match sender key
static const uint16_t MASTER_KEY[4] = {
  0x0011, 0x2233, 0x4455, 0x6677
};

//round keys storage
static uint16_t RK[ROUNDS];

//speck32/64 key schedule (same as sender)
static void speck32_64_key_schedule(const uint16_t K[4], uint16_t rk[ROUNDS]) {
  uint16_t l0 = K[0], l1 = K[1], l2 = K[2];
  rk[0] = K[3];

  for (uint8_t i = 0; i < (ROUNDS - 1); i++) {
    uint16_t new_l = (uint16_t)((ROR16(l0, 7) + rk[i]) ^ i);
    rk[i + 1] = (uint16_t)(ROL16(rk[i], 2) ^ new_l);

    //shift l registers
    l0 = l1;
    l1 = l2;
    l2 = new_l;
  }
}

//speck block encryption (32-bit block split into two 16-bit halves)
static void speck32_encrypt(uint16_t &x, uint16_t &y, const uint16_t rk[ROUNDS]) {
  for (uint8_t i = 0; i < ROUNDS; i++) {
    x = (uint16_t)((ROR16(x, 7) + y) ^ rk[i]);
    y = (uint16_t)(ROL16(y, 2) ^ x);
  }
}

//ctr mode state
static uint32_t ctr;        //counter / nonce
static uint8_t  ks[4];      //keystream buffer (32-bit block)
static uint8_t  ks_idx = 4; //current keystream index

//generate new keystream block by encrypting counter
static void refill_keystream() {
  uint16_t x = (uint16_t)(ctr >> 16);
  uint16_t y = (uint16_t)(ctr & 0xFFFF);

  speck32_encrypt(x, y, RK);

  uint32_t out = ((uint32_t)x << 16) | y;

  //split block into bytes
  ks[0] = (uint8_t)(out & 0xFF);
  ks[1] = (uint8_t)((out >> 8) & 0xFF);
  ks[2] = (uint8_t)((out >> 16) & 0xFF);
  ks[3] = (uint8_t)((out >> 24) & 0xFF);

  ks_idx = 0;
  ctr++;    //increment counter for ctr mode
}

//xor incoming byte with keystream
static uint8_t stream_xor(uint8_t b) {
  if (ks_idx >= 4) refill_keystream();
  return (uint8_t)(b ^ ks[ks_idx++]);
}

//wait for 's''y''n''c' then read 4-byte nonce
static bool try_read_sync(uint32_t &nonce_out) {
  static uint8_t st = 0;   //sync state machine

  while (Serial.available() > 0) {
    uint8_t c = (uint8_t)Serial.read();

    if (st == 0 && c == 'S') st = 1;
    else if (st == 1 && c == 'Y') st = 2;
    else if (st == 2 && c == 'N') st = 3;
    else if (st == 3 && c == 'C') {
      //read 4-byte nonce
      while (Serial.available() < 4) { /* wait */ }

      uint32_t n = 0;
      n |= (uint32_t)(uint8_t)Serial.read();
      n |= (uint32_t)(uint8_t)Serial.read() << 8;
      n |= (uint32_t)(uint8_t)Serial.read() << 16;
      n |= (uint32_t)(uint8_t)Serial.read() << 24;

      nonce_out = n;
      st = 0;
      return true;
    } else {
      st = 0; //reset on mismatch
    }
  }
  return false;
}

void setup() {
  Serial.begin(9600);                   //uart setup
  speck32_64_key_schedule(MASTER_KEY, RK);

  Serial.println("Waiting for SYNC...");
}

void loop() {
  static bool synced = false;

  //synchronization phase
  if (!synced) {
    uint32_t nonce;
    if (try_read_sync(nonce)) {
      ctr = nonce;
      ks_idx = 4;   //force keystream refill
      synced = true;
      Serial.print("Synced. Nonce = ");
      Serial.println(nonce);
    }
    return;
  }

  //decrypt incoming ciphertext bytes
  if (Serial.available() > 0) {
    uint8_t cipher = (uint8_t)Serial.read();
    uint8_t plain  = stream_xor(cipher);

    Serial.print("Cipher: ");
    Serial.print(cipher);
    Serial.print("  Plain: ");
    Serial.println(plain);
  }
}