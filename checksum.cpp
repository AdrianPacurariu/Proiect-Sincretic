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
 *
 * Aceste informatii au fost preluate de pe:
 * - http://users.ece.cmu.edu/~koopman/crc/index.html
 * - https://www.computing.dcu.ie/~humphrys/Notes/Networks/data.polynomial.html
 * - https://ro.wikipedia.org/wiki/Cyclic_redundancy_check
 * - https://ro.wikipedia.org/wiki/Distan%C8%9B%C4%83_Hamming
 * - https://en.wikipedia.org/wiki/Cyclic_redundancy_check#table
 * - https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
 * - https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
 * - https://www.lammertbies.nl/comm/info/crc-calculation
 *********************************************************************/

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

/* Folosim polinoamele Reversed/Reflected, pentru a opera corect incepand de la cel mai putin semnificativ bit (LSB) catre cel mai semnificatib bit (MSB),
adica facilitam ideea de Little Endian, astfel in toate operatiile de prelucrare a polinomului
si de aflare a codurilor CRC, vom shifta spre dreapta si nu spre stanga.

"Valoare initiala" reprezinta valoarea stocata in registrul care ne va da suma de control. (variabila rezultat).
"XOR final" ne spune daca inainte de a da rezultatul final, se face un XOR cu o valoare predefinita.

Pentru acest proiect eu am ales doar cate o reprezentare/forma de CRC. Pentru fiecare CRC exista cel putin alte 5 forme folosite in diverse scopuri.

Documentatie/catalog CRC-uri: https://reveng.sourceforge.io/crc-catalogue/all.htm
*/


/* Nume CRC                Polinom                    Reprezentare             Valoare initiala            XOR final

 CRC-7/MMC                  0x48                    Reversed/Reflected              0x7F                   Da. (0x7F)
 CRC-16/ARC                0xA001                   Reversed/Reflected             0x0000                 Nu. (0x0000)
 CRC-32/BZIP2            0xEDB88320                 Reversed/Reflected           0xFFFFFFFF              Da. (0xFFFFFFFF)

*/

#define SIZE 256 /* Numarul de constante din tabelele de cautare. */
#define polinomCRC32 0xEDB88320 
#define polinomCRC16 0xA001
#define polinomCRC7 0x48

/* Predefiniri tipuri de date.
uint32_t = intreg pe 32 de biti fara semn.
Valori posibile: [0, 2^32-1] = [0, 4294967295];

uint16_t = intreg pe 16 biti fara semn
Valori posibile: [0, 2^16-1] = [0, 65535];

Desi codul CRC7 este alcatuit numai din 7 biti, nu se poate folosi tipul unsigned char sau uint8_t (unsigned int pe 8 biti),
deoarece acest loop : for (CRC7 deimpartit = 0; deimpartit < SIZE; deimpartit++), unde SIZE este 256, nu se va opri niciodata, deoarece valoarea maxima
pe 8 biti unsigned este 255 (11111111). Dupa 255, o va lua iarasi de la 0.
*/

typedef uint32_t CRC32;
typedef uint16_t CRC16;
typedef uint16_t CRC7;

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
Valoarea obtinuta in urma operatiei de XOR va incepe mereu practic cu un bit de 0, deoarece 1 XOR 1 este 0. */

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
            else /* Daca primul bit este 0, se face doar shiftare simpla. */
                octet >>= 1;
        }
        tabel_CRC32[deimpartit] = octet;
    }
}

void initializare_tabel16() {
    CRC16 octet;
    tabel_CRC16_initializat = true;
    for (CRC16 deimpartit = 0; deimpartit < SIZE; deimpartit++) {
        octet = deimpartit;
        for (CRC16 bit = 0; bit < 8; bit++) {
            if (octet & 1) { 
                octet >>= 1; 
                octet ^= polinomCRC16;
            }
            else
                octet >>= 1;
        }
        tabel_CRC16[deimpartit] = octet;
    }
}

void initializare_tabel7() {
    CRC7 octet;
    tabel_CRC7_initializat = true;
    for (CRC7 deimpartit = 0; deimpartit < SIZE; deimpartit++) { 
        octet = deimpartit;
        for (CRC7 bit = 0; bit < 8; bit++) {
            if (octet & 1) { 
                octet >>= 1; 
                octet ^= polinomCRC7;
            }
            else
                octet >>= 1;
        }
        tabel_CRC7[deimpartit] = octet;
    }
}

