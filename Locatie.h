#pragma once
#include "Spectacol.h"
#include <string>

using namespace std;

class Locatie {
public:
    string numeLocatie;
    Spectacol* spectacole[10];
    int nrSpectacole;

    Locatie() {
        nrSpectacole = 0;
    }

    Locatie(string nume) {
        numeLocatie = nume;
        nrSpectacole = 0;
    }

    void AdaugaSpectacol(Spectacol* s) {
        spectacole[nrSpectacole] = s;
        nrSpectacole++;
    }
};