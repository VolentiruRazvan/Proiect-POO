#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <ncurses.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// ======================================================================
// 1. ARHITECTURA DATELOR (ENTERPRISE)
// ======================================================================
struct User { string user, passHash, role; };
struct Seat { int r, c; bool res; string owner; bool selected = false; };
struct Screening { string movie, date, time; double price; vector<vector<Seat>> seats; };
struct Hall { string name; int rows, cols; vector<Screening> schedule; };
struct Cinema { string name, location; vector<Hall> halls; };

void to_json(json& j, const Seat& s) { j = json{ {"r", s.r}, {"c", s.c}, {"res", s.res}, {"owner", s.owner} }; }
void from_json(const json& j, Seat& s) { j.at("r").get_to(s.r); j.at("c").get_to(s.c); j.at("res").get_to(s.res); j.at("owner").get_to(s.owner); s.selected = false; }
void to_json(json& j, const Screening& s) { j = json{ {"movie", s.movie}, {"date", s.date}, {"time", s.time}, {"price", s.price}, {"seats", s.seats} }; }
void from_json(const json& j, Screening& s) { j.at("movie").get_to(s.movie); j.at("date").get_to(s.date); j.at("time").get_to(s.time); j.at("price").get_to(s.price); j.at("seats").get_to(s.seats); }
void to_json(json& j, const Hall& h) { j = json{ {"name", h.name}, {"rows", h.rows}, {"cols", h.cols}, {"schedule", h.schedule} }; }
void from_json(const json& j, Hall& h) { j.at("name").get_to(h.name); j.at("rows").get_to(h.rows); j.at("cols").get_to(h.cols); j.at("schedule").get_to(h.schedule); }
void to_json(json& j, const Cinema& c) { j = json{ {"name", c.name}, {"location", c.location}, {"halls", c.halls} }; }
void from_json(const json& j, Cinema& c) { j.at("name").get_to(c.name); j.at("location").get_to(c.location); j.at("halls").get_to(c.halls); }

const string DB_FILE = "cinemax_ultimate_db.json";
vector<Cinema> network;
User* activeUser = nullptr;
vector<User> users = {
    {"admin", to_string(hash<string>{}("admin")), "Admin"},
    {"casier", to_string(hash<string>{}("casier")), "Casier"}
};

// ======================================================================
// 2. BAZA DE DATE & SEEDING AUTOMAT
// ======================================================================
void saveData() { ofstream f(DB_FILE); f << json(network).dump(4); }

void generateCommercialData() {
    string locs[3][2] = { {"Cinema City Multiplex", "Suceava, Iulius Mall"}, {"Cineplexx Premium", "Bucuresti, Baneasa"}, {"Grand Cinema Digiplex", "Cluj, Vivo"} };
    string movies[5] = { "Matrix 4: Resurrections", "Dune: Part Two", "Oppenheimer", "Avatar 3", "Deadpool & Wolverine" };
    string dates[5] = { "26-05-2026", "27-05-2026", "28-05-2026", "29-05-2026", "30-05-2026" };
    string times[5] = { "12:00", "15:30", "18:00", "20:30", "23:00" };

    for (int i = 0; i < 3; i++) {
        Cinema c; c.name = locs[i][0]; c.location = locs[i][1];
        for (int j = 1; j <= 5; j++) {
            Hall h; h.name = "Sala " + to_string(j) + (j == 1 ? " VIP" : j == 2 ? " IMAX" : " 3D"); h.rows = 8; h.cols = 12;
            for (int k = 0; k < 5; k++) {
                Screening s; s.movie = movies[k]; s.date = dates[k]; s.time = times[k]; s.price = 30.0;
                for (int r = 0; r < h.rows; r++) {
                    vector<Seat> row;
                    for (int c = 0; c < h.cols; c++) row.push_back({ r, c, false, "", false });
                    s.seats.push_back(row);
                }
                h.schedule.push_back(s);
            }
            c.halls.push_back(h);
        }
        network.push_back(c);
    }
    saveData();
}

