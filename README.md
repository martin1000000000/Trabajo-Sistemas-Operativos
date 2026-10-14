# SistOpe: Sistema Operativo y Concurrencia (Entrega 3)

## Propósito de la Aplicación
`SistOpe` es un sistema desarrollado para la asignatura de **Sistemas Operativos (INFO198)**. En esta **Tercera Entrega**, el sistema incorpora conceptos avanzados de **concurrencia, procesamiento paralelo, sincronización mediante semáforos y comunicación distribuida entre procesos mediante sockets TCP**, manteniendo al mismo tiempo la totalidad de las funcionalidades operativas de las Entregas 1 y 2.

---

## 1. Instrucciones de Compilación y Ejecución

El proyecto cuenta con un `Makefile` compatible con entornos Windows (MinGW/MSYS2) y Linux.

### Compilar todo el sistema
```bash
mingw32-make all
```
Esto generará los 4 ejecutables del sistema:
- `SistOpe.exe`: Programa principal con menú interactivo de 8 opciones.
- `multiplicador.exe`: Subproceso independiente de multiplicación de matrices.
- `servidor_juego.exe`: Servidor central para el juego multiplayer por sockets.
- `cliente_juego.exe`: Cliente de juego para cada participante.

### Ejecución del Sistema Principal (`SistOpe`)
```bash
./SistOpe.exe -u PorDefecto -p 123 -f El_Mundo_de_Sofia.txt
```
*(O utilizando cualquier usuario registrado en la base de datos).*

### Ejecución del Juego Multiplayer (Sockets)
Para jugar se requieren al menos 4 jugadores (2 por equipo).
1. En la primera consola, inicia el servidor:
   ```bash
   ./servidor_juego.exe
   ```
2. En consolas independientes (o desde la Opción 3 de `SistOpe`), conecta a los clientes:
   ```bash
   ./cliente_juego.exe "Pedro" "EquipoA"
   ./cliente_juego.exe "Maria" "EquipoB"
   ./cliente_juego.exe "Juan" "EquipoA"
   ./cliente_juego.exe "Ana" "EquipoB"
   ```

*(Nota: Ejecuta `mingw32-make clean` para limpiar todos los ejecutables generados).*

---

## 2. Variables de Entorno (.env)

El sistema parametriza su configuración obligatoriamente mediante variables de entorno (leídas desde el entorno de ejecución o desde el archivo `.env`), sin valores hardcodeados en el código:

| Variable | Descripción / Propósito | Valor por Defecto |
|---|---|---|
| `USER_FILE` | Archivo binario de usuarios | `USUARIOS.DAT` |
| `PERFIL_FILE` | Archivo binario de perfiles | `PERFILES.DAT` |
| `MULTIPLICADOR_EXE` | Ruta del subproceso multiplicador de matrices | `multiplicador.exe` |
| `LIBROS_DIR` | Directorio que contiene los libros de texto (`*.txt`) | `LIBROS` |
| `INDICE_FILE` | Archivo de salida del índice invertido | `indice_invertido.idx` |
| `MAPA_LIBROS_FILE` | Archivo de asociación numérica `ID;nombre_libro` | `mapa_libros.txt` |
| `N_THREADS` | Cantidad de threads trabajadores paralelos | `4` |
| `GAME_LOG` | Archivo de log del juego multiplayer | `juego.log` |
| `GAME_HOST` | Dirección IP o host del servidor de juego | `127.0.0.1` |
| `GAME_PORT` | Puerto de escucha TCP del servidor | `8080` |
| `TABLERO_X` | Cantidad de casillas objetivo del tablero | `50` |
| `DADO_R` | Cantidad de caras del dado aleatorio | `6` |

---

## 3. Problema 1: Procesamiento Paralelo del Índice Invertido

El Problema 1 procesa de forma concurrente todos los libros de texto ubicados en `LIBROS_DIR` y construye un índice invertido global que almacena la frecuencia y ubicación de cada palabra única.

### 3.1 Flujo del Procesamiento Concurrente
```
cola → threads → semáforos → resultados → índice invertido
```

```
[Carpeta LIBROS]
       │
       ▼
 [Listar *.txt y asignar ID único (mapa_libros.txt)]
       │
       ▼
 [Cola de Trabajo Compartida (std::queue)]
       │
       ├─────────────────┬─────────────────┐
       ▼                 ▼                 ▼
  [Thread 1]        [Thread 2]        [Thread N]
   (sem_wait)        (sem_wait)        (sem_wait)
       │                 │                 │
 [Pop de cola]     [Pop de cola]     [Pop de cola]
   (sem_post)        (sem_post)        (sem_post)
       │                 │                 │
 [Conteo local]    [Conteo local]    [Conteo local]
       │                 │                 │
   (sem_wait)        (sem_wait)        (sem_wait)
       └─────────────────┼─────────────────┘
                         ▼
             [Merge en Índice Global]
                    (sem_post)
                         │
                         ▼
               [Join de todos los threads]
                         │
                         ▼
           [Escritura en indice_invertido.idx]
```

### 3.2 Secciones Críticas y Mecanismos de Sincronización
Se emplean **semáforos POSIX (`sem_t`)** para garantizar exclusión mutua estricta y prevenir condiciones de carrera (*Race Conditions*):
1. **SC1 – Extracción de la Cola de Trabajo (`sem_cola`):** Garantiza que cada libro sea tomado de la cola por un único thread. Ningún libro es procesado más de una vez ni simultáneamente por dos hilos.
2. **SC2 – Fusión en el Índice Global (`sem_indice`):** Cada thread cuenta las palabras de forma independiente en una estructura local `std::map<string, int>`. Una vez finalizado el libro, entra a la sección crítica protegida por `sem_indice` para realizar el merge en el índice invertido compartido `std::map<string, map<int, int>>`.
3. **SC3 – Consola Protegida (`sem_consola`):** Coordina los mensajes de progreso en pantalla para evitar intercalado de caracteres entre hilos.

