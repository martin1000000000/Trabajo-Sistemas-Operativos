#include "perfiles.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// ─── Cargar perfiles desde archivo TXT a memoria ─────────
void cargarPerfilesDesdeArchivo(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    ifstream archivo(archivoPerfiles);
    listaPerfiles.cantidad = 0;

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
        p.numOpciones = 0;

        string opciones = linea.substr(posSeparador + 1);
        stringstream ssOpciones(opciones);
        string opcion;

        while (getline(ssOpciones, opcion, ',')) {
            if (p.numOpciones < 10) {
                p.opciones[p.numOpciones] = stoi(opcion);
                p.numOpciones++;
            }
        }

        if (listaPerfiles.cantidad < 10) {
            listaPerfiles.perfiles[listaPerfiles.cantidad] = p;
            listaPerfiles.cantidad++;
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

    for (int i = 0; i < listaPerfiles.cantidad; i++) {
        const Perfil& p = listaPerfiles.perfiles[i];
        archivo << p.nombre << ";";

        for (int j = 0; j < p.numOpciones; j++) {
            archivo << p.opciones[j];
            if (j < p.numOpciones - 1) {
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
    if (listaPerfiles.cantidad >= 10) {
        cout << "[ERROR] Se alcanzo el limite maximo de perfiles (10)." << endl;
        pausar();
        return;
    }
    
    Perfil nuevoPerfil;
    nuevoPerfil.numOpciones = 0;

    cout << endl << "=== Ingresar Nuevo Perfil ===" << endl;

    cout << "Nombre del perfil: ";
    getline(cin, nuevoPerfil.nombre);

    // Verificar que el nombre no exista
    for (int i = 0; i < listaPerfiles.cantidad; i++) {
        if (listaPerfiles.perfiles[i].nombre == nuevoPerfil.nombre) {
            cout << "[ERROR] Ya existe un perfil con el nombre '" << nuevoPerfil.nombre << "'." << endl;
            pausar();
            return;
        }
    }

    // Pedir opciones (una por una hasta que ingrese 0)
    cout << "Ingrese las opciones del perfil (numeros enteros, 0 para terminar):" << endl;
    while (nuevoPerfil.numOpciones < 10) {
        int opcion = leerEntero("  Opcion " + to_string(nuevoPerfil.numOpciones + 1) + " (0 para terminar): ");
        if (opcion == 0) break;
        nuevoPerfil.opciones[nuevoPerfil.numOpciones] = opcion;
        nuevoPerfil.numOpciones++;
    }

    if (nuevoPerfil.numOpciones == 0) {
        cout << "[ERROR] Debe ingresar al menos una opcion." << endl;
        pausar();
        return;
    }

    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Agregar a memoria
        listaPerfiles.perfiles[listaPerfiles.cantidad] = nuevoPerfil;
        listaPerfiles.cantidad++;

        // Guardar en archivo (anexar al final)
        ofstream archivo(archivoPerfiles, ios::app);
        if (archivo.is_open()) {
            archivo << nuevoPerfil.nombre << ";";
            for (int j = 0; j < nuevoPerfil.numOpciones; j++) {
                archivo << nuevoPerfil.opciones[j];
                if (j < nuevoPerfil.numOpciones - 1) archivo << ",";
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
    if (listaPerfiles.cantidad == 0) {
        cargarPerfilesDesdeArchivo(listaPerfiles, archivoPerfiles);
    }

    cout << endl << "=== Lista de Perfiles ===" << endl;

    if (listaPerfiles.cantidad == 0) {
        cout << "No hay perfiles registrados." << endl;
        pausar();
        return;
    }

    cout << "-----------------------------------------------" << endl;
    cout << "Nombre\t\tOpciones" << endl;
    cout << "-----------------------------------------------" << endl;

    for (int i = 0; i < listaPerfiles.cantidad; i++) {
        const Perfil& p = listaPerfiles.perfiles[i];
        cout << p.nombre << "\t\t";

        for (int j = 0; j < p.numOpciones; j++) {
            cout << p.opciones[j];
            if (j < p.numOpciones - 1) cout << ", ";
        }
        cout << endl;
    }

    cout << "-----------------------------------------------" << endl;
    cout << "Total: " << listaPerfiles.cantidad << " perfil(es)" << endl;
    pausar();
}

// ─── Eliminar un perfil por nombre ────────────────────────
void eliminarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles) {
    cout << endl << "=== Eliminar Perfil ===" << endl;

    if (listaPerfiles.cantidad == 0) {
        cout << "No hay perfiles registrados." << endl;
        pausar();
        return;
    }

    string nombreEliminar;
    cout << "Ingrese el nombre del perfil a eliminar: ";
    getline(cin, nombreEliminar);

    // Buscar el perfil
    int indice = -1;
    for (int i = 0; i < listaPerfiles.cantidad; i++) {
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
    for (int j = 0; j < p.numOpciones; j++) {
        cout << p.opciones[j];
        if (j < p.numOpciones - 1) cout << ", ";
    }
    cout << endl;

    // Confirmar eliminación
    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Desplazar elementos
        for (int i = indice; i < listaPerfiles.cantidad - 1; i++) {
            listaPerfiles.perfiles[i] = listaPerfiles.perfiles[i + 1];
        }
        listaPerfiles.cantidad--;

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