void loadData() {
    ifstream f(DB_FILE);
    if (f.is_open()) { try { json j; f >> j; network = j.get<vector<Cinema>>(); } catch (...) { generateCommercialData(); } }
    else { generateCommercialData(); }
}

// ======================================================================
// 3. GUI ENGINE (SOLID BLOCKS & LIVE SEARCH)
// ======================================================================
struct Button { int y, x, w; string text; int id; };

void initUI() {
    initscr(); start_color(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    printf("\033[?1003h\n"); mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL); mouseinterval(0);

    init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Header / Footer
    init_pair(2, COLOR_BLACK, COLOR_CYAN);   // Buton Inapoi/General
    init_pair(3, COLOR_WHITE, COLOR_RED);    // Buton Pericol
    init_pair(4, COLOR_WHITE, COLOR_GREEN);  // Buton Succes / Seat Liber Solid
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); // Text Accent
    init_pair(6, COLOR_WHITE, COLOR_BLACK);  // Borduri
    init_pair(7, COLOR_WHITE, COLOR_RED);    // Seat Ocupat Solid
    init_pair(8, COLOR_WHITE, COLOR_BLUE);   // Meniu Selectat
    init_pair(9, COLOR_BLACK, COLOR_WHITE);  // Meniu Neselectat
    init_pair(10, COLOR_BLACK, COLOR_YELLOW);// Seat Selectat pt Rezervare
}

bool checkClick(MEVENT ev, Button b) { return (ev.y == b.y && ev.x >= b.x && ev.x < b.x + b.w); }

// Corectat: Am adăugat parametrul opțional isHovered
void drawButton(Button b, int color_pair, bool isHovered = false) {
    int attrs = COLOR_PAIR(color_pair) | A_BOLD;
    if (isHovered) attrs |= A_REVERSE; // Inversează culorile pentru a atrage atenția
    attron(attrs);
    mvprintw(b.y, b.x, " %-*s ", b.w - 2, b.text.c_str());
    attroff(attrs);
}

void drawPanel(int y, int x, int h, int w, string title) {
    attron(COLOR_PAIR(6));
    mvwhline(stdscr, y, x, ACS_HLINE, w); mvwhline(stdscr, y + h, x, ACS_HLINE, w);
    mvwvline(stdscr, y, x, ACS_VLINE, h); mvwvline(stdscr, y, x + w, ACS_VLINE, h);
    mvwaddch(stdscr, y, x, ACS_ULCORNER); mvwaddch(stdscr, y, x + w, ACS_URCORNER);
    mvwaddch(stdscr, y + h, x, ACS_LLCORNER); mvwaddch(stdscr, y + h, x + w, ACS_LRCORNER);
    attron(COLOR_PAIR(5) | A_BOLD); mvprintw(y, x + 2, "[ %s ]", title.c_str()); attroff(COLOR_PAIR(5) | A_BOLD);
    attroff(COLOR_PAIR(6));
}

string inputDialog(string prompt, string extraInfo = "") {
    int w = 50, h = 8;
    int y = LINES / 2 - h / 2, x = COLS / 2 - w / 2;

    attron(COLOR_PAIR(6));
    for (int i = 0; i <= h; i++) mvwhline(stdscr, y + i, x, ' ', w + 1);

    drawPanel(y, x, h, w, prompt);

    if (!extraInfo.empty()) {
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(y + 2, x + 2, "%s", extraInfo.c_str());
        attroff(COLOR_PAIR(5) | A_BOLD);
    }

    char input[100];
    mvprintw(y + 4, x + 2, "> ");
    echo(); curs_set(1);
    getnstr(input, 98);
    noecho(); curs_set(0);
    return string(input);
}

