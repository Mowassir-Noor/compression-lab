#!/bin/bash

# Compression Lab Web GUI Launcher
# Starts the Node.js server for the web interface

cd "$(dirname "$0")"

echo "═══════════════════════════════════════════════════"
echo "  🗜️  Compression Lab – Web GUI"
echo "═══════════════════════════════════════════════════"
echo ""
echo "Checking prerequisites..."

# Check if final_app exists
if [ ! -f "../final project/final_app" ]; then
    echo "❌ Error: final_app not found in ../final project/"
    echo "Please build it first:"
    echo "  cd ../final\ project"
    echo "  g++ -std=gnu++17 -O2 final.cpp -o final_app"
    exit 1
fi

# Check if node_modules exists
if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
    echo ""
fi

# Create uploads directory if needed
mkdir -p uploads

echo "✓ All checks passed!"
echo ""
echo "Starting server..."
echo ""

npm start
