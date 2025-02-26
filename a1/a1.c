#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define VALID_SECTION_TYPES "51 34 71"

int MAGIC = 41;
int MIN = 42;
int MAX = 162;
int MIN_SECT = 2;
int MAX_SECT = 20;

// structuri pentru SF_Header
typedef struct
{
    char nume[16];
    int type;
    int offset;
    int size;
} SF_Header;

void listDir(const char *path, int mode_recursive, int min_size, const char *name_ends_with, int *first)
{
    DIR *director = NULL;
    director = opendir(path);
    if (director == NULL)
    {
        perror("Could not open directory");
        return;
    }
    struct dirent *entry = NULL;
    struct stat st;

    while ((entry = readdir(director)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            stat(fullpath, &st);

            if (S_ISDIR(st.st_mode))
            {
                if ((name_ends_with == NULL || strstr(entry->d_name, name_ends_with) != NULL) &&
                    (min_size == -1 || st.st_size > min_size))
                    printf("%s\n", fullpath);

                if (mode_recursive)
                {
                    listDir(fullpath, mode_recursive, min_size, name_ends_with, first);
                }
            }
            else if (S_ISREG(st.st_mode))
            {
                if ((name_ends_with == NULL || strstr(entry->d_name, name_ends_with) != NULL) &&
                    (min_size == -1 || st.st_size > min_size))
                {
                    if (*first)
                    {
                        printf("%s\n", fullpath);
                        *first = 0;
                    }
                    else
                    {
                        printf("%s\n", fullpath);
                    }
                }
            }
        }
    }

    closedir(director);
}

void parseSF(const char *file_path)
{
    // deschidem fisierul folosind read binary

    int fisier = open(file_path, O_RDONLY);

    if (!fisier)
    {
        perror("Eroare la deschiderea fișierului");
        return;
    }

    unsigned char buff[16];
    read(fisier, &buff, 2);

    if (buff[0] != '4' || buff[1] != '1')
    {
        printf("ERROR\nwrong magic\n");
        close(fisier);
        return;
    }

    lseek(fisier,2,SEEK_CUR);

    int version;
    read(fisier, &buff, 4);
    version = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

    if (version < 42 || version > 162)
    {
        printf("ERROR\nwrong version\n");
        close(fisier);
        return;
    }

    int nr_sectiuni;
    read(fisier, &buff, 1);
    nr_sectiuni = buff[0] * 1;
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", nr_sectiuni);
    for (int i = 1; i <= nr_sectiuni; i++)
    {
        char nume[16];
        read(fisier, &nume, 15);

        int type;
        read(fisier, &buff, 2);
        type = buff[0] * 1 + buff[1] * 256;

        int offset;
        read(fisier, &buff, 4);
        offset = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

        int size;
        read(fisier, &buff, 4);
        size = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

        size = size + 1 - 1;
        offset = offset +1 -1;
        version = version + 1 - 1;

        if (type != 34 && type != 71 && type != 51)
        {
            printf("ERROR\ninvalid section type\n");
            close(fisier);
            return;
        }

        printf("section%d: %s %d %d\n", i, nume, type, size);
    }

    close(fisier);
    return;
}

void extract(const char *path, int section_nr, int line_nr)
{
    int fisier = open(path, O_RDONLY);
    SF_Header struct1[18];

    if (!fisier)
    {
        return;
    }
    
    unsigned char buff[16];
    read(fisier, &buff, 2);

    lseek(fisier,2,SEEK_CUR);

    int version;
    read(fisier, &buff, 4);
    version = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

    version = version + 1 - 1;

    int nr_sectiuni;
    read(fisier, &buff, 1);
    nr_sectiuni = buff[0] * 1;

    for (int i = 1; i <= nr_sectiuni; i++)
    {
        char nume[16];
        read(fisier, &nume, 15);

        int type;
        read(fisier, &buff, 2);
        type = buff[0] * 1 + buff[1] * 256;

        int offset;
        read(fisier, &buff, 4);
        offset = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

        int size;
        read(fisier, &buff, 4);
        size = buff[0] * 1 + buff[1] * 256 + buff[2] * 256 * 256 + buff[3] * 256 * 256 * 256;

        struct1[i].size = size;
        struct1[i].type = type;
        struct1[i].offset = offset;
        strcpy(struct1[i].nume, nume);
    }

    unsigned char buff1=2;
    char continut_linie[1024];
    int current_line = 1;

    int accespoint = struct1[section_nr].offset;
    lseek(fisier, accespoint, SEEK_SET);
    int i=0;

    while (buff1 != 0)
    {
        read(fisier, &buff1, 1);

        if (buff1 == 10)
            current_line++;

        else 
        {
          if(current_line==line_nr)
             {
                continut_linie[i++]=buff1;
             }

        }    
    }

    continut_linie[i]=0;
    printf("SUCCESS\n%s",continut_linie);

    close(fisier);
}

