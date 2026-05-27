#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <ncurses.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// === STRUCTURI DE DATE ===
struct User { string username, password_hash, role; };
struct Seat { int row, col; bool is_reserved; };
struct Movie { string title, time; vector<vector<Seat>> seats; };
struct Hall { string name; vector<Movie> movies; };
struct Cinema { string name; vector<Hall> halls; };
struct Network { string name; vector<Cinema> cinemas; };

// === SERIALIZARE JSON ===
void to_json(json& j, const Seat& s);
void from_json(const json& j, Seat& s);
void to_json(json& j, const Movie& m);
void from_json(const json& j, Movie& m);
void to_json(json& j, const Hall& h);
void from_json(const json& j, Hall& h);
void to_json(json& j, const Cinema& c);
void from_json(const json& j, Cinema& c);
void to_json(json& j, const Network& n);
void from_json(const json& j, Network& n);

// === CLASA PRINCIPALĂ (TUI + LOGICĂ) ===
class CMSApp {
private:
    Network db;
    User* activeUser;
    vector<User> users;
    const string DB_FILE = "cinema_network.json";

    // Ncurses Windows
    WINDOW* sideMenu;
    WINDOW* mainWindow;

    void initUI();
    void destroyUI();
    void drawBordersAndLayout();
    void loadData();
    void saveData();

    // Ecrane
    void renderSeatMap(Movie& movie);

public:
    CMSApp();
    ~CMSApp();
    void run();
    bool loginScreen();
    void dashboard();
};