// Magic Bitboard Generator
// Based on the algorithm by Tord Romstad (Stockfish)
// MIT License – see LICENSE file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define USE_32_BIT_MULTIPLICATIONS

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

typedef unsigned long long uint64;

typedef struct square_result
{
  uint64 magic;
  int shift;
} square_result;

typedef struct result
{
  square_result rook[64];
  square_result bishop[64];
} result;

uint64 random_uint64()
{
  uint64 u1, u2, u3, u4;
  u1 = (uint64)(random()) & 0xFFFF;
  u2 = (uint64)(random()) & 0xFFFF;
  u3 = (uint64)(random()) & 0xFFFF;
  u4 = (uint64)(random()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64 random_uint64_fewbits()
{
  return random_uint64() & random_uint64() & random_uint64();
}

int count_1s(uint64 b)
{
  int r;
  for (r = 0; b; r++, b &= b - 1)
    ;
  return r;
}

const int BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8};

int pop_1st_bit(uint64 *bb)
{
  uint64 b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

uint64 index_to_uint64(int index, int bits, uint64 m)
{
  int i, j;
  uint64 result = 0ULL;
  for (i = 0; i < bits; i++)
  {
    j = pop_1st_bit(&m);
    if (index & (1 << i))
      result |= (1ULL << j);
  }
  return result;
}

uint64 rmask(int sq)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1; r <= 6; r++)
    result |= (1ULL << (fl + r * 8));
  for (r = rk - 1; r >= 1; r--)
    result |= (1ULL << (fl + r * 8));
  for (f = fl + 1; f <= 6; f++)
    result |= (1ULL << (f + rk * 8));
  for (f = fl - 1; f >= 1; f--)
    result |= (1ULL << (f + rk * 8));
  return result;
}

uint64 bmask(int sq)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++)
    result |= (1ULL << (f + r * 8));
  for (r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--)
    result |= (1ULL << (f + r * 8));
  for (r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++)
    result |= (1ULL << (f + r * 8));
  for (r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--)
    result |= (1ULL << (f + r * 8));
  return result;
}

uint64 ratt(int sq, uint64 block)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1; r <= 7; r++)
  {
    result |= (1ULL << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
      break;
  }
  for (r = rk - 1; r >= 0; r--)
  {
    result |= (1ULL << (fl + r * 8));
    if (block & (1ULL << (fl + r * 8)))
      break;
  }
  for (f = fl + 1; f <= 7; f++)
  {
    result |= (1ULL << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
      break;
  }
  for (f = fl - 1; f >= 0; f--)
  {
    result |= (1ULL << (f + rk * 8));
    if (block & (1ULL << (f + rk * 8)))
      break;
  }
  return result;
}

uint64 batt(int sq, uint64 block)
{
  uint64 result = 0ULL;
  int rk = sq / 8, fl = sq % 8, r, f;
  for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
  {
    result |= (1ULL << (f + r * 8));
    if (block & (1ULL << (f + r * 8)))
      break;
  }
  return result;
}

int transform(uint64 b, uint64 magic, int bits)
{
#if defined(USE_32_BIT_MULTIPLICATIONS)
  return (unsigned)((int)b * (int)magic ^ (int)(b >> 32) * (int)(magic >> 32)) >> (32 - bits);
#else
  return (int)((b * magic) >> (64 - bits));
#endif
}

uint64 surrounding(int square) {
    int rank = square / 8;
    int file = square % 8;

    uint64 result = 0ULL;

    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0)
                continue;

            int r = rank + dr;
            int f = file + df;

            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                result |= 1ULL << (r * 8 + f);
            }
        }
    }

    return result;
}

square_result find_magic(int sq, int m, int bishop)
{
  square_result r;
  uint64 mask, b[4096], a[4096], used[4096], magic;
  int i, j, k, n, fail;

  mask = bishop ? bmask(sq) : rmask(sq);
  n = count_1s(mask);

  for (i = 0; i < (1 << n); i++)
  {
    b[i] = index_to_uint64(i, n, mask);
    a[i] = bishop ? batt(sq, b[i]) : ratt(sq, b[i]);
  }
  for (k = 0; k < 100000000; k++)
  {
    magic = random_uint64_fewbits();
    if (count_1s((mask * magic) & 0xFF00000000000000ULL) < 6)
      continue;
    for (i = 0; i < 4096; i++)
      used[i] = 0ULL;
    for (i = 0, fail = 0; !fail && i < (1 << n); i++)
    {
      j = transform(b[i], magic, m);
      if (used[j] == 0ULL)
        used[j] = a[i];
      else if (used[j] != a[i])
        fail = 1;
    }

    int use = count_1s(mask & ~surrounding(sq));

    int shift = 64 - use;

    r = (square_result){magic, shift};

    if (!fail)
      return r;
  }
  printf("***Failed***\n");
  return r;
}

int RBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12};

int BBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6};

DLL_EXPORT result GetMagics()
{
  int square;
  result final;
  for (square = 0; square < 64; square++)
  {
    final.rook[square] = find_magic(square, RBits[square], 0);
    final.bishop[square] = find_magic(square, BBits[square], 1);
    // printf("  0x%llxULL >> %d,\n", r.magic, r.shift);
  } 
  return final;
}

int main()
{
  int square;

  FILE *rooks = fopen("rook_magics.bin", "wb");
  FILE *rookshifts = fopen("rook_shifts.bin", "wb");
  FILE *bishops = fopen("bishop_magics.bin", "wb");
  FILE *bishopshifts = fopen("bishop_shifts.bin", "wb");
  if (!rooks || !bishops || !rookshifts || !bishopshifts)
  {
    perror("Failed to open output files");
    return 1;
  }

  printf("const uint64 RMagic[64] = {\n");
  for (square = 0; square < 64; square++)
  {
    square_result r = find_magic(square, RBits[square], 0);
    printf("  0x%llxULL >> %d,\n", r.magic, r.shift);
    fprintf(rooks, "%llx\n", r.magic);
    fprintf(rookshifts, "%d\n", r.shift);
  }
  printf("};\n\n");

  printf("const uint64 BMagic[64] = {\n");
  for (square = 0; square < 64; square++)
  {
    square_result r = find_magic(square, BBits[square], 1);
    printf("  0x%llxULL >> %d,\n", r.magic, r.shift);
    fprintf(bishops, "%llx\n", r.magic);
    fprintf(bishopshifts, "%d\n", r.shift);
  }
  printf("};\n\n");

  fclose(rooks);
  fclose(bishops);

  return 0;
}