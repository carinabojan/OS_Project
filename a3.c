#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

int main()
{

    int fd1 = -1, fd2 = -1;
    if(mkfifo("RESP_PIPE_35342", 0600) != 0)
    {
        perror("Err creating FIFO");
        return 1;
    }

    fd2 = open("REQ_PIPE_35342", O_RDONLY);
    if(fd2 == -1)
    {
        printf("ERROR\ncannot open the request pipe");
        return 1;
    }

    fd1 = open("RESP_PIPE_35342", O_WRONLY);
    if(fd1 == -1)
    {
        printf("ERROR\ncannot create the response pipe");
        return 1;
    }

    char cuv1[] = "CONNECT#";
    if (write(fd1, cuv1, strlen(cuv1)) == -1)
    {
        perror("Error writing to the response pipe");
        close(fd1);
        close(fd2);
        return 1;
    }

    printf("SUCCESS ");

    char buff[255];
    char resp[255];
    char c;
    unsigned int sizeMem = 0;
    volatile char *sharedChar = NULL;
    char *data = NULL;
    int size;

    while(1)
    {
        int sizeR;
        unsigned int offset;
        unsigned int value;
        unsigned int sectionNr;
        unsigned int logical_offset;
        char name[255];
        int sizeName;
        unsigned int nrBytes;
        int shmFd;

        for(int i = 0; i <= 250; i++)
        {

            read(fd2, &c, 1);
            if(c != '#')
            {
                buff[i] = c;
            }
            else
            {
                sizeR = i;
                break;
            }
        }
        if (sizeR == -1)
        {
            perror("Error reading from request pipe");
            break;
        }

        buff[sizeR] = '\0';

        if(strcmp(buff, "PING") == 0)
        {
            strcpy(resp,"PING#PONG#");
            unsigned int nr = 35342;
            write(fd1, resp, strlen(resp));
            write(fd1, &nr, sizeof(nr));
        }
        else if(strcmp(buff, "CREATE_SHM") == 0)
        {
            read(fd2, &sizeMem, sizeof(sizeMem));

            shmFd = shm_open("/fPsYdK", O_CREAT | O_RDWR, 0664);
            if(shmFd < 0)
            {
                perror("Could not aquire shm");
                return 1;
            }
            ftruncate(shmFd, sizeMem);

            sharedChar = (volatile char*)mmap(0, sizeMem, PROT_READ | PROT_WRITE,
                                              MAP_SHARED, shmFd, 0);
            if(sharedChar == (void*)-1)
            {
                strcpy(resp,"CREATE_SHM#ERROR#");
                write(fd1, resp, strlen(resp));
                continue;
            }
            else
            {
                strcpy(resp,"CREATE_SHM#SUCCESS#");
                write(fd1, resp, strlen(resp));
                continue;
            }
        }
        else if(strncmp(buff, "WRITE_TO_SHM", 12) == 0)
        {
            read(fd2, &offset, sizeof(offset));
            read(fd2, &value, sizeof(value));

            if(0 <= offset && offset + 4 <= sizeMem)
            {
                memcpy((void*)(sharedChar+offset),&value,sizeof(value));
                strcpy(resp,"WRITE_TO_SHM#SUCCESS#");
                write(fd1, resp, strlen(resp));
            }
            else
            {
                strcpy(resp,"WRITE_TO_SHM#ERROR#");
                write(fd1, resp, strlen(resp));
            }
        }
        else if(strcmp(buff, "MAP_FILE") == 0)
        {
            for(int i = 0; i <= 250; i++)
            {

                read(fd2, &c, 1);
                if(c != '#')
                {
                    name[i] = c;
                }
                else
                {
                    sizeName = i;
                    break;
                }
            }
            if (sizeName == -1)
            {
                perror("Error reading from request pipe");
                break;
            }

            name[sizeName] = '\0';

            int fd = -1;
            fd = open(name,O_RDONLY);

            if(fd == -1)
            {
                perror("Could not open input file");
                strcpy(resp,"MAP_FILE#ERROR#");
                write(fd1, resp, strlen(resp));
                return 1;
            }

            size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);

            data = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
            if(data == (void*)-1)
            {
                strcpy(resp,"MAP_FILE#ERROR#");
                write(fd1, resp, strlen(resp));
                close(fd);
            }
            else
            {
                strcpy(resp,"MAP_FILE#SUCCESS#");
                write(fd1, resp, strlen(resp));
            }

        }
        else if(strcmp(buff, "READ_FROM_FILE_OFFSET") == 0)
        {
            read(fd2, &offset, sizeof(offset));
            read(fd2, &nrBytes, sizeof(nrBytes));

            int j = 0;

            if(offset + nrBytes > size)
            {
                strcpy(resp,"READ_FROM_FILE_OFFSET#ERROR#");
                write(fd1, resp, strlen(resp));
                continue;
            }


            for(unsigned int i = offset; i < offset + nrBytes; i++)
            {
                sharedChar[j] = data[i];
                j++;
            }

            if(shmFd > 0 && offset + nrBytes <= size && data != NULL)
            {
                strcpy(resp,"READ_FROM_FILE_OFFSET#SUCCESS#");
                write(fd1, resp, strlen(resp));
            }
        }
        else if(strcmp(buff, "READ_FROM_FILE_SECTION") == 0)
        {

            read(fd2, &sectionNr, sizeof(sectionNr));
            read(fd2, &offset, sizeof(offset));
            read(fd2, &nrBytes, sizeof(nrBytes));

            unsigned char c[4];

            c[0] = data[size-4];
            c[1] = data[size-3];

            int nr[4];

            nr[0] = (int)c[0];
            nr[1] = (int)c[1];

            int mySize = nr[1] * 256 + nr[0];

            c[0] = data[size-mySize + 1];

            nr[0] = (int)c[0];

            int nbSec = nr[0];

            int secSize[nbSec];
            int secOffset[nbSec];

            int j = size-mySize + 14;
            for(int i = 0; i < nbSec; i++)
            {
                c[0] = data[j];
                c[1] = data[j+1];
                c[2] = data[j+2];
                c[3] = data[j+3];
                nr[0] = (int)c[0];
                nr[1] = (int)c[1];
                nr[2] = (int)c[2];
                nr[3] = (int)c[3];
                secOffset[i] = nr[0] + nr[1] * 256 + nr[2] * 256 * 256 + nr[3] * 256 * 256 * 256;
                j = j + 4;
                c[0] = data[j];
                c[1] = data[j+1];
                c[2] = data[j+2];
                c[3] = data[j+3];
                nr[0] = (int)c[0];
                nr[1] = (int)c[1];
                nr[2] = (int)c[2];
                nr[3] = (int)c[3];
                secSize[i] = nr[0] + nr[1] * 256 + nr[2] * 256 * 256 + nr[3] * 256 * 256 * 256;
                j = j + 16;
            }
            int l = 0;

            if(sectionNr > nbSec || offset + nrBytes > secSize[sectionNr - 1])
            {
                strcpy(resp,"READ_FROM_FILE_SECTION#ERROR#");
                write(fd1, resp, strlen(resp));
                continue;
            }

            for(int i = offset + secOffset[sectionNr - 1]; i < offset + nrBytes + secOffset[sectionNr - 1]; i++)
            {
                sharedChar[l] = data[i];
                l++;
            }

            strcpy(resp,"READ_FROM_FILE_SECTION#SUCCESS#");
            write(fd1, resp, strlen(resp));

        }
        else if(strcmp(buff, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {

            read(fd2, &logical_offset, sizeof(logical_offset));
            read(fd2, &nrBytes, sizeof(nrBytes));

            printf("%d %d\n", logical_offset, nrBytes);

            unsigned char c[4];

            c[0] = data[size-4];
            c[1] = data[size-3];

            int nr[4];

            nr[0] = (int)c[0];
            nr[1] = (int)c[1];

            int mySize = nr[1] * 256 + nr[0];

            c[0] = data[size-mySize + 1];

            nr[0] = (int)c[0];

            int nbSec = nr[0];

            int secSize[nbSec];
            int secOffset[nbSec];

            int j = size-mySize + 14;
            for(int i = 0; i < nbSec; i++)
            {
                c[0] = data[j];
                c[1] = data[j+1];
                c[2] = data[j+2];
                c[3] = data[j+3];
                nr[0] = (int)c[0];
                nr[1] = (int)c[1];
                nr[2] = (int)c[2];
                nr[3] = (int)c[3];
                secOffset[i] = nr[0] + nr[1] * 256 + nr[2] * 256 * 256 + nr[3] * 256 * 256 * 256;
                j = j + 4;
                c[0] = data[j];
                c[1] = data[j+1];
                c[2] = data[j+2];
                c[3] = data[j+3];
                nr[0] = (int)c[0];
                nr[1] = (int)c[1];
                nr[2] = (int)c[2];
                nr[3] = (int)c[3];
                secSize[i] = nr[0] + nr[1] * 256 + nr[2] * 256 * 256 + nr[3] * 256 * 256 * 256;
                j = j + 16;
            }

            int actSec = 0;
            int nSize = secSize[0] / 3072 * 3072;
            if(secSize[0] % 3072 != 0)
            {
                nSize = nSize + 3072;
            }

            while(logical_offset >= nSize)
            {
                actSec++;
                logical_offset =logical_offset - nSize;
                nSize = secSize[actSec] / 3072 * 3072;
                if(secSize[actSec] % 3072 != 0)
                {
                    nSize = nSize + 3072;
                }
            }

            int l = 0;

            if(actSec > nbSec || logical_offset + nrBytes > secSize[actSec])
            {
                strcpy(resp,"READ_FROM_LOGICAL_SPACE_OFFSET#ERROR#");
                write(fd1, resp, strlen(resp));
                continue;
            }

            for(int i = logical_offset + secOffset[actSec]; i < logical_offset + nrBytes + secOffset[actSec]; i++)
            {
                sharedChar[l] = data[i];
                l++;
            }

            strcpy(resp,"READ_FROM_LOGICAL_SPACE_OFFSET#SUCCESS#");
            write(fd1, resp, strlen(resp));

        }
        else if(strcmp(buff, "EXIT") == 0)
        {
            break;
        }

    }

    close(fd1);
    close(fd2);

    if (data != NULL && data != (void *)-1)
    {
        munmap(data, size);
    }

    if (sharedChar != NULL && sharedChar != (void *)-1)
    {
        munmap((void *)sharedChar, sizeMem);
    }

    if (unlink("RESP_PIPE_35342") != 0)
    {
        perror("Couldn't unlink existing FIFO");
    }

    return 0;
}
