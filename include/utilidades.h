#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>
using namespace std;

// Lee el archivo .env y retorna el valor de una variable
string leerVariableEnv(const string& nombreVariable);

// Valida que la entrada sea un número entero
int leerEntero(const string& mensaje);

// Limpia la pantalla de consola (multiplataforma)
void limpiarPantalla();

// Pausa la consola hasta que el usuario presione Enter
void pausar();

#endif // UTILIDADES_H
