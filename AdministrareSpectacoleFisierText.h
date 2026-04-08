#pragma once
#include "Spectacol.h"
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

class AdministrareSpectacoleFisierText {
public:
    string numeFisier = "Spectacole.txt";

    void AdaugaSpectacol(Spectacol* s) {
        ofstream fisier(numeFisier, ios::app);
        if (fisier.is_open()) {
            fisier << s->numeSpectacol << "," << s->nrRanduri << "," << s->nrColoane << "," << s->matriceLocuri[0][0].pret << "\n";
            fisier.close();
        }
    }

    Spectacol* CautaSpectacolFisier(string nume) {
        ifstream fisier(numeFisier);
        string linie;
        if (fisier.is_open()) {
            while (getline(fisier, linie)) {
                if (linie.find(nume) != string::npos) {
                    stringstream ss(linie);
                    string numeSp, rStr, cStr, pStr;
                    getline(ss, numeSp, ',');
                    getline(ss, rStr, ',');
                    getline(ss, cStr, ',');
                    getline(ss, pStr, ',');

                    Spectacol* gasit = new Spectacol(numeSp, stoi(rStr), stoi(cStr), stoi(pStr));
                    fisier.close();
                    return gasit;
                }
            }
            fisier.close();
        }
        return nullptr;
    }
};