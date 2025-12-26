#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;
// testing
// --- Terminal Colors & Styles ---
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define ITALIC "\033[3m"
#define UNDER "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

static void banner()
{
    cout << CYAN << BOLD
         << "\n"
         << "   ███████╗██╗███╗   ██╗ █████╗ ██╗         " << "\n"
         << "   ██╔════╝██║████╗  ██║██╔══██╗██║         " << "\n"
         << "   █████╗  ██║██╔██╗ ██║███████║██║         " << "\n"
         << "   ██╔══╝  ██║██║╚██╗██║██╔══██║██║         " << "\n"
         << "   ██║     ██║██║ ╚████║██║  ██║███████╗    " << "\n"
         << "   ╚═╝     ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝    " << "\n"
         << "\n"
         << MAGENTA
         << "   Compression Lab: Huffman • LZ77 • RLE" << "\n"
         << RESET << "\n";
}

static bool fileExists(const string &p)
{
    std::error_code ec;
    return fs::exists(p, ec);
}

static uintmax_t fileSizeSafe(const string &p)
{
    std::error_code ec;
    auto sz = fs::file_size(p, ec);
    return ec ? 0 : sz;
}

static string defaultOut(const string &in, const string &ext)
{
    return in + "." + ext;
}

// --- Include algorithms in library mode with macro-based renaming to avoid symbol clashes ---
// Huffman: rename compressFile/decompressFile
#define ALGO_LIB
#define compressFile huffman_compressFile
#define decompressFile huffman_decompressFile
#include "./huffman.cpp"
#undef decompressFile
#undef compressFile
#undef ALGO_LIB

// RLE: rename compressFile/decompressFile
#define ALGO_LIB
#define compressFile rle_compressFile
#define decompressFile rle_decompressFile
#include "./rle_binary.cpp"
#undef decompressFile
#undef compressFile
#undef ALGO_LIB

// LZ77: rename core function names used from file
#define ALGO_LIB
#define compress lz77_compress
#define decompress lz77_decompress
#define writeCompressed lz77_writeCompressed
#define readCompressed lz77_readCompressed
#include "./lz77.cpp"
#undef readCompressed
#undef writeCompressed
#undef decompress
#undef compress
#undef ALGO_LIB

// --- LZ77 file-level wrappers ---
static bool lz77CompressFile(const string &input, const string &output)
{
    ifstream in(input, ios::binary);
    if (!in)
    {
        cerr << RED << "Dosya acilamadi: " << input << RESET << "\n";
        return false;
    }
    vector<char> data((istreambuf_iterator<char>(in)), {});
    in.close();

    auto tokens = lz77_compress(data);
    lz77_writeCompressed(output, tokens);
    return true;
}

static bool lz77DecompressFile(const string &input, const string &output)
{
    auto tokens = lz77_readCompressed(input);
    if (tokens.empty())
    {
        cerr << RED << "Sikistirilmis veri okunamadi veya bos: " << input << RESET << "\n";
        return false;
    }
    auto data = lz77_decompress(tokens);
    ofstream out(output, ios::binary);
    if (!out)
    {
        cerr << RED << "Cikti dosyasi olusturulamadi: " << output << RESET << "\n";
        return false;
    }
    out.write(reinterpret_cast<const char *>(data.data()), data.size());
    return true;
}

// --- Menus ---
static int chooseAlgo()
{
    cout << BOLD << "Hangi algoritmayi kullanmak istersiniz?" << RESET << "\n";
    cout << "  1) " << GREEN << "Huffman" << RESET << " (blok bazli hizli)\n";
    cout << "  2) " << YELLOW << "LZ77" << RESET << " (tekrarlari yakalar)\n";
    cout << "  3) " << BLUE << "RLE" << RESET << " (ardisik byte tekrarlari)\n";
    cout << "Secim (1-3): " << flush;
    int c = 0;
    cin >> c;
    string dummy;
    getline(cin, dummy);
    return c;
}

