// Beachte: Die include Reihenfolge ist nicht beliebig! 
// #include "Serial.h" //>> führt zu 161 Fehlern!
#include "ABI-Socket/Socket.hpp" 
#include "Serial.h"
#include "Date.h"
#include "Controller.h"

Controller::Controller(string host, int port, int kdNr, int anzZaehler)
{
	// für Socket
	this->port = port;
	this->host = host;

	// für Serielle Schnittstelle
	string port_nr = "";
	cout << "COM Port Nummer: ";

	cin >> port_nr;
	cout << endl;
	//port_nr = "2";

	string serieller_port("COM");
	serieller_port += port_nr;
	controllerSerial = new Serial((string)serieller_port, 9600, 8, ONESTOPBIT, NOPARITY); // wurde im 1. Teil (Serial) in der main initialisiert

	// Beispielwerte
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
	// ToDo 
	// 1. Anfrage zusammenbauen
	// 2. Anfrage an Stromzaehler senden

	cout << "Sende Auftrag: " << Controller::mitSteuerzeichen(auftrag) << endl;


	// Antwort von der Schnittstelle:
	// STX | 0;0;0;2;3;4;5 ... 5;6;2;1;0;P | ETX
	string nachricht;
	// ToDo: Nachricht vom Stromzaehler empfangen
	cout << "Empfange Nachricht vom Stromzaehler	" << Controller::mitSteuerzeichen(nachricht) << endl;

	// ToDo Fehlerabfragen erstellen 
	cout << "Kein STX gefunden! - Abbruch" << endl;

	cout << "Kein ETX" << endl;


	// ToDo: Nutzdaten ausschneiden
	string daten;
	cout << "Entferne STX, Prueffziffer und ETX         " << Controller::mitSteuerzeichen(daten) << endl; // Es sollten keine Steuerzeichen sichtbar sein!

	// ToDo: Semikolons entfernen und in das int-Feld werte speichern

	// Ausgabe
	cout << "Wandel den String in ein Feld von Integern um		"; for (int i : werte) cout << i;
	cout << endl << endl;

	// ToDo: Check Pruefziffer
	int checksumRemote = 0; // checksumRemote aus nachricht entnehen; letzte Stelle
	int checksumLocal = -1;  // checksumLocal aus daten lokal berechnen
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

	return -1;
}

// 2.3 Aufgabe
bool Controller::sendeTagesverbraeuche() {
	int verbrauchHeute[24] = { 0 };
	bool ergebnis = false;

	//a) Verbindung zur Seriellen aufnehmen

		// To Do


	// b) für alle Zähler:
		// Verbrauchswerte von der Schnittstelle lesen
		// und in verbrauchHeute stundenweise aufsummieren --> siehe Beispiel in der Aufgabe!

		// To Do

	// c) Schließen der seriellen Schnittstelle

		// To Do


	// d) Verbindung zum Server aufnehmen
		// Hinweis aus der Aufgabe: 
		// Die Klassen Date und Socket aus Material 4 
		// sind zur Implementierung zu verwenden.
	string zeile = "";
		// To Do

	//e) senden des Tagesverbrauchs
		//String zusammenstellen
		// [Kommando]sende   [Kundenummer]4711  [Nr. des Tages im Monat]15                   

		// To Do

		// Kommando senden
	controllerSocket->write(zeile);
	cout << "Controller sendet die Tagesverbraeuche: " << endl;
	cout << "Kommando | K.Nr | Tag | addierte Verbrauchswerte pro Stunde | Pruefziffer" << endl;
	cout << zeile << endl;
	controllerSocket->close();
	ergebnis = true;
	ergebnis = false; // Connect zum Server fehlgeschlagen!

	return ergebnis;
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
