
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>   // inet_pton
#include <sys/socket.h>  //socket, bind, sockaddr, recvfrom, sendto
#include "generate_config.h"

//DNS header structure
/*
struct DNS_HEADER
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};
*/
struct DNS_HEADER
{
    uint16_t id;

    uint16_t flags;

    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};


void send_dns_message(int sockfd, char* buffer, int length, struct sockaddr_in* addr)
{
    sendto(sockfd, buffer, length, 0, (struct sockaddr*)addr, sizeof(*addr));
}

int receive_from(int sockfd, char* buffer, struct sockaddr_in* addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);

    //int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
    int recv_len = recvfrom(sockfd, buffer, 512, 0, (struct sockaddr*)addr, &addr_len);
    //printf("buffer: %s\n", buffer);
    return recv_len;
}

int initialize_DNS_socket(struct sockaddr_in *dns_addr, char* ip)
{
    int dns_sock = socket(AF_INET, SOCK_DGRAM, 0);

    //struct sockaddr_in dns_addr;
    memset(dns_addr, 0, sizeof(*dns_addr));
    dns_addr->sin_family = AF_INET;
    dns_addr->sin_port = htons(53);
    inet_pton(AF_INET, ip, &dns_addr->sin_addr);

    return dns_sock;
}

int initialize_socket_udp()
{
    // UDP socket intercaption
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET; // IPv4 ???
    srv_addr.sin_port = htons(53);  // DNS port
    srv_addr.sin_addr.s_addr = INADDR_ANY;    // any interface

    bind(sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    return sockfd;
}


char* get_qname(unsigned char *buff)
{
    int len = 0;
    while (buff[len] != 0)
    {
        len += buff[len] + 1; // it will take anything until 0x00
    }
    len + 1; // add for 0x00 at the end

    char* out = malloc(len);
    memcpy(out, buff, len);
    return out;
}

char* decode_qname(unsigned char* qname)
{
    char* output = malloc(strlen(qname) + 1);
    int i = 0, j = 0;
    while (qname[i] != 0) {
        int len = qname[i];
        for (int k = 0; k < len; k++) {
            output[j++] = qname[i + k + 1];
        }
        output[j++] = '.';
        i += len + 1;
    }
    output[j - 1] = '\0';  // Замість останньої крапки — кінець рядка
    return output;
}
void send_blocked_response(int sockfd, struct sockaddr_in* client_addr, char* buffer, int *response, int recv_len)
{
    unsigned char dns_q[512];
    memcpy(dns_q, buffer, sizeof(struct DNS_HEADER));  // Копіюємо header

    struct DNS_HEADER* dns_header = (struct DNS_HEADER*) dns_q;

    // QR=1, Opcode=0, AA=1, TC=0, RD=1, RA=1,
    if(*response == 3)
    {
        dns_header->flags = htons(0x8183); // RCODE=3 (NXDOMAIN)
    }
    else if (*response == 2)
    {
        dns_header->flags = htons(0x8182); // RCODE=2 (NXRRSET)
    }
    else if (*response == 1)
    {
        dns_header->flags = htons(0x8181); // RCODE=1 (FORMERR)
    }
    else if (*response == 5)
    {
        dns_header->flags = htons(0x8185); //  RCODE=5 (REFUSED)
    }

    dns_header->ancount = htons(0);
    dns_header->nscount = htons(0);
    dns_header->arcount = htons(0);

    int question_len = recv_len - sizeof(struct DNS_HEADER);
    memcpy(dns_q + sizeof(struct DNS_HEADER), buffer + sizeof(struct DNS_HEADER), question_len);

    sendto(sockfd, dns_q,  sizeof(struct DNS_HEADER) + question_len, 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
}
int main()
{
    int sockfd = initialize_socket_udp();

    if (sockfd == -1)
    {
        perror("socket: error");
        return 1;
    }

    struct sockaddr_in dns_addr;
    char ip[16];

    char buffer[512];
    struct sockaddr_in client_addr;

    char* dns_sever = malloc(16);
    int*  response = malloc(sizeof(int));;
    char* blacklist = malloc(1024);

    int success = read_config(dns_sever, response, blacklist);

    if(success == 0)
    {
        printf("DNS server: %s\n", dns_sever);
        strcpy(ip, dns_sever);
        printf("Response: %d\n", *response);
        printf("Blacklist: %s\n\n", blacklist);
    }
    else
    {
        printf("Would you like to create config file? (y/n): ");
        char choice;
        scanf("%c", &choice);
        if (choice == 'y')
        {
            write_config();
        }
        printf("Okay. To the next timeExiting...\n");
        return 1;
    }

    int dns_sock = initialize_DNS_socket(&dns_addr, ip);
    if (dns_sock == -1)
    {
        perror("socket: DNS error");
        return 1;
    }

    printf("[#] DNS proxy server is running\n\n");
    while (1)
    {
        printf("[+] Waiting for user...\n");

        // receive from client and send to DNS server
        int recv_len = receive_from(sockfd, buffer, &client_addr);
        if (recv_len == -1)
        {
            perror("receive_from_client: error");
            return 1;
        }

        char* qnamee = get_qname((unsigned char*)&buffer[sizeof(struct DNS_HEADER)]);

        unsigned char *reader = decode_qname(qnamee);
        printf("Received from client: %s\n", reader);

        if(strstr(blacklist, reader) != NULL)
        {
            printf("[!] Blacklisted domain reseved!\n");
            send_blocked_response(sockfd, &client_addr, buffer, response, recv_len);
            printf("[+] Sent blocked response to user\n\n");
            continue;
        }

        printf("[+] Sending to DNS server\n");
        send_dns_message(dns_sock, buffer, recv_len, &dns_addr);

        // receive response from DNS server and send to client
        int success_response = receive_from(dns_sock, buffer, &dns_addr);
        if (success_response == -1)
        {
            perror("[@] receive response from DNS: error");
            return 1;
        }
        //printf("Received from DNS: %s\n", buffer);
        send_dns_message(sockfd, buffer, recv_len, &client_addr);
        printf("[+] Sent response to user\n\n");
    }

    free(ip);

    free(dns_sever);
    free(response);
    free(blacklist);
    return 0;

}
