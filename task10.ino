//TASK 10


#include <stdint.h>

//given encrypted bytes
const uint8_t cipher[] = {
  87, 26, 72, 13, 67,
  95, 89, 23, 72, 22,
  73, 11, 87, 30, 73
};
const uint8_t cipherLen = sizeof(cipher);

//Buffer for decrypted candidate plaintext
char plaintext[cipherLen + 1];


//Check if plaintext looks like two lowercase words
bool isTwoLowercaseWords(const char *s) {
  int spaces = 0;

  for (uint8_t i = 0; i < cipherLen; i++) {
    char c = s[i];

    if (c == ' ') {
      spaces++;
    }
    else if (c < 'a' || c > 'z') {
      return false;
    }
  }

  if (spaces != 1) return false;
  if (s[0] == ' ' || s[cipherLen - 1] == ' ') return false;

  return true;
}


//Main brute-force
void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println(F("Starting brute-force of two XOR keys...\n"));

  unsigned long startTotal = millis();   //total time start

  //try all 256×256 key combinations
  for (uint16_t key0 = 0; key0 < 256; key0++) {
    for (uint16_t key1 = 0; key1 < 256; key1++) {

      for (uint8_t i = 0; i < cipherLen; i++) {
        uint8_t useKey = (i & 1) ? key1 : key0;   //this method is faster than %2
        plaintext[i] = (char)(cipher[i] ^ useKey);
      }
      plaintext[cipherLen] = '\0';

      if (isTwoLowercaseWords(plaintext)) {
        unsigned long elapsed = millis() - startTotal;

        Serial.println(F("POSSIBLE MATCH FOUND"));
        Serial.print(F("key0 = "));
        Serial.print(key0);
        Serial.print(F(" (0x"));
        Serial.print(key0, HEX);
        Serial.println(F(")"));

        Serial.print(F("key1 = "));
        Serial.print(key1);
        Serial.print(F(" (0x"));
        Serial.print(key1, HEX);
        Serial.println(F(")"));

        Serial.print(F("Plaintext: \""));
        Serial.print(plaintext);
        Serial.println(F("\""));

        Serial.print(F("Time taken until match (ms): "));
        Serial.println(elapsed);
        Serial.println(F("\n"));
      }
    }
  }

  unsigned long totalElapsed = millis() - startTotal;   //gives final total time

  Serial.println(F("Brute force finished."));
  Serial.print(F("TOTAL EXECUTION TIME (ms): "));
  Serial.println(totalElapsed);
}

void loop() {

}
