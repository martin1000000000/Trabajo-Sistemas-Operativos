#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>
using namespace std;

// Lee el archivo .env y retorna el valor de una variable
string leerVariableEnv(const string& nombreVariable);

// Valida que la entrada sea un número entero
int leerEntero(const string& mensaje);

// Valida que la entrada sea un número real (double)
double leerReal(const string& mensaje);

// Limpia la pantalla de consola (multiplataforma)
void limpiarPantalla();

// Pausa la consola hasta que el usuario presione Enter
void pausar();

// Función auxiliar para contar en un archivo
void conteoTexto(const string& filename);

// Verifica si un string es palíndromo
bool esPalindromo(const string& str);

#endif // UTILIDADES_H
