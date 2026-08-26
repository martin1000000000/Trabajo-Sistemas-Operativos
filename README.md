# SistOpe: Administrador de Usuarios y Perfiles

## Propósito de la aplicación
SistOpe es un sistema desarrollado para la asignatura de Sistemas Operativos (INFO198). En esta primera entrega, el sistema consiste en un módulo base llamado **Administrador de Usuarios y Perfiles**. Su propósito principal es gestionar la creación, eliminación y listado de usuarios y perfiles, manteniendo persistencia de datos a través de archivos de texto plano. Todos los registros y estructuras se gestionan utilizando memoria dinámica (`std::vector`) para garantizar que el sistema pueda crecer sin límites de almacenamiento predefinidos.

## Cómo ejecutar

El proyecto incluye un `Makefile` preparado para entornos de Windows (MinGW/MSYS2).

1. Abre tu terminal y sitúate en el directorio raíz del proyecto (`Trabajo-Sistemas-Operativos`).
2. Compila el código fuente ejecutando el siguiente comando:
   ```bash
   mingw32-make
   ```
3. Una vez compilado correctamente sin errores, ejecuta la aplicación:
   ```bash
   ./SistOpe
   ```

*(Nota: Puedes ejecutar `mingw32-make clean` en cualquier momento para limpiar los archivos binarios generados).*

## Variables de entorno (.env)

El sistema soporta la lectura de variables de entorno mediante un archivo `.env` ubicado en la raíz del proyecto. Este archivo sirve para configurar de forma externa las rutas de los archivos de texto que el sistema usará para almacenar los datos.

Por ahora el archivo contiene:
- `USER_FILE=USUARIOS.TXT`: Define el nombre/ruta del archivo donde se guardarán y leerán los usuarios registrados.
- `PERFIL_FILE=PERFILES.TXT`: Define el nombre/ruta del archivo donde se guardarán y leerán los perfiles creados.

## Contexto de Archivos

- **`src/main.cpp`**: Punto de entrada del programa. Coordina la carga inicial de datos, carga las variables de entorno y despliega el menú principal interactivo.
- **`src/usuarios.cpp` / `include/usuarios.h`**: Contiene toda la lógica para registrar, listar, eliminar y guardar usuarios. Manipula el vector dinámico de `Usuario`.
- **`src/perfiles.cpp` / `include/perfiles.h`**: Gestiona las operaciones de los perfiles (crear, listar, borrar, guardar). Administra el vector dinámico de `Perfil`.
- **`src/utilidades.cpp` / `include/utilidades.h`**: Provee herramientas de uso general como lectura segura de enteros, limpieza de pantalla, pausa y carga del archivo `.env`.
- **`include/estructuras.h`**: Define los structs fundamentales del programa (`Usuario`, `Perfil`, `ListaUsuarios`, `ListaPerfiles`). Utiliza `std::vector` para manejar la memoria dinámicamente.
- **`USUARIOS.TXT` / `PERFILES.TXT`**: Archivos de base de datos en texto plano donde se almacena de forma persistente la información del sistema.

## Flujo de ejecución

1. **Arranque e Inicialización**: Al iniciar, el programa lee el archivo `.env` para obtener las rutas de la base de datos (`USER_FILE` y `PERFIL_FILE`).
2. **Carga en memoria**: Se lee secuencialmente la información almacenada en los archivos `.TXT` y se guarda en memoria dinámica (mediante los vectores `ListaUsuarios` y `ListaPerfiles`).
3. **Despliegue del Menú Principal**: Se presenta la consola interactiva donde el usuario puede navegar a través del módulo de gestión de usuarios o el de perfiles. Los identificadores de opción son tratados internamente con números y la opción 0 permite salir.
4. **Operaciones CRUD**: Al crear o eliminar usuarios/perfiles, las operaciones se ejecutan tanto en la memoria temporal (el `vector`) como directamente reescribiendo o agregando al archivo `.TXT`, asegurando consistencia.
5. **Cierre**: Al seleccionar la opción `0`, se liberan recursos y finaliza la ejecución.

## Limitaciones Conocidas

- **Restricción Inicial de Perfiles:** Según las indicaciones del documento oficial de entrega, el sistema originalmente restringe los perfiles base (tanto al crear un perfil como al asignarlo a un usuario) a ser exclusivamente `"ADMIN"` o `"GENERAL"`. Además, los ingresos son validados automáticamente eliminando espacios y convirtiendo a mayúsculas para evitar errores (ej. "a dmin" -> "ADMIN").
- **Eliminación Segura:** Al intentar eliminar un usuario que tiene un perfil tipo `"ADMIN"`, el sistema está diseñado para enviar una alerta antes de permitir su eliminación para proteger cuentas críticas.
- **Manejo de errores básicos de archivo**: Si el archivo de texto no existe en la primera ejecución, el sistema simplemente lo creará, sin interrumpir el flujo.
