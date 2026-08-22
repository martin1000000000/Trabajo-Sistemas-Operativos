#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
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
    Usuario usuarios[100];
    int cantidad;
};

// ─── Struct Perfil ────────────────────────────────────────
struct Perfil {
    string nombre;       // Nombre del perfil (ej: "ADMIN", "GENERAL")
    int opciones[10];    // Opciones disponibles (ej: {0,1,2,3,4})
    int numOpciones;     // Cantidad de opciones en el array
};

// ─── Lista de Perfiles (en memoria) ──────────────────────
struct ListaPerfiles {
    Perfil perfiles[10];
    int cantidad;
};

#endif // ESTRUCTURAS_H
