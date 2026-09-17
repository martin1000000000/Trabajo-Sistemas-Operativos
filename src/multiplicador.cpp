#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// Función para leer matriz desde archivo
bool leerMatriz(const string& ruta, const string& separador, vector<vector<double>>& matriz) {
    ifstream file(ruta);
    if (!file.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo de matriz: " << ruta << endl;
        return false;
    }

    string linea;
    while (getline(file, linea)) {
        if (linea.empty()) continue;
        
        vector<double> fila;
        size_t pos = 0;
        string token;
        
        // Split manual usando el separador
        while ((pos = linea.find(separador)) != string::npos) {
            token = linea.substr(0, pos);
            if (!token.empty()) {
                fila.push_back(stod(token));
            }
            linea.erase(0, pos + separador.length());
        }
        if (!linea.empty()) {
            fila.push_back(stod(linea));
        }
        
        if (!fila.empty()) {
            matriz.push_back(fila);
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Argumentos esperados: rutaA rutaB separador
    if (argc < 4) {
        cout << "[ERROR Multiplicador] Argumentos insuficientes." << endl;
        cout << "Uso: multiplicador.exe <rutaA> <rutaB> <separador>" << endl;
        return 1;
    }

    string rutaA = argv[1];
    string rutaB = argv[2];
    string separador = argv[3];

    cout << "\n======================================" << endl;
    cout << "  PROGRAMA MULTIPLICADOR DE MATRICES" << endl;
    cout << "======================================" << endl;

    vector<vector<double>> A;
    vector<vector<double>> B;

    if (!leerMatriz(rutaA, separador, A)) return 1;
    if (!leerMatriz(rutaB, separador, B)) return 1;

    if (A.empty() || B.empty()) {
        cout << "[ERROR] Una de las matrices esta vacia." << endl;
        return 1;
    }

    size_t filasA = A.size();
    size_t colsA = A[0].size();
    size_t filasB = B.size();
    size_t colsB = B[0].size();

    // Validar dimensiones (A debe tener todas sus filas del mismo largo, etc)
    for (size_t i = 1; i < filasA; i++) {
        if (A[i].size() != colsA) {
            cout << "[ERROR] La matriz A no tiene dimensiones regulares." << endl;
            return 1;
        }
    }
    for (size_t i = 1; i < filasB; i++) {
        if (B[i].size() != colsB) {
            cout << "[ERROR] La matriz B no tiene dimensiones regulares." << endl;
            return 1;
        }
    }

    if (colsA != filasB) {
        cout << "[ERROR] No se pueden multiplicar. Columnas de A (" << colsA << ") != Filas de B (" << filasB << ")." << endl;
        return 1;
    }

    cout << "[INFO] Multiplicando matriz A (" << filasA << "x" << colsA << ") por matriz B (" << filasB << "x" << colsB << ")..." << endl;

    // Resultado C = A * B
    vector<vector<double>> C(filasA, vector<double>(colsB, 0));

    for (size_t i = 0; i < filasA; i++) {
        for (size_t j = 0; j < colsB; j++) {
            for (size_t k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    cout << "\n--- Resultado ---" << endl;
    for (size_t i = 0; i < filasA; i++) {
        for (size_t j = 0; j < colsB; j++) {
            cout << C[i][j];
            if (j < colsB - 1) cout << " \t ";
        }
        cout << endl;
    }
    cout << "-----------------" << endl;

    return 0;
}
