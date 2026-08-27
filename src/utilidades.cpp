#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

// ─── Leer variable desde archivo .env ─────────────────────
string leerVariableEnv(const string& nombreVariable) {
    ifstream archivo(".env");
    string linea;

    if (!archivo.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo .env" << endl;
        return "";
    }

    while (getline(archivo, linea)) {
        // Ignorar líneas vacías o comentarios
        if (linea.empty() || linea[0] == '#') continue;

        // Buscar el separador '='
        size_t pos = linea.find('=');
        if (pos != string::npos) {
            string clave = linea.substr(0, pos);
            string valor = linea.substr(pos + 1);

            // Eliminar espacios al inicio y final
            while (!clave.empty() && clave.back() == ' ') clave.pop_back();
            while (!valor.empty() && valor.front() == ' ') valor.erase(valor.begin());
            while (!valor.empty() && (valor.back() == '\r' || valor.back() == '\n')) valor.pop_back();

            if (clave == nombreVariable) {
                archivo.close();
                return valor;
            }
        }
    }

    archivo.close();
    cout << "[AVISO] Variable '" << nombreVariable << "' no encontrada en .env" << endl;
    return "";
}

// ─── Leer un entero con validación ────────────────────────
int leerEntero(const string& mensaje) {
    int valor;
    string linea;
    while (true) {
        cout << mensaje;
        if (!getline(cin, linea)) {
            return 0;
        }
        
        stringstream ss(linea);
        if (ss >> valor) {
            string resto;
            if (!(ss >> resto)) {
                return valor;
            }
        }
        
        cout << "[ERROR] Debe ingresar un numero entero." << endl;
    }
}

// ─── Limpiar pantalla ─────────────────────────────────────
void limpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// ─── Pausar consola ───────────────────────────────────────
void pausar() {
    cout << endl << "Presione Enter para continuar...";
    cin.ignore(10000, '\n');
}
