#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include "Loc.h"
#include "Spectacol.h"
#include "Locatie.h"
#include "Jurnal.h"
#include "AdministratorEntitateMemorie.h"
#include "AdministrareLocatiiFisierText.h"
#include "AdministrareSpectacoleFisierText.h"

using namespace std;

string Jurnal::inregistrari[100];
int Jurnal::indexJurnal = 0;

AdministratorEntitateMemorie admin;
AdministrareLocatiiFisierText adminLocatiiFisier;
AdministrareSpectacoleFisierText adminSpectacoleFisier;

void InitializareDate() {
    Locatie* c1 = new Locatie("Cinema City Suceava");
    c1->AdaugaSpectacol(new Spectacol("Dune: Partea II", 8, 10, 25));
    c1->AdaugaSpectacol(new Spectacol("Oppenheimer", 8, 10, 25));
    c1->AdaugaSpectacol(new Spectacol("Deadpool & Wolverine", 8, 10, 30));

    Locatie* c2 = new Locatie("Cinema City Botosani");
    c2->AdaugaSpectacol(new Spectacol("Joker: Folie a Deux", 8, 10, 20));
    c2->AdaugaSpectacol(new Spectacol("Inside Out 2", 8, 10, 20));

    Locatie* c3 = new Locatie("Cinema City Iasi");
    c3->AdaugaSpectacol(new Spectacol("Interstellar", 8, 10, 30));
    c3->AdaugaSpectacol(new Spectacol("The Batman", 8, 10, 25));
    c3->AdaugaSpectacol(new Spectacol("Inception", 8, 10, 25));
    c3->AdaugaSpectacol(new Spectacol("Avatar: Calea Apei", 8, 10, 30));

    Locatie* c4 = new Locatie("Cinema Dorohoi");
    c4->AdaugaSpectacol(new Spectacol("Furiosa: O saga Mad Max", 8, 10, 15));
    c4->AdaugaSpectacol(new Spectacol("Gladiatorul II", 8, 10, 15));

    Locatie* t1 = new Locatie("Sala de teatru Dorohoi");
    t1->AdaugaSpectacol(new Spectacol("O scrisoare pierduta", 8, 10, 40));
    t1->AdaugaSpectacol(new Spectacol("Take, Ianke si Cadir", 8, 10, 35));

    Locatie* t2 = new Locatie("Sala de teatru Suceava");
    t2->AdaugaSpectacol(new Spectacol("Dineu cu prosti", 8, 10, 50));
    t2->AdaugaSpectacol(new Spectacol("Gaitele", 8, 10, 45));

    Locatie* t3 = new Locatie("Sala de teatru Iasi");
    t3->AdaugaSpectacol(new Spectacol("Faust", 8, 10, 70));
    t3->AdaugaSpectacol(new Spectacol("Chirita in provintie", 8, 10, 60));
    t3->AdaugaSpectacol(new Spectacol("Visul unei nopti de vara", 8, 10, 60));

    admin.AdaugaLocatie(c1);
    admin.AdaugaLocatie(c2);
    admin.AdaugaLocatie(c3);
    admin.AdaugaLocatie(c4);
    admin.AdaugaLocatie(t1);
    admin.AdaugaLocatie(t2);
    admin.AdaugaLocatie(t3);
}

