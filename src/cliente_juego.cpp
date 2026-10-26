#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

#include "protocolo_juego.h"
#include "utilidades.h"

using namespace std;

static atomic<bool> g_conectado(true);
static atomic<bool> g_esMiTurno(false);
static string g_miNombre = "";
static string g_miEquipo = "";

// ─── Dibujar barra de progreso del tablero ───────────────
static void dibujarTablero(int posA, int posB, int meta) {
    if (meta <= 0) meta = 50;
    int largoBarra = 30;

    int progresoA = min(largoBarra, (posA * largoBarra) / meta);
    int progresoB = min(largoBarra, (posB * largoBarra) / meta);

    string barraA(progresoA, '=');
    if (progresoA < largoBarra) barraA += '>';
    while (barraA.length() < static_cast<size_t>(largoBarra)) barraA += ' ';

    string barraB(progresoB, '=');
    if (progresoB < largoBarra) barraB += '>';
    while (barraB.length() < static_cast<size_t>(largoBarra)) barraB += ' ';

    cout << "\n--------------------------------------------------" << endl;
    cout << "  ESTADO DE LA CARRERA (Meta: " << meta << " casillas)" << endl;
    cout << "  Equipo A: [" << barraA << "] (" << posA << "/" << meta << ")" << endl;
    cout << "  Equipo B: [" << barraB << "] (" << posB << "/" << meta << ")" << endl;
    cout << "--------------------------------------------------" << endl;
}

// ─── Hilo receptor de mensajes del servidor ──────────────
static void hiloReceptor(SOCKET sock) {
    char buffer[1024];

    while (g_conectado) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            cout << "\n[INFO] Desconectado del servidor de juego." << endl;
            g_conectado = false;
            break;
        }

        buffer[bytes] = '\0';
        stringstream ss(buffer);
        string linea;

        while (getline(ss, linea)) {
            while (!linea.empty() && (linea.back() == '\r' || linea.back() == '\n')) {
                linea.pop_back();
            }
            if (linea.empty()) continue;

            stringstream ssMsg(linea);
            string tipo;
            getline(ssMsg, tipo, '|');

            if (tipo == Protocolo::RESP_INFO) {
                string msg;
                getline(ssMsg, msg);
                cout << "[SERVIDOR] " << msg << endl;

            } else if (tipo == Protocolo::RESP_ASIGNADO) {
                getline(ssMsg, g_miNombre, '|');
                getline(ssMsg, g_miEquipo, '|');
                cout << "\n>>> REGISTRO EXITOSO! <<<" << endl;
                cout << "Jugador: " << g_miNombre << " | Asignado a: " << g_miEquipo << endl;
                cout << "Esperando a que se completen los equipos para iniciar la partida...\n" << endl;

            } else if (tipo == Protocolo::RESP_INICIO) {
                string msg;
                getline(ssMsg, msg);
                cout << "\n==========================================" << endl;
                cout << "  " << msg << endl;
                cout << "==========================================" << endl;

            } else if (tipo == Protocolo::RESP_TABLERO) {
                string strA, strB, strMeta;
                getline(ssMsg, strA, '|');
                getline(ssMsg, strB, '|');
                getline(ssMsg, strMeta, '|');

                int posA = stoi(strA);
                int posB = stoi(strB);
                int meta = stoi(strMeta);

                dibujarTablero(posA, posB, meta);

            } else if (tipo == Protocolo::RESP_JUGADA) {
                string nombre, equipo, dadoStr, posAntStr, posNuevaStr;
                getline(ssMsg, nombre, '|');
                getline(ssMsg, equipo, '|');
                getline(ssMsg, dadoStr, '|');
                getline(ssMsg, posAntStr, '|');
                getline(ssMsg, posNuevaStr, '|');

                cout << "\n[JUGADA] " << nombre << " (" << equipo << ") lanzo el dado y obtuvo: " << dadoStr << "!" << endl;
                cout << "         " << equipo << " avanzo de casilla " << posAntStr << " a " << posNuevaStr << "." << endl;

            } else if (tipo == Protocolo::RESP_TURNO) {
                string nombreTurno, equipoTurno;
                getline(ssMsg, nombreTurno, '|');
                getline(ssMsg, equipoTurno, '|');

                if (nombreTurno == g_miNombre) {
                    g_esMiTurno = true;
                    cout << "\n******************************************" << endl;
                    cout << "  >>> ES TU TURNO DE LANZAR EL DADO! <<<" << endl;
                    cout << "  Presione [ENTER] para tirar el dado..." << endl;
                    cout << "******************************************" << endl;
                } else {
                    g_esMiTurno = false;
                    cout << "[TURNO] Es el turno de " << nombreTurno << " (" << equipoTurno << "). Esperando su tiro..." << endl;
                }

            } else if (tipo == Protocolo::RESP_FIN) {
                string ganador, mensaje;
                getline(ssMsg, ganador, '|');
                getline(ssMsg, mensaje, '|');

                cout << "\n==========================================" << endl;
                cout << "  PARTIDA FINALIZADA" << endl;
                cout << "  " << mensaje << endl;
                cout << "==========================================" << endl;
                cout << "Presione [ENTER] para salir..." << endl;
                g_esMiTurno = false;

            } else if (tipo == Protocolo::RESP_ERROR) {
                string errorMsg;
                getline(ssMsg, errorMsg);
                cout << "[ERROR SERVIDOR] " << errorMsg << endl;
            }
        }
    }
}

