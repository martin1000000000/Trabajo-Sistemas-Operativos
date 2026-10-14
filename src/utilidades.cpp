#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>

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
    string linea;
    int valor;
    while (true) {
        cout << mensaje;
        if (!getline(cin, linea)) {
            return 0;
        } 
        
        stringstream ss(linea);
        char sobrante;
        if ((ss >> valor) && !(ss >> sobrante)) {
            return valor;
        }
        
        cout << "[ERROR] Debe ingresar un numero entero." << endl;
    }
}

// ─── Leer un número real con validación ───────────────────
double leerReal(const string& mensaje) {
    string linea;
    double valor;
    while (true) {
        cout << mensaje;
        if (!getline(cin, linea)) {
            return 0.0;
        }

        stringstream ss(linea);
        char sobrante;
        if ((ss >> valor) && !(ss >> sobrante)) {
            return valor;
        }

        cout << "[ERROR] Debe ingresar un numero real." << endl;
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

// ─── Función auxiliar para contar en un archivo ───────────
void conteoTexto(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo: " << filename << endl;
        return;
    }

    int vocales = 0, consonantes = 0, especiales = 0, palabras = 0;
    char c;
    bool inWord = false;

    while (file.get(c)) {
        if (isalpha(c)) {
            char lower_c = tolower(c);
            if (lower_c == 'a' || lower_c == 'e' || lower_c == 'i' || lower_c == 'o' || lower_c == 'u') {
                vocales++;
            } else {
                consonantes++;
            }
            if (!inWord) {
                palabras++;
                inWord = true;
            }
        } else {
            if (isspace(c)) {
                inWord = false;
            } else {
                especiales++;
                if (!inWord) {
                    palabras++; // Algunas reglas cuentan símbolos sueltos como palabras. Ajustado según sea necesario.
                    inWord = true;
                }
            }
        }
    }
    
    cout << "--- Resultados Conteo ---" << endl;
    cout << "Vocales: " << vocales << endl;
    cout << "Consonantes: " << consonantes << endl;
    cout << "Caracteres especiales: " << especiales << endl;
    cout << "Palabras: " << palabras << endl;
    cout << "-------------------------" << endl;
}

// ─── Verifica si un string es palíndromo ──────────────────
bool esPalindromo(const string& str) {
    string cleaned = "";
    for (char c : str) {
        if (isalnum(c)) {
            cleaned += tolower(c);
        }
    }
    int i = 0;
    int j = cleaned.length() - 1;
    while (i < j) {
        if (cleaned[i] != cleaned[j]) return false;
        i++;
        j--;
    }
    return true;
}

// ─── Obtener variable desde getenv() o .env ───────────────
string obtenerVariableEnvOConfig(const string& nombreVariable, const string& valorDefecto) {
    const char* envVal = getenv(nombreVariable.c_str());
    if (envVal != nullptr && string(envVal).length() > 0) {
        return string(envVal);
    }
    string val = leerVariableEnv(nombreVariable);
    if (!val.empty()) {
        return val;
    }
    return valorDefecto;
}

// ─── Validar variable de entorno como entero ──────────────
bool validarVariableEntero(const string& nombreVariable, int& salida, int minimo, int maximo) {
    string valStr = obtenerVariableEnvOConfig(nombreVariable, "");
    if (valStr.empty()) {
        cout << "[ERROR] Variable de entorno '" << nombreVariable << "' no esta definida." << endl;
        return false;
    }
    try {
        size_t idx = 0;
        int num = stoi(valStr, &idx);
        if (idx != valStr.size()) {
            cout << "[ERROR] Variable '" << nombreVariable << "' ('" << valStr << "') tiene formato numerico invalido." << endl;
            return false;
        }
        if (num < minimo || num > maximo) {
            cout << "[ERROR] Variable '" << nombreVariable << "' debe estar entre " << minimo << " y " << maximo << " (recibido: " << num << ")." << endl;
            return false;
        }
        salida = num;
        return true;
    } catch (...) {
        cout << "[ERROR] Variable '" << nombreVariable << "' ('" << valStr << "') no es un entero valido." << endl;
        return false;
    }
}
