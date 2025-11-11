// Beachte: Die include Reihenfolge ist nicht beliebig! 
// #include "Serial.h" //>> führt zu 161 Fehlern!
#include "ABI-Socket/Socket.hpp" 
#include "Serial.h"
#include "Date.h"
#include "Controller.h"

Controller::Controller(string host, int port, int kdNr, int anzZaehler)
{
	//für Socket
	this->port = port;
	this->host = host;

	//für Serielle Schnittstelle
	string port_nr = "";
	cout << "COM Port Nummer: (2) \n";

	//cin >> port_nr;
	cout << endl;
	port_nr = "3";

	string serieller_port("COM");
	serieller_port += port_nr;
	controllerSerial = new Serial((string)serieller_port, 9600, 8, ONESTOPBIT, NOPARITY); // wurde im 1. Teil (Serial) in der main initialisiert

	controllerSocket = nullptr;
	//Beispielwerte
	this->anzZaehler = anzZaehler;
	this->kdNr = kdNr;
	for (int i = 0; i < 24; ++i) werte[i] = 0;
}


Controller::~Controller(void)
{
	controllerSerial->close();
	delete controllerSerial;
}

int* Controller::ablesenZaehler(int nr) {
	cout << '\n' << "------------- Controller-------------" << endl;

	// Anfrage: SOH | "READ" | STX | <nr ASCII> | ETX | '\n'
	string auftrag;
	auftrag += SOH;
	auftrag += "READ";
	auftrag += STX;
	auftrag += to_string(nr);
	auftrag += ETX;
	// 1. Anfrage zusammenbauen
	// 2. Anfrage an Stromzaehler senden
	controllerSerial->write(auftrag);

	cout << "Sende Auftrag: " << Controller::mitSteuerzeichen(auftrag) << endl;


	// Antwort von der Schnittstelle:
	// STX | 0;0;0;2;3;4;5 ... 5;6;2;1;0;P | ETX
	string nachricht = controllerSerial->readLine();
	// ToDo: Nachricht vom Stromzaehler empfangen
	cout << "Empfange Nachricht vom Stromzaehler	" << Controller::mitSteuerzeichen(nachricht) << endl;

	// ToDo Fehlerabfragen erstellen 
	if (nachricht[0] != STX) {
		cout << "Kein STX gefunden! - Abbruch" << endl;
		return nullptr;
	}
	if (nachricht[nachricht.size() - 1] != STX) {
		cout << "Kein ETX" << endl;
		return nullptr;
	}

	// ToDo: Nutzdaten ausschneiden
	string daten = nachricht;
	daten.erase( 0, 1);
	daten.erase(daten.size() - 1, 1);
	daten.erase(daten.size() - 1, 1);
	cout << "Entferne STX, Prueffziffer und ETX         " << Controller::mitSteuerzeichen(daten) << endl; // Es sollten keine Steuerzeichen sichtbar sein!

	// ToDo: Semikolons entfernen und in das int-Feld werte speichern
	int wert[24] = { 0 };
	for (int i = 0; i < 24; i++) {
		wert[i] = daten[i] - '0';
	}
	// Ausgabe
	cout << "Wandel den String in ein Feld von Integern um		"; for (int i : werte) cout << i;
	cout << endl << endl;

	// ToDo: Check Pruefziffer
	int checksumRemote = nachricht[nachricht.size() - 2] - '0'; // checksumRemote aus nachricht entnehen; letzte Stelle
	int checksumLocal = berechnePruefziffer(wert);  // checksumLocal aus daten lokal berechnen
	if (checksumLocal != checksumRemote) {
		cout << "Pruefziffer falsch (lokal=" << checksumLocal
			<< ", remote=" << checksumRemote << ")" << endl;
		return nullptr;
	}

	return werte;
}

// 2.2 Aufgabe 
int Controller::berechnePruefziffer(const int* werte) {
	// ToDo
	int erg = 0;
	for (int i = 0; i < 24;i++) {
		if (i % 2 == 0) {
			erg += werte[i] * 2;
		}
		else {
			erg += werte[i] * 3;
		}
	}
	return erg;
}

// 2.3 Aufgabe
bool Controller::sendeTagesverbraeuche() {
	int verbrauchHeute[24] = { 0 };
	bool ergebnis = false;

	//a) Verbindung zur Seriellen aufnehmen
	if (!controllerSerial->open()) {
		cout << "Serial konnte nicht geoeffnet werden\n";
		return false;
	}

	// b) für alle Zähler:
		// Verbrauchswerte von der Schnittstelle lesen
		// und in verbrauchHeute stundenweise aufsummieren --> siehe Beispiel in der Aufgabe!
	for (int i = 0; i < anzZaehler; i++) {
		int* input = ablesenZaehler(i);
		for (int j = 0; j < 24; j++) {
			verbrauchHeute[j] += input[j];
		}
	}
	// c) Schließen der seriellen Schnittstelle
	controllerSerial->close();

	// d) Verbindung zum Server aufnehmen
		// Hinweis aus der Aufgabe: 
		// Die Klassen Date und Socket aus Material 4 
		// sind zur Implementierung zu verwenden.
	Socket* controllerSocket = new Socket("127.0.0.1",54321);
	if (!controllerSocket->connect()) return false;
	Date d;
	string zeile = "sende" + kdNr + d.getDay();
	for (int i = 0; i < 24;i++) {
		zeile += to_string(verbrauchHeute[i]) + ';';
	}
	zeile += to_string(berechnePruefziffer(verbrauchHeute));

	//e) senden des Tagesverbrauchs
		//String zusammenstellen
		// [Kommando]sende   [Kundenummer]4711  [Nr. des Tages im Monat]15                  

		// Kommando senden
	controllerSocket->write(zeile);
	cout << "Controller sendet die Tagesverbraeuche: " << endl;
	cout << "Kommando | K.Nr | Tag | addierte Verbrauchswerte pro Stunde | Pruefziffer" << endl;
	cout << zeile << endl;
	controllerSocket->close();

	return true;
}

// ist nicht Bestandteil der Aufgabe
void Controller::eintragenVerbauch(int tag, int* werte)
{
	// To Do
}

// Extra: Steuerzeichen werden nicht auf der Konsole ausgegeben. Hier wird das geändert.
string Controller::mitSteuerzeichen(const string& s) {
	string out;

	// „Sichtbar“-Ansicht
	for (int i = 0; i < (int)s.size(); ++i) {
		unsigned char c = (unsigned char)s[i];
		if (c >= 32 && c < 127) {
			out += (char)c;                 // druckbare ASCII
		}
		else {
			out += '<';
			out += to_string((int)c);       // Steuerzeichen als Zahl
			out += '>';
		}
	}
	return out;
}
