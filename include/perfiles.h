#ifndef PERFILES_H
#define PERFILES_H

#include "estructuras.h"
#include <string>

// Muestra el menú de gestión de perfiles y maneja las opciones
void menuPerfiles(ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

// Ingresa un nuevo perfil (pide datos y lo agrega a memoria + archivo)
void ingresarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

// Lista todos los perfiles desde memoria (o desde archivo si no hay datos cargados)
void listarPerfiles(ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

// Elimina un perfil por nombre
void eliminarPerfil(ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

// Carga perfiles desde el archivo TXT a la lista en memoria
void cargarPerfilesDesdeArchivo(ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

// Guarda toda la lista de perfiles al archivo TXT (reescribe completo)
void guardarPerfilesEnArchivo(const ListaPerfiles& listaPerfiles, const string& archivoPerfiles);

#endif // PERFILES_H
