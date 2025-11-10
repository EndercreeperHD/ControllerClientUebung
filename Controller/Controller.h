#pragma once
//#include "ABI-Socket/Socket.hpp"
#include "String.h"

class Socket;

// Übertragungs-Steuerzeichen
static const unsigned char SOH = 0x01;		// Start of Header
static const unsigned char STX = 0x02;		// Start of Text
static const unsigned char ETX = 0x03;		// End of Text (Ende eines Datenblocks);

class Controller
{
public:
	Controller(string host, int port, int kdNr, int anzZaehler);
	~Controller(void);

	bool sendeTagesverbraeuche();
	void eintragenVerbauch(int tag, int* werte);

	// Optional: Steuerzeichen werden auf der Konsole nicht gedruckt. Diese Funktion macht sie sichtbar.
	string mitSteuerzeichen(const string& s);


private:
	// für Socket
	string host;
	int port;
	Socket* controllerSocket;

	// für Serielle 
	Serial* controllerSerial;

	int kdNr;			// Kundenummer
	int anzZaehler;		// Anzahl Zaehler
	int werte[24];		// Extra

	int* ablesenZaehler(int nr);
	static int berechnePruefziffer(const int* werte);
};

