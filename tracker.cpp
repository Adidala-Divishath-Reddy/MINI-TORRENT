#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
long long int serv_port_num; //= 6127; 
char buf[1024], OWN_IP_ADDRESS[32];
unordered_map<string, string> userIDP;
unordered_map<string, pair<string, list<string>>> groupsTable;
unordered_map<string, list<string>> joinRequestTable;
unordered_map<string, list<pair<string, pair<string, int>>>> filesInfo;
list<string> downloads;
string current_user = "";
struct client_info
{

    int comm_socket_fd;
    struct sockaddr_in client_addr;
};
void pGT()
{
    cout << "\nGroups table is:\n";
    for (auto it : groupsTable)
    {
        cout << (it.first) << ": " << it.second.first;
        for (auto it2 : it.second.second)
            cout << " " << it2 << " ";
        cout << endl;
    }
}
void puserIDP()
{
    cout << "\nuserIDP table is:\n";
    for (auto it : userIDP)
    {
        cout << (it.first) << ": " << it.second;
        cout << endl;
    }
}
void pJRT()
{
    cout << "\njoin request table is:\n";
    for (auto it : joinRequestTable)
    {
        cout << (it.first) << ": ";
        for (auto it2 : it.second)
            cout << " " << it2 << " ";
        cout << endl;
    }
}
static void *service_client_module(void *arg)
{
    struct client_info *a = (struct client_info *)arg;
    cout << "entered service client module\n";
    // pair<long long int, struct sockaddr_in> clientDetails= (struct client_info *)arg;
    // long long int comm_socket_fd = clientDetails.first;

    // long long int bytes_transferred = 0;

    // struct sockaddr_in client_addr;

    // client_addr = clientDetails.second;
    struct client_info *client_info_t = (struct client_info *)arg;
    int comm_socket_fd = client_info_t->comm_socket_fd;

    int bytes_transferred = 0;

    struct sockaddr_in client_addr;

    client_addr = client_info_t->client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    
    printf("New thread created ....\n");
    char str[4096];
    string userID, pswd, groupID;
    while (1)
    {

        memset(buf, 0, sizeof(buf));
        // bytes_transferred = recvfrom(comm_socket_fd, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
        bytes_transferred = read(comm_socket_fd, buf, 1024);
        int i;
        string cmd = "";
        for (i = 0; i < sizeof(buf); i++)
        {
            if (buf[i] == ' ' || buf[i] == '\0')
            {
                break;
            }
            cmd += buf[i];
        }
        if (cmd == "create_user")
        {
            if (current_user != "")
            {
                cout << "Please logout first.Then create new account\n";
            }
            else
            {
                cout << "entered create user\n";
                //memset(buf, 0, sizeof(buf));
                //bytes_transferred = read(comm_socket_fd,buf, 1024);
                //cout<<bytes_transferred<<endl;
                //printf("%s",buf);
                //string temp(buf);
                userID = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == ' ')
                    {
                        break;
                    }
                    userID += buf[i];
                }

                cout << "Username entered is:" << userID << endl;
                //memset(buf, 0, sizeof(buf));
                //bytes_transferred = read(comm_socket_fd, buf, 1024);
                //pswd = string(buf);
                pswd = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == '\0')
                    {
                        break;
                    }
                    pswd += buf[i];
                }
                cout << "password is:" << pswd << endl;
                userIDP[userID] = pswd;
                cout << "Account Successfully Created\n";
            }
        }
        else if (cmd == "login")
        {
            if (current_user != "")
                cout << "Already " << current_user << " logged in. Please logout from " << current_user << " account first\n";
            else
            {
                userID = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == ' ')
                    {
                        break;
                    }
                    userID += buf[i];
                }

                cout << "Username entered is:" << userID << endl;
                //memset(buf, 0, sizeof(buf));
                //bytes_transferred = read(comm_socket_fd, buf, 1024);
                //pswd = string(buf);
                pswd = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == '\0')
                    {
                        break;
                    }
                    pswd += buf[i];
                }
                cout << "password is:" << pswd << endl;
                if (userIDP.find(userID) == userIDP.end())
                    cout << "Username doesn't exist or invalid. Please enter correct user name\n";
                else
                {

                    //printf("%s \t %s \n", pswd, userIDP[userID]);
                    //cout << pswd << endl;
                    //cout << userIDP[userID] << endl;
                    if (pswd == userIDP[userID])
                    {
                        cout << "login successful\n";
                        current_user = userID;
                    }
                    else
                        cout << "invalid password\n";
                }
            }
        }
        else if (cmd == "create_group")
        {
            if (current_user == "")
                cout << "Please login to create a group\n";
            else
            {
                groupID = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == '\0')
                    {
                        break;
                    }
                    groupID += buf[i];
                }

                cout << "group ID entered is:" << groupID << endl;
                if (groupsTable.find(groupID) != groupsTable.end())
                    cout << "Already this is present. Cannot create again. Please check.\n";
                else
                {
                    list<string> l;
                    l.push_back(current_user);
                    groupsTable[groupID].first = current_user;
                    groupsTable[groupID].second = l;
                    cout << groupID << " Group created successfully\n";
                }
            }
        }
        else if (cmd == "join_group")
        {
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                groupID += buf[i];
            }
            cout << "group ID entered is:" << groupID << endl;
            if (groupsTable.find(groupID) == groupsTable.end())
                cout << "Group doesn't exist. Please check.\n";
            else
            {
                if (joinRequestTable.find(groupID) == joinRequestTable.end())
                {
                    list<string> l;
                    joinRequestTable[groupID] = l;
                }
                joinRequestTable[groupID].push_back(current_user);
                cout << "Join request submitted\n";
            }
        }
        else if (cmd == "leave_group")
        {
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                groupID += buf[i];
            }
            cout << "group ID entered is:" << groupID << endl;
            if (groupsTable.find(groupID) == groupsTable.end())
            {
                cout << "Invalid group ID. Group doesn't exist or you are already not a member of this group\n";
            }
            else
            {
                int flag = 0;
                //list<string> l(joinRequestTable[groupID]);
                for (auto it = groupsTable[groupID].second.begin(); it != groupsTable[groupID].second.end(); it++)
                    if (*it == current_user)
                    {
                        groupsTable[groupID].second.erase(it);
                        cout << "Left group\n";
                        flag = 1;
                        break;
                    }
                if (flag == 0)
                    cout << "you are not present in the group.So no need to put leave request\n";
            }
        }
        else if (cmd == "requests")
        {
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == ' ')
                {
                    break;
                }
            }

            //cout << "Username entered is:"<<userID << endl;
            //memset(buf, 0, sizeof(buf));
            //bytes_transferred = read(comm_socket_fd, buf, 1024);
            //pswd = string(buf);
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                groupID += buf[i];
            }
            //cout << "groupID is:" << groupID << endl;

            if (joinRequestTable.find(groupID) == joinRequestTable.end())
            {
                cout << "Invalid group ID. Group is empty or doesn't exist\n";
            }
            else
            {
                if (groupsTable[groupID].first == current_user)
                {
                    list<string> l(joinRequestTable[groupID]);
                    cout << groupID << ":\n";
                    for (auto it : joinRequestTable[groupID])
                        cout << it << "\t";
                    cout << endl;
                }
                else
                    cout << "You are not the owner of this group: " << groupID << endl;
            }
        }
        else if (cmd == "accept_request")
        {
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == ' ')
                {
                    break;
                }
                groupID += buf[i];
            }
            //cout << "groupID is:" << groupID << endl;
            userID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                userID += buf[i];
            }

            //cout << "Username entered is:" << userID << endl;

            //memset(buf, 0, sizeof(buf));
            //bytes_transferred = read(comm_socket_fd, buf, 1024);
            //pswd = string(buf);
            // list<string> l;
            // l.push_back(current_user);
            // groupsTable[groupID].first = current_user;
            // groupsTable[groupID].second=l;
            if (joinRequestTable.find(groupID) == joinRequestTable.end())
            {
                cout << "Invalid group ID. Group is empty or doesn't exist\n";
            }
            else
            {
                if (current_user == groupsTable[groupID].first)
                {
                    //list<string> l(joinRequestTable[groupID]);
                    if (joinRequestTable[groupID].empty())
                        cout << "No requests are there in present group\n";
                    else
                    {
                        int flag = 0;
                        //cout<<groupID<<":\n";
                        for (auto it = joinRequestTable[groupID].begin(); it != joinRequestTable[groupID].end(); it++)
                        {
                            if (*it == userID)
                            {
                                joinRequestTable[groupID].erase(it);
                                groupsTable[groupID].second.push_back(userID);
                                cout << "Accepted request of " + userID + " \n";
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 0)
                            cout << "no request from " + userID + " \n";
                    }
                }
                else
                    cout << "You are not the owner of the group.So you cannot accept the requests\n";
            }
        }
        else if (cmd == "list_groups")
        {
            for (auto it : groupsTable)
            {
                cout << (it.first) << " ";
                //<< " : owner name:" << it.second.first << endl;
                // for (auto it2 : it.second.second)
                //     cout << " " << it2 << " ";
                // cout << endl;
            }
            cout << endl;
        }
        else if (cmd == "logout")
        {
            cout << current_user << ", You logged out successfully.\n";
            current_user = "";
        }
        else if (cmd == "upload_file")
        {
            if (current_user == "")
            {
                cout << "Please login first\n";
            }
            else
            {
                string filepath = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == ' ')
                    {
                        break;
                    }
                    filepath += buf[i];
                }
                groupID = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == '\0')
                    {
                        break;
                    }
                    groupID += buf[i];
                }
                // int port_no;
                // string s = "";
                // for (i = i + 1; i < sizeof(buf); i++)
                // {
                //     if (buf[i] == '\0')
                //     {
                //         break;
                //     }
                //     s += buf[i];
                // }
                // int num = stoi(s);
                // port_no = htons(num);
                if (groupsTable.find(groupID) == groupsTable.end())
                    cout << "Group doesn't exist. Please check.\n";
                else
                {
                    cout << "Please enter your port number\n";
                    bzero(str, sizeof(str));
                    int bytes_transferred = read(comm_socket_fd, str, sizeof(str));
                    cout << "port number of uploader:" << str << endl;
                    string s = (string)str;
                    //cout << "after converting into string:" << s << endl;
                    int n = stoi(s);
                    //cout << "after converting into number:" << n << endl;
                    // int port_no = htons(n);
                    // cout<<"after converting into port no:"<<port_no<<endl;
                    if (filesInfo.find(filepath) == filesInfo.end())
                    {
                        list<string> l;
                        joinRequestTable[filepath] = l;
                    }
                    filesInfo[filepath].push_back({groupID, {"127.0.0.1", n}});
                    cout << "\nfilesInfo table is:\n";
                    for (auto it : filesInfo)
                    {
                        cout << (it.first) << ": ";
                        for (auto it2 : it.second)
                            cout << " " << it2.first << "," << it2.second.first << "," << it2.second.second << " ";
                        cout << endl;
                    }
                }
            }
        }
        else if (cmd == "list_files")
        {
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                groupID += buf[i];
            }
            if (groupsTable.find(groupID) == groupsTable.end())
                cout << "Group doesn't exist. Please check.\n";
            else
            {
                int flag = 0;
                //auto it = ;
                for (auto it : groupsTable[groupID].second)
                {
                    if (it == userID)
                    {
                        flag = 1;
                        break;
                    }

                    // cout << (it.first) << ": " << it.second.first;
                    // for (auto it2 : it.second.second)
                    //     cout << " " << it2 << " ";
                    // cout << endl;
                }
                if (flag == 0)
                    cout << "You are not member of the group. So you cannot see the sharable files\n";
                if (flag == 1)
                {
                    string s = "";
                    for (auto it : filesInfo)
                    {
                        for (auto it2 : it.second)
                        {
                            if (it2.first == groupID)
                            {
                                cout << it.first << " ";
                                //s+=it.first+" ";
                                break;
                            }
                        }
                    }
                }
            }

            cout << endl;
            // bzero(buf, sizeof(buf));
            // strcpy(buf,s.c_str());
            // bytes_transferred = write(comm_socket_fd, buf, sizeof(buf));
        }
        else if (cmd == "download_file")
        {
            if (current_user == "")
            {
                cout << "Please login first\n";
            }
            else
            {
                groupID = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == ' ')
                    {
                        break;
                    }
                    groupID += buf[i];
                }
                string filepath = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == ' ')
                    {
                        break;
                    }
                    filepath += buf[i];
                }

                string destpath = "";
                for (i = i + 1; i < sizeof(buf); i++)
                {
                    if (buf[i] == '\0')
                    {
                        break;
                    }
                    destpath += buf[i];
                }
                if (filesInfo.find(filepath) == filesInfo.end())
                {
                    cout << "File doesn't exist\n";
                }
                else
                {
                    //filesInfo[filepath].push_back({groupID, {"127.0.0.1", port_no}});
                    //list<string> l(filesInfo[filepath]);
                    //cout << groupID << ":\n";
                    for (auto it : filesInfo[filepath])
                    {
                        if (groupID == it.first)
                        {
                            downloads.push_back(filepath);
                            cout << "seeder port stored in table:" << it.second.second << endl;
                            string temps = to_string(it.second.second);
                            //cout << "\nseeder port stored in table:" << temps;
                            char buf[temps.length() + 1];
                            strcpy(buf, temps.c_str());
                            //cout << "\nseeder port stored in table:" << buf<<endl;
                            int sent_recv_bytes = write(a->comm_socket_fd, buf, sizeof(buf));
                            break;
                        }
                    }
                    cout << endl;
                }
            }
        }
        else if (cmd == "show_downloads")
        {
            //string s = "";
            for (auto it : downloads)
                cout << it + " ";
            cout << endl;
            // bzero(buf, sizeof(buf));
            // strcpy(buf, s.c_str());
            // bytes_transferred = write(comm_socket_fd, buf, sizeof(buf));
            // bzero(buf, sizeof(buf));
        }
        else if (cmd == "stop_share")
        {
            groupID = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == ' ')
                {
                    break;
                }
                groupID += buf[i];
            }
            string filepath = "";
            for (i = i + 1; i < sizeof(buf); i++)
            {
                if (buf[i] == '\0')
                {
                    break;
                }
                filepath += buf[i];
            }
            //string s = "";
            int flag = 0;
            // for (auto it : filesInfo[filepath])
            // {
            //     if (it.first == groupID)
            //     {
            //         cout << "stopped sharing of " << filepath << " in " << groupID << " " << endl;
            //         filesInfo[filepath].erase(it);
            //         flag = 1;
            //     }
            // }
            for (auto it = filesInfo[filepath].begin(); it != filesInfo[filepath].end(); it++)
            {
                if ((*it).first == groupID)
                {
                    cout << "stopped sharing of " << filepath << " in " << groupID << " " << endl;
                    filesInfo[filepath].erase(it);
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                cout << "File is already not sharing in " << groupID << endl;
            // bzero(buf, sizeof(buf));
            // strcpy(buf, s.c_str());
            // bytes_transferred = write(comm_socket_fd, buf, sizeof(buf));
            // bzero(buf, sizeof(buf));
        }
        else
        {
            cout << "Not valid command. Please check\n";
            // bzero(buf, sizeof(buf));
            // strcpy(buf, s.c_str());
            // bytes_transferred = write(comm_socket_fd, buf, sizeof(buf));
            // bzero(buf, sizeof(buf));
        }
        //cout<<"rcevd:"<<buf<<endl;
        // printf("Server recvd %lld bytes from client %s:%u\n", bytes_transferred,
        //         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //if(bytes_transferred > 0)
        //cout<<"received from client:"<<buf<<endl;
        if (bytes_transferred == 0)
        {
            close(comm_socket_fd);
            break;
        }
    }
}

