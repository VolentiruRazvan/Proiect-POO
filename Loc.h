#pragma once

class Loc {
public:
    int rand;
    int numar;
    bool ocupat;
    int pret;

    Loc() {
        rand = 0;
        numar = 0;
        pret = 0;
        ocupat = false;
    }

    Loc(int r, int n, int p) {
        rand = r;
        numar = n;
        pret = p;
        ocupat = false;
    }
};