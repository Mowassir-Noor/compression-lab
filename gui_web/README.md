# Compression Lab – Web GUI

A modern, interactive web interface for the compression algorithms (Huffman, LZ77, RLE).

## Setup & Launch

### Prerequisites

- Node.js 14+ installed
- The C++ `final_app` binary must be built in `../final project/`

### Installation

```bash
cd gui_web
npm install
```

### Start the Server

```bash
npm start
```

Output:
```
✓ Compression Lab GUI running at http://localhost:3000
✓ Open in browser: http://localhost:3000
```

### Development Mode (auto-reload)

```bash
npm run dev
```

## Features

- **Beautiful Gradient UI** with drag-and-drop file upload
- **Algorithm Selection**: Huffman, LZ77, or RLE
- **Dual Operations**: Compress or Decompress
- **Real-time Stats**: File sizes, compression ratio, space saved
- **Direct Download**: Get compressed/decompressed files instantly
- **Responsive Design**: Works on desktop, tablet, and mobile

## How It Works

1. Select an **algorithm** from the dropdown
2. Choose **compress** or **decompress**
3. Drag & drop a file or click "Browse Files"
4. Click "Process File"
5. View stats and download your result

## Architecture

```
gui_web/
├── server.js              # Express backend (spawns final_app)
├── package.json           # Node dependencies
├── public/
│   ├── index.html         # UI structure
│   ├── style.css          # Modern gradient styling
│   └── script.js          # Interactive frontend
├── uploads/               # Temp file storage (auto-created)
└── README.md              # This file
```

## API Endpoints

### POST `/api/process`

Multipart form upload with:
- `file`: Binary file to process
- `algorithm`: `huffman`, `lz77`, or `rle`
- `operation`: `compress` or `decompress`

**Response:**
```json
{
  "success": true,
  "inputFile": "/path/to/input",
  "outputFile": "/path/to/output",
  "inputSize": 914,
  "outputSize": 528,
  "ratio": "1.73",
  "saving": "42.23",
  "downloadUrl": "/download/output.huff"
}
```

### GET `/download/:filename`

Download processed file.

### POST `/api/cleanup/:filename`

Optional: Remove temporary files.

## Limits

- **File size**: 100MB max (configurable in `server.js`)
- **Timeout**: 60 seconds per operation

## Troubleshooting

**Port 3000 already in use:**
```bash
# Use a different port
PORT=4000 npm start
```

**`final_app` not found:**
Make sure `../final project/final_app` exists and is executable:
```bash
cd ../final\ project
g++ -std=gnu++17 -O2 final.cpp -o final_app
```

**Permission denied on upload:**
```bash
mkdir -p uploads
chmod 755 uploads
```

## Browser Compatibility

- Chrome/Chromium 90+
- Firefox 88+
- Safari 14+
- Edge 90+

---

**Created**: December 26, 2025
