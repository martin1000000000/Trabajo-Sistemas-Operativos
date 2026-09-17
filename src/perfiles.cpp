#include "perfiles.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstring>

using namespace std;

// ─── Cargar perfiles desde archivo BINARIO a memoria ─────────
void cargarPerfilesDesdeArchivo(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    FILE* file = fopen(archivoPerfiles.c_str(), "rb");
    listaPerfiles.perfiles.clear();

    if (!file) {
        // Si el archivo no existe aún, no es error
        return;
    }

    Perfil p;
    while (fread(&p, sizeof(Perfil), 1, file) == 1) {
        listaPerfiles.perfiles.push_back(p);
    }

    fclose(file);
}

// ─── Guardar todos los perfiles al archivo BINARIO ───────────
void guardarPerfilesEnArchivo(const ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    FILE* file = fopen(archivoPerfiles.c_str(), "wb");

    if (!file) {
        cout << "[ERROR] No se pudo abrir el archivo " << archivoPerfiles << " para escritura." << endl;
        return;
    }

    for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
        fwrite(&listaPerfiles.perfiles[i], sizeof(Perfil), 1, file);
    }

    fclose(file);

    // Guardar copia en TXT para auditoría
    string archivoTXT = archivoPerfiles;
    size_t pos = archivoTXT.find(".DAT");
    if (pos != string::npos) {
        archivoTXT.replace(pos, 4, ".TXT");
    } else {
        archivoTXT += ".TXT";
    }

    ofstream outTXT(archivoTXT);
    if (outTXT.is_open()) {
        outTXT << "Nombre;Opciones" << endl;
        for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
            const Perfil& p = listaPerfiles.perfiles[i];
            outTXT << p.nombre << ";";
            for (int j = 0; j < p.num_opciones; j++) {
                outTXT << p.opciones[j];
                if (j < p.num_opciones - 1) outTXT << ",";
            }
            outTXT << endl;
        }
        outTXT.close();
    }
}

// ─── Ingresar un nuevo perfil ─────────────────────────────
void ingresarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {

    Perfil nuevoPerfil;
    memset(&nuevoPerfil, 0, sizeof(Perfil));

    cout << endl << "=== Ingresar Nuevo Perfil ===" << endl;

    string inputTemp;
    do {
        cout << "Nombre del perfil: ";
        getline(cin, inputTemp);

        // Quitar espacios
        inputTemp.erase(remove_if(inputTemp.begin(), inputTemp.end(), ::isspace), inputTemp.end());

        if (inputTemp.empty()) {
            cout << "[ERROR] El nombre del perfil no puede estar vacio." << endl;
        }
    } while (inputTemp.empty());

    // Convertir a mayúsculas
    transform(inputTemp.begin(), inputTemp.end(), inputTemp.begin(), ::toupper);
    strncpy(nuevoPerfil.nombre, inputTemp.c_str(), sizeof(nuevoPerfil.nombre) - 1);

    // Verificar que el nombre no exista
    for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
        if (strcmp(listaPerfiles.perfiles[i].nombre, nuevoPerfil.nombre) == 0) {
            cout << "[ERROR] Ya existe un perfil con el nombre '" << nuevoPerfil.nombre << "'." << endl;
            pausar();
            return;
        }
    }

    // Pedir opciones (una por una hasta que ingrese 0)
    cout << "Ingrese las opciones del perfil (numeros enteros mayores a 0, 0 para terminar):" << endl;
    nuevoPerfil.num_opciones = 0;
    while (nuevoPerfil.num_opciones < 20) {
        int opcion = leerEntero("  Opcion " + to_string(nuevoPerfil.num_opciones + 1) + " (0 para terminar): ");
        if (opcion == 0) break;
        
        if (opcion < 0) {
            cout << "[ERROR] Las opciones no pueden ser negativas. Ingrese una opcion mayor a 0." << endl;
            continue;
        }
        
        nuevoPerfil.opciones[nuevoPerfil.num_opciones] = opcion;
        nuevoPerfil.num_opciones++;
    }

    if (nuevoPerfil.num_opciones == 0) {
        cout << "[ERROR] Debe ingresar al menos una opcion." << endl;
        pausar();
        return;
    }

    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Agregar a memoria
        listaPerfiles.perfiles.push_back(nuevoPerfil);

        // Guardar en archivo
        guardarPerfilesEnArchivo(listaPerfiles, archivoPerfiles);
        cout << endl << "[OK] Perfil '" << nuevoPerfil.nombre << "' ingresado correctamente." << endl;
    } else {
        cout << "Operacion cancelada." << endl;
    }
    pausar();
}