string toLowerStr(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// ======================================================================
// 4. MODULELE APLICATIEI
// ======================================================================

void showArchive(Screening& scr) {
    clear();
    attron(COLOR_PAIR(1) | A_BOLD); mvwhline(stdscr, 0, 0, ' ', COLS);
    mvprintw(0, 2, " ARHIVA REZERVARI: %s ", scr.movie.c_str()); attroff(COLOR_PAIR(1) | A_BOLD);

    map<string, vector<string>> resMap;
    for (int r = 0; r < scr.seats.size(); r++) {
        for (int c = 0; c < scr.seats[0].size(); c++) {
            if (scr.seats[r][c].res) {
                string seatName = string(1, 'A' + r) + (c < 9 ? "0" : "") + to_string(c + 1);
                resMap[scr.seats[r][c].owner].push_back(seatName);
            }
        }
    }

    drawPanel(2, 2, LINES - 5, COLS - 5, "LISTA CLIENTI");
    int y = 4;
    double totalIncasari = 0;

    for (auto const& [client, locuri] : resMap) {
        string locuriStr = "";
        for (auto& l : locuri) locuriStr += l + " ";
        double dePlata = locuri.size() * scr.price;
        totalIncasari += dePlata;

        mvprintw(y++, 4, "Client: %-20s | Locuri: %-20s | Plata: %.2f RON", client.c_str(), locuriStr.c_str(), dePlata);
    }

    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(LINES - 2, 4, "TOTAL INCASARI SALA: %.2f RON", totalIncasari);
    attroff(COLOR_PAIR(5) | A_BOLD);

    mvprintw(LINES - 2, COLS - 30, "Apasa orice tasta pt INAPOI...");
    getch();
}

void openSeatMap(Screening& scr) {
    Button btnBack = { 3, 2, 15, "< INAPOI", 0 };
    Button btnArchive = { 3, 20, 20, "ARHIVA REZERVARI", 1 };
    Button btnReset = { 3, COLS - 22, 20, "! RESETEAZA SALA", 2 };
    Button btnBook = { LINES - 3, COLS - 25, 22, "$ REZERVA LOCURILE", 3 };

    int curY = 0, curX = 0;

    while (true) {
        clear();
        attron(COLOR_PAIR(1) | A_BOLD); mvwhline(stdscr, 0, 0, ' ', COLS);
        mvprintw(0, 2, " REZERVARI: %s | %s | %s | %.2f RON/Bilet ", scr.movie.c_str(), scr.date.c_str(), scr.time.c_str(), scr.price);
        attroff(COLOR_PAIR(1) | A_BOLD);

        drawButton(btnBack, 2);
        drawButton(btnArchive, 8);
        if (activeUser->role == "Admin") drawButton(btnReset, 3);

        int selectedCount = 0;
        for (auto& r : scr.seats) for (auto& c : r) if (c.selected) selectedCount++;

        if (selectedCount > 0) drawButton(btnBook, 4, true);

        int offY = 8, offX = (COLS - (scr.seats[0].size() * 6)) / 2;
        drawPanel(offY - 3, offX - 4, scr.seats.size() * 2 + 5, scr.seats[0].size() * 6 + 6, "HARTA SALA");
        attron(COLOR_PAIR(1)); mvprintw(offY - 2, (COLS - 14) / 2, " ECRAN CINEMA "); attroff(COLOR_PAIR(1));

        for (int r = 0; r < scr.seats.size(); r++) {
            mvprintw(offY + r * 2, offX - 6, "%c", 'A' + r);
            for (int c = 0; c < scr.seats[0].size(); c++) {
                int dy = offY + r * 2, dx = offX + c * 6;
                bool hover = (r == curY && c == curX);
                Seat& seat = scr.seats[r][c];

                if (seat.res) attron(COLOR_PAIR(7));
                else if (seat.selected) attron(COLOR_PAIR(10) | A_BOLD);
                else attron(COLOR_PAIR(4));

                if (hover) attron(A_REVERSE);

                mvprintw(dy, dx, " %02d ", c + 1);

                if (hover) attroff(A_REVERSE);
                if (seat.res) attroff(COLOR_PAIR(7));
                else if (seat.selected) attroff(COLOR_PAIR(10) | A_BOLD);
                else attroff(COLOR_PAIR(4));
            }
        }

        if (scr.seats[curY][curX].res) {
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(LINES - 3, offX, "STATUS: Ocupat de -> %s", scr.seats[curY][curX].owner.c_str());
            attroff(COLOR_PAIR(5) | A_BOLD);
        }
        else {
            mvprintw(LINES - 3, offX, "STATUS: Liber. Selecteaza cu ENTER / CLICK.");
        }

        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            for (auto& r : scr.seats) for (auto& c : r) c.selected = false;
            break;
        }
        if (ch == KEY_UP && curY > 0) curY--;
        if (ch == KEY_DOWN && curY < scr.seats.size() - 1) curY++;
        if (ch == KEY_LEFT && curX > 0) curX--;
        if (ch == KEY_RIGHT && curX < scr.seats[0].size() - 1) curX++;

        if (ch == '\n' || ch == '\r' || ch == ' ') {
            if (!scr.seats[curY][curX].res) {
                scr.seats[curY][curX].selected = !scr.seats[curY][curX].selected;
            }
        }

        if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK && (ev.bstate & BUTTON1_CLICKED || ev.bstate & BUTTON1_PRESSED)) {
                if (checkClick(ev, btnBack)) {
                    for (auto& r : scr.seats) for (auto& c : r) c.selected = false; break;
                }
                if (checkClick(ev, btnArchive)) { showArchive(scr); continue; }
                if (activeUser->role == "Admin" && checkClick(ev, btnReset)) {
                    for (auto& r : scr.seats) for (auto& c : r) { c.res = false; c.owner = ""; c.selected = false; }
                    saveData(); continue;
                }
                if (selectedCount > 0 && checkClick(ev, btnBook)) {
                    char buf[50]; snprintf(buf, sizeof(buf), "TOTAL: %.2f RON (%d bilete)", selectedCount * scr.price, selectedCount);
                    string client = inputDialog("Numele clientului:", string(buf));
                    if (!client.empty()) {
                        for (auto& r : scr.seats) for (auto& c : r) {
                            if (c.selected) { c.res = true; c.owner = client; c.selected = false; }
                        }
                        saveData();
                    }
                    continue;
                }

                for (int r = 0; r < scr.seats.size(); r++) {
                    for (int c = 0; c < scr.seats[0].size(); c++) {
                        int dy = offY + r * 2, dx = offX + c * 6;
                        if (ev.y == dy && ev.x >= dx && ev.x <= dx + 3) {
                            curY = r; curX = c;
                            if (!scr.seats[r][c].res) scr.seats[r][c].selected = !scr.seats[r][c].selected;
                        }
                    }
                }
            }
        }
    }
}

