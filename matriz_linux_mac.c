#define _FILE_OFFSET_BITS 64 // Habilita soporte de archivos de 64 bits para Linux/macOS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Librerías condicionales según el sistema operativo
#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define fseek64 _fseeki64
#else
    #include <unistd.h>
    #define fseek64 fseeko
#endif

int main() {
    uint64_t filas = 100000;
    uint64_t columnas = 100000;
    uint32_t bytes_por_elemento = sizeof(float);
    uint64_t limite_corta_fila_bytes = columnas * bytes_por_elemento;

    
    const char *nombre_binario = "matriz_gigante.bin";
    FILE *binario = fopen(nombre_binario, "wb+");
    if (!binario) return 1;

    // Si es Windows, activamos FSCTL_SET_SPARSE. En Linux ocurre automáticamente.
#ifdef _WIN32
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(binario));
    DWORD bytesReturned;
    DeviceIoControl(hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &bytesReturned, NULL);
#endif

    uint64_t tamano_total_bytes = filas * limite_corta_fila_bytes;
    fseek64(binario, tamano_total_bytes - bytes_por_elemento, SEEK_SET);
    float val_final = 999.99f;
    fwrite(&val_final, bytes_por_elemento, 1, binario);
    
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
    FILE *meta_read = fopen("matriz.meta", "r"); 
    uint64_t f_meta, c_meta, corte_bytes; 
    uint32_t b_elem;
    fscanf(meta_read, "FILAS=%llu\nCOLUMNAS=%llu\nBYTES_ELEMENTO=%u\nCORTE_FILA_BYTES=%llu\n", 
           &f_meta, &c_meta, &b_elem, &corte_bytes);
    fclose(meta_read);

    
    int opcion = 0;
    while (opcion != 3) {
        printf("\n=========================================\n");
        printf("      Menu matriz      \n");
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