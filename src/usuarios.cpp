#include "usuarios.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// ─── Cargar usuarios desde archivo TXT a memoria ─────────
void cargarUsuariosDesdeArchivo(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    ifstream archivo(archivoUsuarios);
    listaUsuarios.cantidad = 0;

    if (!archivo.is_open()) {
        // Si el archivo no existe aún, no es error
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        // Eliminar posible \r al final (Windows)
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();

        // Formato: id;nombre;username;password;perfil
        stringstream ss(linea);
        string campo;
        Usuario u;

        // ID
        if (getline(ss, campo, ';')) u.id = stoi(campo);
        // Nombre
        if (getline(ss, campo, ';')) u.nombre = campo;
        // Username
        if (getline(ss, campo, ';')) u.username = campo;
        // Password
        if (getline(ss, campo, ';')) u.password = campo;
        // Perfil
        if (getline(ss, campo, ';')) u.perfil = campo;

        if (listaUsuarios.cantidad < 100) {
            listaUsuarios.usuarios[listaUsuarios.cantidad] = u;
            listaUsuarios.cantidad++;
        }
    }

    archivo.close();
}

// ─── Guardar todos los usuarios al archivo TXT ───────────
void guardarUsuariosEnArchivo(const ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    ofstream archivo(archivoUsuarios);

    if (!archivo.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo " << archivoUsuarios << " para escritura." << endl;
        return;
    }

    for (int i = 0; i < listaUsuarios.cantidad; i++) {
        const Usuario& u = listaUsuarios.usuarios[i];
        archivo << u.id << ";" << u.nombre << ";" << u.username << ";" << u.password << ";" << u.perfil << endl;
    }

    archivo.close();
}

// ─── Ingresar un nuevo usuario ────────────────────────────
void ingresarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    if (listaUsuarios.cantidad >= 100) {
        cout << "[ERROR] Se alcanzo el limite maximo de usuarios (100)." << endl;
        pausar();
        return;
    }

    Usuario nuevoUsuario;

    cout << endl << "=== Ingresar Nuevo Usuario ===" << endl;

    nuevoUsuario.id = leerEntero("ID: ");

    // Verificar que el ID no exista
    for (int i = 0; i < listaUsuarios.cantidad; i++) {
        if (listaUsuarios.usuarios[i].id == nuevoUsuario.id) {
            cout << "[ERROR] Ya existe un usuario con ID " << nuevoUsuario.id << "." << endl;
            pausar();
            return;
        }
    }

    cout << "Nombre: ";
    getline(cin, nuevoUsuario.nombre);

    cout << "Username: ";
    getline(cin, nuevoUsuario.username);

    cout << "Password: ";
    getline(cin, nuevoUsuario.password);

    // Solicitar perfil con validación
    while (true) {
        cout << "Perfil (ADMIN/GENERAL): ";
        getline(cin, nuevoUsuario.perfil);
        if (nuevoUsuario.perfil == "ADMIN" || nuevoUsuario.perfil == "GENERAL") {
            break;
        }
        cout << "[ERROR] Perfil debe ser ADMIN o GENERAL." << endl;
    }

    // Agregar a memoria
    listaUsuarios.usuarios[listaUsuarios.cantidad] = nuevoUsuario;
    listaUsuarios.cantidad++;

    // Guardar en archivo (reescribir todo)
    guardarUsuariosEnArchivo(listaUsuarios, archivoUsuarios);

    cout << endl << "[OK] Usuario '" << nuevoUsuario.nombre << "' ingresado correctamente." << endl;
    pausar();
}

// ─── Listar todos los usuarios desde memoria ─────────────
void listarUsuarios(const ListaUsuarios& listaUsuarios) {
    cout << endl << "=== Lista de Usuarios ===" << endl;

    if (listaUsuarios.cantidad == 0) {
        cout << "No hay usuarios registrados." << endl;
        pausar();
        return;
    }

    cout << "---------------------------------------------------------------" << endl;
    cout << "ID\tNombre\t\t\tUsername\tPerfil" << endl;
    cout << "---------------------------------------------------------------" << endl;

    for (int i = 0; i < listaUsuarios.cantidad; i++) {
        const Usuario& u = listaUsuarios.usuarios[i];
        cout << u.id << "\t" << u.nombre << "\t\t" << u.username << "\t\t" << u.perfil << endl;
    }

    cout << "---------------------------------------------------------------" << endl;
    cout << "Total: " << listaUsuarios.cantidad << " usuario(s)" << endl;
    pausar();
}

// ─── Eliminar un usuario por ID ───────────────────────────
void eliminarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
    cout << endl << "=== Eliminar Usuario ===" << endl;

    if (listaUsuarios.cantidad == 0) {
        cout << "No hay usuarios registrados." << endl;
        pausar();
        return;
    }

    int idEliminar = leerEntero("Ingrese el ID del usuario a eliminar: ");

    // Buscar el usuario
    int indice = -1;
    for (int i = 0; i < listaUsuarios.cantidad; i++) {
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
    if (u.perfil == "ADMIN") {
        cout << endl;
        cout << "╔══════════════════════════════════════════════╗" << endl;
        cout << "║  [ADVERTENCIA] Este usuario tiene perfil     ║" << endl;
        cout << "║  ADMIN. Eliminarlo puede afectar el sistema. ║" << endl;
        cout << "╚══════════════════════════════════════════════╝" << endl;
    }

    // Confirmar eliminación
    string confirmacion;
    cout << "¿Desea eliminar este usuario? (S/N): ";
    getline(cin, confirmacion);

    if (confirmacion == "S" || confirmacion == "s") {
        // Desplazar elementos para llenar el hueco
        for (int i = indice; i < listaUsuarios.cantidad - 1; i++) {
            listaUsuarios.usuarios[i] = listaUsuarios.usuarios[i + 1];
        }
        listaUsuarios.cantidad--;

        // Reescribir archivo
        guardarUsuariosEnArchivo(listaUsuarios, archivoUsuarios);

        cout << "[OK] Usuario eliminado correctamente." << endl;
    } else {
        cout << "Operacion cancelada." << endl;
    }

    pausar();
}

// ─── Menú de Gestión de Usuarios ──────────────────────────
void menuUsuarios(ListaUsuarios& listaUsuarios, const string& archivoUsuarios) {
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
                ingresarUsuario(listaUsuarios, archivoUsuarios);
                break;
            case 2:
                listarUsuarios(listaUsuarios);
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
