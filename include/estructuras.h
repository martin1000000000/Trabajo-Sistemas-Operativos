#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <vector>
using namespace std;

// ─── Struct Usuario ───────────────────────────────────────
struct Usuario {
    int id;
    string nombre;
    string username;
    string password;
    string perfil;  // Perfil asignado (ej: "ADMIN", "GENERAL", etc.)
};

// ─── Lista de Usuarios (en memoria) ──────────────────────
struct ListaUsuarios {
    vector<Usuario> usuarios;
};

// ─── Struct Perfil ────────────────────────────────────────
struct Perfil {
    string nombre;       // Nombre del perfil (ej: "ADMIN", "GENERAL")
    vector<int> opciones;    // Opciones disponibles (ej: {0,1,2,3,4})
};

// ─── Lista de Perfiles (en memoria) ──────────────────────
struct ListaPerfiles {
    vector<Perfil> perfiles;
};

#endif // ESTRUCTURAS_H