static int chooseOp()
{
    cout << BOLD << "Islem: " << RESET << "1) Sikistir  2) Coz\n";
    cout << "Secim (1-2): " << flush;
    int c = 0;
    cin >> c;
    string dummy;
    getline(cin, dummy);
    return c;
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    while (true)
    {
        banner();
        int algo = chooseAlgo();
        if (algo < 1 || algo > 3)
        {
            cout << YELLOW << "Gecersiz secim. Cikis yapiliyor." << RESET << "\n";
            break;
        }
        int op = chooseOp();
        if (op != 1 && op != 2)
        {
            cout << YELLOW << "Gecersiz secim. Cikis yapiliyor." << RESET << "\n";
            break;
        }

        string inFile, outFile;
        cout << CYAN << "Girdi dosyasi yolu: " << RESET << flush;
        getline(cin, inFile);
        if (!fileExists(inFile))
        {
            cout << RED << "Dosya bulunamadi: " << inFile << RESET << "\n";
            continue;
        }

        bool ok = false;
        auto start = high_resolution_clock::now();

        if (algo == 1)
        {
            // Huffman
            if (op == 1)
            {
                string blockStr;
                size_t block = 64 * 1024; // 64KB default
                cout << DIM << "[Huffman] Blok boyutu (byte, varsayilan 65536): " << RESET << flush;
                getline(cin, blockStr);
                if (!blockStr.empty())
                {
                    try
                    {
                        block = static_cast<size_t>(stoull(blockStr));
                    }
                    catch (...)
                    {
                    }
                }
                outFile = defaultOut(inFile, "huff");
                cout << CYAN << "Cikti dosyasi (Enter = " << outFile << "): " << RESET << flush;
                string tmp;
                getline(cin, tmp);
                if (!tmp.empty())
                    outFile = tmp;

                huffman_compressFile(inFile, outFile, block);
                ok = true; // huffman functions already report
            }
            else
            {
                cout << CYAN << "Cikti dosyasi: " << RESET << flush;
                getline(cin, outFile);
                if (outFile.empty())
                    outFile = inFile + ".out";
                huffman_decompressFile(inFile, outFile);
                ok = true;
            }
        }
        else if (algo == 2)
        {
            // LZ77
            if (op == 1)
            {
                outFile = defaultOut(inFile, "lz77");
                cout << CYAN << "Cikti dosyasi (Enter = " << outFile << "): " << RESET << flush;
                string tmp;
                getline(cin, tmp);
                if (!tmp.empty())
                    outFile = tmp;
                ok = lz77CompressFile(inFile, outFile);
            }
            else
            {
                cout << CYAN << "Cikti dosyasi: " << RESET << flush;
                getline(cin, outFile);
                if (outFile.empty())
                    outFile = inFile + ".out";
                ok = lz77DecompressFile(inFile, outFile);
            }
        }
        else if (algo == 3)
        {
            // RLE
            if (op == 1)
            {
                outFile = defaultOut(inFile, "rle");
                cout << CYAN << "Cikti dosyasi (Enter = " << outFile << "): " << RESET << flush;
                string tmp;
                getline(cin, tmp);
                if (!tmp.empty())
                    outFile = tmp;
                ok = rle_compressFile(inFile, outFile);
            }
            else
            {
                cout << CYAN << "Cikti dosyasi: " << RESET << flush;
                getline(cin, outFile);
                if (outFile.empty())
                    outFile = inFile + ".out";
                ok = rle_decompressFile(inFile, outFile);
            }
        }

        auto stop = high_resolution_clock::now();
        auto dur = duration_cast<milliseconds>(stop - start);

        if (ok)
        {
            uintmax_t inSz = fileSizeSafe(inFile);
            uintmax_t outSz = fileSizeSafe(outFile);
            cout << GREEN << BOLD << "\n✔ Islem tamamlandi!" << RESET << "\n";
            cout << "Girdi Boyutu  : " << inSz << " byte\n";
            cout << "Cikti Boyutu  : " << outSz << " byte\n";
            if (op == 1 && outSz > 0)
            {
                double ratio = static_cast<double>(inSz) / static_cast<double>(outSz);
                double saving = (1.0 - (static_cast<double>(outSz) / max(1.0, static_cast<double>(inSz)))) * 100.0;
                cout << "Sikistirma Orani: " << fixed << setprecision(2) << ratio << "x\n";
                cout << "Alan Tasarrufu : %" << fixed << setprecision(2) << saving << "\n";
            }
            cout << "Sure           : " << dur.count() << " ms\n\n";
        }
        else
        {
            cout << RED << BOLD << "✖ Islem basarisiz." << RESET << "\n\n";
        }

        cout << ITALIC << "Baska bir islem yapmak ister misiniz? (y/n): " << RESET << flush;
        string again;
        getline(cin, again);
        if (again != "y" && again != "Y")
            break;
        cout << "\n";
    }

    cout << MAGENTA << "Gorusuruz!" << RESET << "\n";
    return 0;
}
