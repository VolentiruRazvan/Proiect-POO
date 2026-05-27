#include "CinemaCMS.hpp"
#include <fstream>
#include <functional>

// === IMPLEMENTARE SERIALIZARE ===
void to_json(json& j, const Seat& s) { j = json{ {"row", s.row}, {"col", s.col}, {"is_reserved", s.is_reserved} }; }
void from_json(const json& j, Seat& s) { j.at("row").get_to(s.row); j.at("col").get_to(s.col); j.at("is_reserved").get_to(s.is_reserved); }
void to_json(json& j, const Movie& m) { j = json{ {"title", m.title}, {"time", m.time}, {"seats", m.seats} }; }
void from_json(const json& j, Movie& m) { j.at("title").get_to(m.title); j.at("time").get_to(m.time); j.at("seats").get_to(m.seats); }
void to_json(json& j, const Hall& h) { j = json{ {"name", h.name}, {"movies", h.movies} }; }
void from_json(const json& j, Hall& h) { j.at("name").get_to(h.name); j.at("movies").get_to(h.movies); }
void to_json(json& j, const Cinema& c) { j = json{ {"name", c.name}, {"halls", c.halls} }; }
void from_json(const json& j, Cinema& c) { j.at("name").get_to(c.name); j.at("halls").get_to(c.halls); }
void to_json(json& j, const Network& n) { j = json{ {"name", n.name}, {"cinemas", n.cinemas} }; }
void from_json(const json& j, Network& n) { j.at("name").get_to(n.name); j.at("cinemas").get_to(n.cinemas); }

// === IMPLEMENTARE CLASĂ CMSApp ===
CMSApp::CMSApp() : activeUser(nullptr) {
    // Definire credențiale criptate
    users = {
        {"admin", to_string(hash<string>{}("admin")), "Admin"},
        {"casier", to_string(hash<string>{}("casier")), "Casier"}
    };
    loadData();
}

CMSApp::~CMSApp() {
    destroyUI();
}

void CMSApp::loadData() {
    ifstream file(DB_FILE);
    if (file.is_open()) {
        json j; file >> j; db = j.get<Network>();
    }
    else {
        // Date de bază dacă JSON-ul nu există
        db.name = "Reteaua Moldova";
        Cinema c{ "Cinema Premium Suceava" };
        Hall h{ "Sala 1 VIP" };
        Movie m{ "Interstellar", "19:00" };

        for (int i = 0; i < 8; i++) {
            vector<Seat> r;
            for (int j = 0; j < 12; j++) r.push_back({ i, j, false });
            m.seats.push_back(r);
        }
        h.movies.push_back(m); c.halls.push_back(h); db.cinemas.push_back(c);
        saveData();
    }
}

void CMSApp::saveData() {
    ofstream file(DB_FILE); file << json(db).dump(4);
}