int main() {
    InitializareDate();
    Jurnal::AdaugaInregistrare("A inceput programul");

    int optiune = -1;
    Locatie* locCurent = nullptr;
    Spectacol* specCurent = nullptr;

    while (optiune != 0) {
        system("clear");
        cout << "=== MENIU ===\n";
        cout << "1. Alege Locatie\n";
        cout << "2. Alege Spectacol\n";
        cout << "3. Cauta locatie in memorie\n";
        cout << "4. Vezi locuri\n";
        cout << "5. Rezerva loc\n";
        cout << "6. Goleste locuri\n";
        cout << "7. Jurnal actiuni\n";
        cout << "--- TEMA FISIERE ---\n";
        cout << "8. Salveaza o locatie noua in fisier\n";
        cout << "9. Modifica nume locatie in fisier\n";
        cout << "10. Salveaza un spectacol nou in fisier\n";
        cout << "11. Cauta spectacol in fisier\n";
        cout << "0. Iesire\n";

        if (locCurent != nullptr) cout << "\nLocatie aleasa: " << locCurent->numeLocatie << "\n";
        if (specCurent != nullptr) cout << "Spectacol ales: " << specCurent->numeSpectacol << "\n";

        cout << "\nAlege: ";
        cin >> optiune;

        if (optiune == 1) {
            for (int i = 0; i < admin.nrLocatii; i++) {
                cout << (i + 1) << " - " << admin.locatii[i]->numeLocatie << "\n";
            }
            cout << "Alege numarul: ";
            int l;
            cin >> l;
            locCurent = admin.locatii[l - 1];
            specCurent = nullptr;
            Jurnal::AdaugaInregistrare("A ales locatia " + locCurent->numeLocatie);
        }
        else if (optiune == 2) {
            if (locCurent != nullptr) {
                for (int i = 0; i < locCurent->nrSpectacole; i++) {
                    cout << (i + 1) << " - " << locCurent->spectacole[i]->numeSpectacol << "\n";
                }
                cout << "Alege numarul: ";
                int s;
                cin >> s;
                specCurent = locCurent->spectacole[s - 1];
                Jurnal::AdaugaInregistrare("A ales spectacolul " + specCurent->numeSpectacol);
            }
        }
        else if (optiune == 3) {
            cout << "Ce locatie cauti? ";
            cin.ignore();
            string textCautat;
            getline(cin, textCautat);

            Locatie* gasit = admin.CautaLocatieDupaNume(textCautat);

            if (gasit != nullptr) {
                cout << "Gasit: " << gasit->numeLocatie << "\n";
                locCurent = gasit;
                specCurent = nullptr;
            }
            else {
                cout << "Nu exista.\n";
            }
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
        else if (optiune == 4) {
            if (specCurent != nullptr) {
                specCurent->AfisareLocuri();
                cout << "\nApasa Enter pentru a continua...";
                cin.ignore(256, '\n');
                cin.get();
            }
        }
        else if (optiune == 5) {
            if (specCurent != nullptr) {
                specCurent->AfisareLocuri();
                cout << "Rand (Litera A-H): ";
                string litera;
                cin >> litera;
                char literaMare = toupper(litera[0]);
                int r = literaMare - 'A' + 1;

                cout << "Loc (1-10): ";
                int c;
                cin >> c;

                if (r < 1 || r > specCurent->nrRanduri || c < 1 || c > specCurent->nrColoane) {
                    cout << "Rand sau loc invalid!\n";
                }
                else if (specCurent->matriceLocuri[r - 1][c - 1].ocupat == true) {
                    cout << "Loc ocupat!\n";
                }
                else {
                    specCurent->matriceLocuri[r - 1][c - 1].ocupat = true;
                    cout << "Rezervat. Costa: " << specCurent->matriceLocuri[r - 1][c - 1].pret << "\n";
                    string logText = "A rezervat la " + specCurent->numeSpectacol;
                    Jurnal::AdaugaInregistrare(logText);
                }
                cout << "\nApasa Enter pentru a continua...";
                cin.ignore(256, '\n');
                cin.get();
            }
        }
        else if (optiune == 6) {
            if (specCurent != nullptr) {
                specCurent->GolesteLocuri();
                cout << "Locuri resetate.\n";
                cout << "\nApasa Enter pentru a continua...";
                cin.ignore(256, '\n');
                cin.get();
            }
        }
        else if (optiune == 7) {
            Jurnal::ArataInregistrari();
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
        else if (optiune == 8) {
            cout << "Nume locatie noua: ";
            cin.ignore();
            string nume;
            getline(cin, nume);
            Locatie* lNou = new Locatie(nume);
            adminLocatiiFisier.AdaugaLocatie(lNou);
            cout << "Salvat in Locatii.txt\n";
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
        else if (optiune == 9) {
            cout << "Numele locatiei de modificat: ";
            cin.ignore();
            string numeVechi;
            getline(cin, numeVechi);
            cout << "Numele nou: ";
            string numeNou;
            getline(cin, numeNou);
            adminLocatiiFisier.ModificaLocatie(numeVechi, numeNou);
            cout << "Modificat in fisier.\n";
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
        else if (optiune == 10) {
            cout << "Nume spectacol nou: ";
            cin.ignore();
            string nume;
            getline(cin, nume);
            Spectacol* sNou = new Spectacol(nume, 8, 10, 20);
            adminSpectacoleFisier.AdaugaSpectacol(sNou);
            cout << "Salvat in Spectacole.txt cu valori standard.\n";
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
        else if (optiune == 11) {
            cout << "Ce spectacol cauti in fisier? ";
            cin.ignore();
            string nume;
            getline(cin, nume);
            Spectacol* gasit = adminSpectacoleFisier.CautaSpectacolFisier(nume);
            if (gasit != nullptr) {
                cout << "Gasit: " << gasit->numeSpectacol << " cu " << gasit->nrRanduri << " randuri.\n";
            }
            else {
                cout << "Nu a fost gasit in fisier.\n";
            }
            cout << "\nApasa Enter pentru a continua...";
            cin.ignore(256, '\n');
            cin.get();
        }
    }
    return 0;
}