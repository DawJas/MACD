#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

#define NUMBER_OF_SAMPLES 1000
#define USELLES_DATA 1600 //defines first x lines that we want to skip in input data, in BTC case we skip 1600 first lines
						  //because these dont have price of BTC (only thing that we care about) 

#define DEBUG 1          //if set to 1, outputs every transactions's detail
#define STARTING_BALANCE 1000
using namespace std;

void InputData(const string FileName, float* array) {
	fstream FileStream;
	FileStream.open(FileName, ios::in);
	string buffer;
	for (int i = 0; i < USELLES_DATA; i++)getline(FileStream, buffer);
	for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {

		getline(FileStream, buffer, ',');//data
		getline(FileStream, buffer, ',');//txVolume
		getline(FileStream, buffer, ',');//adjustedtXVolume
		getline(FileStream, buffer, ',');//txCount
		getline(FileStream, buffer, ',');//marketCap
		getline(FileStream, buffer, ',');//CENA
		array[i] = stod(buffer);
		getline(FileStream, buffer);//rest of the line
	}
	FileStream.close();
}

void OutputData(const string FileName, float* array) {
	fstream FileStream;
	FileStream.open(FileName, ios::out);
	for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
		FileStream << array[i] << endl; ;
	}
	FileStream.close();
}

float EMA(float* samples, int peroid, int current) {
	float alpha = 2 / (peroid + 1);
	float nominator = 0;
	float denominator = 0;
	for (int i = 0; i < peroid; i++) {
		if (current - i >= 0) {
			nominator += pow((1 - alpha), i) * samples[current - i];
			denominator += pow(1 - alpha, i);
		}
	}
	return nominator / denominator;
}

void CalculateIndicators(float* price, float* MACD, float* SIGNAL, int currentDay) {
	float EMA12=0, EMA26 = 0;
	SIGNAL[currentDay] = 0.0;

	EMA12 = EMA(price, 12, currentDay);

	EMA26 = EMA(price, 26, currentDay);

	MACD[currentDay] = EMA12- EMA26;
	
	SIGNAL[currentDay] = EMA(MACD, 9, currentDay);

}

void Buy(float& balance, float& BTCcount, int currentDay, float* price) {
	BTCcount += balance / price[currentDay];
	if (DEBUG && balance != 0) {
		cout << "Zakup dnia: " << currentDay << " po cenie: " << price[currentDay] << endl;
		cout << "Balans przed zakupem: " << balance << " zakupiona ilosc: " << BTCcount << endl << endl;

	}
	balance = 0;
}

void Sell(float& balance, float& BTCcount, int currentDay, float* price) {
	balance += price[currentDay] * BTCcount;
	if (DEBUG && BTCcount != 0) {
		cout << "Sprzedaz dnia: " << currentDay << " po cenie: " << price[currentDay] << endl;
		cout << "Ilosc sprzadana: " << BTCcount << " balans po: " << balance << endl << endl;
	}
	BTCcount = 0;
}


void Simulation(float balance, float*SIGNAL, float* MACD, float*price, float& BTCcount) {
	
	printf("START BALANCE:   %.2f\n", balance);
	for (int i = 1; i < NUMBER_OF_SAMPLES; i++) {
		CalculateIndicators(price, MACD, SIGNAL, i);
		if ((SIGNAL[i] > MACD[i] && SIGNAL[i - 1] < MACD[i - 1]) ||
			(SIGNAL[i] < MACD[i] && SIGNAL[i - 1] > MACD[i - 1]))
		{
			if (SIGNAL[i - 1] > MACD[i - 1]) {
				if(MACD[i]>0)
					Buy(balance, BTCcount, i, price);
			}
			else {
				if (MACD[i] < 0)
					Sell(balance, BTCcount, i, price);
			}
		}

	}

	Sell(balance, BTCcount, NUMBER_OF_SAMPLES-1, price);
	printf("END BALANCE:     %.2f\n", balance);
	//cout << "Profit: x" << balance / STARTING_BALANCE;
}

int main() {

	float price[NUMBER_OF_SAMPLES], MACD[NUMBER_OF_SAMPLES], SIGNAL[NUMBER_OF_SAMPLES],balance = STARTING_BALANCE, BTCcount = 0;

	InputData("BTC.csv", price);


		MACD[0] = 0;
		SIGNAL[0] = 0;


	Simulation(balance, SIGNAL, MACD, price, BTCcount);

	OutputData("MACD.csv", MACD);
	OutputData("SIGNAL.csv", SIGNAL);

	return 0;
}