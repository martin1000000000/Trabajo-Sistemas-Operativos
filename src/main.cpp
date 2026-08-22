#include <iostream>
#include "estructuras.h"
#include "utilidades.h"
#include "usuarios.h"
#include "perfiles.h"

using namespace std;

int main() {
    // ─── 1. Leer variables de entorno desde .env ──────────
    string archivoUsuarios = leerVariableEnv("USER_FILE");
    string archivoPerfiles = leerVariableEnv("PERFIL_FILE");

    if (archivoUsuarios.empty()) archivoUsuarios = "USUARIOS.TXT";
    if (archivoPerfiles.empty()) archivoPerfiles = "PERFILES.TXT";

    cout << "[INFO] Archivo de usuarios: " << archivoUsuarios << endl;
    cout << "[INFO] Archivo de perfiles: " << archivoPerfiles << endl;

    // ─── 2. Cargar datos desde archivos a memoria ─────────
    ListaUsuarios listaUsuarios;
    listaUsuarios.cantidad = 0;
    cargarUsuariosDesdeArchivo(listaUsuarios, archivoUsuarios);
    cout << "[INFO] Usuarios cargados: " << listaUsuarios.cantidad << endl;

    ListaPerfiles listaPerfiles;
    listaPerfiles.cantidad = 0;
    cargarPerfilesDesdeArchivo(listaPerfiles, archivoPerfiles);
    cout << "[INFO] Perfiles cargados: " << listaPerfiles.cantidad << endl;

    // ─── 3. Menú principal ────────────────────────────────
    int opcion;

    do {
        limpiarPantalla();
        cout << "===================================" << endl;
        cout << "          === SistOpe ===" << endl;
        cout << "===================================" << endl;
        cout << "  1) Gestion de Usuarios" << endl;
        cout << "  2) Gestion de Perfiles" << endl;
        cout << "  0) Salir" << endl;
        cout << "===================================" << endl;

        opcion = leerEntero("Opcion: ");

        switch (opcion) {
            case 1:
                menuUsuarios(listaUsuarios, archivoUsuarios, listaPerfiles);
                break;
            case 2:
                menuPerfiles(listaPerfiles, archivoPerfiles);
                break;
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
