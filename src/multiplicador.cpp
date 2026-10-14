#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cctype>

using namespace std;

// Función para validar estrictamente que una cadena sea un número real sin texto adicional
bool validarNumeroEstricto(const string& str, double& valor) {
    // 1. Quitar espacios en blanco al inicio y al final
    size_t start = 0;
    while (start < str.size() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n')) {
        start++;
    }
    size_t end = str.size();
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n')) {
        end--;
    }

    if (start >= end) {
        // Campo vacío o solo espacios
        return false;
    }

    string trimmed = str.substr(start, end - start);

    // 2. Intentar parsear el número
    try {
        size_t idx = 0;
        valor = stod(trimmed, &idx);

        // Debe haber consumido TODA la cadena trimmed (sin letras ni caracteres extra)
        if (idx != trimmed.size()) {
            return false;
        }

        if (isnan(valor) || isinf(valor)) {
            return false;
        }
    } catch (...) {
        return false;
    }

    return true;
}

// Función para leer matriz desde archivo con validación estricta de formato
bool leerMatriz(const string& ruta, const string& separador, vector<vector<double>>& matriz) {
    ifstream file(ruta);
    if (!file.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo de matriz: " << ruta << endl;
        return false;
    }

    string linea;
    int numLinea = 0;
    while (getline(file, linea)) {
        numLinea++;
        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();
        }

        // Ignorar líneas vacías
        size_t s = 0;
        while (s < linea.size() && (linea[s] == ' ' || linea[s] == '\t')) s++;
        if (s == linea.size()) {
            continue;
        }

        vector<double> fila;
        size_t start = 0;
        size_t pos = 0;

        while ((pos = linea.find(separador, start)) != string::npos) {
            string token = linea.substr(start, pos - start);
            double val = 0.0;
            if (!validarNumeroEstricto(token, val)) {
                cout << "[ERROR] Existe texto adicional o formato incorrecto en la linea " 
                     << numLinea << " del archivo '" << ruta << "' (se encontro: '" << token << "')." << endl;
                return false;
            }
            fila.push_back(val);
            start = pos + separador.length();
        }

        // Último token de la línea
        string ultimoToken = linea.substr(start);
        double val = 0.0;
        if (!validarNumeroEstricto(ultimoToken, val)) {
            cout << "[ERROR] Existe texto adicional o formato incorrecto en la linea " 
                 << numLinea << " del archivo '" << ruta << "' (se encontro: '" << ultimoToken << "')." << endl;
            return false;
        }
        fila.push_back(val);

        matriz.push_back(fila);
    }

    return true;
}

int main(int argc, char* argv[]) {
    string rutaA = "";
    string rutaB = "";
    string separador = "";
    string usuario = "";
    string perfil = "";

    // Permitir ejecución con o sin perfiles:
    // 1) Sin argumentos: modo interactivo directo (por separado, sin perfiles)
    // 2) 3 argumentos: <rutaA> <rutaB> <separador> (por separado, sin perfiles)
    // 3) 5 argumentos: <rutaA> <rutaB> <separador> <usuario> <perfil> (llamada desde SistOpe u otro con perfiles)
    if (argc == 1) {
        cout << "\n======================================" << endl;
        cout << "  PROGRAMA MULTIPLICADOR DE MATRICES" << endl;
        cout << "======================================" << endl;
        cout << "Ingrese ruta matriz A: ";
        getline(cin, rutaA);
        cout << "Ingrese ruta matriz B: ";
        getline(cin, rutaB);
        cout << "Ingrese separador (ej. '#'): ";
        getline(cin, separador);
    } else if (argc >= 4) {
        rutaA = argv[1];
        rutaB = argv[2];
        separador = argv[3];
        if (argc >= 5) usuario = argv[4];
        if (argc >= 6) perfil = argv[5];

        cout << "\n======================================" << endl;
        cout << "  PROGRAMA MULTIPLICADOR DE MATRICES" << endl;
        if (!usuario.empty() || !perfil.empty()) {
            cout << "  Usuario: " << (usuario.empty() ? "-" : usuario)
                 << " | Perfil: " << (perfil.empty() ? "-" : perfil) << endl;
        }
        cout << "======================================" << endl;
    } else {
        cout << "[ERROR Multiplicador] Argumentos insuficientes." << endl;
        cout << "Uso por consola: " << argv[0] << " <rutaA> <rutaB> <separador> [usuario] [perfil]" << endl;
        cout << "O ejecute sin argumentos para ingresar las matrices de forma interactiva." << endl;
        return 1;
    }

    if (rutaA.empty() || rutaB.empty()) {
        cout << "[ERROR] Las rutas de las matrices no pueden estar vacias." << endl;
        return 1;
    }

    if (separador.empty()) {
        cout << "[ERROR] El separador no puede estar vacio." << endl;
        return 1;
    }

    vector<vector<double>> A;
    vector<vector<double>> B;

    if (!leerMatriz(rutaA, separador, A)) return 1;
    if (!leerMatriz(rutaB, separador, B)) return 1;

    if (A.empty() || B.empty()) {
        cout << "[ERROR] Una de las matrices esta vacia o no contiene filas validas." << endl;
        return 1;
    }

    size_t filasA = A.size();
    size_t colsA = A[0].size();
    size_t filasB = B.size();
    size_t colsB = B[0].size();

    // Validar dimensiones (A debe tener todas sus filas del mismo largo, etc)
    for (size_t i = 1; i < filasA; i++) {
        if (A[i].size() != colsA) {
            cout << "[ERROR] Existe texto adicional o formato incorrecto: la matriz A no tiene dimensiones regulares (fila " 
                 << (i + 1) << " tiene " << A[i].size() << " columnas, se esperaban " << colsA << ")." << endl;
            return 1;
        }
    }
    for (size_t i = 1; i < filasB; i++) {
        if (B[i].size() != colsB) {
            cout << "[ERROR] Existe texto adicional o formato incorrecto: la matriz B no tiene dimensiones regulares (fila " 
                 << (i + 1) << " tiene " << B[i].size() << " columnas, se esperaban " << colsB << ")." << endl;
            return 1;
        }
    }

    if (colsA != filasB) {
        cout << "[ERROR] No se pueden multiplicar. Columnas de A (" << colsA << ") != Filas de B (" << filasB << ")." << endl;
        return 1;
    }

    cout << "[INFO] Multiplicando matriz A (" << filasA << "x" << colsA << ") por matriz B (" << filasB << "x" << colsB << ")..." << endl;

    // Resultado C = A * B
    vector<vector<double>> C(filasA, vector<double>(colsB, 0.0));

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
