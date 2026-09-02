#include <stdio.h> // Maneja operaciones basicas de entrada y salida de informacion
#include <stdlib.h> // Gestiona la memoria
#include <stdint.h> // Estandariza los enteros sin importar el CPU del computador
#include <windows.h> // Permite funciones de la api de windows
#include <io.h> // Sirve de puente entre el lenguaje y windows

int main() {
    uint64_t filas = 100000;
    uint64_t columnas = 100000;
    uint32_t bytes_por_elemento = sizeof(float); 

    // Generación archivo de metadata
    FILE *meta = fopen("matriz.meta", "w"); //creación archivo .meta
    if (!meta) {
        printf("Error al crear archivo de metadata.\n");
        return 1;
    }
    // Limite de cortes de bytes por fila y columna (400.000)
    uint64_t limite_corta_fila_bytes = columnas * bytes_por_elemento; 
    fprintf(meta, "FILAS=%llu\nCOLUMNAS=%llu\nBYTES_ELEMENTO=%u\nCORTE_FILA_BYTES=%llu\n", // Escribe el metadata en texto dentro del archivo .meta
            (unsigned long long)filas, 
            (unsigned long long)columnas, 
            bytes_por_elemento, 
            (unsigned long long)limite_corta_fila_bytes);
    fclose(meta);
    printf("[1/2] Archivo 'matriz.meta' generado con exito.\n");

    // Creacion archivo disperso
    const char *nombre_binario = "matriz_gigante.bin"; // Creación del archivo en el disco
    FILE *binario = fopen(nombre_binario, "wb+"); // Se especifica que los datos no se traduciran a texto, manteniendo la cantidad de bytes exactos de los float
    if (!binario) {
        printf("Error al crear el binario.\n");
        return 1;
    }

    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(binario));
    DWORD bytesReturned;
    DeviceIoControl(hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &bytesReturned, NULL); // Se activa el archivo disperso al permitir la asigancion de espacio vitual logico en lugar de fisico

    // Definir límite de 37.2 GB
    uint64_t tamano_total_bytes = filas * limite_corta_fila_bytes;
    _fseeki64(binario, tamano_total_bytes - bytes_por_elemento, SEEK_SET); // Se dirige al ultimo byte de la matriz
    float val_final = 999.99f;
    fwrite(&val_final, bytes_por_elemento, 1, binario); // Escribe el valor de 999.99 para obligar a windows a que expanda el espacio virtualmente hasta el tamaño total del archivo (37.2 GB) sin escribir gigabytes intermedios

    // Escribe datos de prueba y los rellena entre las posiciones [0][0] a [9][9] de la matriz
    for (uint64_t i = 0; i < 10; ++i) {
        for (uint64_t j = 0; j < 10; ++j) {
            uint64_t pos = (i * limite_corta_fila_bytes) + (j * bytes_por_elemento); // Determina la posición exacta del byte dentro del disco donde debe estar [i][j]
            _fseeki64(binario, pos, SEEK_SET);
            float v = (float)(i + j + 1);
            fwrite(&v, bytes_por_elemento, 1, binario); // Escribe fisicamente en disco los 4 bytes del float
        }
    }
    fflush(binario);
    fclose(binario); // Termina la generación del archivo binario
    printf("[2/2] Archivo binario 'matriz_gigante.bin' listo (100.000 x 100.000).\n\n"); 
    FILE *meta_read = fopen("matriz.meta", "r"); // Abre el archivo .meta en modo lectura
    uint64_t f_meta, c_meta, corte_bytes; // Declara las variables donde se cargaran los valores del metadata
    uint32_t b_elem;
    fscanf(meta_read, "FILAS=%llu\nCOLUMNAS=%llu\nBYTES_ELEMENTO=%u\nCORTE_FILA_BYTES=%llu\n", //extrae el texto del metadata y se lo asigna a las variables
           &f_meta, &c_meta, &b_elem, &corte_bytes);
    fclose(meta_read);

    // Menu interactivo
    int opcion = 0;
    while (opcion != 3) {
        printf("\n=========================================\n");
        printf("      Menu matriz      \n");
        printf("=========================================\n");
        // Hacemos uso de un contador que comprueba que en cualquier fila o columna hay 100.000 elementos
        printf("1. Contar/Verificar elementos de una FILA\n");
        printf("2. Contar/Verificar elementos de una COLUMNA\n");
        printf("3. Salir\n");
        printf("Selecciona una opcion: ");
        scanf("%d", &opcion);

        if (opcion == 1) {
            uint64_t fila_sel;
            printf("\nIngresa el numero de fila (0 a %llu): ", f_meta - 1);
            scanf("%llu", &fila_sel);

            if (fila_sel >= f_meta) {
                printf("Fila fuera de rango.\n");
                continue;
            }

            FILE *f_bin = fopen(nombre_binario, "rb");
            uint64_t offset_inicial = fila_sel * corte_bytes;
            _fseeki64(f_bin, offset_inicial, SEEK_SET);

            uint64_t contador = 0;
            float temp_val;
            
            // Recorrido de los 100.000 datos
            for (uint64_t j = 0; j < c_meta; ++j) {
                if (fread(&temp_val, b_elem, 1, f_bin) == 1) {
                    contador++;
                }
            }
            fclose(f_bin);

            printf("\n[RESULTADO FILA %llu]:\n", fila_sel);
            printf(" -> Posiciones leidas exitosamente: %llu de %llu\n", contador, c_meta);
            printf(" -> Rango en bytes procesado: %llu al %llu\n", offset_inicial, offset_inicial + corte_bytes - 1);

        } else if (opcion == 2) {
            uint64_t col_sel;
            printf("\nIngresa el numero de columna (0 a %llu): ", c_meta - 1);
            scanf("%llu", &col_sel);

            if (col_sel >= c_meta) {
                printf("Columna fuera de rango.\n");
                continue;
            }

            FILE *f_bin = fopen(nombre_binario, "rb");
            uint64_t contador = 0;
            float temp_val;

            // Recorrido saltando fila por fila con el offset de corte
            for (uint64_t i = 0; i < f_meta; ++i) {
                uint64_t offset_pos = (i * corte_bytes) + (col_sel * b_elem);
                _fseeki64(f_bin, offset_pos, SEEK_SET);
                if (fread(&temp_val, b_elem, 1, f_bin) == 1) {
                    contador++;
                }
            }
            fclose(f_bin);

            printf("\n[RESULTADO COLUMNA %llu]:\n", col_sel);
            printf(" -> Posiciones leidas exitosamente: %llu de %llu\n", contador, f_meta);
            printf(" -> Demuestra que la matriz tiene %llu filas verdaderas en esta columna.\n", f_meta);

        } else if (opcion != 3) {
            printf("Opcion invalida.\n");
        }
    }

    printf("\nPrograma finalizado.\n");
    return 0;
}