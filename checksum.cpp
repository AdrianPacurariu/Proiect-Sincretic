/**********************************************************************
 * Checksum - Suma de control folosind codurile CRC.
 * Acest algoritm este folosit pentru a verifica integritatea datelor, respectiv daca datele au fost transmise/receptionate cu erori.
 * Se bazeaza pe teoria polinoamelor de lungime maxima.
 *
 * Reprezentari polinomiale folosite: CRC-7, CRC-16, CRC-32.
 *
 * CRC-7 = x7 + x3 + 1
 * CRC-16 = x16 + x15 + x2 + 1
 * CRC-32 = x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1
 *
 *
 * Codurile polinomiale sunt bazate pe tratarea sirurilor de biți ca reprezentări de polinoame cu coeficienti 0 si 1. 
 * Ex.: 110001 = x5 + x4 + x0, deoarece bitii de pe pozitiile 5, 4 si 0 sunt 1.
 *
 * Distanta Hamming dintre doua siruri de intrare = numarul de pozitii(biti) care difera.
 * Ex:
 *      input_string_1 = "100100";
 *      input_string_2 = "110110";
 * => HD = 2. Intre cele doua siruri de intrare difera pozitiile 4 si 1.
 * Deci Distanta Hamming este egala cu numarul de biti de 1 din input_string_1 XOR(^) input_string_2.
 *
 *
 * Updates:
 * - renuntare la libraria <bitset> si la functia de convertire in binar a unui string de intrare, voi incerca
 * sa fac asta direct prin intermediul librariei <inttypes.h> si a tipurilor predefinite.
 * - renuntare la reprezentarea polinomiala CRC-1 deoarece este doar bitul de paritate. (LSB)
 *
 * Abordare rezolvare:
 * 1) 
 *
 *
 * Aceste informatii au fost preluate de pe:
 * - http://users.ece.cmu.edu/~koopman/crc/index.html
 * - https://www.computing.dcu.ie/~humphrys/Notes/Networks/data.polynomial.html
 * - https://ro.wikipedia.org/wiki/Cyclic_redundancy_check
 * - https://ro.wikipedia.org/wiki/Distan%C8%9B%C4%83_Hamming
 *********************************************************************/

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

#define SIZE 256 /* Numarul de constante din tabelele de cautare. */
#define polinomCRC32 0xEDB88320 
#define polinomCRC16 0xA001
#define polinomCRC7 0x48
/* Folosim polinoamele "Reversed", pentru a opera corect incepand de la cel mai putin semnificativ bit (LSB) catre cel mai semnificatib bit (MSB),
adica facilitam ideea de Little Endian, astfel in toate operatiile de prelucrare a polinomului
si de aflare a codurilor CRC, vom shifta spre dreapta si nu spre stanga. */

/* Predefiniri tipuri de date.
uint32_t = intreg pe 32 de biti fara semn.
Valori posibile: [0, 2^32-1].

uint16_t = intreg pe 16 biti fara semn
Valori posibile: [0, 2^16-1];

uint8_t = intreg pe 8 biti fara semn
Valori posibile: [0, 2^8-1]
*/

typedef uint32_t CRC32;
typedef uint16_t CRC16;
typedef uint8_t CRC7;

/* Intai tabelele de cautare nu sunt initializate. */

bool tabel_CRC32_initializat = false;
bool tabel_CRC16_initializat = false;
bool tabel_CRC7_initializat = false;

/* Aici vor fi definite tabelele de cautare pentru fiecare reprezentare polinomiala.
Fiecare tabel va contine 256 de constante pe 32, 16 si 8 biti. (dublu cuvant, cuvant, octet)
Aceste tabele imbunatatesc performanta si viteza algoritmului deoarece se va lucra la nivel de octet (bytewise) si nu la nivel de bit,
astfel reducandu-se numarul de iteratii.

Practic, cunoscand valoarea octetului celui mai semnificativ al codului CRC precum si
urmatorul octet care urmeaza a fi prelucrat din string-ul de intrare, putem sa calculam urmatoarea valoare din CRC intr-o singura iteratie, decat sa facem 8 iteratii separat, pentru fiecare bit in parte.
Urmatoare valoare care urmeaza a fi adaugata la codul CRC este determinata facand XOR intre octetul cel mai semnificativ din CRC si octetul la care ne aflam in string-ul de intrare.
Valoarea obtinuta in urma operatiei de XOR va incepe mereu practic cu un bit de 0, deoarece
1 XOR 1 este 0. */

CRC32 tabel_CRC32[SIZE];
CRC16 tabel_CRC16[SIZE];
CRC7 tabel_CRC7[SIZE];

void initializare_tabel32() {
    CRC32 octet;
    tabel_CRC32_initializat = true;
    /* Cu acest for se calculeaza resturile, care urmeaza a fi adaugate in tabelul de cautare. */
    /* Un cod CRC este in esenta restul unei operatii de impartire. */
    for (CRC32 deimpartit = 0; deimpartit < SIZE; deimpartit++) { /* Intr-un octet pot fi stocate valori intre 0-255. */
        /* La inceput restul se ia ca fiind chiar deimpartitul actual. */
        octet = deimpartit; /* Restul. */
        for (CRC32 bit = 0; bit < 8; bit++) {
            if (octet & 1) { /* Daca primul bit este 1, facem XOR cu polinomul. */
                octet >>= 1; /* Shiftare la dreapta cu o pozitie. */
                octet ^= polinomCRC32;
            }
            else /* Daca primul bit este 0, nu facem nimic. */
                octet >>= 1;
        }
        tabel_CRC32[deimpartit] = octet;
    }
}

CRC32 calculCRC32(string input) {
    CRC32 rezultat = 0xFFFFFFFF; /* Valoarea de inceput a codului CRC, 32 de 1. */

    /* Initializam tabelul daca nu a fost deja initializat. */
    if (tabel_CRC32_initializat == false)
        initializare_tabel32();

    for (size_t bit = 0; bit < input.length(); bit++) {
        char caracter = input[bit];
        CRC32 termen = (caracter ^ rezultat) & 0xFF;
        rezultat = (rezultat >> 8) ^ tabel_CRC32[termen];
    }

    return ~rezultat; /* ^ 0xFFFFFFFF */
}

int main() {
    cout << hex << calculCRC32("Proiect Sincretic") << endl;
    cout << hex << calculCRC32("The quick brown fox jumps over the lazy dog");
    return 0;
}