#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <algorithm>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "protocolo_juego.h"
#include "utilidades.h"

using namespace std;

// ─── Estructura de Jugador Conectado ─────────────────────
struct Jugador {
    SOCKET socket;
    string nombre;
    string equipo; // "EquipoA" o "EquipoB"
    bool activo;
};

// ─── Estado Global del Servidor ──────────────────────────
static mutex g_mutexJuego;
static mutex g_mutexLog;

static vector<shared_ptr<Jugador>> g_jugadores;
static int g_posEquipoA = 0;
static int g_posEquipoB = 0;
static int g_metaTablero = 50;
static int g_carasDado = 6;
static bool g_partidaIniciada = false;
static bool g_partidaFinalizada = false;
static size_t g_indiceTurno = 0;

static string g_archivoLog = "juego.log";
static mt19937 g_rng(static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count()));

// ─── Función de Registro de Log ──────────────────────────
// Formato requerido: YYYY-MM-DD HH:MM:SS;jugador;equipo;ACCION;detalle
static void registrarLog(const string& jugador, const string& equipo, const string& accion, const string& detalle = "") {
    lock_guard<mutex> lock(g_mutexLog);

    auto ahora = chrono::system_clock::now();
    time_t tiempoT = chrono::system_clock::to_time_t(ahora);
    tm tmInfo;
    localtime_s(&tmInfo, &tiempoT);

    stringstream ssFecha;
    ssFecha << put_time(&tmInfo, "%Y-%m-%d %H:%M:%S");

    string lineaLog = ssFecha.str() + ";" + jugador + ";" + equipo + ";" + accion;
    if (!detalle.empty()) {
        lineaLog += ";" + detalle;
    }

    // Mostrar en consola del servidor
    cout << "[LOG] " << lineaLog << endl;

    // Persistir en archivo de log
    ofstream fLog(g_archivoLog, ios::app);
    if (fLog.is_open()) {
        fLog << lineaLog << "\n";
        fLog.flush();
        fLog.close();
    }
}

// ─── Envío de Mensajes a Clientes ────────────────────────
static void enviarMensaje(SOCKET sock, const string& mensaje) {
    string msgCompleto = mensaje + "\n";
    send(sock, msgCompleto.c_str(), static_cast<int>(msgCompleto.length()), 0);
}

static void broadcast(const string& mensaje) {
    for (const auto& j : g_jugadores) {
        if (j->activo) {
            enviarMensaje(j->socket, mensaje);
        }
    }
}

// ─── Obtener siguiente jugador activo alternando equipos ───
static string g_ultimoEquipoTurno = "";

static shared_ptr<Jugador> obtenerSiguienteTurno() {
    if (g_jugadores.empty()) return nullptr;

    string equipoBuscado = (g_ultimoEquipoTurno == "EquipoA") ? "EquipoB" : "EquipoA";
    size_t total = g_jugadores.size();

    // 1. Priorizar encontrar el siguiente jugador activo del equipo contrario
    for (size_t i = 1; i <= total; ++i) {
        size_t idx = (g_indiceTurno + i) % total;
        if (g_jugadores[idx]->activo && g_jugadores[idx]->equipo == equipoBuscado) {
            g_indiceTurno = idx;
            g_ultimoEquipoTurno = equipoBuscado;
            return g_jugadores[idx];
        }
    }

    // 2. Si el otro equipo no tiene jugadores activos, buscar cualquier jugador activo
    for (size_t i = 1; i <= total; ++i) {
        size_t idx = (g_indiceTurno + i) % total;
        if (g_jugadores[idx]->activo) {
            g_indiceTurno = idx;
            g_ultimoEquipoTurno = g_jugadores[idx]->equipo;
            return g_jugadores[idx];
        }
    }
    return nullptr;
}

static shared_ptr<Jugador> obtenerTurnoActual() {
    if (g_jugadores.empty()) return nullptr;
    if (g_indiceTurno < g_jugadores.size() && g_jugadores[g_indiceTurno]->activo) {
        return g_jugadores[g_indiceTurno];
    }
    return obtenerSiguienteTurno();
}

// ─── Notificar estado del juego ──────────────────────────
static void notificarTurnoYTablero() {
    auto jTurno = obtenerTurnoActual();
    if (!jTurno) return;

    // Enviar posiciones actuales del tablero
    broadcast("TABLERO|" + to_string(g_posEquipoA) + "|" + to_string(g_posEquipoB) + "|" + to_string(g_metaTablero));
    
    // Anunciar turno
    broadcast("TURNO|" + jTurno->nombre + "|" + jTurno->equipo);
    registrarLog(jTurno->nombre, jTurno->equipo, "CAMBIO_TURNO");
}

