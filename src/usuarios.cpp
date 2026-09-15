#include "usuarios.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <cstring>

using namespace std;

// ─── Cargar usuarios desde archivo BINARIO a memoria ─────────
void cargarUsuariosDesdeArchivo(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    FILE* file = fopen(archivoUsuarios.c_str(), "rb");
    listaUsuarios.usuarios.clear();

    if (!file) {
        // Si el archivo no existe aún, no es error
        return;
    }

    Usuario u;
    while (fread(&u, sizeof(Usuario), 1, file) == 1) {
        listaUsuarios.usuarios.push_back(u);
    }

    fclose(file);
}

// ─── Guardar todos los usuarios al archivo BINARIO ───────────
void guardarUsuariosEnArchivo(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    // Ordena de menor a mayor por id
    sort(listaUsuarios.usuarios.begin(), listaUsuarios.usuarios.end(), [](const Usuario& a, const Usuario& b) {
        return a.id < b.id;
    });

    FILE* file = fopen(archivoUsuarios.c_str(), "wb");

    if (!file) {
        cout << "[ERROR] No se pudo abrir el archivo " << archivoUsuarios << " para escritura." << endl;
        return;
    }

    for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
        fwrite(&listaUsuarios.usuarios[i], sizeof(Usuario), 1, file);
    }

    fclose(file);

    // Guardar copia en TXT para auditoría
    string archivoTXT = archivoUsuarios;
    size_t pos = archivoTXT.find(".DAT");
    if (pos != string::npos) {
        archivoTXT.replace(pos, 4, ".TXT");
    } else {
        archivoTXT += ".TXT";
    }

    ofstream outTXT(archivoTXT);
    if (outTXT.is_open()) {
        outTXT << "ID;Nombre;Username;Perfil" << endl;
        for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
            const Usuario& u = listaUsuarios.usuarios[i];
            outTXT << u.id << ";" << u.nombre << ";" << u.username << ";" << u.perfil << endl;
        }
        outTXT.close();
    }
}

// ─── Ingresar un nuevo usuario ────────────────────────────
void ingresarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios, const ListaPerfiles& listaPerfiles) {

    Usuario nuevoUsuario;
    memset(&nuevoUsuario, 0, sizeof(Usuario)); // Limpiar basura de memoria

    cout << endl << "=== Ingresar Nuevo Usuario ===" << endl;

    int maxId = 0;
    for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
        if (listaUsuarios.usuarios[i].id > maxId) {
            maxId = listaUsuarios.usuarios[i].id;
        }
    }
    nuevoUsuario.id = maxId + 1;
    cout << "ID asignado automaticamente: " << nuevoUsuario.id << endl;

    string inputTemp;
    do {
        cout << "Nombre: ";
        getline(cin, inputTemp);
        if (inputTemp.empty()) {
            cout << "[ERROR] El nombre no puede estar vacio. Intente nuevamente." << endl;
        }
    } while (inputTemp.empty());
    strncpy(nuevoUsuario.nombre, inputTemp.c_str(), sizeof(nuevoUsuario.nombre) - 1);

    do {
        cout << "Username: ";
        getline(cin, inputTemp);
        if (inputTemp.empty()) {
            cout << "[ERROR] El username no puede estar vacio. Intente nuevamente." << endl;
        }
    } while (inputTemp.empty());
    strncpy(nuevoUsuario.username, inputTemp.c_str(), sizeof(nuevoUsuario.username) - 1);

    do {
        cout << "Password: ";
        getline(cin, inputTemp);
        if (inputTemp.empty()) {
            cout << "[ERROR] La password no puede estar vacia. Intente nuevamente." << endl;
        }
    } while (inputTemp.empty());
    strncpy(nuevoUsuario.password, inputTemp.c_str(), sizeof(nuevoUsuario.password) - 1);

    bool perfilValido = false;
    do {
        cout << "Perfiles disponibles:" << endl;
        for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
            cout << " - " << listaPerfiles.perfiles[i].nombre << endl;
        }
        cout << "Perfil: ";
        getline(cin, inputTemp);

        // Quitar espacios
        inputTemp.erase(remove_if(inputTemp.begin(), inputTemp.end(), ::isspace), inputTemp.end());
        // Convertir a mayúsculas
        transform(inputTemp.begin(), inputTemp.end(), inputTemp.begin(), ::toupper);

        for (size_t i = 0; i < listaPerfiles.perfiles.size(); i++) {
            if (inputTemp == listaPerfiles.perfiles[i].nombre) {
                perfilValido = true;
                break;
            }
        }

        if (!perfilValido) {
            cout << "[ERROR] Perfil invalido. Debe ser uno de los listados." << endl;
        }
    } while (!perfilValido);
    strncpy(nuevoUsuario.perfil, inputTemp.c_str(), sizeof(nuevoUsuario.perfil) - 1);

    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Agregar a memoria
        listaUsuarios.usuarios.push_back(nuevoUsuario);

        // Guardar en archivo (esto también ordena)
        guardarUsuariosEnArchivo(listaUsuarios, archivoUsuarios);
        cout << endl << "[OK] Usuario '" << nuevoUsuario.nombre << "' ingresado correctamente." << endl;
    } else {
        cout << "Operacion cancelada." << endl;
    }
    pausar();
}

