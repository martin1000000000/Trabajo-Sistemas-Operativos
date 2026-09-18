#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cctype>
#include "estructuras.h"
#include "utilidades.h"
#include "usuarios.h"
#include "perfiles.h"

using namespace std;

// Función auxiliar para contar en un archivo
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

int main(int argc, char* argv[]) {
    string argUser = "";
    string argPass = "";
    string argFile = "";

    // Parsear argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            argUser = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            argPass = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            argFile = argv[i + 1];
            i++;
        }
    }

    if (argUser.empty() || argPass.empty() || argFile.empty()) {
        cout << "[ERROR] Faltan argumentos. Uso: ./SistOpe -u <user> -p <pass> -f <file>" << endl;
        return 1;
    }

    // ─── 1. Leer variables de entorno desde .env ──────────
    string archivoUsuarios = leerVariableEnv("USER_FILE");
    string archivoPerfiles = leerVariableEnv("PERFIL_FILE");
    string multiplicadorExe = leerVariableEnv("MULTIPLICADOR_EXE");

    // Asumir .DAT porque usamos modo binario
    if (archivoUsuarios.empty()) archivoUsuarios = "USUARIOS.DAT";
    if (archivoPerfiles.empty()) archivoPerfiles = "PERFILES.DAT";
    if (multiplicadorExe.empty()) multiplicadorExe = "multiplicador.exe";

    // ─── 2. Cargar datos desde archivos a memoria ─────────
    ListaUsuarios listaUsuarios;
    cargarUsuariosDesdeArchivo(listaUsuarios, archivoUsuarios);

    ListaPerfiles listaPerfiles;
    cargarPerfilesDesdeArchivo(listaPerfiles, archivoPerfiles);

    // ─── 3. Autenticación ─────────────────────────────────
    bool validLogin = false;
    string loggedProfile = "";
    string loggedName = "";

    for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
        if (argUser == listaUsuarios.usuarios[i].username && argPass == listaUsuarios.usuarios[i].password) {
            validLogin = true;
            loggedProfile = listaUsuarios.usuarios[i].perfil;
            loggedName = listaUsuarios.usuarios[i].nombre;
            break;
        }
    }

    // Comentar estas lineas para probar rapidamente si no tienes base de datos cargada:
    if (!validLogin) {
        cout << "[ERROR] Credenciales invalidas para el usuario '" << argUser << "'." << endl;
        return 1;
    }

    // Cargar permisos del perfil
    int allowedOptions[20];
    int numAllowedOptions = 0;
    for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
        if (strcmp(listaPerfiles.perfiles[i].nombre, loggedProfile.c_str()) == 0) {
            numAllowedOptions = listaPerfiles.perfiles[i].num_opciones;
            for (int j = 0; j < numAllowedOptions; j++) {
                allowedOptions[j] = listaPerfiles.perfiles[i].opciones[j];
            }
            break;
        }
    }

    // ─── 4. Menú principal ────────────────────────────────
    int opcion;
    do {
        limpiarPantalla();
        cout << "===================================" << endl;
        cout << "          MENU PRINCIPAL" << endl;
        cout << "  Usuario: " << loggedName << " | Perfil: " << loggedProfile << endl;
        cout << "===================================" << endl;
        cout << "  1) Admin de usuarios y perfiles" << endl;
        cout << "  2) Multiplica matrices NxM" << endl;
        cout << "  3) Juego" << endl;
        cout << "  4) Es palindromo?" << endl;
        cout << "  5) Calcular f(x) = x*x + 2x + 8" << endl;
        cout << "  6) Conteo texto (usar archivo de consola -f)" << endl;
        cout << "  7) Conteo texto (ingresar nueva ruta)" << endl;
        cout << "  0) Salir" << endl;
        cout << "===================================" << endl;

        opcion = leerEntero("Opcion: ");

        // Verificar permisos
        bool hasPermission = false;
        if (opcion == 0) {
            hasPermission = true;
        } else if (opcion == 1) {
            hasPermission = (loggedProfile == "ADMIN");
        } else {
            for (int i = 0; i < numAllowedOptions; i++) {
                if (allowedOptions[i] == opcion) {
                    hasPermission = true;
                    break;
                }
            }
        }

        if (!hasPermission && opcion != 0) {
            if (opcion == 1) {
                cout << "[ACCESO DENEGADO] Solo disponible para perfil ADMIN." << endl;
            } else {
                cout << "[ACCESO DENEGADO] Tu perfil " << loggedProfile << " no tiene permiso para la opcion " << opcion << "." << endl;
            }
            pausar();
            continue; // Volver al inicio del bucle
        }

        switch (opcion) {
            case 1: {
                int subOpt;
                do {
                    limpiarPantalla();
                    cout << "--- ADMINISTRACION DE USUARIOS Y PERFILES ---" << endl;
                    cout << "1) Gestion de Usuarios" << endl;
                    cout << "2) Gestion de Perfiles" << endl;
                    cout << "0) Volver" << endl;
                    subOpt = leerEntero("Opcion: ");
                    if (subOpt == 1) menuUsuarios(listaUsuarios, archivoUsuarios, listaPerfiles);
                    else if (subOpt == 2) menuPerfiles(listaPerfiles, archivoPerfiles);
                } while (subOpt != 0);
                break;
            }
            case 2: {
                string rutaA, rutaB, sep;
                cout << "Ingrese ruta matriz A: ";
                getline(cin, rutaA);
                cout << "Ingrese ruta matriz B: ";
                getline(cin, rutaB);
                cout << "Ingrese separador (ej. '#'): ";
                getline(cin, sep);
                
                string cmd = multiplicadorExe + " \"" + rutaA + "\" \"" + rutaB + "\" \"" + sep + "\" \"" + loggedName + "\" \"" + loggedProfile + "\"";
                cout << "Ejecutando: " << cmd << endl;
                system(cmd.c_str());
                pausar();
                break;
            }
            case 3:
                cout << "En construccion..." << endl;
                pausar();
                break;
            case 4: {
                string text;
                cout << "Ingrese texto a validar: ";
                getline(cin, text);
                cout << "1) Validar  2) Cancelar" << endl;
                int optPal = leerEntero("Opcion: ");
                if (optPal == 1) {
                    if (esPalindromo(text)) cout << "Es palindromo!" << endl;
                    else cout << "NO es palindromo." << endl;
                }
                pausar();
                break;
            }
            case 5: {
                int optF;
                do {
                    limpiarPantalla();
                    cout << "===================================" << endl;
                    cout << "  Calcular f(x) = x*x + 2x + 8" << endl;
                    cout << "===================================" << endl;
                    cout << "  1) Ingresar X y calcular" << endl;
                    cout << "  0) VOLVER" << endl;
                    cout << "===================================" << endl;
                    optF = leerEntero("Opcion: ");
                    if (optF == 1) {
                        double x = leerReal("Ingrese el valor de X: ");
                        double res = (x * x) + (2 * x) + 8;
                        cout << "f(" << x << ") = " << res << endl;
                        pausar();
                    }
                } while (optF != 0);
                break;
            }
            case 6: {
                cout << "Conteo sobre archivo de parametro: " << argFile << endl;
                conteoTexto(argFile);
                int volver;
                do {
                    volver = leerEntero("[0] VOLVER: ");
                } while (volver != 0);
                break;
            }
            case 7: {
                string path;
                cout << "Ingrese path del archivo: ";
                getline(cin, path);
                conteoTexto(path);
                int volver;
                do {
                    volver = leerEntero("[0] VOLVER: ");
                } while (volver != 0);
                break;
            }
            case 0:
                cout << endl << "Saliendo del sistema... Hasta luego!" << endl;
                break;
            default:
                cout << "[ERROR] Opcion no valida." << endl;
                pausar();
                break;
        }
    } while (opcion != 0);

    return 0;
}
