#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void choice_dns(char *ip)
{
    printf("Choice popular dns-server or write your own:\n");

    printf("[1] DNS google\n");
    printf("[2] Cloudflare DNS\n");
    printf("[3] Write DNS server manually \n");
    int choice = 0;

    while(1)
    {
        scanf("%d", &choice);

        if (choice == 1)
        {
            strcpy(ip, "8.8.8.8");

            break;
        }
        if (choice == 2)
        {
            strcpy(ip, "1.1.1.1");
            break;
        }
        if (choice == 3)
        {
            fgets(ip, sizeof(ip), stdin);
            break;
        }
    }
}

int write_type_responses()
{
    printf("Write type of DNS proxy server's response for blacklisted domains:\n");
    printf("5 - Refused\n");
    printf("1 - Invalid form\n");
    printf("2 - Server fail\n");
    printf("3 - No such domain\n");

    int type_responses = -1;
    scanf("%d", &type_responses);

    char input[10];
    while(type_responses != 1 && type_responses != 2 && type_responses != 3 && type_responses != 5)
    {
        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            type_responses = (int)strtol(input, NULL, 10);
        }
        if(type_responses != 1 && type_responses != 2 && type_responses != 3 && type_responses != 5)
        {
            printf("Invalid type of DNS proxy server's response for blacklisted domains\n");
            printf("Please enter again:\n");
        }
    }
    return type_responses;
}

char* write_blacklist()
{
    printf("Write domain names for blacklist, separating by space and place enter to finish:\n");


    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);  //CLEAN stdin before getline

    size_t len = 1024;
    char* blacklist = (char*)malloc(len);
    getline(&blacklist, &len, stdin);

    return blacklist;
}

void write_config()
{
    char dns[16];
    choice_dns(dns);


    char *blacklist = write_blacklist();
    if (blacklist == NULL)
    {
        printf("Error while writing blacklist\n");
    }

    int type_responses = write_type_responses();

    //write to file
    FILE *file = fopen("dns_proxy_server.conf", "w");
    fprintf(file, "[config]\n");
    fprintf(file, "dns_server=%s\n", dns);
    fprintf(file, "type_responses=%d\n", type_responses);
    fprintf(file, "blacklist=%s\n", blacklist);
    fclose(file);

    free(blacklist);
}

int read_config(char* dns_server, int* type_responses, char* blacklist)
{
    FILE *file = fopen("dns_proxy_server.conf", "r");
    if (file == NULL)
    {
        printf("Error while reading config\n");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;


    while ((read = getline(&line, &len, file)) != -1)
    {
        if (strstr(line, "dns_server="))
        {
            strcpy(dns_server, strtok(line, "dns_server="));
            dns_server[strlen(dns_server) - 1] = '\0';
        }
        else if (strstr(line, "type_responses="))
        {
            char temp[2];
            strcpy(temp, strtok(line, "type_responses="));
            *type_responses = atoi(temp);
        }
        else if (strstr(line, "blacklist="))
        {
            char* temp = line;
            int distanation = strcspn(temp, "=");
            temp += distanation +1;
            temp = strtok(temp, "\n");
            memcpy(blacklist, temp, strlen(temp));
        }
    }
    free(line);
    fclose(file);
    return 0;
}