### 3.3 Formato de Salida
- **`mapa_libros.txt`**: Mapeo determinista `ID;nombre_libro`:
  ```text
  1;biografias_Vida_de_Cervantes.txt
  2;ciencia_ficcion_De_la_Tierra_a_la_Luna.txt
  3;ciencias_El_Mundo_de_Sofia_tomo1.txt
  ```
- **`indice_invertido.idx`**: Palabras ordenadas alfabéticamente e IDs de libro ordenados numéricamente:
  ```text
  cervantes;(2,160);(6,80)
  ciencia;(2,200);(6,100);(7,60)
  mundo;(2,3960);(3,70000);(6,1980);(7,480)
  quijote;(2,17230);(6,8615)
  ```

---

## 4. Problema 2: Juego Multiplayer mediante Sockets

### 4.1 Arquitectura del Sistema
```
clientes → sockets → servidor → estado compartido del juego
```
- **Servidor Central (`servidor_juego`):** Administra el socket listener TCP, almacena en memoria el estado compartido (posiciones de los equipos, turnos, jugadores conectados), sincroniza eventos mediante mutex y persiste el archivo de log.
- **Clientes (`cliente_juego`):** Cada jugador cuenta con su propia consola interactiva conectada por sockets TCP al servidor. Un hilo receptor actualiza el tablero en tiempo real mientras el hilo principal lee las entradas del jugador.

### 4.2 Reglas del Juego
- **Número de Jugadores:** Mínimo 4 jugadores en total (2 por equipo); máximo 8 jugadores.
- **Equipos:** 2 equipos (`EquipoA` y `EquipoB`).
- **Tablero:** Carrera lineal de $X$ posiciones (definido por `TABLERO_X`, default: 50).
- **Dado:** En su turno, el jugador tira un dado de $R$ caras ($1 \dots R$, definido por `DADO_R`, default: 6).
- **Acciones Permitidas:**
  - `CONECTAR|<nombre>|<equipo>`: Registro en la partida.
  - `TIRAR`: Lanzamiento de dado cuando corresponde el turno.
  - `SALIR`: Desconexión voluntaria.
- **Condición de Inicio:** La partida comienza automáticamente cuando ambos equipos cuentan con al menos 2 jugadores conectados. Antes de esto, los jugadores permanecen informados en estado de espera.
- **Control de Turnos:** Round-robin alternado entre los equipos. El servidor rechaza acciones de jugadores que no estén en su turno.
- **Condición de Término:** El primer equipo en alcanzar o superar la casilla $X$ gana la carrera. El servidor transmite el ganador a todos los participantes.
- **Manejo de Desconexiones:** Si un jugador se desconecta abruptamente o voluntariamente, el servidor continúa activo para los demás participantes, registra el evento en el log y transfiere el turno de forma segura.

### 4.3 Registro de Eventos (Archivo de Log)
El servidor registra de forma persistente y sincronizada cada evento relevante en `GAME_LOG` (`juego.log`) con el formato exacto:
```text
YYYY-MM-DD HH:MM:SS;jugador;equipo;ACCION;informacion_adicional
```

Ejemplo de ejecución real registrada:
```text
2026-10-14 16:24:48;Pedro;EquipoA;CONECTA
2026-10-14 16:24:48;Pedro;EquipoA;ASIGNA_EQUIPO;EquipoA
2026-10-14 16:24:48;Maria;EquipoB;CONECTA
2026-10-14 16:24:48;Maria;EquipoB;ASIGNA_EQUIPO;EquipoB
2026-10-14 16:24:48;Juan;EquipoA;CONECTA
2026-10-14 16:24:48;Juan;EquipoA;ASIGNA_EQUIPO;EquipoA
2026-10-14 16:24:49;Ana;EquipoB;CONECTA
2026-10-14 16:24:49;Ana;EquipoB;ASIGNA_EQUIPO;EquipoB
2026-10-14 16:24:49;SISTEMA;TODOS;INICIO_PARTIDA;Tablero=50;Dado=6
2026-10-14 16:24:49;Pedro;EquipoA;CAMBIO_TURNO
2026-10-14 16:24:49;Pedro;EquipoA;LANZA_DADO;4
2026-10-14 16:24:49;Pedro;EquipoA;AVANZA;0;4
2026-10-14 16:24:49;Maria;EquipoB;CAMBIO_TURNO
2026-10-14 16:24:50;Maria;EquipoB;LANZA_DADO;6
2026-10-14 16:24:50;Maria;EquipoB;AVANZA;0;6
2026-10-14 16:24:51;Pedro;EquipoA;DESCONECTA
```

---

## 5. Funcionalidades Heredadas (Entregas 1 y 2)

Todas las funciones anteriores continúan 100% operativas:
- **Opción 1:** Administración de Usuarios y Perfiles (CRUD binario con archivos espejo).
- **Opción 2:** Multiplicador de Matrices (`multiplicador.exe`), con validación estricta de formato y capacidad de ejecución independiente sin requerir perfiles.
- **Opción 4:** Validador de Palíndromos (flujo ordenado: primero opciones de validar/cancelar y luego solicitud de texto).
- **Opción 5:** Función matemática $f(x) = x^2 + 2x + 8$.
- **Opciones 6 y 7:** Conteo unificado de texto (palabras, vocales, consonantes, caracteres especiales) sobre el archivo de consola `-f` o cualquier ruta manual.
