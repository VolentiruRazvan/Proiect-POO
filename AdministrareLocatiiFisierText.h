#pragma once
#include "Locatie.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

class AdministrareLocatiiFisierText {
public:
    string numeFisier = "Locatii.txt";

    void AdaugaLocatie(Locatie* l) {
        ofstream fisier(numeFisier, ios::app);
        if (fisier.is_open()) {
            fisier << l->numeLocatie << "\n";
            fisier.close();
        }
    }

    Locatie* CautaLocatieFisier(string nume) {
        ifstream fisier(numeFisier);
        string linie;
        if (fisier.is_open()) {
            while (getline(fisier, linie)) {
                if (linie.find(nume) != string::npos) {
                    Locatie* gasit = new Locatie(linie);
                    fisier.close();
                    return gasit;
                }
            }
            fisier.close();
        }
        return nullptr;
    }

    void ModificaLocatie(string numeVechi, string numeNou) {
        ifstream fisier(numeFisier);
        string linii[100];
        int nrLinii = 0;
        string linie;

        if (fisier.is_open()) {
            while (getline(fisier, linie)) {
                if (linie == numeVechi) {
                    linii[nrLinii] = numeNou;
                }
                else {
                    linii[nrLinii] = linie;
                }
                nrLinii++;
            }
            fisier.close();
        }

        ofstream fisierIesire(numeFisier);
        if (fisierIesire.is_open()) {
            for (int i = 0; i < nrLinii; i++) {
                fisierIesire << linii[i] << "\n";
            }
            fisierIesire.close();
        }
    }
};