// ─── Main del Cliente ────────────────────────────────────
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    cout << "==========================================" << endl;
    cout << "   CLIENTE DE JUEGO MULTIPLAYER (SOCKETS)" << endl;
    cout << "==========================================" << endl;

    // 1. Configuración y validación de Host y Puerto desde variables de entorno
    string host = obtenerVariableEnvOConfig("GAME_HOST");
    int puerto = 0;
    if ((argc < 4 && host.empty()) || (argc < 5 && !validarVariableEntero("GAME_PORT", puerto, 1024, 65535))) {
        cout << "[ERROR] Las variables de entorno GAME_HOST o GAME_PORT no estan definidas o son invalidas." << endl;
        return 1;
    }

    string nombreJugador = "";
    string equipoElegido = "";

    // Parsear parámetros CLI si fueron provistos:
    // cliente_juego.exe [nombre] [equipo] [host] [puerto]
    if (argc >= 2) nombreJugador = argv[1];
    if (argc >= 3) equipoElegido = argv[2];
    if (argc >= 4) host = argv[3];
    if (argc >= 5) puerto = stoi(argv[4]);

    // Solicitar de forma interactiva si falta nombre
    if (nombreJugador.empty()) {
        cout << "Ingrese su nombre de jugador: ";
        getline(cin, nombreJugador);
    }

    if (equipoElegido.empty()) {
        cout << "Seleccione equipo:" << endl;
        cout << "  1) Equipo A" << endl;
        cout << "  2) Equipo B" << endl;
        cout << "  3) Asignacion automatica" << endl;
        int opt = leerEntero("Opcion (1-3): ");
        if (opt == 1) equipoElegido = "EquipoA";
        else if (opt == 2) equipoElegido = "EquipoB";
        else equipoElegido = "AUTO";
    }

#ifdef _WIN32
    // 2. Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "[ERROR] Fallo al inicializar Winsock." << endl;
        return 1;
    }
#endif

    // 3. Crear Socket y Conectar
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cout << "[ERROR] No se pudo crear socket." << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(puerto));
    inet_pton(AF_INET, host.c_str(), &(serverAddr.sin_addr));

    cout << "\n[INFO] Conectando a " << host << ":" << puerto << "..." << endl;
    if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[ERROR] No se pudo conectar al servidor en " << host << ":" << puerto << "." << endl;
        cout << "[TIP] Asegurese de iniciar primero 'servidor_juego'." << endl;
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    cout << "[INFO] Conectado exitosamente con el servidor." << endl;

    // Enviar registro inicial
    string msgRegistro = Protocolo::CMD_CONECTAR + "|" + nombreJugador + "|" + equipoElegido + "\n";
    send(sock, msgRegistro.c_str(), static_cast<int>(msgRegistro.length()), 0);

    // 4. Iniciar hilo receptor
    thread receptor(hiloReceptor, sock);

    // 5. Bucle de entrada del usuario en consola
    while (g_conectado) {
        string entrada;
        if (!getline(cin, entrada)) break;

        if (!g_conectado) break;

        if (entrada == "salir" || entrada == "exit") {
            string msg = Protocolo::CMD_SALIR + "\n";
            send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
            g_conectado = false;
            break;
        }

        // Si es el turno y el usuario presiona Enter (o escribe cualquier cosa), enviar tiro de dado
        if (g_esMiTurno) {
            string msg = Protocolo::CMD_TIRAR + "\n";
            send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
            g_esMiTurno = false;
        } else {
            cout << "[AVISO] Aun no es tu turno de lanzar el dado." << endl;
        }
    }

    // 6. Cierre ordenado
    closesocket(sock);
    if (receptor.joinable()) {
        receptor.join();
    }
#ifdef _WIN32
    WSACleanup();
#endif

    cout << "\nGracias por jugar. Hasta luego!" << endl;
    return 0;
}
