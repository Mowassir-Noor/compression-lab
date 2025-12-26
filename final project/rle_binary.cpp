#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <filesystem>
#include <iomanip>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;
using namespace std;

/*
 * Binary-Safe RLE Compression Algorithm
 *
 * Format: Uses escape byte (0xFF) to mark compressed runs
 * - For runs of 4+ identical bytes: [0xFF][count][byte]
 * - For literal bytes: output as-is (escape 0xFF as 0xFF 0x00)
 * - Count is stored as 1 byte (max run = 255)
 */

const uint8_t ESCAPE_BYTE = 0xFF;
const int MIN_RUN_LENGTH = 4; // Minimum run length to compress

// Binary-safe RLE Compression
vector<uint8_t> rleCompressBinary(const vector<uint8_t> &input)
{
    if (input.empty())
        return {};

    vector<uint8_t> compressed;
    size_t i = 0;

    while (i < input.size())
    {
        uint8_t currentByte = input[i];
        size_t runLength = 1;

        // Count consecutive identical bytes
        while (i + runLength < input.size() &&
               input[i + runLength] == currentByte &&
               runLength < 255)
        {
            runLength++;
        }

        if (runLength >= MIN_RUN_LENGTH)
        {
            // Encode as: ESCAPE_BYTE + count + byte
            compressed.push_back(ESCAPE_BYTE);
            compressed.push_back(static_cast<uint8_t>(runLength));
            compressed.push_back(currentByte);
            i += runLength;
        }
        else
        {
            // Output literal bytes
            for (size_t j = 0; j < runLength; j++)
            {
                if (currentByte == ESCAPE_BYTE)
                {
                    // Escape the escape byte: 0xFF -> 0xFF 0x00
                    compressed.push_back(ESCAPE_BYTE);
                    compressed.push_back(0x00);
                }
                else
                {
                    compressed.push_back(currentByte);
                }
            }
            i += runLength;
        }
    }

    return compressed;
}

// Binary-safe RLE Decompression
vector<uint8_t> rleDecompressBinary(const vector<uint8_t> &compressed)
{
    if (compressed.empty())
        return {};

    vector<uint8_t> decompressed;
    size_t i = 0;

    while (i < compressed.size())
    {
        if (compressed[i] == ESCAPE_BYTE)
        {
            if (i + 1 >= compressed.size())
            {
                cerr << "Error: Unexpected end of compressed data" << endl;
                break;
            }

            uint8_t count = compressed[i + 1];

            if (count == 0x00)
            {
                // Escaped escape byte: 0xFF 0x00 -> 0xFF
                decompressed.push_back(ESCAPE_BYTE);
                i += 2;
            }
            else
            {
                // Run-length encoded: ESCAPE + count + byte
                if (i + 2 >= compressed.size())
                {
                    cerr << "Error: Unexpected end of compressed data" << endl;
                    break;
                }
                uint8_t byte = compressed[i + 2];
                for (uint8_t j = 0; j < count; j++)
                {
                    decompressed.push_back(byte);
                }
                i += 3;
            }
        }
        else
        {
            // Literal byte
            decompressed.push_back(compressed[i]);
            i++;
        }
    }

    return decompressed;
}

// Read binary file into vector
vector<uint8_t> readBinaryFile(const string &filename)
{
    ifstream file(filename, ios::binary | ios::ate);
    if (!file)
    {
        cerr << "Error: Cannot open file: " << filename << endl;
        return {};
    }

    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        cerr << "Error: Failed to read file: " << filename << endl;
        return {};
    }

    return buffer;
}

// Write vector to binary file
bool writeBinaryFile(const string &filename, const vector<uint8_t> &data)
{
    ofstream file(filename, ios::binary);
    if (!file)
    {
        cerr << "Error: Cannot create file: " << filename << endl;
        return false;
    }

    file.write(reinterpret_cast<const char *>(data.data()), data.size());
    return file.good();
}

// Compress a file
bool compressFile(const string &inputFile, const string &outputFile)
{
    cout << "Reading file: " << inputFile << endl;

    vector<uint8_t> input = readBinaryFile(inputFile);
    if (input.empty())
    {
        cerr << "Error: Input file is empty or could not be read" << endl;
        return false;
    }

    cout << "Compressing..." << endl;
    vector<uint8_t> compressed = rleCompressBinary(input);

    if (!writeBinaryFile(outputFile, compressed))
    {
        return false;
    }

    double ratio = (compressed.size() * 100.0) / input.size();

    cout << "\n=== Compression Complete ===" << endl;
    cout << "Original size:   " << input.size() << " bytes" << endl;
    cout << "Compressed size: " << compressed.size() << " bytes" << endl;
    cout << "Compression ratio: " << ratio << "%" << endl;

    if (compressed.size() < input.size())
    {
        cout << "Space saved: " << (input.size() - compressed.size()) << " bytes" << endl;
    }
    else
    {
        cout << "Note: File did not compress well (random/already compressed data)" << endl;
    }

    return true;
}

