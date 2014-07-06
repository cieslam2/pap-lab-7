#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024

// *** klient sprawdza, czy pobral caly plik poprzez porownanie rozmiaru
// oryginalnego pliku (otrzymuje przed pobieraniem pliku) z rozmiarem pliku pobranego - wiersz 113

int main(int argc, char* argv[]) {
	SSL_METHOD *my_ssl_method;
	SSL_CTX *my_ssl_ctx;
	SSL *my_ssl;

    int sockd;
    int counter;
    int fd;
    struct sockaddr_in xferServer;
    int returnStatus;
	char buf[MAXBUF];
    int bufDl;
	char *filename;
	char *destination;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <ip address> <port> <filename> [dest filename]\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[2]);
    filename = argv[3];

	if (argc == 5) {
 		destination = argv[4];
	}

	OpenSSL_add_all_algorithms();
	SSL_library_init();
	SSL_load_error_strings();
	my_ssl_method = TLSv1_client_method();
	my_ssl_ctx = SSL_CTX_new(my_ssl_method);

	if ((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

    /* utworz gniazdo */
    sockd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockd == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

	memset(&xferServer, 0, sizeof(xferServer));
    /* wpisz informacje o serwerze */
    xferServer.sin_family = AF_INET;
    xferServer.sin_addr.s_addr = inet_addr(argv[1]);
    xferServer.sin_port = htons(port);

	bind(sockd, (struct sockaddr*) & xferServer, sizeof(xferServer));

    /* polacz z serwerem */
    returnStatus = connect(sockd, (struct sockaddr*) & xferServer, sizeof(xferServer));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not connect to server!\n");
        exit(1);
    }
	puts("\n1-Poloczono");

	SSL_set_fd(my_ssl,sockd);

	if (SSL_connect(my_ssl) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	printf("Polaczenie na podstawie: [version,cipher]:[%s,%s]\n", SSL_get_version(my_ssl), SSL_get_cipher(my_ssl));

    /* wyslij nazwe pliku */

	strcpy(buf, filename);
    bufDl = strlen(buf);

	if ((returnStatus = SSL_write(my_ssl, buf, bufDl)) != bufDl) {
		puts("Przeslano zla liczbe bajtow!");
		exit(1);
	}

    //....
    if (returnStatus == -1) {
        fprintf(stderr, "Could not send filename to server!\n");
        exit(1);
    }

	printf("2-Wyslano nazwe pliku %s\n",filename);

    /* zamknij polaczenie jednostronnie */
    //shutdown(sockd,SHUT_WR);

	// odczytanie rozmiaru pliku
	long rozmiar = 0;
	int otrzymano = 0;

	if ((otrzymano = (SSL_read(my_ssl, &rozmiar, sizeof(rozmiar) ))) ==-1) {
		perror("blad f. read()-pobieranie rozmiaru pliku");
	}

	rozmiar = htonl(rozmiar);
	printf("3-Otrzymano rozmiar pliku = %ldB\n", rozmiar );

    /* otworz plik do zapisu */

	// zapis do pliku
	if (argc == 5) {
		if ((fd = open(destination, O_WRONLY|O_CREAT|O_TRUNC,0644) ) == -1) {
			close(fd);
			perror("f. open()-otwieranie pliku do zapisu" ) ;
			exit(2);
		}
		printf("4a-Odtworzono/utworzono plik: %s\n",destination);
 	}

	//wypisanie na ekran
	if (argc == 4) {
		printf("\n\tZawartosc pliku: %s \n",filename);
		puts("---------------------------------------------");
	}

    /* czytaj plik z serwera
	// porownanie z oryginalnym rozmiarem i sprawdzenie, czy zerowa dl. rozmiaru
	if( (counter = SSL_read(my_ssl, buf, MAXBUF - 1)) != rozmiar || !rozmiar  ) {
		printf(" Brak pliku %s!\n\n",buf);
		close(fd);
		exit(1);
	}
	else {
		buf[counter] = '\0';
		if (argc == 4)
			printf("%s",buf);
		else if (argc == 5) {
		// zapisanie do pliku
  			if ( (write(fd,buf,counter) ) != counter ) {
				perror("blad f. write() - zapisywanie do pliku");
			}
		}
	}*/

    while ((counter = SSL_read(my_ssl, buf, MAXBUF)) > 0) {
		if (counter == 0) {
			break;
		}

		buf[counter] = '\0';
		printf("%s",buf);
    }

	// ***
	if (argc == 4) {
		puts("---------------------------------------------\n");
		printf("4-Przeczytano plik: %s ", filename);
	} else if (argc == 5) {
		printf("4b-zapisano do pliku: %s ", destination);
	}

	printf("(rozmiar prawidlowy)\n");

	SSL_shutdown(my_ssl);
	SSL_free(my_ssl);
	SSL_CTX_free(my_ssl_ctx);

	close(fd);
  	close(sockd);

	puts("5-Zakonczono polaczenie\n");

  	return 0;
}