// ─── Verificar condición de inicio ───────────────────────
// Condición: al menos 2 equipos y cada equipo con al menos 2 jugadores (mínimo 4)
static void verificarCondicionInicio() {
    if (g_partidaIniciada) return;

    int cantA = 0;
    int cantB = 0;
    for (const auto& j : g_jugadores) {
        if (j->activo) {
            if (j->equipo == "EquipoA") cantA++;
            else if (j->equipo == "EquipoB") cantB++;
        }
    }

    if (cantA >= 2 && cantB >= 2) {
        g_partidaIniciada = true;
        cout << "\n==========================================" << endl;
        cout << "  [SERVIDOR] CONDICION DE INICIO CUMPLIDA!" << endl;
        cout << "  Iniciando partida multiplayer..." << endl;
        cout << "==========================================" << endl;

        registrarLog("SISTEMA", "TODOS", "INICIO_PARTIDA", "Tablero=" + to_string(g_metaTablero) + ";Dado=" + to_string(g_carasDado));
        broadcast("INICIO|Comienza la partida! Meta: " + to_string(g_metaTablero) + " casillas.");
        
        g_indiceTurno = 0;
        notificarTurnoYTablero();
    } else {
        string estadoEspera = "Esperando jugadores para iniciar... EquipoA: " + to_string(cantA) + "/2 | EquipoB: " + to_string(cantB) + "/2";
        cout << "[SERVIDOR] " << estadoEspera << endl;
        broadcast("INFO|" + estadoEspera);
    }
}

// ─── Hilo de atención por cliente ────────────────────────
static void atenderCliente(SOCKET sockCliente) {
    shared_ptr<Jugador> jugadorActual = nullptr;
    char buffer[1024];

    while (true) {
        int bytesRecv = recv(sockCliente, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecv <= 0) {
            // Desconexión detectada
            break;
        }

        buffer[bytesRecv] = '\0';
        stringstream ss(buffer);
        string linea;

        while (getline(ss, linea)) {
            while (!linea.empty() && (linea.back() == '\r' || linea.back() == '\n')) {
                linea.pop_back();
            }
            if (linea.empty()) continue;

            // Procesar comando del cliente
            stringstream ssCmd(linea);
            string comando;
            getline(ssCmd, comando, '|');

            if (comando == Protocolo::CMD_CONECTAR) {
                string nombre, equipoDeseado;
                getline(ssCmd, nombre, '|');
                getline(ssCmd, equipoDeseado, '|');

                if (nombre.empty()) nombre = "Jugador_" + to_string(sockCliente);

                lock_guard<mutex> lock(g_mutexJuego);

                // Validar si el nombre ya existe
                bool existe = false;
                for (const auto& j : g_jugadores) {
                    if (j->activo && j->nombre == nombre) {
                        existe = true;
                        break;
                    }
                }
                if (existe) {
                    enviarMensaje(sockCliente, "ERROR|El nombre '" + nombre + "' ya esta en uso. Elige otro.");
                    continue;
                }

                // Asignar equipo
                int conteoA = 0, conteoB = 0;
                for (const auto& j : g_jugadores) {
                    if (j->activo) {
                        if (j->equipo == "EquipoA") conteoA++;
                        else conteoB++;
                    }
                }

                string equipoAsignado = equipoDeseado;
                if (equipoAsignado != "EquipoA" && equipoAsignado != "EquipoB") {
                    equipoAsignado = (conteoA <= conteoB) ? "EquipoA" : "EquipoB";
                } else if (equipoAsignado == "EquipoA" && conteoA >= 4) {
                    equipoAsignado = "EquipoB";
                } else if (equipoAsignado == "EquipoB" && conteoB >= 4) {
                    equipoAsignado = "EquipoA";
                }

                jugadorActual = make_shared<Jugador>();
                jugadorActual->socket = sockCliente;
                jugadorActual->nombre = nombre;
                jugadorActual->equipo = equipoAsignado;
                jugadorActual->activo = true;

                g_jugadores.push_back(jugadorActual);

                registrarLog(nombre, equipoAsignado, "CONECTA");
                registrarLog(nombre, equipoAsignado, "ASIGNA_EQUIPO", equipoAsignado);

                enviarMensaje(sockCliente, "ASIGNADO|" + nombre + "|" + equipoAsignado);
                broadcast("INFO|El jugador '" + nombre + "' se unio a " + equipoAsignado + ".");

                verificarCondicionInicio();

            } else if (comando == Protocolo::CMD_TIRAR) {
                lock_guard<mutex> lock(g_mutexJuego);

                if (!g_partidaIniciada) {
                    enviarMensaje(sockCliente, "ERROR|La partida aun no ha comenzado. Esperando jugadores.");
                    continue;
                }
                if (g_partidaFinalizada) {
                    enviarMensaje(sockCliente, "ERROR|La partida ya ha finalizado.");
                    continue;
                }

                auto jTurno = obtenerTurnoActual();
                if (!jTurno || jTurno->socket != sockCliente) {
                    enviarMensaje(sockCliente, "ERROR|No es tu turno de lanzar el dado.");
                    continue;
                }

                // Tirar dado de R caras
                uniform_int_distribution<int> dist(1, g_carasDado);
                int dado = dist(g_rng);

                registrarLog(jTurno->nombre, jTurno->equipo, "LANZA_DADO", to_string(dado));

                int posAnterior = 0;
                int posNueva = 0;

                if (jTurno->equipo == "EquipoA") {
                    posAnterior = g_posEquipoA;
                    g_posEquipoA += dado;
                    posNueva = g_posEquipoA;
                } else {
                    posAnterior = g_posEquipoB;
                    g_posEquipoB += dado;
                    posNueva = g_posEquipoB;
                }

                registrarLog(jTurno->nombre, jTurno->equipo, "AVANZA", to_string(posAnterior) + ";" + to_string(posNueva));

                // Notificar jugada a todos
                broadcast("JUGADA|" + jTurno->nombre + "|" + jTurno->equipo + "|" + to_string(dado) + "|" + to_string(posAnterior) + "|" + to_string(posNueva));

                // Verificar término
                if (posNueva >= g_metaTablero) {
                    g_partidaFinalizada = true;
                    registrarLog(jTurno->nombre, jTurno->equipo, "GANA", "Posicion=" + to_string(posNueva));
                    registrarLog("SISTEMA", "TODOS", "FIN_PARTIDA", "Ganador=" + jTurno->equipo);

                    broadcast("FIN|" + jTurno->equipo + "|" + jTurno->equipo + " alcanzo la casilla " + to_string(posNueva) + " y GANA LA CARRERA!");
                } else {
                    // Pasar al siguiente turno
                    obtenerSiguienteTurno();
                    notificarTurnoYTablero();
                }

            } else if (comando == Protocolo::CMD_SALIR) {
                break;
            }
        }
    }

    // Manejo de desconexión del cliente
    {
        lock_guard<mutex> lock(g_mutexJuego);
        if (jugadorActual && jugadorActual->activo) {
            jugadorActual->activo = false;
            registrarLog(jugadorActual->nombre, jugadorActual->equipo, "DESCONECTA");
            broadcast("INFO|El jugador '" + jugadorActual->nombre + "' se ha desconectado.");

            // Si estaba en turno, avanzar al siguiente
            if (g_partidaIniciada && !g_partidaFinalizada) {
                notificarTurnoYTablero();
            }
        }
    }

    closesocket(sockCliente);
}

