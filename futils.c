#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

// Estructura del EXT2 (Superbloque)
typedef struct {
    uint32_t s_inodes_count;      // Numero total de inodos
    uint32_t s_blocks_count;      // Número total de bloques
    uint32_t s_r_blocks_count;    // Número total de bloques reservados
    uint32_t s_free_blocks_count; // Número total de bloques libres
    uint32_t s_free_inodes_count; // Número total de inodos libres
    uint32_t s_first_data_block;  // Primer bloque de datos
    uint32_t s_log_block_size;    // ID de tamaño de bloque
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;  
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;  
    uint32_t s_mtime;             // Última vez que se ha montado
    uint32_t s_wtime;             // Última vez que se ha modificado
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;             // Número mágico (0xEF53 para el EXT2)
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;         // Última comprobación
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;         // Primer inodo
    uint16_t s_inode_size;        // Tamaño del inodo
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t  s_uuid[16];
    char     s_volume_name[16];   // Nombre del disco
} __attribute__((packed)) Ext2Superblock;

// Estructura del FAT16 (Sector de arranque)
typedef struct {
    uint8_t  ignored[3];
    char     system_name[8];      // Nombre del sistema de archivos
    uint16_t sector_size;         // Tamaño del sector
    uint8_t  sec_per_cluster;     // Número de sectores por clúster
    uint16_t reserved_sectors;    // Número de sectores reservados
    uint8_t  num_fats;            // Número de tablas de FAT
    uint16_t root_entries;        // Número de entradas en el directorio raíz
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;         // Número de sectores por FAT
    uint16_t sec_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t  drive_number;
    uint8_t  reserved;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];    // Nombre del disco
    char     file_system_type[8];
} __attribute__((packed)) Fat16BootSector;

// --- Funciones auxiliares ---
//Esta función pasa el tiempo de formato timestamp a formato DD/MM/YYYY HH:MM:SS
void format_time(uint32_t timestamp) {
    time_t rawtime = (time_t)timestamp;
    struct tm *info = localtime(&rawtime);
    char buffer[80];
    if (info == NULL) {
        printf("Unknown\n");
    } else {
        strftime(buffer, 80, "%a %b %d %H:%M:%S %Y", info);
        printf("%s\n", buffer);
    }
}

// --- Funciones principales ---
// Esta función procesa y lee la información de un sistema de archivos EXT2 y la muestra por pantalla
void process_ext2(int fd) {
    Ext2Superblock ext2;

    //Empezamos por la adreça 1024 ya que es donde está el superbloque del EXT2
    lseek(fd, 1024, SEEK_SET);
    read(fd, &ext2, sizeof(Ext2Superblock));

    printf("\n------ Filesystem Information ------\n\n");
    printf("Filesystem: EXT2\n\n");
    
    //Info del inodo
    printf("INODE INFO\n");
    printf("Size: %u\n", ext2.s_inode_size);
    printf("Num Inodes: %u\n", ext2.s_inodes_count);
    printf("First Inode: %u\n", ext2.s_first_ino);
    printf("Inodes Group: %u\n", ext2.s_inodes_per_group);
    printf("Free Inodes: %u\n", ext2.s_free_inodes_count);
    
    //Info del bloque
    printf("\nINFO BLOCK\n");
    printf("Block size: %u\n", 1024 << ext2.s_log_block_size);
    printf("Reserved blocks: %u\n", ext2.s_r_blocks_count);
    printf("Free blocks: %u\n", ext2.s_free_blocks_count);
    printf("Total blocks: %u\n", ext2.s_blocks_count);
    printf("First block: %u\n", ext2.s_first_data_block);
    printf("Group blocks: %u\n", ext2.s_blocks_per_group);
    printf("Group frags: %u\n", ext2.s_frags_per_group);
    
    //Info del volumen
    printf("\nINFO VOLUME\n");
    printf("Volume name: %.16s\n", ext2.s_volume_name);
    printf("Last Checked: ");  format_time(ext2.s_lastcheck);
    printf("Last Mounted: ");  format_time(ext2.s_mtime);
    printf("Last Written: ");  format_time(ext2.s_wtime);
    printf("\n");
}

// Esta función procesa y lee la información de un sistema de archivos EXT2 y la muestra por pantalla
void process_fat16(int fd) {
    Fat16BootSector fat16;

    //Empezamos desde el principio ya que es donde esta el sector de arranquue del FAT16
    lseek(fd, 0, SEEK_SET);
    read(fd, &fat16, sizeof(Fat16BootSector));

    printf("\n------ Filesystem Information ------\n\n");
    printf("Filesystem: FAT16\n\n");

    printf("System name: %.8s\n", fat16.system_name);
    printf("Sector size: %u\n", fat16.sector_size);
    printf("Sectors per cluster: %u\n", fat16.sec_per_cluster);
    printf("Reserved sectors: %u\n", fat16.reserved_sectors);
    printf("# of FATs: %u\n", fat16.num_fats);
    printf("Max root entries: %u\n", fat16.root_entries);
    printf("Sectors per FAT: %u\n", fat16.fat_size_16);
    printf("Label: %.11s\n", fat16.volume_label);
}

int main(int argc, char* argv[]) {
    //Si tenemos menos de 3 argumentos o el primer argumento no es --info mostramos error
    if (argc != 3 || strcmp(argv[1], "--info") != 0) {
        printf("Usage: ./fsutils --info <file_system>\n");
        return 1;
    }

    int fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror("ERROR: Could not open the file system");
        return 1;
    }

    //Miramos si el sistema de archivos es del tipo EXT2 o FAT16 mirando el número mágico del sistema de archivos
    uint16_t magic;
    lseek(fd, 1024 + 56, SEEK_SET);
    read(fd, &magic, 2);

    //Si el número mágico es 0xEF53 es un sistema EXT2, sino es FAT16
    if (magic == 0xEF53) {
        process_ext2(fd);
    } else {
        process_fat16(fd);
    }

    close(fd);
    return 0;
}