#include "indice.h"
#include "utilidades.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <thread>
#include <semaphore.h>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

// ─── Estructura de libro en cola ─────────────────────────
struct TareaLibro {
    int id;
    string path;
    string nombreArchivo;
};

// ─── Variables globales para los threads ──────────────────
static queue<TareaLibro> g_colaTrabajo;
static sem_t g_semCola;
static sem_t g_semIndice;
static sem_t g_semConsola;

// Estructura: palabra -> (id_libro -> conteo)
static map<string, map<int, int>> g_indiceGlobal;
static int g_librosCompletados = 0;

// ─── Función para limpiar y normalizar palabras con soporte UTF-8 ───
static void procesarTextoLibro(const string& ruta, map<string, int>& conteoLocal) {
    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) return;

    string linea;
    while (getline(archivo, linea)) {
        string palabraActual = "";
        size_t i = 0;
        size_t n = linea.size();

        while (i < n) {
            unsigned char c = static_cast<unsigned char>(linea[i]);

            // Caracteres ASCII estándar
            if (c < 128) {
                if (isalnum(c)) {
                    palabraActual += static_cast<char>(tolower(c));
                } else {
                    if (!palabraActual.empty()) {
                        conteoLocal[palabraActual]++;
                        palabraActual.clear();
                    }
                }
                i++;
            }
            // Caracteres UTF-8 de 2 bytes (ej. Á, É, Í, Ó, Ú, Ñ, á, é, í, ó, ú, ñ, ü)
            else if (c == 0xC3 && i + 1 < n) {
                unsigned char c2 = static_cast<unsigned char>(linea[i + 1]);
                palabraActual += static_cast<char>(0xC3);
                // Si es mayúscula en UTF-8 (0x80 a 0x9F), pasar a minúscula (+ 0x20)
                if (c2 >= 0x80 && c2 <= 0x9F) {
                    palabraActual += static_cast<char>(c2 + 0x20);
                } else {
                    palabraActual += static_cast<char>(c2);
                }
                i += 2;
            }
            // Otros caracteres multibyte (puntuación especial como guiones largos —, comillas «», etc.)
            else {
                // Delimitador multibyte
                if (!palabraActual.empty()) {
                    conteoLocal[palabraActual]++;
                    palabraActual.clear();
                }
                // Avanzar según la longitud del código UTF-8
                if ((c & 0xE0) == 0xC0) i += 2;
                else if ((c & 0xF0) == 0xE0) i += 3;
                else if ((c & 0xF8) == 0xF0) i += 4;
                else i++;
            }
        }

        if (!palabraActual.empty()) {
            conteoLocal[palabraActual]++;
            palabraActual.clear();
        }
    }
    archivo.close();
}

// ─── Función que ejecuta cada thread trabajador ───────────
static void workerThread(int threadId) {
    while (true) {
        TareaLibro libroActual;
        bool hayLibro = false;

        // SC1: Extracción atómica de la cola compartida
        sem_wait(&g_semCola);
        if (!g_colaTrabajo.empty()) {
            libroActual = g_colaTrabajo.front();
            g_colaTrabajo.pop();
            hayLibro = true;
        }
        sem_post(&g_semCola);

        if (!hayLibro) {
            break; // No quedan más libros por procesar
        }

        // Registro de inicio en consola protegida
        sem_wait(&g_semConsola);
        cout << "[Worker " << threadId << "] Iniciando procesamiento de libro ID " 
             << libroActual.id << " (" << libroActual.nombreArchivo << ")..." << endl;
        sem_post(&g_semConsola);

        // Conteo local sin bloquear el índice global
        map<string, int> conteoLocal;
        procesarTextoLibro(libroActual.path, conteoLocal);

        // SC2: Merge protegido en el índice global
        sem_wait(&g_semIndice);
        for (const auto& par : conteoLocal) {
            g_indiceGlobal[par.first][libroActual.id] += par.second;
        }
        g_librosCompletados++;
        int progreso = g_librosCompletados;
        sem_post(&g_semIndice);

        // Registro de finalización en consola protegida
        sem_wait(&g_semConsola);
        cout << "[Worker " << threadId << "] Completado libro ID " << libroActual.id 
             << " (" << conteoLocal.size() << " palabras unicas). Progreso: " 
             << progreso << " libros." << endl;
        sem_post(&g_semConsola);
    }
}