// ─── Main del Servidor ───────────────────────────────────
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    cout << "==========================================" << endl;
    cout << "  SERVIDOR DE JUEGO MULTIPLAYER (SOCKETS)" << endl;
    cout << "==========================================" << endl;

    // 1. Leer variables de entorno
    g_archivoLog = obtenerVariableEnvOConfig("GAME_LOG", "juego.log");
    
    int puerto = DEFAULT_GAME_PORT;
    validarVariableEntero("GAME_PORT", puerto, 1024, 65535);

    int metaX = 50;
    validarVariableEntero("TABLERO_X", metaX, 10, 1000);
    g_metaTablero = metaX;

    int carasR = 6;
    validarVariableEntero("DADO_R", carasR, 2, 100);
    g_carasDado = carasR;

    cout << "[CONFIG] Archivo de Log: " << g_archivoLog << endl;
    cout << "[CONFIG] Puerto TCP    : " << puerto << endl;
    cout << "[CONFIG] Meta Tablero  : " << g_metaTablero << " casillas" << endl;
    cout << "[CONFIG] Caras del Dado: " << g_carasDado << endl;
    cout << "------------------------------------------" << endl;

    // 2. Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "[ERROR] Fallo al inicializar Winsock." << endl;
        return 1;
    }

    // 3. Crear socket de escucha
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "[ERROR] No se pudo crear socket servidor." << endl;
        WSACleanup();
        return 1;
    }

    // Reutilización de dirección
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(puerto));

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[ERROR] Fallo al enlazar socket al puerto " << puerto << "." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "[ERROR] Fallo en listen." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "[INFO] Servidor activo escuchando en puerto " << puerto << "..." << endl;
    cout << "[INFO] Esperando conexiones de clientes. Presione Ctrl+C para finalizar.\n" << endl;

    // 4. Bucle principal de aceptación de clientes
    vector<thread> hilosClientes;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            cout << "[AVISO] Error al aceptar cliente entrante." << endl;
            continue;
        }

        char ipCliente[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipCliente, INET_ADDRSTRLEN);
        cout << "[CONEXION] Nuevo cliente conectado desde " << ipCliente << ":" << ntohs(clientAddr.sin_port) << endl;

        // Lanzar hilo independiente por cliente
        hilosClientes.emplace_back(atenderCliente, clientSocket);
    }

    // Cierre controlado de recursos
    closesocket(serverSocket);
    for (auto& t : hilosClientes) {
        if (t.joinable()) t.join();
    }
    WSACleanup();

    return 0;
}