int listBrowser(string title, vector<string> items, bool canAdd, bool canDelete, int& realSel) {
    int sel = 0, scroll = 0, maxVisible = LINES - 16;
    if (maxVisible < 1) maxVisible = 1;
    string searchQ = "";

    Button btnAdd = { LINES - 4, 5, 20, "+ ADAUGA NOU", 1 };
    Button btnDel = { LINES - 4, 30, 20, "- STERGE SELECTAT", 2 };
    Button btnBack = { LINES - 4, COLS - 20, 15, "< INAPOI", 3 };

    while (true) {
        vector<int> filtered;
        for (int i = 0; i < items.size(); i++) {
            if (searchQ.empty() || toLowerStr(items[i]).find(toLowerStr(searchQ)) != string::npos) filtered.push_back(i);
        }

        if (sel >= filtered.size()) sel = filtered.size() - 1;
        if (sel < 0) sel = 0;

        clear();
        attron(COLOR_PAIR(1)); mvwhline(stdscr, 0, 0, ' ', COLS); mvprintw(0, 2, " %s ", title.c_str()); attroff(COLOR_PAIR(1));

        drawPanel(2, 2, 2, COLS - 5, "CAUTARE LIVE");
        mvprintw(3, 4, "🔍 %s_", searchQ.c_str());

        drawPanel(5, 2, LINES - 11, COLS - 5, "REZULTATE");

        for (int i = 0; i < maxVisible && i + scroll < filtered.size(); i++) {
            int idx = i + scroll;
            int yPos = 7 + i * 2;

            if (idx == sel) attron(COLOR_PAIR(8) | A_BOLD);
            else attron(COLOR_PAIR(9));

            mvprintw(yPos, 5, " %-60s ", items[filtered[idx]].c_str());

            if (idx == sel) attroff(COLOR_PAIR(8) | A_BOLD);
            else attroff(COLOR_PAIR(9));
        }

        if (filtered.empty()) {
            attron(COLOR_PAIR(5)); mvprintw(7, 5, "Niciun rezultat gasit pentru: %s", searchQ.c_str()); attroff(COLOR_PAIR(5));
        }

        if (canAdd) drawButton(btnAdd, 4);
        if (canDelete && !filtered.empty()) drawButton(btnDel, 3);
        drawButton(btnBack, 2);

        int ch = getch();
        if (ch == KEY_UP && sel > 0) { sel--; if (sel < scroll) scroll--; }
        else if (ch == KEY_DOWN && sel < filtered.size() - 1) { sel++; if (sel >= scroll + maxVisible) scroll++; }
        else if (ch == '\n' || ch == '\r') { if (!filtered.empty()) { realSel = filtered[sel]; return realSel; } }
        else if (ch == 27) { return -1; }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!searchQ.empty()) { searchQ.pop_back(); sel = 0; scroll = 0; }
        }
        else if (ch >= 32 && ch <= 126 && ch != KEY_MOUSE) {
            searchQ += (char)ch; sel = 0; scroll = 0;
        }

        if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK && (ev.bstate & BUTTON1_CLICKED || ev.bstate & BUTTON1_PRESSED)) {
                if (canAdd && checkClick(ev, btnAdd)) return -2;
                if (canDelete && !filtered.empty() && checkClick(ev, btnDel)) { realSel = filtered[sel]; return -3; }
                if (checkClick(ev, btnBack)) return -1;

                for (int i = 0; i < maxVisible && i + scroll < filtered.size(); i++) {
                    if (ev.y == 7 + i * 2 && ev.x >= 5 && ev.x <= 65) {
                        realSel = filtered[i + scroll]; return realSel;
                    }
                }
            }
        }
    }
}

