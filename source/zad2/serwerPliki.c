#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

// katalogi
#include <errno.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024

// *** po odczytaniu nazwy pliku wykonywane jest sprawdzenie,
// czy plik istnieje w biezacym katalogu (przeszukiwanie biezacego katalogu) - wiersz 108

int main(int argc, char *argv[]) {
	SSL_METHOD *my_ssl_method;
	SSL_CTX *my_ssl_ctx;
	SSL *my_ssl;

    int socket1,socket2;
    socklen_t addrlen;
    struct sockaddr_in xferServer, xferClient;
    int returnStatus;
	int opcja;

	if (argc < 2) {
		printf("usage: %s port \n", argv[0]);
		return -1;
	}

	OpenSSL_add_all_algorithms();
	SSL_library_init();
	SSL_load_error_strings();

	my_ssl_method = TLSv1_server_method();
	my_ssl_ctx = SSL_CTX_new(my_ssl_method);

	SSL_CTX_use_certificate_file(my_ssl_ctx, "server.pem", SSL_FILETYPE_PEM);
	SSL_CTX_use_PrivateKey_file(my_ssl_ctx, "server.pem", SSL_FILETYPE_PEM);

	if (SSL_CTX_check_private_key(my_ssl_ctx)) {
		puts("### Klucz poprawny ###");
	} else {
		fprintf(stderr, "Niepoprawny klucz\n");
		exit(-1);
	}

    /* utworz gniazdo */
    socket1 = socket(AF_INET, SOCK_STREAM, 0);

    if (socket1 == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    opcja = 1;

	setsockopt(socket1,SOL_SOCKET, SO_REUSEADDR,(void *)&opcja, sizeof(opcja) );

	int port = atoi(argv[1]);

	memset(&xferServer, 0, sizeof(xferServer));
    /* przypisz adres gniazdu */
    xferServer.sin_family = AF_INET;
    xferServer.sin_addr.s_addr = INADDR_ANY;
    xferServer.sin_port = htons(port);

    returnStatus = bind(socket1, (struct sockaddr*)&xferServer, sizeof(xferServer));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not bind to socket!\n");
        exit(1);
    }

    returnStatus = listen(socket1, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Could not listen on socket!\n");
        exit(1);
    }

    for (;;) {
        int fd;
        int i, readCounter, writeCounter;
        //char* bufptr;
        char buf[MAXBUF];
        char filename[MAXBUF];
		short jestPlik = 0;

        /* czekaj na klienta */
		puts("czekam..");
        addrlen = sizeof(xferClient);
        socket2 = accept(socket1, (struct sockaddr*)&xferClient, &addrlen);
    	puts("1-Klient polaczony");

        if (socket2 == -1) {
            fprintf(stderr, "Could not accept connection!\n");
            exit(1);
        }

		if ((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
			ERR_print_errors_fp(stderr);
			exit(-1);
		}

		SSL_set_fd(my_ssl,socket2);

		if (SSL_accept(my_ssl) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(-1);
		}

		printf("Connection made with [version,cipher]:[%s,%s]\n", SSL_get_version(my_ssl), SSL_get_cipher(my_ssl));

        /* pobierz nazwe pliku */
        i = 0;

        if ((readCounter = SSL_read(my_ssl, filename + i, MAXBUF)) > 0) {
            i += readCounter;
        }

        if (readCounter == -1) {
            fprintf(stderr, "Could not read filename from socket!\n");
            close(socket2);
            continue;
        } else {
		    filename[i] = '\0';
			printf("2-szukam pliku: %s\n", filename);

			//szukanie pliku w katalogu ***
			DIR * katalog;
			struct dirent * pozycja;

			if (!(katalog = opendir("."))) {
				perror("opendir");
				return 1;
			}

			/* zmienna erro jest ustawiana tylko w przypadku błędu */
			errno = 0;

			while ((pozycja = readdir(katalog))) {
				//puts(pozycja->d_name);
				if(	!strcmp(pozycja->d_name,filename) ) {
					printf("3-plik: %s znaleziony!\n",filename);
					jestPlik = 1;

					break;
				}
				errno = 0;
			}

			if (errno) {
				perror("readdir");
				return 1;
			}

			closedir(katalog);
		}

		if (!jestPlik) {
			printf("3-nie znaleziono pliku: %s !! \n", filename);
		} else {
			//przesylanie rozmiaru pliku
			//puts("4-przesylanie rozmiaru pliku");
			struct stat info_plik;

			if ((stat(filename,&info_plik)) != -1) {
				long rozmiar = htonl(info_plik.st_size);
				int sent;
				if ( (sent =(SSL_write (my_ssl, &rozmiar, sizeof(rozmiar) )))  != sizeof(rozmiar) ) {
					perror("blad f. write()-przesylanie rozmiaru pliku");
				} else {
					printf("4-przeslano rozmiar = %iB \n",ntohl(rozmiar) );
				}
			} else {
				perror("blad f. stat()");
			}

		    /* otworz plik do czytania */
		    fd = (open(filename, O_RDONLY));

		    if (fd == -1) {
		        fprintf(stderr, "Could not open file for reading!\n");
		        //close(socket2);
		        continue;
		    } else {
				printf("5-otworzono plik: %s\n",filename);
			}

		    readCounter = 0;

		    /* czytaj plik i przesylaj do klienta */
		    while((readCounter = read(fd, buf, MAXBUF)) > 0) {
				if(readCounter == 0) {
					break;
				}

				buf[readCounter] = '\0';
		        writeCounter = 0;
		        writeCounter = SSL_write(my_ssl, buf, readCounter);

		        if (writeCounter == -1) {
					fprintf(stderr, "Could not write file to client!\n");
					continue;
		        }

		    }
			printf("6-wyslano plik: %s\n",filename);
		}

		SSL_shutdown(my_ssl);
		SSL_free(my_ssl);
		close(fd);
    	close(socket2);
    }

	SSL_CTX_free(my_ssl_ctx);
  	close (socket1);
  	return 0;
}