// ─── Listar todos los usuarios desde memoria ─────────────
void listarUsuarios(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    // Si no hay datos en memoria, intentar cargar desde archivo
    if (listaUsuarios.usuarios.size() == 0) {
        cargarUsuariosDesdeArchivo(listaUsuarios, archivoUsuarios);
    }

    cout << endl << "=== Lista de Usuarios ===" << endl;

    if (listaUsuarios.usuarios.size() == 0) {
        cout << "No hay usuarios registrados." << endl;
        pausar();
        return;
    }

    cout << "---------------------------------------------------------------" << endl;
    cout << left << setw(5) << "ID" 
         << setw(25) << "Nombre" 
         << setw(15) << "Username" 
         << "Perfil" << endl;
    cout << "---------------------------------------------------------------" << endl;

    for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
        const Usuario& u = listaUsuarios.usuarios[i];
        cout << left << setw(5) << u.id 
             << setw(25) << u.nombre 
             << setw(15) << u.username 
             << u.perfil << endl;
    }

    cout << "---------------------------------------------------------------" << endl;
    cout << "Total: " << listaUsuarios.usuarios.size() << " usuario(s)" << endl;
    pausar();
}

// ─── Eliminar un usuario por ID ───────────────────────────
void eliminarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    cout << endl << "=== Eliminar Usuario ===" << endl;

    if (listaUsuarios.usuarios.size() == 0) {
        cout << "No hay usuarios registrados." << endl;
        pausar();
        return;
    }

    int idEliminar = leerEntero("Ingrese el ID del usuario a eliminar: ");

    // Buscar el usuario
    int indice = -1;
    for (size_t i = 0; i < listaUsuarios.usuarios.size(); i++) {
        if (listaUsuarios.usuarios[i].id == idEliminar) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        cout << "[ERROR] No se encontro un usuario con ID " << idEliminar << "." << endl;
        pausar();
        return;
    }

    // Mostrar datos del usuario encontrado
    const Usuario& u = listaUsuarios.usuarios[indice];
    cout << "Usuario encontrado: " << u.nombre << " (" << u.username << ") - Perfil: " << u.perfil << endl;

    // Alerta si es ADMIN
    if (strcmp(u.perfil, "ADMIN") == 0) {
        cout << endl;
        cout << "╔══════════════════════════════════════════════╗" << endl;
        cout << "║  [ADVERTENCIA] Este usuario tiene perfil     ║" << endl;
        cout << "║  ADMIN. Eliminarlo puede afectar el sistema. ║" << endl;
        cout << "╚══════════════════════════════════════════════╝" << endl;
    }

    // Confirmar eliminación
    cout << endl << "  1) guardar   2) cancelar" << endl;
    int opcionGuardar = leerEntero("Opcion : ");

    if (opcionGuardar == 1) {
        // Desplazar elementos para llenar el hueco
        listaUsuarios.usuarios.erase(listaUsuarios.usuarios.begin() + indice);

        // Reescribir archivo
        guardarUsuariosEnArchivo(listaUsuarios, archivoUsuarios);

        cout << "[OK] Usuario eliminado correctamente." << endl;
    } else {
        cout << "Operacion cancelada." << endl;
    }

    pausar();
}

// ─── Menú de Gestión de Usuarios ──────────────────────────
void menuUsuarios(ListaUsuarios& listaUsuarios, const string& archivoUsuarios, const ListaPerfiles& listaPerfiles) {
    int opcion;

    do {
        limpiarPantalla();
        cout << "===================================" << endl;
        cout << "  Modulo - Gestion de Usuarios" << endl;
        cout << "===================================" << endl;
        cout << "  1) Ingresar Usuario" << endl;
        cout << "  2) Listar Usuarios" << endl;
        cout << "  3) Eliminar Usuario" << endl;
        cout << "  0) Volver" << endl;
        cout << "===================================" << endl;

        opcion = leerEntero("Opcion: ");

        switch (opcion) {
            case 1:
                ingresarUsuario(listaUsuarios, archivoUsuarios, listaPerfiles);
                break;
            case 2:
                listarUsuarios(listaUsuarios, archivoUsuarios);
                break;
            case 3:
                eliminarUsuario(listaUsuarios, archivoUsuarios);
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
