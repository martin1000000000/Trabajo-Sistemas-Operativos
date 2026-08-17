# SistOpe – Administrador de Usuarios y Perfiles

## Propósito

Proyecto para la asignatura **INFO198 – Sistemas Operativos**.  
Módulo de consola en C++ para gestionar usuarios y perfiles con persistencia en archivos de texto.

## Variables de Entorno

El programa lee el archivo `.env` en la raíz del proyecto para determinar los archivos de datos:

| Variable | Descripción | Valor por defecto |
|---|---|---|
| `USER_FILE` | Archivo donde se guardan los usuarios | `USUARIOS.TXT` |
| `PERFIL_FILE` | Archivo donde se guardan los perfiles | `PERFILES.TXT` |

## Compilación

```bash
g++ src/*.cpp -o SistOpe
```

## Ejecución

```bash
./SistOpe
```

En Windows:
```cmd
SistOpe.exe
```

## Estructura del Proyecto

```
SistOpe/
├── .env                  ← Variables de entorno
├── .gitignore
├── README.md
├── USUARIOS.TXT          ← Generado al guardar usuarios
├── PERFILES.TXT          ← Generado al guardar perfiles
└── src/
    ├── main.cpp          ← Punto de entrada
    ├── estructuras.h     ← Structs globales
    ├── utilidades.h/.cpp ← Funciones auxiliares
    ├── usuarios.h/.cpp   ← Módulo de usuarios
    └── perfiles.h/.cpp   ← Módulo de perfiles
```

## Integrantes

- Martin Arrigo
- Nicolas Toro
- Benjamin Neira
- Diego Mora 
