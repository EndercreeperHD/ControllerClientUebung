// Version 16.10.25 - Schwaiger
#include <winsock2.h>
#include <ws2tcpip.h>   // fuer inet_pton
#pragma comment(lib, "Ws2_32.lib")   // WinSock automatisch linken
// #pragma comment(lib, "Mswsock.lib") // optional, meist nicht noetig
#include <iostream>
#include <string>
#include <cstring>      // fuer memset
#include "../ABI-Socket/Socket.hpp"

using namespace std;

Socket::Socket(std::string host, int port)
{
    this->port = port;
    this->host = host;

    // 1) WinSock laden (Version 2.0)
    WSADATA m_wsa;
    int ok = WSAStartup(MAKEWORD(2, 0), &m_wsa);
    if (ok != 0) {
        cerr << "Error: WSAStartup fehlgeschlagen, Code " << ok << endl;
    }

    // 2) TCP-Client-Socket erzeugen
    this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket == INVALID_SOCKET) {
        cerr << "Error: socket() fehlgeschlagen" << endl;
    }
}

Socket::Socket(SOCKET socket)
{
    // Wird vom Server bei accept() verwendet: uebernimmt fertigen Arbeitssocket
    this->socket = socket;
}

bool Socket::connect()
{
    // Zieladresse (Server) vorbereiten
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((short)this->port);

    // String-IP in binaere Form umwandeln (z. B. "127.0.0.1")
    int ptonOk = inet_pton(addr.sin_family, this->host.c_str(), &addr.sin_addr);
    if (ptonOk != 1) {
        cerr << "Error: inet_pton() konnte Host nicht umwandeln (falsche IP?)" << endl;
        return false;
    }

    // Verbindung herstellen
    int rc = ::connect(socket, (SOCKADDR*)&addr, sizeof(SOCKADDR));
    if (rc == SOCKET_ERROR) {
        // Typisch: Server laeuft noch nicht -> man kann es spaeter erneut probieren
        return false;
    }
    return true;
}

int Socket::dataAvailable()
{
    // Nicht-blockierend per select(): wartet max. 1 Sekunde
    // Rueckgabe >0: Daten verfuegbar; 0: Timeout; <0: Fehler
    fd_set set;
    FD_ZERO(&set);
    FD_SET(socket, &set);

    timeval timeout;
    timeout.tv_sec = (long)1;   // 1 Sekunde warten
    timeout.tv_usec = (long)0;

    int rc = select(0, &set, NULL, NULL, &timeout);
    return rc;
}

void Socket::write(int b)
{
    // Sende einen 32-Bit-Integer in 4 Bytes (Little-Endian, wie im Original).
    // Hinweis: Sender und Empfaenger muessen dasselbe Format erwarten.

    unsigned char tmp[4];
    tmp[0] = (unsigned char)(b & 0xFF);
    tmp[1] = (unsigned char)((b >> 8) & 0xFF);
    tmp[2] = (unsigned char)((b >> 16) & 0xFF);
    tmp[3] = (unsigned char)((b >> 24) & 0xFF);

    int sent = send(socket, (const char*)tmp, 4, 0);
    if (sent != 4) {
        cerr << "Warnung: write(int) hat nicht alle 4 Bytes gesendet" << endl;
    }
}

void Socket::write(char* b, int len)
{
    // Urspruengliche Lehrlogik: Ein Char-Feld wird mit einem Newline '\n' abgeschlossen
    // Wir veraendern den Eingabepuffer NICHT,
    // sondern bauen lokal genau len Bytes zusammen und stellen sicher,
    // dass das LETZTE Byte ein '\n' ist (wie im Unterrichtsbeispiel "Hallo" mit Laenge 6).
    // Dadurch bleibt das beobachtete Verhalten kompatibel.

    if (len <= 0) return;

    // Lokaler Arbeits-Puffer exakt in der Laenge, die gesendet werden soll
    char* out = new char[len];

    // 1) len-1 Bytes kopieren (oder weniger, wenn len==1)
    int copyCount = len - 1;
    for (int i = 0; i < copyCount; i++) {
        out[i] = b[i];
    }

    // 2) Letztes Byte explizit als Newline setzen
    out[len - 1] = '\n';

    // 3) Exakt len Bytes senden
    int sent = send(socket, out, len, 0);
    if (sent != len) {
        cerr << "Warnung: write(char*,len) hat nicht alle Bytes gesendet" << endl;
    }

    delete[] out;
}

void Socket::write(std::string s)
{
    // Strings fuer den Unterricht immer als Zeile mit '\n' senden:
    s += '\n';
    int total = (int)s.length();

    // Einfacher send-Aufruf (fuer die Schule ausreichend)
    int sent = send(socket, s.c_str(), total, 0);
    if (sent != total) {
        cerr << "Warnung: write(string) hat nicht alle Bytes gesendet" << endl;
    }
}

int Socket::read()
{
    // Liest 4 Bytes und baut daraus einen Integer (Little-Endian)
    unsigned char tmp[4];
    int need = 4;
    int got = 0;

    // Einfache Schleife bis 4 Bytes da sind (blockierend)
    while (got < need) {
        int r = recv(socket, (char*)tmp + got, need - got, 0);
        if (r <= 0) {
            // Verbindung beendet oder Fehler
            cerr << "Warnung: read(int) fehlgeschlagen oder Verbindung beendet" << endl;
            return 0;
        }
        got += r;
    }

    int value = 0;
    value |= (int)tmp[0];
    value |= ((int)tmp[1]) << 8;
    value |= ((int)tmp[2]) << 16;
    value |= ((int)tmp[3]) << 24;

    return value;
}

int Socket::read(char* b, int len)
{
    // Liest bis zu len Bytes (blockierend, kann weniger zurueckliefern, wenn Sender beendet)
    if (len <= 0) return 0;
    int r = recv(socket, b, len, 0);
    if (r <= 0) {
        // r==0 => Gegenstelle hat ordentlich geschlossen
        // r<0 => Fehler
        return 0;
    }
    return r;
}

std::string Socket::readLine()
{
    // Liest Zeichen bis Newline '\n' (blockierend)
    char buffIn[256];
    int pos = -1;

    do {
        pos++;
        int r = recv(socket, buffIn + pos, 1, 0);
        if (r <= 0) {
            // Verbindung beendet oder Fehler
            return string(""); // leere Zeile als Notfall
        }
    } while (buffIn[pos] != '\n' && pos < 255);

    buffIn[pos] = '\0';  // '\n' wird ersetzt, String sauber terminiert
    return string(buffIn);
}

void Socket::close()
{
    // Arbeitssocket schliessen und WinSock aufraeumen
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
}
