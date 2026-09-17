#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <vector>
using namespace std;

// ─── Struct Usuario ───────────────────────────────────────
struct Usuario {
    int id;
    char nombre[50];
    char username[50];
    char password[50];
    char perfil[20];  // Perfil asignado (ej: "ADMIN", "GENERAL", etc.)
};

// ─── Lista de Usuarios (en memoria) ──────────────────────
struct ListaUsuarios {
    vector<Usuario> usuarios;
};

// ─── Struct Perfil ────────────────────────────────────────
struct Perfil {
    char nombre[20];       // Nombre del perfil (ej: "ADMIN", "GENERAL")
    int opciones[20];      // Opciones disponibles (ej: {0,1,2,3,4})
    int num_opciones;      // Cantidad de opciones configuradas
};

// ─── Lista de Perfiles (en memoria) ──────────────────────
struct ListaPerfiles {
    vector<Perfil> perfiles;
};

#endif // ESTRUCTURAS_H
