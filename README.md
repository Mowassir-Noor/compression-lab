# Compression Lab – Web GUI Guide

A browser-based interface for your compression tools (Huffman, LZ77, RLE). This README covers setup and usage from the project root.

## Prerequisites
- Node.js 14+ installed
- C++17 compiler (g++)

## Build the core compressor
```bash
cd "final project"
g++ -std=gnu++17 -O2 final.cpp -o final_app
```

## Install and run the Web GUI
```bash
cd "gui_web"
npm install
npm start
```
Then open your browser at:
```
http://localhost:3000
```

## Usage (UI flow)
1) Choose algorithm: Huffman, LZ77, or RLE.
2) Choose operation: Compress or Decompress.
3) Upload a file (drag & drop or click).
4) Click "Process File".
5) Download the result.

## Filenames
- Compression: preserves original name, appends algorithm extension
  - Example: `report.pdf` → `report.pdf.huff` (or `.lz77`, `.rle`).
- Decompression: automatically restores the original name from the compressed filename
  - Example: `report.pdf.huff` → `report.pdf`.

## Troubleshooting
- **Port 3000 busy:** run with another port
  ```bash
  PORT=4000 npm start
  ```
- **final_app not found:** rebuild it (see build step above).
- **Permissions on uploads:** ensure `gui_web/uploads/` exists and is writable
  ```bash
  mkdir -p gui_web/uploads
  chmod 755 gui_web/uploads
  ```

## Project layout
```
algoProje/
├── final project/        # C++ core (final.cpp, final_app)
├── gui_web/              # Web GUI (Express + static frontend)
└── README.md             # This guide
```