// ─── Listar todos los perfiles desde memoria ─────────────
void listarPerfiles(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    // Si no hay datos en memoria, intentar cargar desde archivo
    if (listaPerfiles.perfiles.size() == 0) {
        cargarPerfilesDesdeArchivo(listaPerfiles, archivoPerfiles);
    }

    cout << endl << "=== Lista de Perfiles ===" << endl;

    if (listaPerfiles.perfiles.size() == 0) {
        cout << "No hay perfiles registrados." << endl;
        pausar();
        return;
    }

    cout << "-----------------------------------------------" << endl;
    cout << left << setw(20) << "Nombre" << "Opciones" << endl;
    cout << "-----------------------------------------------" << endl;

    for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
        const Perfil& p = listaPerfiles.perfiles[i];
        cout << left << setw(20) << p.nombre;

        for (int j = 0; j < p.num_opciones; j++) {
            cout << p.opciones[j];
            if (j < p.num_opciones - 1) cout << ", ";
        }
        cout << endl;
    }

    cout << "-----------------------------------------------" << endl;
    cout << "Total: " << listaPerfiles.perfiles.size() << " perfil(es)" << endl;
    pausar();
}

// ─── Eliminar un perfil por nombre ────────────────────────
void eliminarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    cout << endl << "=== Eliminar Perfil ===" << endl;

    if (listaPerfiles.perfiles.size() == 0) {
        cout << "No hay perfiles registrados." << endl;
        pausar();
        return;
    }

    string nombreEliminar;
    cout << "Ingrese el nombre del perfil a eliminar: ";
    getline(cin, nombreEliminar);

    // Quitar espacios
    nombreEliminar.erase(remove_if(nombreEliminar.begin(), nombreEliminar.end(), ::isspace), nombreEliminar.end());

    // Convertir a mayúsculas
    transform(nombreEliminar.begin(), nombreEliminar.end(), nombreEliminar.begin(), ::toupper);

    // Buscar el perfil
    int indice = -1;
    for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
        if (strcmp(listaPerfiles.perfiles[i].nombre, nombreEliminar.c_str()) == 0) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        cout << "[ERROR] No se encontro un perfil con el nombre '" << nombreEliminar << "'." << endl;
        pausar();
        return;
    }

    // Mostrar datos del perfil encontrado
    const Perfil& p = listaPerfiles.perfiles[indice];
    cout << "Perfil encontrado: " << p.nombre << " - Opciones: ";
    for (int j = 0; j < p.num_opciones; j++) {
        cout << p.opciones[j];
        if (j < p.num_opciones - 1) cout << ", ";
    }
    cout << endl;

    // Alerta si es ADMIN
    if (strcmp(p.nombre, "ADMIN") == 0) {
        cout << endl;
        cout << "╔══════════════════════════════════════════════╗" << endl;
        cout << "║  [ADVERTENCIA] Estas eliminando el perfil    ║" << endl;
        cout << "║  ADMIN. Eliminarlo puede afectar el sistema. ║" << endl;
        cout << "╚══════════════════════════════════════════════╝" << endl;
    }

    // Confirmar eliminación
    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Eliminar elemento del vector
        listaPerfiles.perfiles.erase(listaPerfiles.perfiles.begin() + indice);

        // Reescribir archivo
        guardarPerfilesEnArchivo(listaPerfiles, archivoPerfiles);

        cout << "[OK] Perfil eliminado correctamente." << endl;
    } else {
        cout << "Operacion cancelada." << endl;
    }

    pausar();
}

// ─── Menú de Gestión de Perfiles ──────────────────────────
void menuPerfiles(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    int opcion;

    do {
        limpiarPantalla();
        cout << "===================================" << endl;
        cout << "  Modulo - Gestion de Perfiles" << endl;
        cout << "===================================" << endl;
        cout << "  1) Ingresar Perfil" << endl;
        cout << "  2) Listar Perfiles" << endl;
        cout << "  3) Eliminar Perfil" << endl;
        cout << "  0) Volver" << endl;
        cout << "===================================" << endl;

        opcion = leerEntero("Opcion: ");

        switch (opcion) {
            case 1:
                ingresarPerfil(listaPerfiles, archivoPerfiles);
                break;
            case 2:
                listarPerfiles(listaPerfiles, archivoPerfiles);
                break;
            case 3:
                eliminarPerfil(listaPerfiles, archivoPerfiles);
                break;
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
            default:
                cout << "[ERROR] Opcion no valida." << endl;
                pausar();
                break;
        }
    } while (opcion != 0);
}