// ─── Función principal de creación del índice ─────────────
void crearIndiceInvertidoParalelo() {
    cout << "\n==========================================" << endl;
    cout << "  CREACION PARALELA DE INDICE INVERTIDO" << endl;
    cout << "==========================================" << endl;

    // 1. Obtener y validar variables de entorno (sin nombres hardcodeados en código)
    string carpetaLibros = obtenerVariableEnvOConfig("LIBROS_DIR");
    string archivoSalida = obtenerVariableEnvOConfig("INDICE_FILE");
    string archivoMapa = obtenerVariableEnvOConfig("MAPA_LIBROS_FILE");

    if (carpetaLibros.empty() || archivoSalida.empty() || archivoMapa.empty()) {
        cout << "[ERROR] Faltan variables de entorno requeridas (LIBROS_DIR, INDICE_FILE o MAPA_LIBROS_FILE). Abortando." << endl;
        return;
    }

    int nThreads = 0;
    if (!validarVariableEntero("N_THREADS", nThreads, 1, 64)) {
        cout << "[ERROR] La variable N_THREADS es invalida o no esta definida. Abortando." << endl;
        return;
    }

    cout << "[CONFIG] Carpeta de libros: " << carpetaLibros << endl;
    cout << "[CONFIG] Archivo de indice: " << archivoSalida << endl;
    cout << "[CONFIG] Archivo mapa IDs : " << archivoMapa << endl;
    cout << "[CONFIG] Numero de threads: " << nThreads << endl;
    cout << "------------------------------------------" << endl;

    // 2. Validar que la carpeta de libros exista y sea accesible
    if (!fs::exists(carpetaLibros) || !fs::is_directory(carpetaLibros)) {
        cout << "[ERROR] La carpeta de libros '" << carpetaLibros << "' no existe o no es un directorio." << endl;
        return;
    }

    // 3. Identificar libros *.txt disponibles
    vector<string> archivosEncontrados;
    try {
        for (const auto& entry : fs::directory_iterator(carpetaLibros)) {
            if (entry.is_regular_file()) {
                string ext = entry.path().extension().string();
                transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                string filename = entry.path().filename().string();
                if (ext == ".txt" && filename != "ENLACE_A_LIBROS.txt") {
                    archivosEncontrados.push_back(entry.path().string());
                }
            }
        }
    } catch (const exception& e) {
        cout << "[ERROR] No se pudo leer el contenido de la carpeta '" << carpetaLibros << "': " << e.what() << endl;
        return;
    }

    if (archivosEncontrados.empty()) {
        cout << "[ERROR] No se encontraron archivos de texto (*.txt) en la carpeta '" << carpetaLibros << "'." << endl;
        return;
    }

    sort(archivosEncontrados.begin(), archivosEncontrados.end());

    // 4. Asignar IDs y crear archivo de mapa (ID;nombre_libro)
    ofstream fMapa(archivoMapa);
    if (!fMapa.is_open()) {
        cout << "[ERROR] No se pudo crear el archivo de mapa: " << archivoMapa << endl;
        return;
    }

    // Limpiar cola y estructuras globales
    while (!g_colaTrabajo.empty()) g_colaTrabajo.pop();
    g_indiceGlobal.clear();
    g_librosCompletados = 0;

    cout << "[INFO] Libros identificados (" << archivosEncontrados.size() << "):" << endl;
    for (size_t i = 0; i < archivosEncontrados.size(); ++i) {
        int idLibro = static_cast<int>(i + 1);
        string path = archivosEncontrados[i];
        string nombre = fs::path(path).filename().string();

        fMapa << idLibro << ";" << nombre << "\n";
        g_colaTrabajo.push({ idLibro, path, nombre });

        cout << "  [" << idLibro << "] " << nombre << endl;
    }
    fMapa.close();
    cout << "[INFO] Mapa de IDs guardado exitosamente en '" << archivoMapa << "'." << endl;

    // 5. Inicializar semáforos
    sem_init(&g_semCola, 0, 1);
    sem_init(&g_semIndice, 0, 1);
    sem_init(&g_semConsola, 0, 1);

    // 6. Lanzar threads trabajadores
    cout << "\n[INFO] Iniciando " << nThreads << " threads trabajadores..." << endl;
    auto tInicio = chrono::high_resolution_clock::now();

    vector<thread> workers;
    for (int i = 0; i < nThreads; ++i) {
        workers.emplace_back(workerThread, i + 1);
    }

    // 7. Esperar finalización de todos los threads (join)
    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }

    auto tFin = chrono::high_resolution_clock::now();
    chrono::duration<double> duracion = tFin - tInicio;

    // Destruir semáforos
    sem_destroy(&g_semCola);
    sem_destroy(&g_semIndice);
    sem_destroy(&g_semConsola);

    cout << "\n[INFO] Todos los threads finalizaron correctamente." << endl;
    cout << "[INFO] Tiempo de procesamiento paralelo: " << duracion.count() << " segundos." << endl;

    // 8. Escribir el índice invertido final
    cout << "[INFO] Escribiendo indice invertido en '" << archivoSalida << "'..." << endl;
    ofstream fIndice(archivoSalida);
    if (!fIndice.is_open()) {
        cout << "[ERROR] No se pudo crear el archivo de indice invertido: " << archivoSalida << endl;
        return;
    }

    // Formato exacto especificado: palabra;(id1,conteo1);(id2,conteo2)...
    for (const auto& entry : g_indiceGlobal) {
        fIndice << entry.first;
        for (const auto& libroPar : entry.second) {
            fIndice << ";(" << libroPar.first << "," << libroPar.second << ")";
        }
        fIndice << "\n";
    }
    fIndice.close();

    cout << "==========================================" << endl;
    cout << "  INDICE INVERTIDO GENERADO CON EXITO" << endl;
    cout << "  Libros procesados    : " << archivosEncontrados.size() << endl;
    cout << "  Palabras unicas      : " << g_indiceGlobal.size() << endl;
    cout << "  Archivo generado     : " << archivoSalida << endl;
    cout << "  Mapa de IDs generado : " << archivoMapa << endl;
    cout << "  Tiempo transcurrido  : " << duracion.count() << " s" << endl;
    cout << "==========================================" << endl;
}
