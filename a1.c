#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

void listRec(const char *path, long value, const char *name)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    int sn = strlen(name);

    dir = opendir(path);
    if(dir == NULL)
    {
        perror("Could not open directory");
        return;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {
                if(value != -1)
                {
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        listRec(fullPath, value, name);
                    }
                    if(S_ISREG(statbuf.st_mode) && statbuf.st_size < value)
                    {
                        printf("%s\n", fullPath);
                    }
                }
                else if(sn != 0)
                {
                    char ends[512];
                    int sp = strlen(fullPath);
                    strcpy(ends, fullPath + sp - sn);
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        if(strcmp(name,ends) == 0)
                            printf("%s\n", fullPath);
                        listRec(fullPath,value,name);
                    }
                    if(S_ISREG(statbuf.st_mode))
                    {
                        if(strcmp(name,ends) == 0)
                            printf("%s\n", fullPath);
                    }
                }
                else
                {
                    printf("%s\n", fullPath);
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        listRec(fullPath, value, name);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void listDir(const char *path,long value, const char* name)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    int sn = strlen(name);

    dir = opendir(path);
    if(dir == NULL)
    {
        perror("Could not open directory");
        return;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {
                if(value != -1)
                {
                    if(S_ISREG(statbuf.st_mode) && statbuf.st_size < value)
                    {
                        printf("%s\n", fullPath);
                    }
                }
                else if(sn != 0)
                {
                    char ends[512];
                    int sp = strlen(fullPath) - 1;
                    strcpy(ends, fullPath + sp - sn + 1);
                    if(S_ISDIR(statbuf.st_mode))
                    {
                        if(strcmp(name,ends) == 0)
                            printf("%s\n", fullPath);
                    }
                    if(S_ISREG(statbuf.st_mode))
                    {
                        if(strcmp(name,ends) == 0)
                            printf("%s\n", fullPath);
                    }
                }
                else
                {
                    printf("%s\n", fullPath);
                }
            }
        }
    }
    closedir(dir);
}

