# SistOpe: Administrador de Usuarios y Perfiles (Entrega 2)

## Propósito de la aplicación
SistOpe es un sistema desarrollado para la asignatura de Sistemas Operativos (INFO198). En esta **Segunda Entrega**, el sistema ha evolucionado de un simple gestor a un entorno de consola interactivo más completo. Ahora implementa persistencia de memoria binaria, control de acceso mediante login por consola, validación estricta de permisos por perfiles, y llamadas a subprocesos externos.

## Cómo ejecutar

El proyecto incluye un `Makefile` preparado para entornos de Windows (MinGW/MSYS2) que compilará tanto el programa principal como el subprograma matemático externo.

1. Abre tu terminal y sitúate en el directorio raíz del proyecto (`Trabajo-Sistemas-Operativos`).
2. Compila el código fuente ejecutando el siguiente comando:
   ```bash
   mingw32-make
   ```
3. Una vez compilado, debes ejecutar el sistema pasando obligatoriamente tus credenciales de acceso y un archivo de prueba por parámetro:
   ```bash
   ./SistOpe.exe -u <usuario> -p <contraseña> -f <archivo_de_texto.txt>
   ```
   *Ejemplo:* `./SistOpe.exe -u admin -p 123 -f prueba.txt`

*(Nota: Puedes ejecutar `mingw32-make clean` en cualquier momento para limpiar los ejecutables generados).*

## Nuevas Características (Entrega 2)

### 1. Parámetros de Interfaz de Línea de Comandos (CLI)
El sistema ahora valida que se inicien con los parámetros requeridos (`-u`, `-p`, `-f`). Si el usuario o contraseña no coinciden exactamente con la base de datos binaria, se bloquea el acceso.

### 2. Base de Datos Binaria y Archivos Espejo
Por requerimiento de la rúbrica, la lectura y escritura de los *structs* se realiza copiando el bloque completo de memoria en formato binario (archivos `.DAT`). Sin embargo, para mantener la transparencia y facilitar la corrección, el sistema implementa **Archivos Espejo**: cada vez que guarda en binario, genera automáticamente un clon en texto plano (`USUARIOS.TXT` y `PERFILES.TXT`) para fácil auditoría humana.

### 3. Autoincremento y Validación Dinámica de Perfiles
Al registrar un nuevo usuario en la Opción 1 (exclusiva para perfil ADMIN), el sistema le asigna un `ID` automáticamente. Además, ya no está bloqueado estáticamente a "ADMIN" y "GENERAL"; el sistema lee la lista de perfiles reales desde la base de datos y solo permite elegir perfiles válidos existentes.

### 4. Menú Interactivo con Permisos por Perfil
Se implementó un menú de 7 opciones. El sistema carga la matriz numérica de opciones habilitadas para el perfil del usuario logueado. Si un usuario intenta acceder a una opción para la cual no tiene permiso asignado, el acceso es denegado dinámicamente.

### 5. Multiplicador de Matrices Independiente (Subproceso)
La Opción 2 de multiplicación de matrices está desacoplada. Se programó como una "caja negra matemática" independiente (`multiplicador.exe`). Al elegir esta opción, el sistema principal invoca internamente a este subprograma pasándole solo las rutas y el separador. 

### 6. Contador de Palabras y Letras (Opciones 6 y 7)
Un motor unificado lee archivos de texto de cualquier tamaño, contando palabras, vocales, consonantes y caracteres especiales. Puede procesar automáticamente el archivo recibido por el argumento inicial `-f` (Opción 6) o recibir una nueva ruta manual (Opción 7). Es capaz de procesar sin problemas los libros gigantes (>50MB) de la carpeta `LIBROS`.

## Variables de entorno (.env)

El sistema lee las rutas maestras de su base de datos desde un archivo `.env` ubicado en la raíz.
- `USER_FILE=USUARIOS.DAT`: Define el archivo binario principal de usuarios.
- `PERFIL_FILE=PERFILES.DAT`: Define el archivo binario principal de perfiles.
- `MULTIPLICADOR_EXE=multiplicador.exe`: Define el nombre/ruta del subprograma externo que multiplica matrices (Opción 2).