// Decompress a file
bool decompressFile(const string &inputFile, const string &outputFile)
{
    cout << "Reading compressed file: " << inputFile << endl;

    vector<uint8_t> compressed = readBinaryFile(inputFile);
    if (compressed.empty())
    {
        cerr << "Error: Compressed file is empty or could not be read" << endl;
        return false;
    }

    cout << "Decompressing..." << endl;
    vector<uint8_t> decompressed = rleDecompressBinary(compressed);

    if (!writeBinaryFile(outputFile, decompressed))
    {
        return false;
    }

    cout << "\n=== Decompression Complete ===" << endl;
    cout << "Compressed size:   " << compressed.size() << " bytes" << endl;
    cout << "Decompressed size: " << decompressed.size() << " bytes" << endl;

    return true;
}

// Text string compression (for demo purposes)
string rleCompressText(const string &input)
{
    vector<uint8_t> inputVec(input.begin(), input.end());
    vector<uint8_t> compressed = rleCompressBinary(inputVec);
    return string(compressed.begin(), compressed.end());
}

string rleDecompressText(const string &compressed)
{
    vector<uint8_t> compressedVec(compressed.begin(), compressed.end());
    vector<uint8_t> decompressed = rleDecompressBinary(compressedVec);
    return string(decompressed.begin(), decompressed.end());
}

#ifndef ALGO_LIB
int main()
{
    int choice;
    string input, output;

    cout << "=== RLE (Run-Length Encoding) ARACI ===\n";
    cout << "1. Sikistir (Compress)\n";
    cout << "2. Coz (Decompress)\n";
    cout << "3. Cikis\n";

    while (true)
    {
        cout << "\nSeciminiz (1-3): ";
        cin >> choice;

        // Enter tuşu temizliği
        string dummy;
        getline(cin, dummy);

        if (choice == 3)
        {
            cout << "Cikis yapiliyor...\n";
            break;
        }

        switch (choice)
        {
        case 1: // --- SIKIŞTIRMA VE PERFORMANS ÖLÇÜMÜ ---
        {
            cout << "Sikistirilacak dosya adi: ";
            getline(cin, input);
            cout << "Cikti dosya adi: ";
            getline(cin, output);

            // Dosya var mı kontrolü (Basitçe)
            if (!fs::exists(input))
            {
                cerr << "Hata: Dosya bulunamadi -> " << input << endl;
                break;
            }

            // 1. Süreyi Başlat
            auto start = high_resolution_clock::now();

            // Sıkıştırma Fonksiyonunu Çağır
            compressFile(input, output);

            // 2. Süreyi Durdur
            auto stop = high_resolution_clock::now();

            // 3. Hesaplamalar
            // RLE çok hızlı olduğu için MİKROSANİYE (µs) kullanıyoruz
            auto duration = duration_cast<microseconds>(stop - start);

            uintmax_t originalSize = fs::file_size(input);
            uintmax_t compressedSize = fs::file_size(output);

            double ratio = (double)originalSize / (double)compressedSize;
            double saving = (1.0 - ((double)compressedSize / (double)originalSize)) * 100.0;

            // 4. Raporu Yazdır
            cout << "\n========================================\n";
            cout << "          PERFORMANS RAPORU             \n";
            cout << "========================================\n";
            cout << "Orijinal Boyut   : " << originalSize << " bytes\n";
            cout << "Sikistirilmis    : " << compressedSize << " bytes\n";
            cout << "Gecen Sure       : " << duration.count() << " µs (Mikrosaniye)\n";
            cout << "Sikistirma Orani : " << fixed << setprecision(2) << ratio << "\n";
            cout << "Alan Tasarrufu   : %" << fixed << setprecision(2) << saving << "\n";
            cout << "========================================\n";
            break;
        }
        case 2: // --- ÇÖZME ---
        {
            cout << "Cozulecek dosya adi: ";
            getline(cin, input);
            cout << "Cikti dosya adi: ";
            getline(cin, output);

            if (!fs::exists(input))
            {
                cerr << "Hata: Dosya bulunamadi -> " << input << endl;
                break;
            }

            decompressFile(input, output);
            cout << "Islem tamamlandi.\n";
            break;
        }
        default:
            cout << "Gecersiz secim. Tekrar deneyin.\n";
        }
    }

    return 0;
}
#endif // ALGO_LIB