void CMSApp::initUI() {
    initscr(); start_color(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Box Borders
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Liber
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Ocupat / Erori
    init_pair(4, COLOR_BLACK, COLOR_WHITE);  // Highlight Menu
    init_pair(5, COLOR_YELLOW, COLOR_BLACK); // Titluri
}

void CMSApp::destroyUI() { endwin(); }

void CMSApp::drawBordersAndLayout() {
    clear();
    attron(COLOR_PAIR(1));
    box(stdscr, 0, 0);
    mvwhline(stdscr, 2, 1, ACS_HLINE, COLS - 2); // Header separator
    mvwvline(stdscr, 3, 30, ACS_VLINE, LINES - 4); // Left Menu separator

    // T junctions
    mvwaddch(stdscr, 2, 0, ACS_LTEE);
    mvwaddch(stdscr, 2, COLS - 1, ACS_RTEE);
    mvwaddch(stdscr, 2, 30, ACS_TTEE);
    mvwaddch(stdscr, LINES - 1, 30, ACS_BTEE);
    attroff(COLOR_PAIR(1));

    // Header Content
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(1, 2, " CMS: %s ", db.name.c_str());
    attroff(COLOR_PAIR(5) | A_BOLD);
    if (activeUser) {
        mvprintw(1, COLS - 35, " User: %s | Rol: %s ", activeUser->username.c_str(), activeUser->role.c_str());
    }
    refresh();
}

bool CMSApp::loginScreen() {
    clear();
    attron(COLOR_PAIR(1)); box(stdscr, 0, 0); attroff(COLOR_PAIR(1));

    int start_y = LINES / 2 - 4;
    int start_x = COLS / 2 - 15;

    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(start_y, start_x, "=== AUTENTIFICARE SECURE ===");
    attroff(COLOR_PAIR(5) | A_BOLD);

    mvprintw(start_y + 2, start_x, "User: ");
    mvprintw(start_y + 3, start_x, "Pass: ");

    char user[50], pass[50];
    echo(); curs_set(1);
    mvgetnstr(start_y + 2, start_x + 6, user, 49);
    noecho();

    // Mascare parolă
    int i = 0, ch;
    move(start_y + 3, start_x + 6);
    while ((ch = getch()) != '\n' && ch != '\r' && i < 49) {
        if (ch == 127 || ch == '\b' || ch == KEY_BACKSPACE) { if (i > 0) i--; }
        else { pass[i++] = ch; }
    }
    pass[i] = '\0';
    curs_set(0);

    string hashedPass = to_string(hash<string>{}(pass));
    for (auto& u : users) {
        if (u.username == user && u.password_hash == hashedPass) {
            activeUser = &u;
            return true;
        }
    }

    attron(COLOR_PAIR(3) | A_BLINK);
    mvprintw(start_y + 5, start_x, " Acces Respins! Tasta pt Retry.");
    attroff(COLOR_PAIR(3) | A_BLINK);
    getch();
    return false;
}

void CMSApp::renderSeatMap(Movie& m) {
    int max_r = m.seats.size();
    int max_c = m.seats[0].size();
    int cur_y = 0, cur_x = 0;

    // Offset pentru "Right Panel"
    int off_y = 6;
    int off_x = 35;

    while (true) {
        drawBordersAndLayout();

        // Meniu Stânga Pasiv
        mvprintw(4, 2, " [ ] Inapoi la meniu (Q)");

        // Header Sală
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(4, off_x, " FILM: %s | ORA: %s ", m.title.c_str(), m.time.c_str());
        attroff(COLOR_PAIR(5) | A_BOLD);

        // Randare Scaune
        for (int r = 0; r < max_r; r++) {
            for (int c = 0; c < max_c; c++) {
                int dy = off_y + r * 2;
                int dx = off_x + c * 4;
                bool hover = (r == cur_y && c == cur_x);
                bool res = m.seats[r][c].is_reserved;

                if (hover) attron(COLOR_PAIR(4));
                else if (res) attron(COLOR_PAIR(3));
                else attron(COLOR_PAIR(2));

                mvprintw(dy, dx, "[%c]", res ? 'X' : '_');

                attroff(COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4));
            }
        }

        mvprintw(LINES - 3, off_x, "Sageti/Mouse: Navigare | ENTER/Click: Rezerva");
        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        switch (ch) {
        case KEY_UP: if (cur_y > 0) cur_y--; break;
        case KEY_DOWN: if (cur_y < max_r - 1) cur_y++; break;
        case KEY_LEFT: if (cur_x > 0) cur_x--; break;
        case KEY_RIGHT: if (cur_x < max_c - 1) cur_x++; break;
        case '\n': case '\r': case KEY_ENTER:
            m.seats[cur_y][cur_x].is_reserved = !m.seats[cur_y][cur_x].is_reserved;
            saveData(); break;
        case KEY_MOUSE:
            MEVENT ev;
            if (getmouse(&ev) == OK && (ev.bstate & BUTTON1_CLICKED)) {
                for (int r = 0; r < max_r; r++) {
                    for (int c = 0; c < max_c; c++) {
                        int dy = off_y + r * 2;
                        int dx = off_x + c * 4;
                        if (ev.y == dy && ev.x >= dx && ev.x <= dx + 2) {
                            m.seats[r][c].is_reserved = !m.seats[r][c].is_reserved;
                            cur_y = r; cur_x = c; saveData();
                        }
                    }
                }
            }
            break;
        }
    }
}

void CMSApp::dashboard() {
    int choice = 0;
    string opts[] = { "1. Vizualizare Bilete", "2. Configurare (Admin)", "3. Iesire" };
    int num_opts = 3;

    while (true) {
        drawBordersAndLayout();

        for (int i = 0; i < num_opts; i++) {
            if (i == choice) attron(COLOR_PAIR(4));
            mvprintw(5 + i * 2, 2, " %s ", opts[i].c_str());
            if (i == choice) attroff(COLOR_PAIR(4));
        }

        mvprintw(6, 35, "Selectati o actiune din panoul din stanga.");
        refresh();

        int ch = getch();
        switch (ch) {
        case KEY_UP: choice = (choice - 1 + num_opts) % num_opts; break;
        case KEY_DOWN: choice = (choice + 1) % num_opts; break;
        case '\n': case '\r': case KEY_ENTER:
            if (choice == 0 && !db.cinemas.empty()) {
                renderSeatMap(db.cinemas[0].halls[0].movies[0]);
            }
            else if (choice == 1) {
                if (activeUser->role == "Admin") {
                    mvprintw(8, 35, "-> Modul configurare deschis (In dev)...");
                }
                else {
                    attron(COLOR_PAIR(3));
                    mvprintw(8, 35, "-> EROARE: Drepturi insuficiente!");
                    attroff(COLOR_PAIR(3));
                }
                getch();
            }
            else if (choice == 2) {
                return; // Logout
            }
            break;
        }
    }
}

void CMSApp::run() {
    initUI();
    while (true) {
        activeUser = nullptr;
        if (loginScreen()) { dashboard(); }
        else { break; } // Exit if login failed or closed
    }
    destroyUI();
}