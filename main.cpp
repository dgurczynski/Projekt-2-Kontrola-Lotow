#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <map>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

const int SZEROKOSC = 60;
const int WYSOKOSC = 10;

class Samolot {
public:
    char identyfikator;
    int x, y;
    int docelowa_wysokosc;
    char symbol;
    int kierunek_ruchu;
    bool oczekuje_na_zmiane;
    char nowy_kierunek;
    int nowe_wznoszenie;

    Samolot(char id, int pozycja_poczatkowa_Y, bool z_lewej) {
        identyfikator = id;
        y = pozycja_poczatkowa_Y;
        docelowa_wysokosc = 0;
        symbol = '=';
        oczekuje_na_zmiane = false;
        if (z_lewej) {
            x = 0;
            kierunek_ruchu = 1;
        }
        else {
            x = SZEROKOSC - 6;
            kierunek_ruchu = -1;
        }
    }

    void wykonaj_ruch() {
        x += kierunek_ruchu;
        if (oczekuje_na_zmiane) {
            symbol = nowy_kierunek;
            docelowa_wysokosc = nowe_wznoszenie;
            oczekuje_na_zmiane = false;
        }
        if (symbol == '/' && docelowa_wysokosc > 0) {
            if (y > 0) y--;
            docelowa_wysokosc--;
            if (docelowa_wysokosc == 0) symbol = '=';
        }
        else if (symbol == '\\' && docelowa_wysokosc > 0) {
            if (y < WYSOKOSC - 1) y++;
            docelowa_wysokosc--;
            if (docelowa_wysokosc == 0) symbol = '=';
        }
    }

    bool czy_opuscil_ekran() const {
        return x < -5 || x > SZEROKOSC + 5;
    }

    string stan() const {
        string tekst;
        if (kierunek_ruchu == 1) {
            tekst += symbol;
            tekst += "(";
            tekst += identyfikator;
            tekst += to_string(docelowa_wysokosc);
            tekst += ")";
        }
        else {
            tekst += "(";
            tekst += identyfikator;
            tekst += to_string(docelowa_wysokosc);
            tekst += ")";
            tekst += symbol;
        }
        return tekst;
    }
};

class Gra {
    vector<Samolot> samoloty;
    map<char, Samolot*> mapa_samolotow;
    int licznik_samolotow = 0;
    int ostatnia_znana_Y = -3;
    int licznik_tur = 0;
    int bezpiecznie_przeprowadzone = 0;
    bool trwanie_gry = true;

public:
    void wyczysc_ekran() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void rysowanie_planszy() {
        vector<string> ekran(WYSOKOSC, string(SZEROKOSC, ' '));
        vector<Samolot> aktywne_samoloty;

        for (auto& samolot : samoloty) {
            if (samolot.czy_opuscil_ekran()) {
                bezpiecznie_przeprowadzone++;
                continue;
            }
            aktywne_samoloty.push_back(samolot);
            if (samolot.x >= 0 && samolot.x < SZEROKOSC && samolot.y >= 0 && samolot.y < WYSOKOSC) {
                string tekst = samolot.stan();
                if (samolot.x + (int)tekst.size() <= SZEROKOSC) {
                    for (size_t i = 0; i < tekst.size(); ++i) {
                        ekran[samolot.y][samolot.x + i] = tekst[i];
                    }
                }
            }
        }

        samoloty = aktywne_samoloty;
        mapa_samolotow.clear();
        for (auto& samolot : samoloty) {
            mapa_samolotow[samolot.identyfikator] = &samolot;
        }

        cout << "=" << string(SZEROKOSC, '=') << "=" << endl;
        for (const auto& wiersz : ekran) {
            cout << "|" << wiersz << "|" << endl;
        }
        cout << "=" << string(SZEROKOSC, '=') << "=" << endl;
        cout << "Tura: " << licznik_tur << " | Bezpiecznie przeprowadzone: " << bezpiecznie_przeprowadzone << endl;
    }

    bool sprawdz_kolizje() {
        for (size_t i = 0; i < samoloty.size(); ++i) {
            for (size_t j = i + 1; j < samoloty.size(); ++j) {
                int roznica_x = abs(samoloty[i].x - samoloty[j].x);
                int roznica_y = abs(samoloty[i].y - samoloty[j].y);
                if (roznica_x <= 2 && roznica_y <= 2) return true;
            }
        }
        return false;
    }

    void aktualizuj_pozycje() {
        for (auto& samolot : samoloty) {
            samolot.wykonaj_ruch();
        }
    }

    void dodaj_nowy_samolot() {
        if (licznik_tur % 5 == 0 && rand() % 3 == 0) {
            int y;
            int proby = 0;
            do {
                y = rand() % WYSOKOSC;
                proby++;
            } while ((abs(y - ostatnia_znana_Y) <= 1) && proby < 10);

            Samolot nowy_samolot('A' + licznik_samolotow++ % 26, y, rand() % 2 == 0);
            ostatnia_znana_Y = y;

            samoloty.push_back(nowy_samolot);
            mapa_samolotow[nowy_samolot.identyfikator] = &samoloty.back();
        }
    }

    void wykonaj_polecenie(string komenda) {
        if (komenda.empty()) return;

        transform(komenda.begin(), komenda.end(), komenda.begin(), [](unsigned char c) {
            return std::toupper(c);
            });

        char identyfikator = komenda[0];
        if (mapa_samolotow.find(identyfikator) == mapa_samolotow.end()) return;

        Samolot* samolot = mapa_samolotow[identyfikator];

        if (komenda.size() >= 3) {
            if (komenda[1] == '/') {
                samolot->oczekuje_na_zmiane = true;
                samolot->nowy_kierunek = '/';
                samolot->nowe_wznoszenie = komenda[2] - '0';
            }
            else if (komenda[1] == '\\') {
                samolot->oczekuje_na_zmiane = true;
                samolot->nowy_kierunek = '\\';
                samolot->nowe_wznoszenie = komenda[2] - '0';
            }
        }
        else if (komenda.size() == 2 && komenda[1] == 'C') {
            samolot->oczekuje_na_zmiane = false;
        }
    }

    void uruchom() {
        srand(time(0));

        while (trwanie_gry) {
            cout << "Wprowadz komende: ";
            string wejscie;
            getline(cin, wejscie);

            if (wejscie.empty()) continue;

            if (wejscie == " ") {
                licznik_tur++;
                aktualizuj_pozycje();
                if (sprawdz_kolizje()) {
                    trwanie_gry = false;
                    break;
                }
                dodaj_nowy_samolot();
                wyczysc_ekran();
                rysowanie_planszy();
            }
            else {
                wykonaj_polecenie(wejscie);
            }
        }

        wyczysc_ekran();
        rysowanie_planszy();
        cout << "Kolizja! Gra zakonczona." << endl;
        cout << "Wynik: " << licznik_tur << " tur, " << bezpiecznie_przeprowadzone << " samolotow bezpiecznie przeprowadzonych." << endl;
    }
};

int main() {
    Gra gra;
    gra.uruchom();
    return 0;
}