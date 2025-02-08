// cl /nologo stoi.c && stoi.exe 

#include <stdio.h>

unsigned long long stoi(char string[8]) {
  // NOTE(ziv): I don't do any safety checks NOT PRODUCTION CODE
  unsigned long long *value_as_int = (unsigned long long  *)string; 
  unsigned long long integer = *value_as_int;

  // use printf("%016llx\n", integer); to see intermediate states
                                                          // intermediate (decimal) | weird value
  integer = integer & 0x0f0f0f0f0f0f0f0f;                 // 8 7 6 5 4 3 2 1        |
  integer = integer * 2561 >> 8 & 0x00ff00ff00ff00ff;     //     78 56 34 12        | (1<<8)*10+1     = 2561
  integer = integer * 6553601 >> 16 & 0x0000ffff0000ffff; //     5678   1234        | (1<<16)*100+1   = 6553601 
  integer = integer * 42949672960001 >> 32;               //        12345678        | (1<<32)*10000+1 = 42949672960001 
  return integer;
}

int main() {
  char value[8] = "12345678";
  printf("%lld\n", stoi(value));
  return 0;
}

