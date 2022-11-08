#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <algorithm>
using namespace std;
//#include "common.h"
#define LENGTH 524288
// #define DEST_PORT 6127
// #define SERVER_IP_ADDRESS "127.0.0.1"
int DEST_PORT, my_port_num, temp_port_num;
char SERVER_IP_ADDRESS[32], OWN_IP_ADDRESS[32];
struct client_info
{
    string filename;
    int comm_socket_fd;
    struct sockaddr_in client_addr;
};

static void *service_client_module(void *arg)
{
    struct client_info *a = (struct client_info *)arg;
    printf("entered service client module\n");
    struct client_info *client_info_t = (struct client_info *)arg;
    int comm_socket_fd = client_info_t->comm_socket_fd;

    int bytes_transferred = 0;

    struct sockaddr_in client_addr;

    client_addr = client_info_t->client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    printf("New thread created ....\n");
    char str[4096];
    string userID, pswd, groupID;
    //while (1)
    //{
    //   int sent_recv_bytes = read(a->comm_socket_fd, str, sizeof(str));
    //   cout<<"received from client "<<str<<endl;
    //}
    int i;
    string cmd = "";
    bytes_transferred = read(comm_socket_fd, str, sizeof(str));
    string s = (string)str;
    for (i = 0; i < s.length(); i++)
    {
        if (s[i] == ' ' || s[i] == '\0')
        {
            break;
        }
        cmd += s[i];
    }
    groupID = "";
    for (i = i + 1; i < s.length(); i++)
    {
        if (s[i] == ' ')
        {
            break;
        }
        groupID += s[i];
    }
    string filename = "";
    for (i = i + 1; i < s.length(); i++)
    {
        if (s[i] == ' ')
        {
            break;
        }
        filename += s[i];
    }
    string destpath = "";
    for (i = i + 1; i < s.length(); i++)
    {
        if (s[i] == '\0')
        {
            break;
        }
        destpath += s[i];
    }
    cout<<"Filename is:"<<filename;
    strcpy(str, filename.c_str());
    cout<<"Opening thi file: "<<str;
    FILE *fs = fopen(str, "rb");
    if (fs == NULL)
    {
        printf("ERROR: File %s not found.\n", "test/new.mp4");
        exit(1);
    }
    char sdbuf[LENGTH];
    bzero(sdbuf, LENGTH);
    int fs_block_sz;

    // bpt::ptime start, stop;
    // start = bpt::microsec_clock::local_time();
    int totalBytes = 0;
    double transferRate = 0.0;
    cout<<"receiver port no:"<<a->comm_socket_fd<<endl;
    while ((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
    {
        if (send(a->comm_socket_fd, sdbuf, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", "test/new.mp4", errno);
            break;
        }
        // stop = bpt::microsec_clock::local_time();
        // bpt::time_duration dur = stop - start;
        // double seconds = dur.total_milliseconds() / 1000.0;
        totalBytes += fs_block_sz;
        //transferRate = totalBytes / seconds; // b/s
        cout << fs_block_sz <<" "<<totalBytes<< endl;
        if(fs_block_sz != LENGTH)
            break;
        bzero(sdbuf, LENGTH);
    }
    
    cout<<"file downloaded successfully\n";
    //char buf[] = "completed";
    //cout<<buf<<endl;
    //bzero(sdbuf,LENGTH);
    //strcpy(sdbuf,s.c_str());
    //send(a->comm_socket_fd, buf, strlen(buf), 0);
}
static void *waitForNode(void *arg)
{
    long long int master_sock_tcp_fd = 0,
                  bytes_transferred = 0,
                  opt = 1;
    socklen_t addr_len = 0;
    long long int comm_socket_fd = 0;
    fd_set readfds;
    struct sockaddr_in server_addr, client_addr;
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = my_port_num;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(struct sockaddr);
    if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("socket bind failed\n");
        return NULL;
    }

    if (listen(master_sock_tcp_fd, 5) < 0)
    {
        printf("listen failed\n");
        return NULL;
    }

    while (1)
    {

        FD_ZERO(&readfds);
        FD_SET(master_sock_tcp_fd, &readfds);
        printf("blocked on select System call...\n");
        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(master_sock_tcp_fd, &readfds))
        {
            printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");
            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (comm_socket_fd < 0)
            {
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }

            printf("Connection accepted from client : %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            pthread_t client_thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            struct client_info *clientDetails = (struct client_info *)calloc(1, sizeof(struct client_info));
            clientDetails->comm_socket_fd = comm_socket_fd;
            memcpy(&clientDetails->client_addr, &client_addr, sizeof(struct sockaddr_in));
            pthread_create(&client_thread, &attr, service_client_module, (void *)clientDetails);
            //pthread_kill(x,SIGSTOP);
        }
    }
}
void setup_tcp_communication()
{
    pthread_t client_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // struct client_info *clientDetails = (struct client_info *)calloc(1, sizeof(struct client_info));
    // clientDetails->comm_socket_fd = comm_socket_fd;
    // memcpy(&clientDetails->client_addr, &client_addr, sizeof(struct sockaddr_in));
    pthread_create(&client_thread, &attr, waitForNode, NULL);
    int sockfd = 0, sent_recv_bytes = 0;
    socklen_t addr_len = 0;
    addr_len = sizeof(struct sockaddr);
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = DEST_PORT;
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
    connect(sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr));
PROMPT_USER:
    string str;
    char user_id[4096], pswd[4096], group_id[4096];
    // cout << "Operation and there syntaxes. Choose what you need:\nTo Create User Account: create_user <user_id> <passwd>\nFor Login: login <user_id> <passwd>\nTo Create Group: create_group <group_id>\nTo Join Group: join_group <group_id>\nTo Leave Group: leave_group <group_id>\nTo List pending join: requests list_requests <group_id>\nTo Accept Group Joining Request: accept_request <group_id> <user_id>\nTo List All Group In Network: list_groups\nTo List All sharable Files In Group: list_files <group_id>\nTo Upload File: upload_file <file_path> <group_id>\nTo Download File: download_file <group_id> <file_name> <destination_path>\nTo Logout: logout\nTo Show_downloads: show_downloads\nStop sharing: stop_share <group_id> <file_name>\n";
    getline(cin, str);
    cout << "Given input is:" << str << endl;
    char buf[str.size() + 1];
    bzero(buf, sizeof(buf));
    strcpy(buf, str.c_str());
    sent_recv_bytes = write(sockfd, buf, sizeof(buf));
    string cmd = "";
    int i;
    for (i = 0; i < str.length(); i++)
    {
        if (str[i] == ' ' || str[i] == '\0')
        {
            break;
        }
        cmd += str[i];
    }
    if(cmd == "upload_file"){
        string s;
        cin>>s;
        bzero(buf, sizeof(buf));
        strcpy(buf, s.c_str());
        sent_recv_bytes = write(sockfd, buf, sizeof(buf));
    }
    if (cmd == "download_file")
    {
        if(fork() == 0){
        int bytes_transferred = read(sockfd, buf, 1024);
        cout<<"received seeder port no: "<<buf<<endl;
        string s = (string)buf;
        int seed_port_no =htons(stoi(s));
        temp_port_num = DEST_PORT;
        DEST_PORT = seed_port_no;
        cout<<"After converion, seeder port no:"<<DEST_PORT<<endl;
        int tsockfd = 0, tsent_recv_bytes = 0;
        socklen_t taddr_len = 0;
        taddr_len = sizeof(struct sockaddr);
        struct sockaddr_in tdest;
        tdest.sin_family = AF_INET;
        tdest.sin_port = DEST_PORT;
        struct hostent *thost = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
        tdest.sin_addr = *((struct in_addr *)thost->h_addr);
        tsockfd = socket(AF_INET, SOCK_STREAM, 0); //IPPROTO_TCP
        connect(tsockfd, (struct sockaddr *)&tdest, sizeof(struct sockaddr));
        bzero(buf, sizeof(buf));
        strcpy(buf, str.c_str());
        tsent_recv_bytes = write(tsockfd, buf, sizeof(buf));
        cout<<"comm_sock_fd before send:"<<tsockfd<<endl;
        cout<<"sent to seeder:"<<buf<<endl;
        string groupID = "";
        for (i = i + 1; i < str.length(); i++)
        {
            if (str[i] == ' ')
            {
                break;
            }
            groupID += str[i];
        }
        cout<<"Breaking down the msg\ngroupID:"<<groupID<<endl;
        string filename = "";
        for (i = i + 1; i < str.length(); i++)
        {
            if (str[i] == ' ')
            {
                break;
            }
            filename += str[i];
        }
        string destpath = "";
        for (i = i + 1; i < str.length(); i++)
        {
            if (str[i] == '\0')
            {
                break;
            }
            destpath += str[i];
        }
        cout<<"filename:"<<filename<<" "<<"destpath:"<<destpath<<endl;
        // char *filepath;
        // strcpy(filepath, (destpath).c_str());
        // char *fr_name = filepath;
        char fr_name[destpath.length() + 1];
        strcpy(fr_name, (destpath).c_str());
        cout<<"destination file name:"<<fr_name<<endl;
        FILE *fr = fopen(fr_name, "wb");
        if (fr == NULL)
            printf("File %s Cannot be opened file on server.\n", fr_name);
        else
        {
            cout<<"file created in destination and started downloading\n";
            char revbuf[LENGTH];
            bzero(revbuf, LENGTH);
            int fr_block_sz = 0;

            // bpt::ptime start, stop;
            // start = bpt::microsec_clock::local_time();
            int totalBytes = 0;
            double transferRate = 0.0;

            while ((fr_block_sz = recv(tsockfd, revbuf, LENGTH, 0)) > 0)
            {
                // if((string)revbuf == "completed"){
                //     cout<<"Last Chunk\n";
                //     break;
                // }
                // stop = bpt::microsec_clock::local_time();
                // bpt::time_duration dur = stop - start;
                // double seconds = dur.total_milliseconds() / 1000.0;
                totalBytes += fr_block_sz;
                //transferRate = totalBytes / seconds; // b/s
                cout << fr_block_sz <<" "<<totalBytes<< endl;
                int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                if (write_sz < fr_block_sz)
                {
                    cout << "File write failed on server.\n";
                }
                bzero(revbuf, LENGTH);
                // if ( fr_block_sz == 0 || fr_block_sz != LENGTH)
                // {
                //     break;
                // }
            }

            if (fr_block_sz < 0)
            {
                if (errno == EAGAIN)
                {
                    printf("recv() timed out.\n");
                }
                else
                {
                    fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                    exit(1);
                }
            }
            cout<<"File Downloaded Succesfully\n";
            fclose(fr);
        }
        DEST_PORT = temp_port_num;
    }
    }
    // sent_recv_bytes= read(sockfd, buf, 1024);
    // cout<<"result for command:"<<buf<<endl;
    // cout << "sent bytes:" << sent_recv_bytes << endl;
    goto PROMPT_USER;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "error! Enter destination ip addres and port number\n";
        return 0;
    }
    string s = argv[3];
    strcpy(SERVER_IP_ADDRESS, s.c_str());
    s = argv[4];
    int num = stoi(s);
    DEST_PORT = htons(num);
    s = argv[2];
    num = stoi(s);
    my_port_num = htons(num);
    s = argv[1];
    strcpy(OWN_IP_ADDRESS, s.c_str());
    cout<<"own IP Address:"<<OWN_IP_ADDRESS<<"OWN port no:"<<my_port_num<<"SERVER_IP_ADDRESS:"<<SERVER_IP_ADDRESS<<"Dest port no:"<<DEST_PORT;
    setup_tcp_communication();
    printf("application quits\n");
    return 0;
}
