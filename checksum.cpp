/**********************************************************************
 * Checksum - Suma de control folosind codurile CRC.
 * Acest algoritm este folosit pentru a verifica integritatea datelor, respectiv daca datele au fost transmise/receptionate cu erori.
 * Se bazeaza pe teoria polinoamelor de lungime maxima.
 *
 * Reprezentari polinomiale folosite: CRC-1, CRC-7, CRC-16, CRC-32.
 *
 * CRC-1 = x+1 - de obicei bit de paritate
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
 * Aceste informatii au fost preluate de pe:
 * - http://users.ece.cmu.edu/~koopman/crc/index.html
 * - https://www.computing.dcu.ie/~humphrys/Notes/Networks/data.polynomial.html
 * - https://ro.wikipedia.org/wiki/Cyclic_redundancy_check
 * - https://ro.wikipedia.org/wiki/Distan%C8%9B%C4%83_Hamming
 *********************************************************************/

#include <iostream>
#include <string>
#include <bitset>

using namespace std;

void stringToBinary(string input) {
    string binary_result;
    
    for (char& c : input)
        binary_result += bitset<8>(c).to_string();
    cout << binary_result << endl;
}

int main() {
    stringToBinary("Hello world!");
    stringToBinary("Teststring");
    stringToBinary("CRC");
    return 0;
}