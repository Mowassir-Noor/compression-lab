# Compression Lab – Final Project

A unified, interactive CLI tool integrating **three compression algorithms**: Huffman, LZ77, and RLE.

## Features

- **Colorful ANSI Terminal UI** with ASCII banner and intuitive menu
- **Huffman Encoding**: Block-based compression with configurable block size (default 64KB)
- **LZ77**: Dictionary-based compression capturing repetitive patterns
- **RLE**: Run-Length Encoding for sequences of identical bytes
- **Performance Metrics**: Real-time compression ratios, file sizes, and execution time
- **Easy Defaults**: Auto-generated output filenames (`.huff`, `.lz77`, `.rle`)

## Building

```bash
cd "final project"
g++ -std=gnu++17 -O2 final.cpp -o final_app
```

## Usage

Run the application:

```bash
./final_app
```

### Menu Flow

1. Select an algorithm (Huffman, LZ77, or RLE)
2. Choose operation (Compress or Decompress)
3. Provide input file path
4. Confirm output file (or use defaults)
5. View performance stats
6. Choose to run another operation or exit

### Example: Compress with Huffman

```
Choose: 1 (Huffman)
Operation: 1 (Compress)
Input file: ../myfile.txt
Block size: (press Enter for 65536 bytes)
Output file: (press Enter for myfile.txt.huff)
```

### Example: Decompress with LZ77

```
Choose: 2 (LZ77)
Operation: 2 (Decompress)
Input file: myfile.txt.lz77
Output file: myfile_restored.txt
```

## File Structure

```
final project/
├── final.cpp          # Main integration & UI
├── final_app          # Compiled binary
└── README.md          # This file
```

## Algorithm Details

### Huffman
- Creates optimal prefix codes based on character frequency
- Encodes with canonical code tables for efficient storage
- Best for text with varied character frequencies

### LZ77
- Finds repeating patterns using a sliding window (4KB)
- Matches sequences of 3+ identical bytes
- Efficient for text with repetition

### RLE
- Compresses runs of 4+ identical bytes
- Escape-safe binary implementation
- Fastest but works best on data with many repetitions

## Performance Notes

- **Huffman**: Good general compression, handles text well (~41–42% savings typical)
- **LZ77**: Slower but effective on repetitive data
- **RLE**: Fastest but limited to highly repetitive files

All algorithms report execution time in milliseconds and compression statistics.

---

**Created**: December 26, 2025  
**Author**: Compression Lab Team
