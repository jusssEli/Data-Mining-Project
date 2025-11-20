#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

int main() {
    ifstream infile("transactions1.txt"); 
    if (!infile.is_open()) {
        cerr << "ERROR: Could not open transactions1.txt" << endl;
        return 1;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string first, last;
        ss >> first >> last;

        vector<string> purchased;
        string product;
        while (ss >> product) purchased.push_back(product);

        cout << first << " " << last << endl; 
        cout << "Purchased: ";
        for (const string& p : purchased) cout << p << " ";
        cout << endl;

        cout << "Next Recommendations: ";
        for (const string& p : purchased) {
            if (p == "Smartphone") cout << "Phone_Case ";
            else if (p == "Phone_Case") cout << "Screen_Protector ";
            else if (p == "Screen_Protector") cout << "Charger_(Wall_Adapter) ";
            else if (p == "Charger_(Wall_Adapter)") cout << "Charging_Cable_(USB-C_/_Lightning) ";
            else if (p == "Charging_Cable_(USB-C_/_Lightning)") cout << "Wireless_Charger ";
            else if (p == "Wireless_Charger") cout << "Portable_Power_Bank ";
            else if (p == "Portable_Power_Bank") cout << "Phone_Grip ";
            else if (p == "Phone_Grip") cout << "Bluetooth_Earbuds ";
            else if (p == "Bluetooth_Earbuds") cout << "Bluetooth_Over-Ear_Headphones ";
            else if (p == "Bluetooth_Over-Ear_Headphones") cout << "Smartwatch ";
            else if (p == "Smartwatch") cout << "Tablet ";
            else if (p == "Tablet") cout << "Smartphone "; 
            else cout << "(unknown product) ";
        }

        cout << endl << "---------------------------------------" << endl;
    }

    return 0;
}
