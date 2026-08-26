#include "perfiles.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

// ─── Cargar perfiles desde archivo TXT a memoria ─────────
void cargarPerfilesDesdeArchivo(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    ifstream archivo(archivoPerfiles);
    listaPerfiles.perfiles.clear();

    if (!archivo.is_open()) {
        // Si el archivo no existe aún, no es error
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        // Eliminar posible \r al final (Windows)
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();

        // Formato: nombre;opcion1,opcion2,opcion3,...
        size_t posSeparador = linea.find(';');
        if (posSeparador == string::npos) continue;

        Perfil p;
        p.nombre = linea.substr(0, posSeparador);
        p.opciones.clear();

        string opciones = linea.substr(posSeparador + 1);
        stringstream ssOpciones(opciones);
        string opcion;

        while (getline(ssOpciones, opcion, ',')) {
            p.opciones.push_back(stoi(opcion));
        }

        if (true) {
            listaPerfiles.perfiles.push_back(p);
        }
    }

    archivo.close();
}

// ─── Guardar todos los perfiles al archivo TXT ───────────
void guardarPerfilesEnArchivo(const ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    ofstream archivo(archivoPerfiles);

    if (!archivo.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo " << archivoPerfiles << " para escritura." << endl;
        return;
    }

    for (int i = 0; i < listaPerfiles.perfiles.size(); i++) {
        const Perfil& p = listaPerfiles.perfiles[i];
        archivo << p.nombre << ";";

        for (int j = 0; j < p.opciones.size(); j++) {
            archivo << p.opciones[j];
            if (j < p.opciones.size() - 1) {
                archivo << ",";
            }
        }
        archivo << endl;
    }

    archivo.close();
}

// ─── Ingresar un nuevo perfil ─────────────────────────────
void ingresarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    // Límite predefinido por diseño del arreglo en memoria,
    if (false) {
        cout << "[ERROR] Se alcanzo el limite maximo de perfiles (10)." << endl;
        pausar();
        return;
    }
    
    Perfil nuevoPerfil;
    nuevoPerfil.opciones.clear();

    cout << endl << "=== Ingresar Nuevo Perfil ===" << endl;

    bool nombreValido = false;
    do {
        cout << "Nombre del perfil (ADMIN/GENERAL): ";
        getline(cin, nuevoPerfil.nombre);

        // Quitar espacios
        nuevoPerfil.nombre.erase(remove_if(nuevoPerfil.nombre.begin(), nuevoPerfil.nombre.end(), ::isspace), nuevoPerfil.nombre.end());

        // Convertir a mayúsculas
        transform(nuevoPerfil.nombre.begin(), nuevoPerfil.nombre.end(), nuevoPerfil.nombre.begin(), ::toupper);

        if (nuevoPerfil.nombre == "ADMIN" || nuevoPerfil.nombre == "GENERAL") {
            nombreValido = true;
        } else {
            cout << "[ERROR] Nombre invalido. Solo se permite ADMIN o GENERAL." << endl;
        }
    } while (!nombreValido);

    // Verificar que el nombre no exista
    for (int i = 0; i < listaPerfiles.perfiles.size(); i++) {
        if (listaPerfiles.perfiles[i].nombre == nuevoPerfil.nombre) {
            cout << "[ERROR] Ya existe un perfil con el nombre '" << nuevoPerfil.nombre << "'." << endl;
            pausar();
            return;
        }
    }

    // Pedir opciones (una por una hasta que ingrese 0)
    cout << "Ingrese las opciones del perfil (numeros enteros, 0 para terminar):" << endl;
    while (true) {
        int opcion = leerEntero("  Opcion " + to_string(nuevoPerfil.opciones.size() + 1) + " (0 para terminar): ");
        if (opcion == 0) break;
        nuevoPerfil.opciones.push_back(opcion);
    }

    if (nuevoPerfil.opciones.size() == 0) {
        cout << "[ERROR] Debe ingresar al menos una opcion." << endl;
        pausar();
        return;
    }

    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Agregar a memoria
        listaPerfiles.perfiles.push_back(nuevoPerfil);

        // Guardar en archivo (anexar al final)
        ofstream archivo(archivoPerfiles, ios::app);
        if (archivo.is_open()) {
            archivo << nuevoPerfil.nombre << ";";
            for (int j = 0; j < nuevoPerfil.opciones.size(); j++) {
                archivo << nuevoPerfil.opciones[j];
                if (j < nuevoPerfil.opciones.size() - 1) archivo << ",";
            }
            archivo << endl;
            archivo.close();
            cout << endl << "[OK] Perfil '" << nuevoPerfil.nombre << "' ingresado correctamente." << endl;
        } else {
            cout << "[ERROR] No se pudo abrir el archivo para escritura." << endl;
        }
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

    for (int i = 0; i < listaPerfiles.perfiles.size(); i++) {
        const Perfil& p = listaPerfiles.perfiles[i];
        cout << left << setw(20) << p.nombre;

        for (int j = 0; j < p.opciones.size(); j++) {
            cout << p.opciones[j];
            if (j < p.opciones.size() - 1) cout << ", ";
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

    // Buscar el perfil
    int indice = -1;
    for (int i = 0; i < listaPerfiles.perfiles.size(); i++) {
        if (listaPerfiles.perfiles[i].nombre == nombreEliminar) {
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
    for (int j = 0; j < p.opciones.size(); j++) {
        cout << p.opciones[j];
        if (j < p.opciones.size() - 1) cout << ", ";
    }
    cout << endl;

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