void parseHeader(const char* path)
{
    int fd = -1;
    fd = open(path,O_RDONLY);

    if(fd == -1)
    {
        perror("Could not open file");
        return;
    }

    lseek(fd, 0, SEEK_END);
    char magic[3];
    int mySize;
    unsigned char c[10];

    lseek(fd, -4, SEEK_END);
    read(fd, c, 2);
    mySize = c[1] * 256 + c[0];
    read(fd,magic, 2);
    magic[2] = '\0';

    lseek(fd, -mySize, SEEK_END);

    int version;
    read(fd,c,1);
    version = c[0];

    int nbSec;
    read(fd,c,1);
    nbSec = c[0];

    unsigned char secName[nbSec][11];
    int secSize[nbSec];
    int secType[nbSec];

    for(int i = 0; i < nbSec; i++)
    {
        read(fd, secName[i], 10);
        secName[i][10] = '\0';
        read(fd, c, 2);
        secType[i]= c[1] * 256 + c[0];
        read(fd, c, 4);
        read(fd, c, 4);
        secSize[i] = c[3]* 256 * 256 * 256 + c[2] * 256 * 256 + c[1] * 256 + c[0];
    }

    if(strcmp(magic, "Ow") != 0)
    {
        printf("ERROR\nwrong magic");
        return;
    }

    if(version < 75 || version > 140)
    {
        printf("ERROR\nwrong version");
        return;
    }

    if(nbSec > 19 || (nbSec < 6 && nbSec != 2))
    {
        printf("ERROR\nwrong sect_nr");
        return;
    }

    for(int i = 0; i < nbSec; i++)
    {
        if(secType[i] != 57 && secType[i] != 93 && secType[i] != 49 && secType[i] != 40)
        {
            printf("ERROR\nwrong sect_types");
            return;
        }
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", nbSec);
    for(int i = 0; i < nbSec; i ++)
    {
        printf("section%d: %s %d %d\n", i+1, secName[i], secType[i], secSize[i]);
    }

}

void printSection(const char* path, int section, int line)
{
    int fd = -1;
    fd = open(path,O_RDONLY);

    if(fd == -1)
    {
        printf("ERROR\ninvalid file");
        return;
    }

    lseek(fd, 0, SEEK_END);
    char magic[3];
    int mySize;
    unsigned char c[10];

    lseek(fd, -4, SEEK_END);
    read(fd, c, 2);
    mySize = c[1] * 256 + c[0];
    read(fd,magic, 2);
    magic[2] = '\0';


    lseek(fd, -mySize, SEEK_END);

    read(fd,c,1);

    int nbSec;
    read(fd,c,1);
    nbSec = c[0];

    unsigned char secName[nbSec][11];
    int secSize[nbSec];
    int secOffset[nbSec];

    for(int i = 0; i < nbSec; i++)
    {
        read(fd, secName[i], 10);
        secName[i][10] = '\0';
        read(fd, c, 2);
        read(fd, c, 4);
        secOffset[i] = c[3]* 256 * 256 * 256 + c[2] * 256 * 256 + c[1] * 256 + c[0];
        read(fd, c, 4);
        secSize[i] = c[3]* 256 * 256 * 256 + c[2] * 256 * 256 + c[1] * 256 + c[0];
    }

    lseek(fd,secOffset[section-1], SEEK_SET);
    if(section > nbSec)
    {
        printf("ERROR\ninvalid section");
        return;
    }

    char a, b;
    int cnt = 1;
    read(fd,&a,1);
    int lineInd[1000];
    lineInd[cnt] = 0;
    for(int i = 1; i < secSize[section-1]; i++)
    {

        b = a;
        read(fd,&a,1);

        if(b == 13 && a == 10)
        {
            cnt++;
            lineInd[cnt] = i + 1;
        }

    }
    lseek(fd, secOffset[section-1] + lineInd[cnt - line + 1], SEEK_SET);
    int m;
    if(line == 1)
        m = secSize[section-1] - lineInd[cnt - line + 1] + 1;
    else
        m = lineInd[cnt-line + 2] - lineInd[cnt - line + 1];
    char* strLine = (char*)malloc(m * sizeof(char));
    for(int i = 0; i < m-1; i++)
    {
        read(fd,&a,1);
        strLine[i] = a;
    }
    strLine[m] = '\0';
    if(cnt < line)
    {
        printf("ERROR\ninvalid line");
        return;
    }
    printf("SUCCESS\n");
    printf("%s", strLine);
    free(strLine);

}

void findSf(const char* path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);

    if(dir == NULL)
    {
        perror("Could not open directory");
        return;
    }

    int fd1;
    int ok;
    int lineNr;
    unsigned char a, b;
    int cnt;

    while((entry = readdir(dir)) != NULL)
    {
        ok = 1;
        lineNr = 0;
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0)
            {

                if(S_ISDIR(statbuf.st_mode))
                {
                    findSf(fullPath);
                }

                if(S_ISREG(statbuf.st_mode))
                {
                    fd1 = -1;
                    fd1 = open(fullPath,O_RDONLY);

                    if(fd1 == -1)
                    {
                        printf("ERROR\ninvalid file");
                        return;
                    }
                    lseek(fd1, 0, SEEK_END);
                    char magic[3];
                    int mySize;
                    unsigned char c[10];

                    lseek(fd1, -4, SEEK_END);
                    read(fd1, c, 2);
                    mySize = c[1] * 256 + c[0];
                    read(fd1,magic, 2);
                    magic[2] = '\0';

                    lseek(fd1, -mySize, SEEK_END);

                    int version;
                    read(fd1,c,1);
                    version = c[0];

                    int nbSec;
                    read(fd1,c,1);
                    nbSec = c[0];

                    unsigned char secName[nbSec][11];
                    int secSize[nbSec];
                    int secType[nbSec];
                    int secOffset[nbSec];

                    for(int i = 0; i < nbSec; i++)
                    {
                        read(fd1, secName[i], 10);
                        secName[i][10] = '\0';
                        read(fd1, c, 2);
                        secType[i]= c[1] * 256 + c[0];
                        read(fd1, c, 4);
                        secOffset[i] = c[3]* 256 * 256 * 256 + c[2] * 256 * 256 + c[1] * 256 + c[0];
                        read(fd1, c, 4);
                        secSize[i] = c[3]* 256 * 256 * 256 + c[2] * 256 * 256 + c[1] * 256 + c[0];
                    }

                    if(strcmp(magic, "Ow") != 0)
                    {
                        ok = 0;
                    }

                    if(version < 75 || version > 140)
                    {
                        ok = 0;
                    }

                    if(nbSec > 19 || (nbSec < 6 && nbSec != 2))
                    {
                        ok = 0;
                    }

                    for(int i = 0; i < nbSec; i++)
                    {
                        if(secType[i] != 57 && secType[i] != 93 && secType[i] != 49 && secType[i] != 40)
                        {
                            ok = 0;
                        }
                    }
                    if(ok == 1)
                    {
                        //printf("%s", fullPath);
                        for(int i = 0; i < nbSec; i++)
                        {
                            cnt = 1;
                            lseek(fd1,secOffset[i], SEEK_SET);
                            char* sir = (char*)calloc(secSize[i], sizeof(char));
                            read(fd1,sir,secSize[i]);
                            a = sir[0];
                            for(int j = 1; j <= secSize[i]; j++)
                            {
                                b = a;
                                a = sir[j];

                                if(b == 13 && a == 10)
                                {
                                    cnt++;
                                }
                                if(cnt > 14)
                                {
                                    lineNr = 1;
                                    break;
                                }
                                //printf("%d ", cnt);
                            }
                            free(sir);
                        }
                        if(lineNr == 1)
                        {
                            printf("%s\n", fullPath);
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}

int main(int argc, char** argv)
{
    if(argc >= 2)
    {
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("35342\n");
        }

        if(strcmp(argv[1], "list") == 0)
        {
            char dirName[512] = "";
            long value = -1;
            char name[512] = "";
            int recursive = -1;
            int ok = 1;
            if(argc >= 6)
            {
                printf("ERROR\n");
                printf("Too many arguments");
            }
            for(int i = 2; i < argc; i++)
            {
                if(strncmp(argv[i], "path=", 5) == 0)
                {
                    strcpy(dirName, (argv[i] + 5));
                }
                if(i == argc -1 && strlen(dirName) == 0)
                {
                    printf("ERROR\n");
                    printf("Path is invalid");
                }

                if(strncmp(argv[i], "size_smaller=",13) == 0)
                {
                    value = atoi(argv[i] + 13);
                    if(value < 0)
                    {
                        printf("ERROR\n");
                        printf("Size is invalid");
                        ok=0;
                    }
                }
                if(strncmp(argv[i], "name_ends_with=", 15) == 0)
                {
                    strcpy(name, (argv[i] + 15));
                    if(strlen(name) == 0)
                    {
                        printf("ERROR\n");
                        printf("String is invalid");
                        ok=0;
                    }
                }
                if(strncmp(argv[i], "recursive", 9) == 0)
                {
                    recursive = 1;
                }
                else if(strstr(argv[i], "recursive") == NULL && recursive == -1)
                {
                    recursive = 0;
                }
            }
            if(ok == 1)
            {
                printf("SUCCESS\n");
                if(recursive == 0)
                {
                    listDir(dirName, value, name);
                }
                else
                {
                    listRec(dirName, value, name);
                }
            }
        }

        if(strcmp(argv[1], "parse") == 0)
        {
            char fileName[512];
            if(strncmp(argv[2], "path=", 5) == 0)
            {
                strcpy(fileName, (argv[2] + 5));
                parseHeader(fileName);
            }
            else
            {
                printf("ERROR\n");
                printf("Path is invalid");
            }
        }
        if(strcmp(argv[1], "extract") == 0)
        {
            char fileName[512];
            int section, line;
            if(strncmp(argv[2], "path=", 5) == 0)
            {
                strcpy(fileName, (argv[2] + 5));
            }
            if(strncmp(argv[3], "section=",8) == 0)
            {
                section = atoi(argv[3] + 8);
            }
            if(strncmp(argv[4], "line=",5) == 0)
            {
                line = atoi(argv[4] + 5);
            }
            printSection(fileName,section,line);
        }

        if(strcmp(argv[1], "findall") == 0)
        {
            char fileName[512];
            if(strncmp(argv[2], "path=", 5) == 0)
            {
                strcpy(fileName, (argv[2] + 5));
            }
            printf("SUCCESS\n");
            findSf(fileName);
        }
    }
    return 0;
}