void masterController() {
    bool isAdmin = (activeUser->role == "Admin");
    int selC = 0;
    while (true) {
        vector<string> locOpts;
        for (auto& c : network) locOpts.push_back(c.name + " (" + c.location + ")");

        int cIdx = listBrowser("MANAGEMENT LOCATII", locOpts, isAdmin, isAdmin, selC);
        if (cIdx == -1) break;
        if (cIdx == -2) { Cinema nc; nc.name = inputDialog("Nume Cinema"); nc.location = inputDialog("Oras/Locatie"); if (!nc.name.empty()) { network.push_back(nc); saveData(); } continue; }
        if (cIdx == -3) { network.erase(network.begin() + selC); saveData(); selC = 0; continue; }

        int selH = 0;
        while (true) {
            vector<string> hallOpts;
            for (auto& h : network[cIdx].halls) hallOpts.push_back(h.name + " | Cap: " + to_string(h.rows * h.cols) + " locuri");

            int hIdx = listBrowser(network[cIdx].name + " > SALI", hallOpts, isAdmin, isAdmin, selH);
            if (hIdx == -1) break;
            if (hIdx == -2) {
                Hall nh; nh.name = inputDialog("Nume Sala");
                try { nh.rows = stoi(inputDialog("Randuri")); nh.cols = stoi(inputDialog("Coloane")); network[cIdx].halls.push_back(nh); saveData(); }
                catch (...) {} continue;
            }
            if (hIdx == -3) { network[cIdx].halls.erase(network[cIdx].halls.begin() + selH); saveData(); selH = 0; continue; }

            int selS = 0;
            while (true) {
                vector<string> scrOpts;
                for (auto& s : network[cIdx].halls[hIdx].schedule) {
                    char buf[100]; snprintf(buf, sizeof(buf), "%s @ %s | %.2f RON | %s", s.date.c_str(), s.time.c_str(), s.price, s.movie.c_str());
                    scrOpts.push_back(string(buf));
                }

                int sIdx = listBrowser(network[cIdx].name + " > " + network[cIdx].halls[hIdx].name + " > PROGRAM", scrOpts, isAdmin, isAdmin, selS);
                if (sIdx == -1) break;
                if (sIdx == -2) {
                    Screening ns; ns.movie = inputDialog("Titlu Film"); ns.date = inputDialog("Data (ZZ-LL-AAAA)"); ns.time = inputDialog("Ora (HH:MM)");
                    try { ns.price = stod(inputDialog("Pret Bilet (RON)")); }
                    catch (...) { ns.price = 30.0; }
                    if (!ns.movie.empty()) {
                        for (int r = 0; r < network[cIdx].halls[hIdx].rows; r++) {
                            vector<Seat> row; for (int c = 0; c < network[cIdx].halls[hIdx].cols; c++) row.push_back({ r, c, false, "", false }); ns.seats.push_back(row);
                        }
                        network[cIdx].halls[hIdx].schedule.push_back(ns); saveData();
                    } continue;
                }
                if (sIdx == -3) { network[cIdx].halls[hIdx].schedule.erase(network[cIdx].halls[hIdx].schedule.begin() + selS); saveData(); selS = 0; continue; }

                openSeatMap(network[cIdx].halls[hIdx].schedule[sIdx]);
            }
        }
    }
}

