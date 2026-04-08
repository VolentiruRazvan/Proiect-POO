#pragma once
#include "Loc.h"
#include <string>
#include <iostream>

using namespace std;

class Spectacol {
public:
    string numeSpectacol;
    Loc matriceLocuri[10][10];
    int nrRanduri;
    int nrColoane;

    Spectacol() {
        nrRanduri = 0;
        nrColoane = 0;
    }

    Spectacol(string nume, int r, int c, int pretBaza) {
        numeSpectacol = nume;
        nrRanduri = r;
        nrColoane = c;

        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                matriceLocuri[i][j] = Loc(i, j, pretBaza + i * 5);
            }
        }
    }

    void AfisareLocuri() {
        cout << "\n";
        for (int i = 0; i < nrRanduri; i++) {
            char literaRand = (char)('A' + i);
            cout << "Randul " << literaRand << " (Pret " << matriceLocuri[i][0].pret << "): ";
            for (int j = 0; j < nrColoane; j++) {
                if (matriceLocuri[i][j].ocupat == true) {
                    cout << "[X] ";
                }
                else {
                    cout << "[" << (j + 1) << "] ";
                }
            }
            cout << "\n";
        }
        cout << "\n";
    }

    void GolesteLocuri() {
        for (int i = 0; i < nrRanduri; i++) {
            for (int j = 0; j < nrColoane; j++) {
                matriceLocuri[i][j].ocupat = false;
            }
        }
    }
};