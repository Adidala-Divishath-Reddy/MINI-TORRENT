#include <bits/stdc++.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define SA struct sockaddr
using namespace std;

string tracker_ip, uploadFile;
uint16_t tracker_port;
unordered_map<string, set < string>> grp_members;
unordered_map<string, set < string>> grp_requests;
unordered_map<string, string> login;
unordered_map<string, bool> checklogin;

unordered_map<string, string> convert_port;
unordered_map<string, string> piece_wise;
unordered_map<string, string> cur_file_size;
unordered_map<string, string> admin_groups;
vector<string> group_list;



bool ispath(const string &stringS)
{
	struct stat itr;
	if (stat(stringS.c_str(), &itr) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

vector<string> splitString(string stringS, string x)
{
	string temp;
	size_t position = 0;
	vector<string> ans;
	string strt="";
	while ((position = stringS.find(x)) != string::npos)
	{
		temp = stringS.substr(0, position);
		ans.push_back(temp);
		strt;
		stringS.erase(0, position + x.length());

	}

	ans.push_back(stringS);
	strt;
	return ans;
}

pair<int,string> getIPAndPortFromFileName(char* fn){
	string filename = string(fn);
    string line="";
    ifstream input_file(filename);

    if (input_file.is_open()) {
		getline(input_file, line);
		vector<string> trackerIP = splitString(line,":");
		string i="port";
		string ip=trackerIP[0];
		int port= stoi(trackerIP[1]);
		string make_pai="pair";
		return make_pair(port,ip);
        
    }

	else{
		
		printf("Cannot open the file");
		return make_pair(0,"0.0.0.0");
	}
}

void *close_server(void *arg)
{
	while (true)
	{
		string quit;
		getline(cin, quit);
		if (quit == "quit")
		{
			exit(0);
		}
	}
}

unordered_map<string, unordered_map<string, set< string>>> uploadList;

int check_login_validate(string user_id, string password)
{
	if (login.find(user_id) == login.end())
		return -1;

	if (login[user_id] != password)
		return -1;

	if (checklogin.find(user_id) == checklogin.end())
	{
		checklogin.insert({ user_id, true });
		return 0;
	}
	else
	{
		if (checklogin[user_id] == false)
		{
			checklogin[user_id] = true;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

void upload_File(string file_path, string group_id, int client_socket, string client_id)
{
	if (!ispath(file_path))
	{
		write(client_socket, "No file exists", 14);
	}
	else if (grp_members.find(group_id) == grp_members.end())
	{
		write(client_socket, "No Group Exists", 16);
	}
	else if (grp_members[group_id].find(client_id) == grp_members[group_id].end())
	{
		write(client_socket, "No client Present", 18);
	}
	else
	{
		//512kb per chunk
		write(client_socket, "Uploading", 9);
		char file_buffer[524288] = { 0 };
		string strt="file";
		if (read(client_socket, file_buffer, 524288))
		{
			strt+="";
			if (string(file_buffer) == "error") return;
			vector<string> current_file_details = splitString(string(file_buffer), "*$*");
			string filename = splitString(string(current_file_details[0]), "/").back();
			string hashes = "";
			for (size_t i = 4; i < current_file_details.size(); i++)
			{
				hashes += current_file_details[i];
				if (i != current_file_details.size() - 1) hashes += "*$*";
			}
			//cout<<string(file_buffer)<<endl<<hashes<<endl;
			piece_wise[filename] = hashes;

			if (uploadList[group_id].find(filename) != uploadList[group_id].end())
			{
				uploadList[group_id][filename].insert(client_id);
			}
			else
			{
				uploadList[group_id].insert({ filename,
					{
						client_id
					} });
			}

			cur_file_size[filename] = current_file_details[2];

			write(client_socket, "Uploaded", 8);
		}
	}
}

void download_File(string group_id, string file_name, string file_path, int client_socket, string client_id)
{
	if (!ispath(file_path))
	{
		write(client_socket, "No file exists", 14);
	}
	else if (grp_members.find(group_id) == grp_members.end())
	{
		write(client_socket, "No Group Exists", 16);
	}
	else if (grp_members[group_id].find(client_id) == grp_members[group_id].end())
	{
		write(client_socket, "No client Present", 18);
	}
	else
	{
		string strt;
		char file_buffer[524288] = { 0 };
		write(client_socket, "Downloading", 13);
		if (read(client_socket, file_buffer, 524288))
		{
			vector<string> current_file_details = splitString(string(file_buffer), "*$*");
			string message_reply = "";
			if (uploadList[group_id].find(current_file_details[0]) != uploadList[group_id].end())
			{
				for (auto i: uploadList[group_id][current_file_details[0]])
				{
					if (checklogin[i])
					{
						message_reply += convert_port[i] + "*$*";
					}
				}
				message_reply += cur_file_size[current_file_details[0]]+"&&&"+piece_wise[current_file_details[0]];
				
				write(client_socket, &message_reply[0], message_reply.length());
				//cout<<message_reply<<endl;
				uploadList[group_id][file_name].insert(client_id);
			}
			else
			{
				write(client_socket, "File not found", 14);
			}
		}
	}
}

//connection of clients using threads and handle commands
void connection(int client_socket)
{
	string client_id, client_group_id, current, input;
	cout << "Thread created" << to_string(client_socket) << "\n";

	while (true)
	{
		char buffer[2048] = { 0 };
		vector<string> input_array;
		if (read(client_socket, buffer, 2048) <= 0)
		{
			checklogin[client_id] = false;
			close(client_socket);
			break;
		}

		input = string(buffer);
		cout << "Request from client" << input << "\n";
		stringstream stringarray(input);

		while (stringarray >> current)
		{
			input_array.push_back(current);
		}

		if (input_array[0] == "create_user")
		{
			if (input_array.size() == 3)
			{
				string user_id=input_array[1];
				string password=input_array[2];
				int res;
				if (login.find(user_id) == login.end())
				{
					login.insert({ user_id, password });
					res = 0;
				}
				else
				{
					res = -1;
				}

				if (res == -1)
				{
					write(client_socket, "User already exists", 20);
				}
				else
				{
					write(client_socket, "Account Sucessfully Created", 28);
				}
			}
			else
			{
				write(client_socket, "Invalid Arguments", 18);
			}
		}
		else if (input_array[0] == "login")
		{
			if (input_array.size() == 3)
			{
				char login_buffer[1024];
				string cur_client_address;
				client_id = input_array[1];
				int res = check_login_validate(input_array[1], input_array[2]);
				if (res == 1)
				{
					write(client_socket, "User has already logged in", 28);
				}
				else if (res == 0)
				{
					write(client_socket, "Login Successful", 17);
					read(client_socket, login_buffer, 1024);
					cur_client_address = string(login_buffer);
					convert_port[client_id] = cur_client_address;
				}
				else
				{
					write(client_socket, "User_id/Password is incorrect", 30);
				}
			}
			else
			{
				write(client_socket, "Invalid Arguments", 18);
			}
		}
		else if (input_array[0] == "logout")
		{
			checklogin[client_id] = false;
			write(client_socket, "Logout Successful", 20);
		}
		else if (input_array[0] == "upload_file")
		{
			if (input_array.size() != 3)
			{
				write(client_socket, "Invalid Arguments", 18);
				//close(client_socket);
				
			}

			else{
				upload_File(input_array[1], input_array[2], client_socket, client_id);
			}
		}
		else if (input_array[0] == "download_file")
		{
			if (input_array.size() != 4)
			{
				write(client_socket, "Invalid Arguments", 18);
			}
			else{
			download_File(input_array[1], input_array[2], input_array[3], client_socket, client_id);
			}
		}
		else if (input_array[0] == "accept_request")
		{
			if (input_array.size() != 3)
			{
				write(client_socket, "Invalid Arguments", 18);
			}

			else{string group_id = input_array[1];
			string user_id = input_array[2];
			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "No group found", 13);
			}
			else if (admin_groups.find(group_id)->second != client_id)
			{
				write(client_socket, "You are not admin", 17);
			}
			else
			{
				write(client_socket, "Request accepted.", 18);
				grp_members[group_id].insert(user_id);
				grp_requests[group_id].erase(user_id);
			}
			}
		}
		else if (input_array[0] == "create_group")
		{
			if (input_array.size() != 2)
			{
				write(client_socket, "Invalid Arguments", 18);
			}

			else{string group_id = input_array[1];
			int res=0;
			for (string i: group_list)
			{
				if (i == group_id)
				{
					write(client_socket, "Group Already Exists", 21);
					res=1;
				}
			}

			if(res==0){group_list.push_back(group_id);
			admin_groups.insert({ group_id, client_id });
			grp_members[group_id].insert(client_id);
			client_group_id = input_array[1];
			write(client_socket, "Group Successfully Created", 35);
			}
			}

		}
		else if (input_array[0] == "join_group")
		{
			if (input_array.size() != 2)
			{
				write(client_socket, "Invalid Arguments", 18);
			}

			else{string group_id = input_array[1];
			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "Wrong Group ID", 14);
			}
			else if (grp_members[group_id].find(client_id) != grp_members[group_id].end())
			{
				write(client_socket, "You are already present in group", 32);
			}
			else
			{
				grp_requests[group_id].insert(client_id);
				write(client_socket, "Group request successfully sent", 31);
			}
			}
		}
		else if (input_array[0] == "list_requests")
		{
			if (input_array.size() != 2)
			{
				write(client_socket, "Invalid Arguments", 18);
			}

			else{string group_id = input_array[1];
			string requests = "";

			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "No group found*$*", 18);
			}

			if (admin_groups[group_id] != client_id)
			{
				write(client_socket, "You are not admin*$*", 20);
			}
			else if (grp_requests[group_id].size() == 0)
			{
				write(client_socket, "No requests*$*", 15);
			}
			else
			{
				for (auto i = grp_requests[group_id].begin(); i != grp_requests[group_id].end(); i++)
				{
					requests += string(*i) + "*$*";
				}

				write(client_socket, &requests[0], requests.length());
			}
			}
		}
		else if (input_array[0] == "leave_group")
		{
			if (input_array.size() != 2)
			{
				write(client_socket, "Invalid Arguments", 18);
			}
			else{
			string group_id=input_array[1];
			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "No group found", 14);
			}
			
			else if (grp_members[group_id].find(client_id) != grp_members[group_id].end())
			{
				int res=0;
				if (admin_groups[group_id]!= client_id)
				{
				grp_members[group_id].erase(client_id);
				cout<<"";
				write(client_socket, "Group left succesfully", 23);
				int vart=0;
				}
				else{
				write(client_socket, "You are admin, you cannot leave", 35);
				}
			}
			else
			{
				int vart=-1;
				if(vart>0)
				{
					printf("\n");
				}
				vart+=1;
				write(client_socket, "You are not part of group", 25);
			}
			}
		}
		else if (input_array[0] == "list_groups")
		{  
			int vart=-1;
			if (input_array.size() != 1)
			{
				if(vart>0)
				{
					cout<<"Invalid arguments"<<endl;
				}
				write(client_socket, "Invalid Arguments", 18);
				vart+=1;
			}

			else if (group_list.size() == 0)
			{
				write(client_socket, "No groups avail*$*", 18);
			}

			else{string groups = "";
			for (int i = 0; i < group_list.size(); i++)
			{  
				while(i<0)
				{
					cout<<groups;
					cout<<endl;

				}
				groups += group_list[i] + "*$*";
			}
			if(vart==-9999)
				printf("Done");

			write(client_socket, &groups[0], groups.length() + 1);
			}

		}
		else if (input_array[0] == "list_files")
		{
			if (input_array.size() != 2)
			{
				write(client_socket, "Invalid Arguments", 18);
			}

			else{string group_id = input_array[1];
			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "No group found*$*", 18);
			}
			else if (uploadList[group_id].size() != 0)
			{
				string list_files_reply = "";
				for (auto i: uploadList[group_id])
				{
					list_files_reply += i.first + "*$*";
				}

				write(client_socket, &list_files_reply[0], list_files_reply.length());

			}
			else
			{
				write(client_socket, "No files found*$*", 18);
			}
			}
		}
		else if (input_array[0] == "stop_share")
		{
			if (input_array.size() != 3)
			{
				write(client_socket, "Invalid Arguments", 17);
			}
			else{
			string group_id = input_array[1];
			string file_name = input_array[2];
			if (admin_groups.find(group_id) == admin_groups.end())
			{
				write(client_socket, "No group found", 13);
			}
			else if (uploadList[group_id].find(file_name) != uploadList[group_id].end())
			{
				uploadList[group_id][file_name].erase(client_id);
				write(client_socket, "Stopped sharing the file from group", 35);
				if (uploadList[group_id][file_name].size() == 0)
				{
					uploadList[group_id].erase(file_name);
				}
			}
			else
			{
				write(client_socket, "No file shared in the group", 28);
			}
			}
		}
		else if (input_array[0] == "show_downloads")
		{
			write(client_socket, "Downloads", 10);
		}
		else
		{
			write(client_socket, "Invalid Command", 16);
		}
	}

	close(client_socket);
}

