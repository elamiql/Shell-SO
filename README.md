# MyShell

<div style = "text-align: justify;">

MyShell es una shell básica implementada en C que ofrece funcionalidades esenciales para la ejecución de comandos y manejo de procesos en sistemas Unix. Esta shell incluye soporte avanzado para pipes, historial interactivo, comando miprof para analisis y una interfaz de usuario mejorada con navegación por teclas.

</div>

## Caracteristicas
### Interfaz del usuario

- Prompt personalizado con colores e informacion del sistema (usuario@hostname:directorio$ SCHELL )
- Navegacion con flechas para acceder a historial de comandos (arriba/abajo)
- Modo raw para captura de teclas especiales 

### Sistema de historial

- Almacenamiento persistente de hasta 100 comandos ejecutados
- Navegacion con las teclas arriba / abajo
- Comando `print_history` para visualizar todo el historial

### Pipes y Comandos compuestos

- Soporte para pipes `|` entre multiples comandos
- Parsing que maneja caracteres especiales y espacios
- Ejecucion concurrente de procesos conectados por pipes

### Comandos internos

`cd` - Cambio de directorio 
- Navegar entre directorios del sistema
- Manejo de errores para directorios inexistentes

`miprof`- Herramienta para analisis y monitoreo de procesos
- `miprof ejec comando`: Ejecuta el comando y muestra el codigo de salida
- `miprof ejecsave archivo comando`: Ejecuta el comando y guarda la salida en un archivo
- `miprof ejecutar tiempo comando`: Ejecuta comando con limite de tiempo
    - Medicion del tiempo real, tiempo de usuario y tiempo de sistema
    - Monitoreo de memoria maxima utilizada
    - Terminacion automatica si excede el tiempo limite
    - Reporte detallado de recursos utilizados

### Funcionalidades del Sistema
- Ejecucion de comandos internos con `execvp()`
- Manejo de procesos con `fork()` y `wait()`
- Gestion de memoria automatica con liberacion de recursos
- Manejo de señales para control de procesos (SIGALARM, SIGKILL)
- Soporte para comillas en argumentos de comandos

### Comandos especiales
- `exit`: Salida limpia de la shell
- `print_history`: Muestra el historial completo de comandos

## Requisitos del Sistema 
- Compilador C
- Sistema operativo basado en Unix/Linux (Linux, macOS, etc)

## Instrucciones de Compilación
Para compilar el código en un sistema operativo Unix, siga las siguientes instrucciones:

1. Clone el repositorio:

    ```bash
    git clone https://github.com/elamiql/Shell-SO.git
    ```

2. Ingrese a la carpeta del repositorio con:
    ```bash
    cd Shell-SO
    ```

3. Ejecute el siguiente comando:
    ```bash
    make
    ```


## Ejecución

Para iniciar la shell ejecuta:
```bash
./myshell
```

## Ejemplos de uso

### Comandos basicos

```bash
SCHELL: ls -la
SCHELL: pwd
```

### Pipes
```bash
SCHELL: ls -la | grep .txt | wc -l
SCHELL: cat archivo.txt | sort | uniq
SCHELL: ps aux | grep python | head -5
```

### miprof
```bash
SCHELL: miprof ejec ls -la
SCHELL: miprof ejecsave resultado.txt find . -name "*.c"
SCHELL: miprof ejecutar 10 sleep 5
```

### Comando adicional para limpiar los archivos residuales:
  ```bash
  make clean
  ```

### Herramientas utilizadas

<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/1/19/C_Logo.png" width="80">
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://upload.wikimedia.org/wikipedia/commons/3/35/Tux.svg" height="80">
</p>