CRC32 calculCRC32(string input) {
    CRC32 rezultat = 0xFFFFFFFF; /* Valoarea initiala stocata in registru, 32 de 1. */

    for (size_t bit = 0; bit < input.length(); bit++) {
        CRC32 termen = (input[bit] ^ rezultat) & 0xFF;
        rezultat = (rezultat >> 8) ^ tabel_CRC32[termen];
    }

    return rezultat ^ 0xFFFFFFFF; /* sau: ~rezultat. */
}

CRC16 calculCRC16(string input) {
    CRC16 rezultat = 0; /* Valoarea initiala este 0. */
    
    for (size_t bit = 0; bit < input.length(); bit++) {
        CRC16 termen = (input[bit] ^ rezultat) & 0xFF;
        rezultat = (rezultat >> 8) ^ tabel_CRC16[termen];
    }
    return rezultat; /* Fara XOR. */
}

/* Functia calculCRC7 nu da un output corect momentan.
Conform https://reveng.sourceforge.io/crc-catalogue/all.htm pentru input-ul "123456789" trebuie sa obtin 0x75, si eu obtin 0x5a.
*/
CRC7 calculCRC7(string input) {
    CRC7 rezultat = 0x7F; /* Valoarea initiala este 0x7F deoarece avem nevoie de doar 7 biti. */

    for (size_t bit = 0; bit < input.length(); bit++) {
        CRC7 termen = (input[bit] ^ rezultat) & 0xFF;
        rezultat = (rezultat >> 1) ^ tabel_CRC7[termen];
    }
    return rezultat ^ 0x7F;
}

int main() {
    enum optiuni { iesire, initializare, calcul_CRC32, calcul_CRC16, calcul_CRC7 };
    string sir_intrare;
    int opt;

    cout << "Program de calculare a sumei de control folosind codurile CRC." << endl;
    cout << "Alegeti una dintre optiuni: " << endl;
    for (;;) {
        cout << "1. Initializare tabele de cautare CRC32, CRC16, CRC7." << endl;
        cout << "2. Calculare suma de control CRC32 pentru un sir dat de la tastatura." << endl;
        cout << "3. Calculare suma de control CRC16 pentru un sir dat de la tastatura." << endl;
        cout << "4. Calculare suma de control CRC7 pentru un sir dat de la tastatura." << endl;
        cout << endl << "0. Iesire program." << endl;
        cout << "Dati optiunea: ";
        cin >> opt;
        switch ((optiuni)opt)
        {
        case iesire: cout << "Ati parasit programul."; return 0;
        case initializare:
            initializare_tabel32();
            initializare_tabel16();
            initializare_tabel7();
            cout << "Tabelele de cautare CRC32, CRC16, CRC7 au fost initializare." << endl;
            break;
        case calcul_CRC32:
            if (!tabel_CRC32_initializat)
                cout << "Se recomanda initializarea tabelului de cautare CRC32 intai." << endl;
            else {
                cout << "Dati sirul de intrare: "; cin.get();
                getline(cin, sir_intrare);
                cout << "Cod CRC32 obtinut pentru sirul de intrare " << sir_intrare << ": " << hex << calculCRC32(sir_intrare) << endl;
            }
            break;
        case calcul_CRC16:
            if (!tabel_CRC16_initializat)
                cout << "Se recomanda initializarea tabelului de cautare CRC16 intai." << endl;
            else {
                cout << "Dati sirul de intrare: "; cin.get();
                getline(cin, sir_intrare);
                cout << "Cod CRC16 obtinut pentru sirul de intrare " << sir_intrare << ": " << hex << calculCRC16(sir_intrare) << endl;
            }
            break;
        case calcul_CRC7:
            if (!tabel_CRC7_initializat)
                cout << "Se recomanda initializarea tabelului de cautare CRC7 intai." << endl;
            else {
                cout << "Dati sirul de intrare: "; cin.get();
                getline(cin, sir_intrare);
                cout << "Cod CRC7 obtinut pentru sirul de intrare " << sir_intrare << ": " << hex << calculCRC7(sir_intrare) << endl;
          }
                break;
        default: cout << "Optiune incorecta." << endl; break;
        }
    }
}