int main(int argc, char *argv[])
{
	int socket_id;
	struct sockaddr_in server_addr;
	//tracker_ip = argv[1];
	pthread_t close_thread;
	pair<int,string> p=getIPAndPortFromFileName(argv[1]);
	tracker_ip = p.second;
	tracker_port = p.first;//stoi(argv[4]);
	//tracker_port = stoi(argv[2]);
	int opt = 1;
	int addrlen = sizeof(server_addr);

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		printf("socket creation failed\n");
		return -1;
	}

	cout << "Tracker socket created.\n";

	if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		printf("setsockopt of tracker\n");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	int vart;
	server_addr.sin_port = htons(tracker_port);
	if (inet_pton(AF_INET, &tracker_ip[0], &server_addr.sin_addr) <= 0)
	{
		
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}
	if (bind(socket_id, (SA*) &server_addr, sizeof(server_addr)) < 0)
	{
		printf("Binding failed\n");
		return -1;
	}


	printf("\n");	printf("Binding completed.\n");

	//queue_size of 5
	if (listen(socket_id, 5) < 0)
	{
		printf("listen failed\n");
		return -1;
	}

	printf("Tracker Listening for clients\n");

	vector<thread> threads;

	//checks for command quit
	if (pthread_create(&close_thread, NULL, close_server, NULL) == -1)
	{
		printf("pthread creation error\n");
		return -1;
	}

	while (true)
	{
		int client_socket;

		if ((client_socket = accept(socket_id, (SA*) &server_addr, (socklen_t*) &addrlen)) < 0)
		{
			printf("Acceptance error during connection of client\n");
		}
       printf("\n");
	   string str;
		printf("Connection of client Accepted");
		printf("\n");

		//adding all clients to tracker in while loop
		threads.push_back(thread(connection, client_socket));
	}


	//making sure all threads are executed
	for (auto i = threads.begin(); i != threads.end(); i++)
	{
		if (i->joinable())
		{
			strt;
			i->join();
		}
		
	}

	printf("Exit\n");
	return 0;
}
