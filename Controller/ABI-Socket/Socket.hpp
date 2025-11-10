// Version 16.10.25 - Schwaiger
#ifndef SOCKET_HPP
#define SOCKET_HPP

// Einfache Client/Arbeits-Socket-Klasse fuer TCP (Windows/WinSock2)
// Diese Klasse wird sowohl als Client (Konstruktor mit Host/Port)
// als auch serverseitig als Arbeitssocket (Konstruktor mit SOCKET) benutzt.

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

using namespace std;

class Socket {
public:
    // Client-Konstruktor: bereitet einen TCP-Socket fuer Verbindungen zu host:port vor.
    // Macht intern: WSAStartup(), socket()
    Socket(string host, int port);

    // Server-seitiger Konstruktor: uebernimmt einen bereits verbundenen SOCKET
    // (kommt direkt aus ServerSocket::accept()).
    Socket(SOCKET socket);

    // Baut eine Verbindung zu (host,port) auf.
    // true = Verbindung steht, false = (noch) nicht erreichbar, blockert.
    bool connect();

    // Prueft nicht-blockierend, ob Daten verfuegbar sind (select mit Timeout ~1s).
    // Rueckgabe >0: Daten da; 0: Timeout; <0: Fehler.
    int dataAvailable();

    // Sendet einen 32-Bit-Integer als 4 Bytes (Little-Endian).
    void write(int b);

    // Sendet genau len Bytes aus dem Puffer b.
    // Didaktischer Modus: Das letzte Byte wird als '\n' gesendet (Zeilenabschluss).
    void write(char* b, int len);

    // Sendet einen String plus Newline '\n'.
    void write(string s);

    // Liest 4 Bytes und baut daraus einen 32-Bit-Integer (Little-Endian).
    int read();

    // Liest bis zu len Bytes (blockierend). Rueckgabe: tatsaechlich gelesene Bytes.
    int read(char* b, int len);

    // Liest bis zum Newline '\n' (max. 255 Zeichen) und liefert die Zeile ohne '\n'.
    string readLine();

    // Schliesst den Socket und ruft WSACleanup() auf.
    void close();

private:
    string host;                    // Ziel-IP als Text, z. B. "127.0.0.1"
    int    port = 0;                // Ziel-Port, z. B. 5586
    SOCKET socket = INVALID_SOCKET; // WinSock-Socket (Client oder verbundener Arbeitssocket)
};

#endif // SOCKET_HPP
