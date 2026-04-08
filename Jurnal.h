#pragma once
#include <string>
#include <iostream>

using namespace std;

class Jurnal {
public:
    static string inregistrari[100];
    static int indexJurnal;

    static void AdaugaInregistrare(string actiune) {
        if (indexJurnal < 100) {
            inregistrari[indexJurnal] = actiune;
            indexJurnal++;
        }
    }

    static void ArataInregistrari() {
        cout << "--- JURNAL ---\n";
        for (int i = 0; i < indexJurnal; i++) {
            cout << inregistrari[i] << "\n";
        }
    }
};