void serverInit()
{

   
    long long int master_sock_tcp_fd = 0, 
        bytes_transferred = 0,

                  opt = 1;
    socklen_t addr_len = 0;
    long long int comm_socket_fd = 0; 
    fd_set readfds;                   
    struct sockaddr_in server_addr,
        client_addr;

    
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;    
    server_addr.sin_port = serv_port_num;  
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    addr_len = sizeof(struct sockaddr);
    if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("socket bind failed\n");
        return;
    }

    if (listen(master_sock_tcp_fd, 5) < 0)
    {
        printf("listen failed\n");
        return;
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
            // memset(buf, 0, sizeof(buf));
            // bytes_transferred = read(comm_socket_fd, &buf, sizeof(buf));
            // cout << "hii" << endl;
            // printf("%s", buf);
            // bytes_transferred = write(comm_socket_fd, &buf, sizeof(buf));
            /* Create a new thread to service this new client*/
            /* Server infinite loop for servicing the client*/
            pthread_t client_thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            struct client_info *clientDetails = (struct client_info *)calloc(1, sizeof(struct client_info));
            clientDetails->comm_socket_fd = comm_socket_fd;
            memcpy(&clientDetails->client_addr, &client_addr, sizeof(struct sockaddr_in));
            pthread_create(&client_thread, &attr, service_client_module, (void *)clientDetails);
        }
    }
}

int main(int argc, char *argv[])
{
    string s = (string)argv[2];
    int num = stoi(s);
    serv_port_num = htons(num);
    s = argv[1];
    strcpy(OWN_IP_ADDRESS, s.c_str());
    serverInit();
    return 0;
}