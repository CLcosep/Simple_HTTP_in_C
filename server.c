#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

int main() {

    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4 only
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // fill in local IP 


    // resolve local address info for binding
    int status = getaddrinfo(NULL, "8080", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // create socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Socket error.");
        exit(-1);
    }

    // resolve "address already inuse" issue
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    // bind socket to address/port 
    int bind_result = bind(sockfd, res->ai_addr, res->ai_addrlen);
    printf("Bind result is: %d\n", bind_result);
    if (bind_result < 0) {
        perror("Bind error");
        exit(-1);
    }

    // res not needed after bind()
    freeaddrinfo(res);

    // listen for incoming connection
    int listen_result = listen(sockfd, 20);     //20 = backlog size
    if (listen_result < 0) {
        perror("Listen error");
        exit(-1);
    }

    // accept
    // main server loop: one client connection per iteration
    while (1) {
        
        // waiting for request to received
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        int newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (newfd < 0) {
            perror("Accept error");
            continue;
        }
        printf("Got a connection!\n");
       
        int max_len = 1000;
        char received_request[max_len];
        memset(received_request, 0, max_len);
       
        // read client raw HTTP request
        int num_bytes = recv(newfd, (void *)received_request, max_len, 0);
        if (num_bytes == -1){
            perror("Reception error: ");
            close(newfd);
            continue;
        }

        if (num_bytes == 0) {
            close(newfd);
            continue;
        }
        
        // handle only GET requests
        if (strncmp(received_request, "GET", 3) == 0) {
            printf("Received HTTP GET request!\n");

            // to read html file
            FILE * index_file = fopen("index.html", "r");
            if (!index_file) {
                perror("fopen");
                char *err = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
                send(newfd, err, strlen(err), 0);
                close(newfd);
                continue;
            }

            // read file into fixed sized stack buffer
            char file_buf[8192];
            size_t file_size = fread(file_buf, 1, sizeof file_buf, index_file);
            fclose(index_file);

            char header[256];
            snprintf(header, sizeof header,
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n",
                file_size);

            send(newfd, header, strlen(header), 0);
            send(newfd, file_buf, file_size, 0);

        } 

        close(newfd);
    }
    
    return 0;
}