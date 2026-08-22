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
g++ src/*.cpp -Iinclude -o SistOpe
# O si prefieres automatizar la tarea, usar:
# make
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
├── Makefile              ← Automatización de compilación
├── README.md
├── USUARIOS.TXT          ← Generado al guardar usuarios
├── PERFILES.TXT          ← Generado al guardar perfiles
├── include/              ← Archivos de cabecera (.h)
│   ├── estructuras.h
│   ├── perfiles.h
│   ├── usuarios.h
│   └── utilidades.h
└── src/                  ← Código fuente (.cpp)
    ├── main.cpp          ← Punto de entrada
    ├── perfiles.cpp      
    ├── usuarios.cpp      
    └── utilidades.cpp    
```

## Integrantes (Grupo X)

- Martin Arrigo
- Nicolas Toro
- Benjamin Neira
- Diego Mora 

