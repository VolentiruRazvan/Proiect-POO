#pragma once
#include "Locatie.h"
#include <string>

using namespace std;

class AdministratorEntitateMemorie {
public:
    Locatie* locatii[10];
    int nrLocatii;

    AdministratorEntitateMemorie() {
        nrLocatii = 0;
    }

    void AdaugaLocatie(Locatie* l) {
        locatii[nrLocatii] = l;
        nrLocatii++;
    }

    Locatie* CautaLocatieDupaNume(string numeCautat) {
        for (int i = 0; i < nrLocatii; i++) {
            if (locatii[i] != nullptr) {
                if (locatii[i]->numeLocatie.find(numeCautat) != string::npos) {
                    return locatii[i];
                }
            }
        }
        return nullptr;
    }
};