#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <io.h>

int main() {
    uint64_t filas = 100000;
    uint64_t columnas = 100000;
    uint32_t bytes_por_elemento = sizeof(float);

    // 1. GENERAR ARCHIVO DE METADATA (.meta)
    FILE *meta = fopen("matriz.meta", "w");
    if (!meta) {
        printf("Error al crear archivo de metadata.\n");
        return 1;
    }
    
    uint64_t limite_corta_fila_bytes = columnas * bytes_por_elemento; 
    fprintf(meta, "FILAS=%llu\nCOLUMNAS=%llu\nBYTES_ELEMENTO=%u\nCORTE_FILA_BYTES=%llu\n",
            (unsigned long long)filas, 
            (unsigned long long)columnas, 
            bytes_por_elemento, 
            (unsigned long long)limite_corta_fila_bytes);
    fclose(meta);
    printf("[1/2] Archivo 'matriz.meta' generado con exito.\n");

    // 2. CREAR EL ARCHIVO BINARIO DISPERSO
    const char *nombre_binario = "matriz_gigante.bin";
    FILE *binario = fopen(nombre_binario, "wb+");
    if (!binario) {
        printf("Error al crear el binario.\n");
        return 1;
    }

    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(binario));
    DWORD bytesReturned;
    DeviceIoControl(hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &bytesReturned, NULL);

    // Definir límite de 37.2 GB
    uint64_t tamano_total_bytes = filas * limite_corta_fila_bytes;
    _fseeki64(binario, tamano_total_bytes - bytes_por_elemento, SEEK_SET);
    float val_final = 999.99f;
    fwrite(&val_final, bytes_por_elemento, 1, binario);

    // Escribir datos de prueba en [0][0] a [9][9]
    for (uint64_t i = 0; i < 10; ++i) {
        for (uint64_t j = 0; j < 10; ++j) {
            uint64_t pos = (i * limite_corta_fila_bytes) + (j * bytes_por_elemento);
            _fseeki64(binario, pos, SEEK_SET);
            float v = (float)(i + j + 1);
            fwrite(&v, bytes_por_elemento, 1, binario);
        }
    }
    fflush(binario);
    fclose(binario);
    printf("[2/2] Archivo binario 'matriz_gigante.bin' listo (100.000 x 100.000).\n\n");

    // LECTURA DE METADATA
    FILE *meta_read = fopen("matriz.meta", "r");
    uint64_t f_meta, c_meta, corte_bytes;
    uint32_t b_elem;
    fscanf(meta_read, "FILAS=%llu\nCOLUMNAS=%llu\nBYTES_ELEMENTO=%u\nCORTE_FILA_BYTES=%llu\n",
           &f_meta, &c_meta, &b_elem, &corte_bytes);
    fclose(meta_read);

    // 3. MENÚ INTERACTIVO
    int opcion = 0;
    while (opcion != 3) {
        printf("\n=========================================\n");
        printf("      DEMOSTRADOR DE MATRIZ 37.2 GB      \n");
        printf("=========================================\n");
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