void findall(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }

    struct dirent *entry;
    struct stat statbuf;

    int count_valid_SF = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // Ignoră directorul curent și directorul părinte
        }

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        if (stat(fullpath, &statbuf) == -1)
        {
            perror("Stat error");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) // Verificăm dacă este un director
        {
            findall(fullpath); // Traversăm recursiv subdirectorul
        }
        else if (S_ISREG(statbuf.st_mode)) // Verificăm dacă este un fișier regulat
        {
            int fisier = open(fullpath, O_RDONLY);
            if (fisier == -1)
            {
                perror("Eroare la deschiderea fișierului");
                continue;
            }

            unsigned char buff[16];
            read(fisier, &buff, 2);

            if (buff[0] == '4' && buff[1] == '1') // Verificăm magicul SF
            {
                int nr_sectiuni;
                read(fisier, &buff, 1);
                nr_sectiuni = buff[0] * 1;

                int count_section_34 = 0;

                for (int i = 1; i <= nr_sectiuni; i++)
                {
                    int type;
                    read(fisier, &buff, 2);
                    type = buff[0] * 1 + buff[1] * 256;

                    if (type == 34) // Verificăm tipul secțiunii
                    {
                        count_section_34++;
                    }
                }

                if (count_section_34 >= 4)
                {
                    printf("%s\n", fullpath);
                    count_valid_SF++;
                }
            }

            close(fisier);
        }
    }

    closedir(dir);
}



int main(int argc, char **argv)
{
    if (argc >= 2)
    {

        if (strcmp(argv[1], "variant") == 0)
        {
            printf("36235\n");
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            int min_size = -1;
            char *path;
            char *ends_with = NULL;
            int mode_recursive = 0;
            int first = 0;

            for (int i = 1; i < argc; i++)
            {

                if (strcmp(argv[i], "recursive") == 0)
                {
                    mode_recursive = 1;
                }
                else if (strncmp(argv[i], "size_greater=", 13) == 0)
                {
                    min_size = atoi(argv[i] + 13);
                }
                else if (strncmp(argv[i], "name_ends_with=", 15) == 0)
                {
                    ends_with = argv[i] + 15;
                }

                else if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = argv[i] + 5;
                }
            }

            printf("SUCCESS\n");
            listDir(path, mode_recursive, min_size, ends_with, &first);
        }
        else if (strcmp(argv[1], "parse") == 0)
        {
            const char *path;
            for (int i = 1; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = argv[i] + 5;
                }
            }
            parseSF(path);
        }
        else if (strcmp(argv[1], "extract") == 0)
        {
            const char *path;
            int section = -1;
            int line = -1;

            for (int i = 1; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = argv[i] + 5;
                }
                else if (strncmp(argv[i], "section=", 8) == 0)
                {
                    section = atoi(argv[i] + 8);
                }
                else if (strncmp(argv[i], "line=", 5) == 0)
                {
                    line = atoi(argv[i] + 5);
                }
            }
            extract(path, section, line);
        }
        else if (strcmp(argv[1], "findall") == 0)
        {
            const char *path;
            for (int i = 1; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = argv[i] + 5;
                }
            }
            printf("SUCCESS\n");
            findall(path);
        }
    }
    return 0;
}