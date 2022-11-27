# MINI-TORRENT
Implemented Peer-to-Peer Group Based File Sharing System using C++. 
First user should login and then users can share, download files from the group they belong to.
Download will be parallel with multiple pieces from multiple peers.

# How to Run ?

    Command to compile tracker : g++ tracker.cpp -o tracker -pthread

    Command to compile client : g++ client.cpp -o client -pthread -lcrypto

    Command to run tracker : ./tracker tracker_info.txt

    Command to run client : ./client <IP>:<PORT> tracker_info.txt 
    (port>1000 and ip- loopback addresses)

Note : tracker_info.txt contains the ip, port details of all trackers

# Commands

a. Create User Account: create_user <user_id> <passwd>

b. Login: login <user_id> <passwd>

c. Create Group: create_group <group_id>

d. Join Group: join_group <group_id>

e. Leave Group: leave_group <group_id>

f. List pending join: requests list_requests <group_id>

g. Accept Group Joining Request: accept_request <group_id> <user_id>

h. List All Group In Network: list_groups

i. List All sharable Files In Group: list_files <group_id>

j. Upload File: upload_file <file_path> <group_id>

k. Download File: download_file <group_id> <file_name> <destination_path>

l. Logout: logout

m. Show_downloads: show_downloads
