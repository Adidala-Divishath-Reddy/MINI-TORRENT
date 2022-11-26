#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <fcntl.h>
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
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

#define MAX_CHUNK_SIZE 524288
#define SIZE 32768
#define SA struct sockaddr
string FileName, tracker_ip, peer_ip;
int check_login = 0, check_sha = 1;
uint16_t tracker_port, peer_port;
unordered_map<string, string> file_to_path;

// man



vector<string> splitString(string stringS, string x)
{
	string temp1, temp;
	size_t position = 0;
	vector<string> ans;
	temp1 = " ";
	while ((position = stringS.find(x)) != string::npos)
	{
		temp = stringS.substr(0, position);
		temp1 = temp1 + "a";
		ans.push_back(temp);
		stringS.erase(0, position + x.length());
	}

	ans.push_back(stringS);
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

// man
long long file_size(char *path)
{
	FILE *fp;
	string t = "";
	long size = -1;
	FILE *file_pointer = fopen(path, "rb");
	fp = NULL;
	size = -1;
	if (file_pointer)
	{
		fseek(file_pointer, 0, SEEK_END);
		
		size = ftell(file_pointer) + 1;
		fclose(file_pointer);
	}
	else
	{
		printf("File not found.\n");

		return -1;
	}

	return size;
}

// class file_of_peer
// {
// 	public:
// 	string serverPeerIP;
// 	string filename;
// 	long long int filesize;
// 	file_of_peer(int p, string x, string y)
// 	{
// 		serverPeerIP = x;
// 		filesize = p;
// 		filename = x;
// 	}
// };

typedef struct peerFileDetails
{
	string serverPeerIP;
	string filename;
	long long filesize;
} peerFileDetails;

unordered_map<string, vector<int>> chunk_info;
vector<string> pieceSha;
unordered_map<string, string> downloads;
vector<vector<string>> current_chunks;
unordered_map<string, unordered_map<string, bool>> upload_list;

void singlehash(string &hash, string stringS)
{
	unsigned char array[20];
	if (SHA1(reinterpret_cast<const unsigned char *>(&stringS[0]), stringS.length(), array))
	{
		for (int i = 0; i < 20; i++)
		{
			char current[3];
			sprintf(current, "%02x", array[i] & 0xff);
			hash += string(current);
		}
	}
	else
	{
		printf("Error in hashing\n");
	}

	hash += "*$*";
}

// ma
string combinehash(char *file_path)
{
	long long abc_size = file_size(file_path);
	vector<string> v;
	string geth = "", stringS = "", test = "";
	int custom, curt;

	FILE *file_pointer = fopen(file_path, "r");
	int curret_seg = (abc_size / MAX_CHUNK_SIZE) + 1;
	char poll[SIZE + 1];

	if (file_pointer)
	{
		for (int i = 1; i <= curret_seg; i++)
		{
			custom = 0;
			while (custom < MAX_CHUNK_SIZE && (curt = fread(poll, 1, min(SIZE - 1, MAX_CHUNK_SIZE - custom), file_pointer)))
			{
			
				poll[curt] = '\0';
				custom += strlen(poll);

				stringS += poll;
				memset(poll, 0, sizeof(poll));
			}

			singlehash(geth, stringS);
		}

		fclose(file_pointer);
	}
	else
	{
		printf("File not found.\n");
	}

	for (int p = 0; p < 3; p++)
	{
		geth.pop_back();
	}
	return geth;
}

void bitvector(string filename, long long int low, long long int high, int last)
{
	if (last == 0)
	{
		chunk_info[filename][low] = 1;
	}
	else
	{
		vector<int> tmp(high - low + 1, 1);
		chunk_info[filename] = tmp;
	}
}

// class chunk_details
// {
// 	public:
// 	string serverPeerIP;
// 	string filename;
// 	long long int chunkNum;
// 	string destination;
// 	chunk_details(long long int p, string x, string y, string z)
// 	{
// 		chunkNum = p;
// 		filename = x;
// 		serverPeerIP = y;
// 		destination = z;
// 	}
// };

typedef struct reqdChunkDetails
{
	string serverPeerIP;
	string filename;
	long long chunkNum;
	string destination;
} reqdChunkDetails;

string peer_connection(char *PeerP, char *PortI, string scumm)
{
	uint16_t peerPort = stoi(string(PortI));
	int socket_peer = 0;
	struct sockaddr_in current_peer_serv_addr;

	if ((socket_peer = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return "error";
	}
	//cout << "Socket Created to peer" << endl;
	current_peer_serv_addr.sin_family = AF_INET;
	current_peer_serv_addr.sin_port = htons(peerPort);
	string d="";
	if (inet_pton(AF_INET, PeerP, &current_peer_serv_addr.sin_addr) < 0)
	{
		cout<<"ERROR:";
		perror("Peer Connection Error(INET)");
	}
	if (connect(socket_peer, (struct sockaddr *)&current_peer_serv_addr, sizeof(current_peer_serv_addr)) < 0)
	{
		cout<<"ERROR";
		perror("Peer Connection Error");
	}
	//cout << "Connected to peer" << endl;

	string command_xx = splitString(scumm, "*$*").front();

	if (command_xx == "current_path_file")
	{
		if (send(socket_peer, &scumm[0], strlen(&scumm[0]), MSG_NOSIGNAL) == -1)
		{
			printf("Error in socket peer in file_path\n");
			return "error";
		}
		//cout << "Sent command to peer - " << command_xx << endl;
		char reply_back[10240] = {0};
		if (read(socket_peer, reply_back, 10240) < 0)
		{
			printf("error in socket reading in current_path\n");
			return "error";
		}
		//cout << "server replied - " << string(reply_back) << endl;
		file_to_path[splitString(scumm, "*$*").back()] = string(reply_back);
	}
	else if (command_xx == "current_chunk_vector_details")
	{
		if (send(socket_peer, &scumm[0], strlen(&scumm[0]), MSG_NOSIGNAL) == -1)
		{
			printf("error in socket reading in current_chunk_vector\n");
			return "error";
		}
		//cout << "Sent command to peer - " << command_xx << endl;
		char reply_back[10240] = {0};
		if (read(socket_peer, reply_back, 10240) < 0)
		{
			printf("error in socket reading in current_chunk_vector\n");
			return "error";
		}
		//cout << string(reply_back) << endl;
		close(socket_peer);
		return string(reply_back);
	}
	else if (command_xx == "current_chunk")
	{
		if (send(socket_peer, &scumm[0], strlen(&scumm[0]), MSG_NOSIGNAL) == -1)
		{
			printf("error in socket reading in current_chunk\n");
			return "error";
		}
		//cout << "Sent command to peer - " << command_xx << endl;
		vector<string> tik = splitString(scumm, "*$*");
		string hash = "";
		string despath = tik[3];
		char *filepath = &despath[0];
		long long int chunkNum = stoll(tik[2]);
		int number;
		int techn = 0;
		char current_buff[MAX_CHUNK_SIZE];

		string fuc = "";
		while (techn < MAX_CHUNK_SIZE)
		{
			number = read(socket_peer, current_buff, MAX_CHUNK_SIZE - 1);
			if (number <= 0)
			{
				break;
				cout<<"peer error";
			}

			current_buff[number] = 0;
			fstream outline(filepath, std::fstream::in | std::fstream::out | std::fstream::binary);
			int techie=0;
			outline.seekp(chunkNum * MAX_CHUNK_SIZE + techn, ios::beg);
			techie+=1;
			outline.write(current_buff, number);
			outline.close();
			//cout << "written at" + to_string(chunkNum * MAX_CHUNK_SIZE + techn) << " next" << to_string(number - 1) << endl;
			techn += number;
			fuc += current_buff;
			bzero(current_buff, MAX_CHUNK_SIZE);
		}

		singlehash(hash, fuc);
		for (int p = 0; p < 3; p++)
		{
			hash.pop_back();
		}

		if (hash != pieceSha[chunkNum])
		{
			check_sha = 0;
		}

		vector<string> filename = splitString(string(filepath), "/");
		string file_name = filename.back();
		bitvector(file_name, chunkNum, chunkNum, 0);
		return "done";
	}

	close(socket_peer);
	return "done";
}

int upload_file(vector<string> input_array, int sock)
{
	string name;
	string fileDetails = "";
	name="file";
	char *filepath = &input_array[1][0];
	string filename = splitString(string(filepath), "/").back();
	name+="upload";
	if (upload_list[input_array[2]].find(filename) != upload_list[input_array[2]].end())
	{
		int a=0;
		cout << "File already uploaded" << endl;
		if (send(sock, "error", 5, MSG_NOSIGNAL) == -1)
		{
			printf("Error in uploading\n");
			return -1;
		}
		return 0;
	}
	else
	{
		upload_list[input_array[2]][filename] = true;
		name="upload";
		file_to_path[filename] = string(filepath);
	}

	string piecewiseHash = combinehash(filepath);
	string filehash = "";
	ostringstream charbuf;
	ifstream in(filepath);
	charbuf << in.rdbuf();
	string contents = charbuf.str();
	string omhas = "";

	unsigned char current_file_buffer_hash[32];
	if (!SHA256(reinterpret_cast<const unsigned char *>(&contents[0]), contents.length(), current_file_buffer_hash))
	{
		name="SHA";
		printf("Error in hashing\n");
	}
	else
	{
		for (int i = 0; i < 32; i++)
		{
			char cur_buff[3];
			sprintf(cur_buff, "%02x", current_file_buffer_hash[i] & 0xff);
			omhas += string(cur_buff);
		}
	}

	filehash = omhas;
	string filesize = to_string(file_size(filepath));

	fileDetails += string(filepath) + "*$*";
	name="$";
	fileDetails += string(peer_ip) + ":" + to_string(peer_port) + "*$*";
	name="$++";
	fileDetails += filesize + "*$*";
	fileDetails += filehash + "*$*";
	name="$--";
	fileDetails += piecewiseHash;
	//cout << fileDetails << endl
		 //<< piecewiseHash << endl;

	if (send(sock, &fileDetails[0], strlen(&fileDetails[0]), MSG_NOSIGNAL) == -1)
	{
		printf("Error in uploading\n");
		return -1;
	}

	char reply_back[10240] = {0};
	read(sock, reply_back, 10240);
	cout << reply_back << endl;

	bitvector(filename, 0, stoll(filesize) / MAX_CHUNK_SIZE + 1, 1);

	return 0;
}

void thread_func(peerFileDetails *peer_file_details)
{
	//cout << "check 1" << flush;
	string scumm = "current_chunk_vector_details*$*" + string(peer_file_details->filename);
	vector<string> serverPeerAddress = splitString(string(peer_file_details->serverPeerIP), ":");
	//cout << serverPeerAddress[0] << " " << serverPeerAddress[1] << endl
		 //<< flush;
	string response = peer_connection(&serverPeerAddress[0][0], &serverPeerAddress[1][0], scumm);
	//cout << response << endl;
	//cout << current_chunks.size() << endl;
	for (size_t i = 0; i < current_chunks.size(); i++)
	{
		if (response[i] == '1')
		{
			current_chunks[i].push_back(string(peer_file_details->serverPeerIP));
			strt;
		}
	}

	delete peer_file_details;
}

void thread_func_2(reqdChunkDetails *pr_re)
{
 string filename = pr_re->filename;
	vector<string> peerP = splitString(pr_re->serverPeerIP, ":");
	
	string destination = pr_re->destination;
	long long int chunkNum = pr_re->chunkNum;
	string s= to_string(chunkNum);	
	string scumm = "current_chunk*$*" + filename + "*$*" + to_string(chunkNum) + "*$*" + destination;
	peer_connection(&peerP[0][0], &peerP[1][0], scumm);
	delete pr_re;
	return;
}

void piecewiseAlgo(vector<string> input_array, vector<string> peers)
{
	string fname;
	vector<thread> threads;

	long long int filesize = stoll(peers.back());
	peers.pop_back();
	long long int curret_seg = (filesize / MAX_CHUNK_SIZE) + 1;
	current_chunks.clear();
	current_chunks.resize(curret_seg);

	vector<thread> threads2;
	fname=input_array[0];


	for (size_t i = 0; i < peers.size(); i++)
	{
		peerFileDetails *pf = new peerFileDetails();
		pf->filesize = curret_seg;
		pf->serverPeerIP = peers[i];
		
		int temp = 0;
		thread t;
		pf->filename = input_array[2];
		threads.push_back(thread(thread_func, pf));
	}
	//cout << "ALL THREADS CREATED\n";

	for (auto it = threads.begin(); it != threads.end(); it++)
	{
		if (it->joinable())
			it->join();
	}

	for (size_t i = 0; i < current_chunks.size(); i++)
	{
		int temp = 0;
		if (current_chunks[i].size() == 0)
		{
			if(temp<0)
			break;
			cout << "All parts of the file are not available." << endl;
			temp += 1;
			return;
		}
		temp = 0;
	}
	//cout << "ALL THREADS EXECUTED\n";

	threads.clear();
	srand((unsigned)time(0));
	long long int rec_segments = 0;
	//cout << "After random\n";
	//cout << input_array.size() << endl;
	string des_path = input_array[3] + "/" + input_array[2];
	//cout << des_path << "\n";
	FILE *file_pointer = fopen(&des_path[0], "w");
	if (file_pointer == 0)
	{
		printf("The file already exists.\n");
		fclose(file_pointer);
		return;
	}
	fclose(file_pointer);
	//cout << "Before ss\n";

	string ss(filesize, '\0');

	int t = 0;
	fstream in(&des_path[0], ios::out | ios::binary);
    fname+=to_string(t);
	in.write(ss.c_str(), strlen(ss.c_str()));
	in.close();
	//cout << "After fstream\n";
	chunk_info[input_array[2]].resize(curret_seg, 0);
	check_sha = 1;

	vector<int> tmp(curret_seg, 0);
	chunk_info[input_array[2]] = tmp;

	string peerToGetFilepath;

	while (rec_segments < curret_seg)
	{
		long long int randompiece;
		while (true)
		{
			randompiece = rand() % curret_seg;
			if (chunk_info[input_array[2]][randompiece] == 0)
				break;
		}

		long long int peersWithThisPiece = current_chunks[randompiece].size();
		string randompeer = current_chunks[randompiece][rand() % peersWithThisPiece];
		string dest = input_array[3] + "/" + input_array[2];
		//chunk_details *req = new chunk_details(randompiece, input_array[2], randompeer, dest);
		reqdChunkDetails *req = new reqdChunkDetails();
		req->destination = dest;
		req->serverPeerIP = randompeer;
		req->chunkNum = randompiece;
		req->filename = input_array[2];
		
		chunk_info[input_array[2]][randompiece] = 1;
         thread th;
		threads2.push_back(thread(thread_func_2, req));
		rec_segments++;
        if(fname=="break")
		  break;
		
		peerToGetFilepath = randompeer; //----
	}

	for (auto it = threads2.begin(); it != threads2.end(); it++)
	{
		
		if (it->joinable()){
			it->join();
		}

	}

	if (check_sha == 0)
	{
		if(check_sha!=0)
		{
			cout<<"file is not downloaded";
			cout<<endl;
		}
		cout << "Downloaded completed. The file may be corrupted.";
		printf("\n");
	}
	else
	{
		if(check_sha==0)
		{
			cout << "Downloaded completed.";
		printf("\n");
		}
		cout << "Download completed. No corruption detected." << endl;
	}

	downloads.insert({input_array[2], input_array[1]});
    int chkv=1;
	vector<string> serverAddress = splitString(peerToGetFilepath, ":");
	peer_connection(&serverAddress[0][0], &serverAddress[1][0], "current_path_file*$*" + input_array[2]);
	return;
}

int download_fileile(vector<string> input_array, int sock)
{
	if (input_array.size() != 4)
	{
		return 0;
	}

	int var=1;
	string fileDetails = "";
	fileDetails += input_array[2] + "*$*";
	fileDetails += input_array[3] + "*$*";
	fileDetails += input_array[1];
	}
	
	if (send(sock, &fileDetails[0], strlen(&fileDetails[0]), MSG_NOSIGNAL) == -1)
	{
		printf("Error in downloading\n");
		return -1;
	}

	char reply_back[524288] = {0};
	read(sock, reply_back, 524288);
	//printf("%s\n", reply_back);

	if (string(reply_back) == "File not found")
	{
		cout << reply_back << endl;
		return 0;
	}
	//printf("%s\n", reply_back);
	vector<string> ran = splitString(string(reply_back), "&&&");

	vector<string> peersWithFile = splitString(ran[0], "*$*");

	vector<string> tmp = splitString(ran[1], "*$*");
	// for (int i = 0; i < ran.size(); i++)
	// {
	// 	printf("%s\n", &ran[i][0]);
	// }
	// for (int i = 0; i < peersWithFile.size(); i++)
	// {
	// 	printf("%s\n", &peersWithFile[i][0]);
	// }
	// for (int i = 0; i < tmp.size(); i++)
	// {
	// 	printf("%s\n", &tmp[i][0]);
	// }
	pieceSha = tmp;

	piecewiseAlgo(input_array, peersWithFile);
	return 0;
}

int connection(vector<string> input_array, int sock)
{
	char reply_back[10240];
	bzero(reply_back, 10240);
	read(sock, reply_back, 10240);
	if (string(reply_back) == "Invalid Arguments")
	{
		cout<<"Here";
		cout << reply_back << endl;
		return 0;
	}

	// if (input_array[0] == "create_user")
	// {
	// 	cout << (reply_back) << "\n";
	// }

	if (input_array[0] == "login")
	{
		if (string(reply_back) == "Login Successful")
		{
			check_login = 1;
			cout << reply_back << endl;
			string peerAddress = peer_ip + ":" + to_string(peer_port);
			write(sock, &peerAddress[0], peerAddress.length());
		}
		else
		{
			cout << reply_back << endl;
		}
	}
	else if (input_array[0] == "logout")
	{
		check_login = 0;
		cout << reply_back << endl;
	}
	else if (input_array[0] == "create_group")
	{
		cout << reply_back << endl;
	}
	else if (input_array[0] == "leave_group")
	{
		cout << reply_back << endl;
	}
	else if (input_array[0] == "accept_request")
	{
		cout << reply_back << endl;
	}
	else if (input_array[0] == "join_group")
	{
		cout << reply_back << endl;
	}
	else if (input_array[0] == "upload_file")
	{
		if (string(reply_back) != "Uploading")
		{
			cout << reply_back << endl;
			return 0;
		}

		cout << reply_back << endl;
		return upload_file(input_array, sock);
	}
	else if (input_array[0] == "download_file")
	{
		if (string(reply_back) != "Downloading")
		{
			cout << reply_back << endl;
			return 0;
		}

		cout << reply_back << endl;
		int var=-1;
		if (downloads.find(input_array[2]) != downloads.end())
		{
   
			cout << "File already downloaded" << endl;
			var=-1;
			return 0;
		}
    
		return download_fileile(input_array, sock);
	}
	else if (input_array[0] == "list_groups")
	{
		string fname;
		fname="efile";
		vector<string> grps = splitString(string(reply_back), "*$*");
		fname="";
		for (size_t i = 0; i < grps.size() - 1; i++)
		{ 
             if(fname=="file")
			 break;
			cout << grps[i] ;
			cout<<endl;
		}
	}
	else if (input_array[0] == "list_requests")
	{
		vector<string> requests = splitString(string(reply_back), "*$*");
		string strt="";
		for (size_t i = 0; i < requests.size() - 1; i++)
		{  
			cout << requests[i] << endl;
		}
	}
	else if (input_array[0] == "list_files")
	{
		vector<string> listOfFiles = splitString(string(reply_back), "*$*");
		int point;
		for (size_t i = 0; i < listOfFiles.size() - 1; i++)
		{
			cout << listOfFiles[i] << endl;
		}
	}
	else if (input_array[0] == "stop_share")
	{
		cout << reply_back << endl;
		upload_list[input_array[1]].erase(input_array[2]);
	}
	else if (input_array[0] == "show_downloads")
	{
		string t;
		cout << reply_back << endl;
		for (auto i : downloads)
		{
			cout << "[C] " << i.second << " " << i.first << endl;
		}
	}
	else
	{
		cout << reply_back << endl;
	}

	return 0;
}

void handleconnection(int client_socket)
{
	string client_uid = "";
	//cout << "Handle Connection";
	char input_client[1024] = {0};

	if (read(client_socket, input_client, 1024) <= 0)
	{
		close(client_socket);
		return;
	}

	vector<string> input_array = splitString(string(input_client), "*$*");
	//cout << string(input_client) << endl;

	if (input_array[0] == "current_chunk")
	{
		
		string sent = "";
		long long int chuckNum = stoll(input_array[2]);
		string current_file_path = file_to_path[input_array[1]];
		char *filepath = &current_file_path[0];
		std::ifstream file_pointer(filepath, std::ios::in | std::ios::binary);
		file_pointer.seekg(chuckNum * MAX_CHUNK_SIZE, file_pointer.beg);
		char buffer[MAX_CHUNK_SIZE] = {0};
		file_pointer.read(buffer, sizeof(buffer));
		int count = file_pointer.gcount();
		int kbc = send(client_socket, buffer, count, 0);
		if (kbc == -1)
		{
			printf("Error in sending file\n");
			exit(1);
		}

		file_pointer.close();
	}
	else if (input_array[0] == "current_chunk_vector_details")
	{
		cout << "function called curr_vec_de\n";
		string file_name = input_array[1];
		string message = "";
		vector<int> chunk = chunk_info[file_name];
		for (int i : chunk)
		{
			message += to_string(i);
		}
		cout << message << endl;
		char *temp = &message[0];
		send(client_socket, temp, strlen(temp), 0);
	}
	else if (input_array[0] == "current_path_file")
	{
		string current_file_path = file_to_path[input_array[1]];
		write(client_socket, &current_file_path[0], current_file_path.length());
	}

	close(client_socket);
	return;
}

void *server_func(void *arg)
{
	string fname;
	int socket_id;
	struct sockaddr_in server_addr;
	int opt = 1;
	int addrlen = sizeof(server_addr);

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		printf("socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	//cout << "Peer server socket created.\n";
	fname="";

	if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		if(fname=="error")
		{
			exit(EXIT_FAILURE);
		}
		printf("setsockopt of peer server\n");
		exit(EXIT_FAILURE);
	}
    server_addr.sin_port = htons(peer_port);
	server_addr.sin_family = AF_INET;
	
     fname+="\n";
	if (inet_pton(AF_INET, &peer_ip[0], &server_addr.sin_addr) <= 0)
	{
		printf("\n");
		printf("Invalid address/ Address not supported \n");
		exit(EXIT_FAILURE);
	}
   string str;
	if (bind(socket_id, (SA *)&server_addr, sizeof(server_addr)) < 0)
	{
		str+="bind";
		printf("Binding failed");
		printf("\n");
		exit(EXIT_FAILURE);
	}

	printf("Binding completed.\n");
    if(str=="binding done")
		printf("Done");
	if (listen(socket_id, 3) < 0)
	{ str+="listen";
		printf("listen failed\n");
		exit(EXIT_FAILURE);
	}

	printf("PeerServer Listening for clients\n");
	if(str=="binding done")
		printf("Done");
    int client_socket;
	vector<thread> threads;
	
	while (((client_socket = accept(socket_id, (SA *)&server_addr, (socklen_t *)&addrlen))))
	{
        if(str=="exit")
		 return NULL;
		printf("Connection of client Accepted\n");
		if(str=="exit")
		 printf("Done");
		threads.push_back(thread(handleconnection, client_socket));
	}

	//making sure all threads are executed
	for (auto i = threads.begin(); i != threads.end(); i++)
	{
		str+="join";
   
		if (i->joinable())
		{   str;
			i->join();
		}
		else
		{
			if(str=="exit")
			{
				break;
			}
		}
	}

	close(socket_id);
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Invalid number of arguments\n");
		return -1;
	}

	vector<string> pos=splitString(argv[1],":");
	peer_ip = pos[0];
	peer_port = stoi(pos[1]);
	pair<int,string> p=getIPAndPortFromFileName(argv[2]);
	tracker_ip = p.second;
	tracker_port = p.first;//stoi(argv[4]);
	struct sockaddr_in serv_addr;
	int sock = 0;
	thread t;
	pthread_t thread_server;
    string strt;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{  
		strt="socket";
		printf("\n");
		printf(" Socket creation error \n");
		return -1;
	}
    strt="";
	printf("Peer socket created successfully");
	printf("\n");

	if (pthread_create(&thread_server, NULL, server_func, NULL) == -1)
	{
		strt="perror";
		printf("pthread error");
		printf("\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	if(strt=="exit")
	return 0;
	 
	serv_addr.sin_port = htons(tracker_port);

	if (inet_pton(AF_INET, &tracker_ip[0], &serv_addr.sin_addr) <= 0)
	{
		strt;
		printf("\nInvalid address/ Address not supported ");
		printf("\n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Connection failed\n");
		return -1;
	}

	while (true)
	{
		string input, current;
		getline(cin, input);
		vector<string> input_array;

		if (input.length() <= 0)
			continue;

		stringstream stringarray(input);

		while (stringarray >> current)
		{
			input_array.push_back(current);
		}

		if (input_array[0] == "login" && check_login == 1)
		{
			cout << "You already have one active session" << endl;
			continue;
		}

		if (input_array[0] != "login" && input_array[0] != "create_user" && check_login == 0)
		{
			strt="login";
			cout << "Please login / create an account";
			cout<<"\n";
			continue;
		}

		if (send(sock, &input[0], strlen(&input[0]), MSG_NOSIGNAL) == -1)
		{
			strt="error";
			printf("Error from server");
			cout<<"\n";
			return -1;
		}

		connection(input_array, sock);
	}

	close(sock);
	return 0;
}
