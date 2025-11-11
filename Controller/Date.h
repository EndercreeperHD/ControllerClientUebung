#pragma once

class Date
{
private:
	int day;
	int month;
	int year;

public:
	Date(void){}
	Date(int day, int month, int year){}
	Date(string date){}
	~Date(void){};

	bool equals(Date date){}
	int compareTo(Date date){}
	int getDay(){return 15;}
	int getMonth(){}
	int getYear(){}
};

