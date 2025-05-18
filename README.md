# rmlib

This is an independent C program for generating magic numbers and shift values used in **magic bitboard move generation**, based on an algorithm originally devised by **Tord Romstad**, one of the creators of [Stockfish](https://stockfishchess.org/).

The generated values are intended for use in high-performance chess engines, allowing fast computation of sliding piece (rook and bishop) attack sets.

---

## Build Instructions

This project builds a **shared library** (`.so` or `.dll`) that can be called from C# or other languages supporting native interop.

### Linux

Install GCC if not already installed, then run:

```bash
make linux
```

### Windows

Install mingw-w64 if not already installed, then run:

```bash
make windows
```

## C Function Signature
```c
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
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

EXPORT result GetMagics(void);
```

## C# Usage Example
```csharp
[StructLayout(LayoutKind.Sequential)]
public struct SquareResult
{
    public ulong Magic;
    public int Shift;
}

[StructLayout(LayoutKind.Sequential)]
public struct Result
{
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
    public SquareResult[] RookResults;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
    public SquareResult[] BishopResults;
}

[DllImport("rmlib.so", EntryPoint = "GetMagics", CallingConvention = CallingConvention.Cdecl)]
public static extern Result GetMagics();
```

## Further Reading
(https://www.chessprogramming.org/Magic_Bitboards)
(https://github.com/official-stockfish/Stockfish)


