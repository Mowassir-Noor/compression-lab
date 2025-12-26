#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <chrono>     // <--- ZAMAN İÇİN YENİ EKLENDİ
#include <filesystem> // <--- DOSYA BOYUTU İÇİN YENİ EKLENDİ
#include <iomanip>    // <--- ONDALIK GÖSTERİM (setprecision) İÇİN

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

using namespace std;

constexpr int WINDOW_SIZE = 4096;
constexpr int LOOKAHEAD_SIZE = 18;
constexpr int MIN_MATCH = 3;

// ---------------- TOKEN ----------------
struct Token
{
    bool is_match;
    uint16_t offset;
    uint8_t length;
    char literal;

    static Token Literal(char c)
    {
        return {false, 0, 0, c};
    }

    static Token Match(uint16_t off, uint8_t len)
    {
        return {true, off, len, 0};
    }
};

// ---------------- MATCH FINDER ----------------
Token findMatch(const vector<char> &data, int pos)
{
    int best_len = 0;
    int best_off = 0;

    int start = max(0, pos - WINDOW_SIZE);
    int end = min((int)data.size(), pos + LOOKAHEAD_SIZE);

    for (int i = start; i < pos; ++i)
    {
        int len = 0;
        while (pos + len < end && data[i + len] == data[pos + len])
        {
            len++;
        }

        if (len > best_len && len >= MIN_MATCH)
        {
            best_len = len;
            best_off = pos - i;
            if (len == LOOKAHEAD_SIZE)
                break;
        }
    }

    if (best_len >= MIN_MATCH)
        return Token::Match(best_off, best_len);

    return Token::Literal(data[pos]);
}

// ---------------- COMPRESS ----------------
vector<Token> compress(const vector<char> &data)
{
    vector<Token> out;
    int pos = 0;

    while (pos < data.size())
    {
        Token t = findMatch(data, pos);
        out.push_back(t);

        if (t.is_match)
            pos += t.length;
        else
            pos += 1;
    }

    return out;
}

// ---------------- DECOMPRESS ----------------
vector<char> decompress(const vector<Token> &tokens)
{
    vector<char> out;

    for (const auto &t : tokens)
    {
        if (!t.is_match)
        {
            out.push_back(t.literal);
        }
        else
        {
            int start = out.size() - t.offset;
            for (int i = 0; i < t.length; i++)
                out.push_back(out[start + i]);
        }
    }
    return out;
}

// ---------------- FILE I/O ----------------
void writeCompressed(const string &name, const vector<Token> &tokens)
{
    ofstream f(name, ios::binary);

    uint32_t count = tokens.size();
    f.write((char *)&count, sizeof(count));

    for (const auto &t : tokens)
    {
        f.write((char *)&t.is_match, 1);
        if (t.is_match)
        {
            f.write((char *)&t.offset, sizeof(t.offset));
            f.write((char *)&t.length, sizeof(t.length));
        }
        else
        {
            f.write(&t.literal, 1);
        }
    }
}

vector<Token> readCompressed(const string &name)
{
    ifstream f(name, ios::binary);
    vector<Token> tokens;

    uint32_t count;
    f.read((char *)&count, sizeof(count));

    for (uint32_t i = 0; i < count; i++)
    {
        Token t;
        f.read((char *)&t.is_match, 1);
        if (t.is_match)
        {
            f.read((char *)&t.offset, sizeof(t.offset));
            f.read((char *)&t.length, sizeof(t.length));
        }
        else
        {
            f.read(&t.literal, 1);
        }
        tokens.push_back(t);
    }
    return tokens;
}

// ---------------- MAIN ----------------
#ifndef ALGO_LIB
int main()
{
    cout << "=== LZ77 SIKISTIRMA ARACI ===\n";
    cout << "1. Sikistir (Compress)\n";
    cout << "2. Coz (Decompress)\n";
    cout << "Seciminiz (1 veya 2): ";

    int choice;
    cin >> choice;

    string dummy;
    getline(cin, dummy);

    string inputFile, outputFile;

    if (choice == 1)
    {
        cout << "Sikistirilacak dosya adi: ";
        getline(cin, inputFile);
        cout << "Cikti dosya adi: ";
        getline(cin, outputFile);

        ifstream in(inputFile, ios::binary);
        if (!in)
        {
            cerr << "Dosya bulunamadi: " << inputFile << endl;
            return 1;
        }
        vector<char> data((istreambuf_iterator<char>(in)), {});
        in.close(); // Okuma bitti, dosyayı kapat

        // --- PERFORMANS ÖLÇÜMÜ BAŞLANGIÇ ---
        auto start = high_resolution_clock::now();

        auto tokens = compress(data);
        writeCompressed(outputFile, tokens);

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);
        uintmax_t originalSize = fs::file_size(inputFile);
        uintmax_t compressedSize = fs::file_size(outputFile);

        double ratio = (double)originalSize / (double)compressedSize;
        double saving = (1.0 - ((double)compressedSize / (double)originalSize)) * 100.0;

        cout << "\n========================================\n";
        cout << "          PERFORMANS RAPORU             \n";
        cout << "========================================\n";
        cout << "Orijinal Boyut   : " << originalSize << " bytes\n";
        cout << "Sikistirilmis    : " << compressedSize << " bytes\n";
        cout << "Gecen Sure       : " << duration.count() << " µs\n";
        cout << "Sikistirma Orani : " << fixed << setprecision(2) << ratio << "\n";
        cout << "Alan Tasarrufu   : %" << fixed << setprecision(2) << saving << "\n";
        cout << "========================================\n";
    }
    else if (choice == 2)
    {
        cout << "Cozulecek dosya adi: ";
        getline(cin, inputFile);
        cout << "Cikti dosya adi: ";
        getline(cin, outputFile);

        auto tokens = readCompressed(inputFile);
        if (tokens.empty())
        {
            cerr << "Dosya okunamadi veya bos: " << inputFile << endl;
            return 1;
        }

        auto data = decompress(tokens);

        ofstream out(outputFile, ios::binary);
        out.write(data.data(), data.size());
        out.close();

        cout << "Basarili! Dosya cozuldu.\n";
        cout << "Cozulmus Boyut: " << data.size() << " bytes\n";
    }
    else
    {
        cout << "Gecersiz secim!\n";
    }

    return 0;
}
#endif // ALGO_LIB
