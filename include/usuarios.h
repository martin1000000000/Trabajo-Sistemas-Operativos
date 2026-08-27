#ifndef USUARIOS_H
#define USUARIOS_H

#include "estructuras.h"
#include <string>

// Muestra el menú de gestión de usuarios y maneja las opciones
void menuUsuarios(ListaUsuarios& listaUsuarios, const string& archivoUsuarios, const ListaPerfiles& listaPerfiles);

// Ingresa un nuevo usuario (pide datos y lo agrega a memoria + archivo)
void ingresarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios, const ListaPerfiles& listaPerfiles);

// Lista todos los usuarios desde memoria (o desde archivo si no hay datos cargados)
void listarUsuarios(ListaUsuarios& listaUsuarios, const string& archivoUsuarios);

// Elimina un usuario por ID (con alerta si es ADMIN)
void eliminarUsuario(ListaUsuarios& listaUsuarios, const string& archivoUsuarios);

// Carga usuarios desde el archivo TXT a la lista en memoria
void cargarUsuariosDesdeArchivo(ListaUsuarios& listaUsuarios, const string& archivoUsuarios);

// Guarda toda la lista de usuarios al archivo TXT (reescribe completo)
void guardarUsuariosEnArchivo(ListaUsuarios& listaUsuarios, const string& archivoUsuarios);

#endif // USUARIOS_H