void printAsciiLogo(int y, int x) {
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y, x, "  ____ _                             __  __ ");
    mvprintw(y + 1, x, " / ___(_)_ __   ___ _ __ ___   __ _  \\ \\/ / ");
    mvprintw(y + 2, x, "| |   | | '_ \\ / _ \\ '_ ` _ \\ / _` |  \\  /  ");
    mvprintw(y + 3, x, "| |___| | | | |  __/ | | | | | (_| |  /  \\  ");
    mvprintw(y + 4, x, " \\____|_|_| |_|\\___|_| |_| |_|\\__,_| /_/\\_\\ ");
    attroff(COLOR_PAIR(5) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(y + 6, x + 10, " ENTERPRISE POS SYSTEM V6 ");
    attroff(COLOR_PAIR(1));
}

int main() {
    loadData(); initUI();
    while (true) {
        clear();
        printAsciiLogo(LINES / 2 - 10, COLS / 2 - 22);

        drawPanel(LINES / 2 - 2, COLS / 2 - 20, 8, 40, "LOGIN TERMINAL");
        char user[50], pass[50];
        mvprintw(LINES / 2 + 1, COLS / 2 - 15, "User: "); mvprintw(LINES / 2 + 3, COLS / 2 - 15, "Pass: ");

        echo(); curs_set(1); mvgetnstr(LINES / 2 + 1, COLS / 2 - 8, user, 49); noecho();

        int i = 0, ch; move(LINES / 2 + 3, COLS / 2 - 8);
        while ((ch = getch()) != '\n' && ch != '\r' && i < 49) {
            if (ch == 127 || ch == '\b' || ch == KEY_BACKSPACE) { if (i > 0) i--; }
            else { pass[i++] = ch; }
        }
        pass[i] = '\0'; curs_set(0);

        string h = to_string(hash<string>{}(pass));
        activeUser = nullptr;
        for (auto& u : users) { if (u.user == string(user) && u.passHash == h) activeUser = &u; }

        if (activeUser) { masterController(); }
        else { attron(COLOR_PAIR(3)); mvprintw(LINES / 2 + 5, COLS / 2 - 10, "ACCES RESPINS!"); attroff(COLOR_PAIR(3)); getch(); }
    }
    printf("\033[?1003l\n"); endwin(); return 0;
}