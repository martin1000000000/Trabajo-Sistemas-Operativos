# ─── Makefile SistOpe (Entrega 3 - Multiplataforma Windows / Linux WSL) ───
CXX      = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -pthread

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    EXT     = .exe
else
    LDFLAGS =
    EXT     =
endif

# 1. Programa Principal SistOpe (con indice invertido paralelo)
SRC_MAIN    = src/main.cpp src/utilidades.cpp src/usuarios.cpp src/perfiles.cpp src/indice.cpp
TARGET_MAIN = SistOpe$(EXT)

# 2. Subproceso Multiplicador de Matrices
SRC_MULTI    = src/multiplicador.cpp
TARGET_MULTI = multiplicador$(EXT)

# 3. Servidor de Juego Multiplayer (Sockets)
SRC_SERVER    = src/servidor_juego.cpp src/utilidades.cpp
TARGET_SERVER = servidor_juego$(EXT)

# 4. Cliente de Juego Multiplayer (Sockets)
SRC_CLIENT    = src/cliente_juego.cpp src/utilidades.cpp
TARGET_CLIENT = cliente_juego$(EXT)

# Compilar todo
all: $(TARGET_MAIN) $(TARGET_MULTI) $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) $(SRC_MAIN) -o $(TARGET_MAIN) $(LDFLAGS)

$(TARGET_MULTI): $(SRC_MULTI)
	$(CXX) $(CXXFLAGS) $(SRC_MULTI) -o $(TARGET_MULTI)

$(TARGET_SERVER): $(SRC_SERVER)
	$(CXX) $(CXXFLAGS) $(SRC_SERVER) -o $(TARGET_SERVER) $(LDFLAGS)

$(TARGET_CLIENT): $(SRC_CLIENT)
	$(CXX) $(CXXFLAGS) $(SRC_CLIENT) -o $(TARGET_CLIENT) $(LDFLAGS)

# Compilar y ejecutar programa principal
run: $(TARGET_MAIN)
	./$(TARGET_MAIN) -u PorDefecto -p 123 -f A.TXT

# Limpiar ejecutables
ifeq ($(OS),Windows_NT)
clean:
	-cmd /c "del /Q /F SistOpe SistOpe.exe multiplicador multiplicador.exe servidor_juego servidor_juego.exe cliente_juego cliente_juego.exe 2>nul"
else
clean:
	-rm -f SistOpe SistOpe.exe multiplicador multiplicador.exe servidor_juego servidor_juego.exe cliente_juego cliente_juego.exe
endif

.PHONY: all run clean
