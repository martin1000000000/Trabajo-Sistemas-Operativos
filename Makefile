# ─── Makefile SistOpe ───────────────────────────────────────
CXX      = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

SRC_MAIN = src/main.cpp src/utilidades.cpp src/usuarios.cpp src/perfiles.cpp
TARGET_MAIN = SistOpe

SRC_MULTI = src/multiplicador.cpp
TARGET_MULTI = multiplicador

# Compilar
all: $(TARGET_MAIN) $(TARGET_MULTI)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) $(SRC_MAIN) -o $(TARGET_MAIN)

$(TARGET_MULTI): $(SRC_MULTI)
	$(CXX) $(CXXFLAGS) $(SRC_MULTI) -o $(TARGET_MULTI)

# Compilar y ejecutar
run: $(TARGET_MAIN)
	./$(TARGET_MAIN) -u admin -p 123 -f prueba.txt

# Limpiar ejecutable
clean:
	rm -f $(TARGET_MAIN) $(TARGET_MAIN).exe $(TARGET_MULTI) $(TARGET_MULTI).exe

.PHONY: all run clean
