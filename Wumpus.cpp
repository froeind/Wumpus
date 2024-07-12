
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>

#include <fstream>

using namespace std;

////////// Hunt the Wumpus - Uraltspiel aus den 70ern - in den 80ern von mir auch auf HP41CV programmiert
/*
Jage den Wumpus, bevor er dich frisst!
Erkunde ein Labyrinth aus Höhlenräumen.
Achte auf Gerüche und Wind, die den Wumpus und Fledermäuse verraten.
Bewege dich geschickt und schieße den Wumpus mit deinen Pfeilen ab.
Erlege den Wumpus mit deinem Pfeil oder sterbe durch
gefressen vom Wumpus oder zu oft in eine Grube gefallen oder vom eigenen Pfeil getroffen
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

unsigned int random(const unsigned int from, const unsigned int to) {
    if (to < from) { return 0; }
    // das immer hier aufgerufen macht das Programm unsäglich langsam
    //srand((unsigned) time(NULL));
    return (unsigned int) (1.0 * rand() / RAND_MAX * (to - from) + from);
}

void checkRandom(const unsigned int from, const unsigned int to) {
    unsigned int t = from;
	cout << "Start\n";
    do {
    	if (t == random(t, to)) {
            cout << t << " ";
            t++;
        }
	} while (t <= to);
	cout << "\nJa, alle!\n";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool compareUnsigned(unsigned int a, unsigned int b) {
    // Priorisiere 0 vor allen anderen Werten
    if (a == 0) {
        return false;
    } else if (b == 0) {
        return true;
    } else {
        // Sortiere die restlichen Werte aufsteigend
        return a < b;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

unsigned int cavesWalkThrough(const unsigned int cave, const  vector<vector<unsigned int>>& caves, const unsigned int& countCaves, const unsigned int& countConnections, vector<bool>& isVisited, unsigned int countNotVisited) {
    if (isVisited[cave - 1]) { return countNotVisited; }
    isVisited[cave - 1] = true;
    countNotVisited--;
    if (countNotVisited == 0) { return countNotVisited; }
    for (unsigned int i = 0; i < countConnections; i++) {
        if (caves[cave - 1][i] != 0) {
            // Danke Gemini, ich muss das Minimum nehmen
            countNotVisited = min(countNotVisited, cavesWalkThrough(caves[cave - 1][i], caves, countCaves, countConnections, isVisited, countNotVisited));
        }
    }
    return countNotVisited;
}
bool cavesAreNotConsistent(const vector<vector<unsigned int>>& caves, const unsigned int& countCaves, const unsigned int& countConnections) {
    vector<bool> isVisited(countCaves);
    unsigned int countNotVisited = countCaves;
    countNotVisited = cavesWalkThrough(1, caves, countCaves, countConnections, isVisited, countNotVisited);
    return (countNotVisited > 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

string showVector(const vector<unsigned int>& caves, bool bracket = true) {
    string show = (bracket ? "[" : "");
    for (unsigned int cave : caves) {
        if (cave != 0) {
            show = show + to_string(cave) + (bracket ? "," : "") + " ";
            //cout << cave << "\n";
        }
    }
    return show.substr(0, show.length() - (bracket ? 2 : 1)) + (bracket ? "]" : "");
    //return show;
}

void showCaves(const vector<vector<unsigned int>>& caves, const unsigned int& countCaves) {
    cout << "Höhlen: " << "\n";
    for (unsigned int i = 0; i < countCaves; i++) {
        // ich durchlaufe alle Höhlen
        cout << i+1 << ": ";
        showVector(caves[i]);
        cout << "\n";
    }
    cout << "\n";
}

void showFocus(const vector<unsigned int>& focus) {
    cout << "Fokus: " << "\n";
    showVector(focus);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool wumpusIsNear(const vector<vector<unsigned int>>& caves, const unsigned int me, const unsigned int wumpus) {
    unsigned int meme = me - 1;
    bool wumpusIsNearB = find(caves[meme].begin(), caves[meme].end(), wumpus) != caves[meme].end();
    if (wumpusIsNearB) { return wumpusIsNearB; };
    for (unsigned int w = 0; w < caves[meme].size(); w++) {
        if (caves[meme][w] != 0) {
            wumpusIsNearB = wumpusIsNearB || (find(caves[caves[meme][w] - 1].begin(), caves[caves[meme][w] - 1].end(), wumpus) != caves[caves[meme][w] - 1].end());
            if (wumpusIsNearB) {
                return wumpusIsNearB;
            }
        }
    }
    return wumpusIsNearB;
}

bool batsOrPitsAreNear(const vector<vector<unsigned int>>& caves, const unsigned int me, const vector<unsigned int>& subfocus) {
    unsigned int meme = me - 1;
    bool batsOrPitsAreNearB = false;
    for (unsigned int s : subfocus) {
        batsOrPitsAreNearB = batsOrPitsAreNearB || (find(caves[meme].begin(), caves[meme].end(), s) != caves[meme].end());
        if (batsOrPitsAreNearB) { return batsOrPitsAreNearB; };
    }
    return batsOrPitsAreNearB;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

vector<unsigned int> trueTarget(const bool& autotest, const bool& debug, const unsigned int& me, const unsigned int& target, const vector<vector<unsigned int>>& caves) {
    // ausgehend von me durchlaufe ich alle Nebenhöhlen und deren Nebenhöhlen bis ich nach maximal 5 Schritten cave erreicht habe oder nicht
    // um doppeltes Betreten eine Höhle zu vermeiden mache ich eine Breitensuche, also werden von jeder ersten Nebenhöhle erst alle zweiten Nebenhöhlen aufgesucht
    // und ich merke mir natürlich alle betretenen Höhlen, dh. was ich schon kenne, verfolge ich woanders nicht weiter
    // und das baue ich jetzt als Liste auf
    // was ich auch noch überwachen muss, und da wäre ein Baum geschickter, wenn der Pfeil auf seinem Weg den Wunmpus trifft ist er auch hinüber
    // dazu darf ich aber nicht einfach alle Höhlen auf Wumpus überprüfen, sondern nur die auf dem konkreten Weg
    // also muss ich mir zusätzlich auch noch die Struktur merken, scheint aber trotzdem einfacher als ein Baum
    if (autotest) {
        cout << "trueTarget" << "\n";
    }
    vector<unsigned int> searched;
    vector<unsigned int> path;
    searched.push_back(me);
    path.push_back(0);
    unsigned int start = 0;
    // ? int ende = searched.size();
    unsigned int ende = 1;
    unsigned int nextende = ende;
    unsigned int actualpath = 0;
    unsigned int fullpath = 0;
    vector<unsigned int> trueTargetIA;
    bool breakloops = false;
    for (unsigned int l = 1; l <= 5; l++) {
        for (unsigned int s = start; s < ende; s++) {
            actualpath = path.at(s) * 10;
            for (unsigned int c = 0; c < caves[0].size(); c++) {
                unsigned int cave = caves[searched.at(s) - 1][c];
                if (cave != 0) {
                    searched.push_back(cave);
                    path.push_back(actualpath + c + 1);
                    if (cave == target) {
                        trueTargetIA.push_back(cave);
                        fullpath = path.back();
                        breakloops = true;
                        break;
                    }
                    nextende++;
                }
            }
            if (breakloops) {
                break;
            }
            start = ende;
            ende = nextende;
        }
        if (breakloops) {
            break;
        }
    }
    if (fullpath > 0) {
        // Zwischenstationen ermitteln, die brauche ich für den Wumpus-Schuß, den er kann auch schon vor dem Ziel getroffen werden
        start = 0;
        for (size_t r = searched.size() - 1ul; r > 0; r--) {
            if (path.at(r) == fullpath) {
                trueTargetIA.push_back(searched.at(r));
                fullpath = fullpath / 10;
                start++;
            };
        }
    } else {
        trueTargetIA[0] = searched.back();
    }
    if (debug) {
        cout << showVector(searched) << "\n";
        cout << showVector(path) << "\n";
        cout << showVector(trueTargetIA) << "\n";
    }
    return trueTargetIA;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int batChangeCave(const bool autotest, int mybatfly, const int cave, const int countLives, const int countArrows, vector<unsigned int> focus, int whichbat, const vector<vector<unsigned int>>& caves) {
    if (autotest) {
        cout << "batChangeCave " << ((mybatfly == 0) ? "Me" : "") << "\n";
    }
    // die erste Fledermaus trägt weiter
    // bei wenig Leben weiter, bei viel Leben kürzer
    // wenn ich noch viele Pfeile habe kürzer, bei wenigen Pfeilen weiter
    // mögliche weitere Fledermäuse flattern auch woandershin, aber alle nicht zum Wumpus
    // und die erste flattert auch noch eine Höhle weiter als die Mitnehmhöhle
    int newcave = 0;
    int howfar = (mybatfly == 0) ? (12-(countLives+countArrows)) : (2);
    for(int b = 0; b < howfar; b++) {
        newcave = 0;
        while (newcave == 0) {
            newcave = (int) caves[(unsigned int) (cave - 1)][random(0, (unsigned int) caves[0].size())];
        }
        if (mybatfly == 0) {
            if (b == howfar - 2) {
                // dahin geht der Transport, da kann auch der Wumpus sein
                mybatfly = newcave;
            }
        }
    }
    while (focus[1] == (unsigned int) newcave) {
        // die Fledermaus fliegt nicht zum Wumpus
        newcave = batChangeCave(autotest, -1, newcave, -1, -1, focus, whichbat, caves);
    }
    focus[2 + (unsigned int) whichbat] = (unsigned int) newcave;
    return (mybatfly == -1) ? newcave : mybatfly;
}

void wumpusChangeCave(const bool autotest, vector<unsigned int>& focus, const vector<vector<unsigned int>>& caves, const unsigned int countBats, unsigned int weight) {
    if (autotest) {
        cout << "wumpusChangeCave weight=" << weight << "\n";
    }
    // Wumpus bewegt sich einfach eine oder mehrere Höhlen weiter
    // Löcher machen ihm nichts aus
    // Fledermäuse vertreibt er
    unsigned int newcave = 0;
    unsigned int start = focus[1];
    while (newcave == 0) {
        while ((weight > 0) && (newcave != focus[0])) {
            newcave = caves[start - 1][random(0, (unsigned int) caves[0].size())];
            if (newcave != 0) {
                start = newcave;
                weight--;
            }
        }
    }
    focus[1] = newcave;
    // werden Fledermäuse vertrieben?
    for (int b = 0; b < (int) countBats; b++) {
        if (focus[2 + (unsigned int) b] == focus[1]) {
            batChangeCave(autotest, -1, (int) focus[1], -1, -1, focus, b, caves);
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main() {
    
    srand((unsigned) time(NULL));
    //cout.rdbuf()->pubsetbuf(0, 0);

    //checkRandom(30, 100);
    //checkRandom(1, 10000);

    bool autotest = false;
    bool debug = false;
    bool showcavesindebug = true;
    
    unsigned int countCavesV;
    unsigned int countConnectionsV;
    if (autotest) {
        countCavesV = random(30, 100);
        countConnectionsV = random(countCavesV / 2, countCavesV - 1);
    } else {
        cout << "Anzahl der Höhlen:\n";
        cin >> countCavesV;
        cout << "Anzahl der Verbindungen:\n";
        cin >> countConnectionsV;
    }
    //countCaves = (unsigned int) max( (int) countCaves, 10);
    const unsigned int countCaves = max(countCavesV, 10u);
    const unsigned int countConnections = min(countConnectionsV, countCaves - 1);
    cout << "Du spielst in " << countCaves << " Höhlen mit maximal " << countConnections << " Verbindungen zwischen den Höhlen." << "\n\n";

    // auf dem HP41 ging das meiner Erinnerung nach so, dass ich das Höhlensystem über AH.N1N2N3N4 (AktuelleHöhle als Vorkommateil und Nebenhöhlen als Nachkommastellen) darstellte
    //unsigned int caves[countCaves][countConnections];
    vector<vector<unsigned int>> caves(countCaves, vector<unsigned int>(countConnections, 0));
    //showCaves(caves, countCaves);

    bool notconsistent;
    do {
        fill(caves.begin(), caves.end(), vector<unsigned int>(countConnections, 0));
        notconsistent = true;
        for (unsigned int i = 0; i < countCaves; i++) {
            // ich durchlaufe alle Höhlen
            for (unsigned int j = 1; j < countConnections; j++) {
                // und suche noch freie Plätze für eine Verbindung
                if (caves[i][j] == 0) {
                    // und versuche für jede maximal viele Verbindungen aufzubauen
                    unsigned int neighbor = random(1, countCaves + 1);
                    // dazu ermittle ich eine zufällige Höhle, die Nachbar werden könnte
                    if (neighbor != i + 1) {
                        // diese muss natürlich eine andere Höhle als die aktuelle sein
                        bool alien = true;
                        for (unsigned int k = 0; k < j; k++) {
                            // und diese darf keine der schon vorhandenen Nachbarhöhlen sein
                            //alien = alien && (caves[i][k] != neighbor);
                            alien = alien && (caves[i][k] != neighbor);
                        }
                        if (alien) {
                            // jetzt habe ich eine Höhle gefunden, die Nachbar werden kann
                            // diese muss aber jetzt auch noch Platz für eine Verbindung haben und diese Verbindung darf noch nicht existieren
                            // den Platz muss ich überprüfen
                            // dass die Verbindung noch nicht existiert habe ich schon ermittelt, weil andernfalls diese Höhle ja hier auftaucht
                            for (unsigned int l = 0; l < countConnections; l++) {
                                if (caves[neighbor - 1][l] == 0) {
                                    // Verbindung möglich, also setzen
                                    caves[i][j] = neighbor;
                                    caves[neighbor - 1][l] = i + 1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        // konsistent ist das System, wenn ich ausgehend von Höhle 1 alle anderen Höhlen erreichen kann
        // die Grundlogik habe ich von Gemini und angepasst (Klasse brauche ich keine)
        notconsistent = cavesAreNotConsistent(caves, countCaves, countConnections);
        //notconsistent = false;
    } while (notconsistent);

    // in C++ genauso kompakt wie in Java, braucht nur dazu das selbstdefinierte compareUnsigned
    for (unsigned int i = 0; i < countCaves; i++) {
        sort(caves[i].begin(), caves[i].end(), compareUnsigned);
    }

    //showCaves(caves, countCaves);

    // Starthöhle, Wumpus, Grube(n), Fledermäuse setzen
    // Pfeile bereitstellen
    unsigned int countBats = 0;
    unsigned int countPits = 0;
    unsigned int countFocus = 0;
    do {
        countBats = max(4u, (random(4u, countCaves + 1) / 10));
        countPits = max(1u, (random(1u, countCaves + 1) / 10));
        countFocus = 1 + 1 + countBats + countPits;
    } while (countFocus > countCaves);
    vector<unsigned int> focus(countFocus);
    unsigned int nextcave;
    bool free;
    for (unsigned int i = 0; i < countFocus; i++) {
        do {
            nextcave = random(1, countCaves + 1);
            free = true;
            for (unsigned int j = 0; j < i; j++) {
                free = free && (focus[j] != nextcave);
            }
        } while (!free);
        focus[i] = nextcave;
    }
    unsigned int countLives = max(5u, (random(1u, countCaves+1) / 100));
    unsigned int countArrows = max(5u, (random(1u, countCaves+1) / 100));
    
    //showFocus(focus);

/*
ofstream outfile("test.txt");
if (outfile.is_open()) {
  outfile << showVector(caves[focus[0] - 1]) << std::endl;
  outfile.close();
} else {
  std::cerr << "Error opening file: test.txt" << std::endl;
}
*/

    string action;
    do {
        // Ausgabe Position und Umgebung (Nachbarhöhlen und eventuell Wumpus) und Frage nach Aktion
        string pewms = "Du bist in Höhle " + to_string(focus[0]) + ", Deine Nachbarhöhle(n): " + showVector(caves[focus[0] - 1], false) + ", Leben: " + to_string(countLives) + ", Pfeile: " + to_string(countArrows) + ", ";
        bool wumpusIsNearB = wumpusIsNear(caves, focus[0], focus[1]);
        vector<unsigned int> subvectorBats(focus.begin() + 2, focus.begin() + 2 + countBats);
        bool batsAreNear = batsOrPitsAreNear(caves, focus[0], subvectorBats);
        vector<unsigned int> subvectorPits(focus.begin() + 2 + countBats, focus.end());
        bool pitsAreNear = batsOrPitsAreNear(caves, focus[0], subvectorPits);
        pewms = pewms + ((wumpusIsNearB) ? "Wumpus ist nahe! " : "") + ((batsAreNear) ? "Fledermäuse sind nahe! " : "") + ((pitsAreNear) ? "Löcher sind nahe! " : "");
        cout << pewms << "\n";
        if (autotest) {
            debug = true;
            //  nun ja, erst mit C++, habe ich die zu große Zufälligkeit bemerkt
            // ich sollte mich nur vorschriftsgemäß bewegen und nur schießen, wenn auch Wumpus in der Nähe ist
            int actionI;
            do {
                actionI = (int) random(1, countCaves + 1);
            } while (find(caves[focus[0] - 1].begin(), caves[focus[0] - 1].end(), actionI) == caves[focus[0] - 1].end());
            unsigned int sign = 0;
            if (wumpusIsNearB) {
                sign = random(1, 8);
            }
            actionI = (sign == 1) ? -actionI : actionI;
            action = to_string(actionI);
            cout << ("Deine Wahl ist " + action) << "\n";
        } else {
            cout << "Was willst Du machen? (Bewegen" << (countArrows > 0 ? ", -Schießen" : "") << ")" << "\n";
            cin >> action;
        }
        bool shoot = false;
        if (action[0] == '-') {
            action = action.substr(1);
            shoot = true;
        }
        // falsche Eingaben muss ich abfangen, da ich - für das Schießen verwende, wird nur noch ein + akzeptiert (sollte so sein) und das + tut nicht weh
        try { nextcave = (unsigned int) stoi(action);
        } catch (invalid_argument const&) { nextcave = focus[0]; }
        if (nextcave == focus[0]) {
            cout << "Du bist schon hier!" << "\n";
        } else if (nextcave == 0) {
            debug = ! debug;
            cout << "Debugging " << (debug ? "" : "de") << "aktiviert!" << "\n";
            if (showcavesindebug) {
                cout << "\n";
                for (unsigned int i = 0; i < countCaves; i++) {
                    cout << (i + 1) << " = " << showVector(caves[i]) << "\n";
                }
                cout << "\n";
                cout << showVector(focus) << "\n";
                showcavesindebug = false;
            }
        } else if (nextcave > countCaves) {
            cout << "Diese Höhle gibt es nicht!" << "\n";
        } else {
            if (shoot) {
                // Ziel kann "irgendwo" liegen,
                // entweder auf dem Weg ist der Wumpus oder nicht,
                // oder Ziel ist gar nicht erreichbar, dann geht es zufällig weiter, dann kann ich auch selber getroffen werden
                // ein Pfeil fliegt maximal 5 Höhlen
                // aber es müssen natürlich auch Pfeile noch da sein
                if (countArrows == 0) {
                    cout << "Du hast keine Pfeile mehr, Du musst also selber dorthin/irgendwohin gehen!" << "\n";
                    shoot = false;
                    nextcave = focus[0];
                } else {
                    countArrows--;
                    // Check auf Existenz des Ziels
                    // wenn es existiert wird wieder nextcave zurückgegeben, ansonsten eine zufällige Höhle
                    vector<unsigned int> target = trueTarget(autotest, debug, focus[0], nextcave, caves);
                    if (autotest) {
                        cout << "trueTarget Check" << "\n";
                    }
                    if (target[0] == nextcave) {
                        // Schuss in erreichbare Höhle, jetzt alle Höhlen bis dahin auf Wumpus testen
                        for (unsigned int c : target) {
                            if (c == focus[1]) {
                                cout << ("Du hast den Wumpus erschossen!") << "\n";
                                return 0;
                            }
                        }
                    } else if (target[0] == focus[0]) {
                                cout << ("Du hast Dich selber erschossen!") << "\n";
                                return 0;
                    }
                    // Wumpus bewegt sich und "betroffene" Fledermäuse auch
                    wumpusChangeCave(autotest, focus, caves, countBats, 1);
                }
            }
            if (!shoot) {
                // Höhle wechseln, Test auf Wumpus, Fledermäuse, Loch
                // in der Reihenfolge, weil zuerst packt der Wumpus zu, dann könnten die Fledermäuse noch vor einem Loch retten und dann wird reingefallen
                // danach eventuelle Bewegung von Wumpus und sein tödlicher Angriff wenn er zu nahe gekommen ist
                focus[0] = nextcave;
                if (focus[1] == nextcave) {
                    cout << "Wumpus hat Dich getötet!";
                    return 0;
                } else {
                    // Fledermäuse
                    vector<unsigned int> subvectorAllBats(focus.begin() + 2, focus.begin() + 2 + countBats);
                    if (find(subvectorAllBats.begin(), subvectorAllBats.end(), nextcave) != subvectorAllBats.end()) {
                        cout << "Hui, eine Fledermaus packt Dich und flattert mit Dir in eine andere Höhle!" << "\n";
                        int mybatfly = 0;
                        int whichbat = 0;
                        while (whichbat >= 0) {
                            vector<unsigned int>::iterator it = find(subvectorAllBats.begin(), subvectorAllBats.end(), nextcave);
                            if (it != subvectorAllBats.end()) {
                                whichbat = (int) distance(subvectorAllBats.begin(), it);
                            } else {
                                whichbat = -1;
                            }
                            if (whichbat >= 0) {
                                // die erste Fledermaus trägt weiter
                                // bei wenig Leben weiter, bei viel Leben kürzer
                                // wenn ich noch viele Pfeile habe kürzer, bei wenigen Pfeilen weiter
                                // mögliche weitere Fledermäuse flattern auch woandershin, aber alle nicht zum Wumpus
                                // und die erste flattert auch noch eine Höhle weiter als die Mitnehmhöhle
                                int batfly = batChangeCave(autotest, mybatfly, (int) nextcave, (int) countLives, (int) countArrows, focus, whichbat, caves);
                                if (mybatfly == 0) { mybatfly = batfly; focus[0] = (unsigned int) mybatfly;}
                                subvectorAllBats[(unsigned int) whichbat] = 0;
                            }
                        }
                    }
                    // und das sind die restlichen Fledermäuse, die auch dann eine Flatter machen
                    for(int b = 0; b < (int) countBats; b++) {
                        if (subvectorAllBats[(unsigned int) b] != 0) {
                            batChangeCave(autotest, -1, (int) subvectorAllBats[(unsigned int) b], -1, -1, focus, b, caves);
                        }
                    }
                    // Löcher
                    vector<unsigned int> subvectorAllPits(focus.begin() + 2, focus.begin() + 2 + countBats);
                    if (find(subvectorAllPits.begin(), subvectorAllPits.end(), focus[0]) != subvectorAllPits.end()) {
                        countLives--;
                        if (countLives == 0) {
                            cout << "Du bist zum letzten Mal in ein Loch gefallen. Du bist tot!" << "\n";
                            //running = false;
                            return 0;
                        } else {
                            cout << "Du bist in ein Loch gefallen!" << "\n";
                        }
                    }
                    // wenn Wumpus in Nachbarhöhle, dann kann er sich bewegen und dann auch zu Dir mit etwas höherer Wahrscheinlichkeit
                    if (find(caves[focus[0] - 1].begin(), caves[focus[0] - 1].end(), focus[1]) != caves[focus[0] - 1].end()) {
                        wumpusChangeCave(autotest, focus, caves, countBats, 5);
                        if (focus[1] == focus[0]) {
                            cout << "Wumpus hat Dich getötet!" << "\n";
                            return 0;
                        }
                    }
                    if (debug) {
                        cout << showVector(focus) << "\n";
                    }
                    // gnädigerweise kannst Du jetzt auch noch ab und zu einen Pfeil finden
                    if (countArrows <= 1) {
                        if (random(1, (unsigned int) caves[0].size()) == 1) {
                            cout << "Oh, Du findest einen Pfeil!" << "\n";
                            countArrows++;
                        }
                    }
                }
            }
        }
    } while (true);

	return 0;
}
