# SAPP (Server Application for PC/CE)

SAPP es una extensión avanzada y ligera para servidores dedicados de **Halo: Custom Edition** y **Halo PC** (`halocded.exe` / `haloded.exe`). Se inyecta en el proceso del servidor para proporcionar herramientas administrativas completas, moderación en tiempo real, soporte de scripting en Lua y mejoras de estabilidad para el juego multijugador.

---

## 🎯 Características Principales

- **Lead Clásico Original (Lead 1):**
  - Mantiene el cálculo balístico, físicas de proyectiles y compensación nativa del motor de Halo CE, sin alteraciones artificiales de No-Lead ni rebobinados de coordenadas de objetos.
- **Motor de Scripting Lua 5.1 / LuaJIT:**
  - Sistema de eventos completo (`EVENT_SPAWN`, `EVENT_DIE`, `EVENT_KILL`, `EVENT_CHAT`, `EVENT_COMMAND`, `EVENT_DAMAGE_APPLICATION`, etc.).
  - Funciones nativas para lectura y manipulación segura de memoria del juego.
- **Administración y Comandos:**
  - Sistema jerárquico de niveles de administrador.
  - Comandos accesibles desde la consola local, el chat del juego (`/comando`), RCON y scripts.
- **Seguridad y Moderación:**
  - Gestión de baneos por IP y hashes de CD Key.
  - Detección heurística de anomalías (Aimbot Scan, Anti-glitch, Anti-lag spawn).
  - Protección contra ataques de saturación en consultas GameSpy (`GSQueryPatch`).
- **Control de Partidas y Mapas:**
  - Sistema de votación de mapas integrado (`mapvote`).
  - Rotación avanzada de mapas y modos de juego (`mapcycle`).
  - Balanceo automático de equipos y estadísticas de jugadores.

---

## 🛠️ Compilación

### Opción 1: Compilación Automática (GitHub Actions)
Este repositorio incluye un flujo de integración continua en [`.github/workflows/build.yml`](.github/workflows/build.yml). Cada vez que realizas un `push` a las ramas `main` o `master` (o activas el flujo manualmente desde la pestaña **Actions** en GitHub):
1. Se compila LuaJIT (`3pp/LuaJIT-2.1.0-beta3`).
2. Se compilan las dependencias `7z` y `SappPacker`.
3. Se compila `sapp.dll` en modo `Release|Win32`.
4. El binario empaquetado resultante queda disponible para descargar en los **Artifacts** de la ejecución.

### Opción 2: Compilación Local (Visual Studio 2015, 2019 o 2022)
1. Abre la solución [`Sapp.sln`](Sapp.sln) en Visual Studio.
2. Asegúrate de seleccionar la configuración **Release** y la plataforma **Win32 (x86)**.
3. Compila primero la dependencia de LuaJIT abriendo una consola de desarrollo de Visual Studio (x86 Native Tools Command Prompt):
   ```cmd
   cd 3pp\LuaJIT-2.1.0-beta3\src
   msvcbuild.bat static
   ```
4. Compila el proyecto `SappPacker` y luego el proyecto `Sapp` (o la solución completa).
5. El archivo generado `sapp.dll` se creará en la carpeta `Release\`.

---

## 🚀 Instalación en el Servidor

1. Copia el archivo `sapp.dll` en la carpeta raíz de tu servidor dedicado de Halo Custom Edition (donde se encuentra `halocded.exe`).
2. En tu carpeta de perfiles del servidor (normalmente dentro de la carpeta del juego o en `Documents\My Games\Halo CE`), crea la carpeta `sapp\`:
   ```
   sapp\
   ├── init.txt
   └── ... (scripts .lua opcionales)
   ```
3. Configura tus comandos y ajustes iniciales dentro de `sapp\init.txt`.
4. Inicia el servidor dedicado. Al arrancar, SAPP se cargará automáticamente y mostrará su mensaje de bienvenida en la consola.

---

## 📜 Estructura del Repositorio

- **`Sapp/`**: Código fuente principal de la extensión SAPP (hooks, motor de comandos, interfaces de memoria de Halo).
- **`Common/`**: Funciones compartidas, utilidades y criptografía RC6.
- **`3pp/`**: Bibliotecas de terceros (LuaJIT, 7-Zip SDK, UPX).
- **`SappPacker/`**: Empaquetador y compresor del ejecutable de SAPP.
- **`ServerReboot/`**: Utilidad de monitoreo y reinicio automático del servidor dedicado.
- **`RemoteConsole/` / `NetworkServer/`**: Módulos de comunicación y administración mediante consola remota.

---

## 👥 Créditos

- **Termy**: Creador original de SAPP.
- **seh, oo, Oxide** y la comunidad de Halo Custom Edition por sus aportes, parches y mantenimiento continuo.
