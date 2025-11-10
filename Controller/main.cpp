#include "Serial.h"
#include "Controller.h"

using namespace std;

int main()
{
	cout << "--------------- Anmeldung Kunde ---------------" << endl;
	cout << "Kundennummer eingeben: (4711)";
	int kundenNr = 4711;
	cin >> kundenNr;

	Controller c("127.0.0.1", 54321, kundenNr, 3);
	if (!c.sendeTagesverbraeuche()) {
		cout << "Connect zum Server fehlgeschlagen!" << endl;
	}

	system("pause"); 
	return 0;
}
