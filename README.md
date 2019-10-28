# FTP-Client
Project Title: FTP Client
Goal of the project
Deeply understand FTP (File Transfer Protocol). Complete a FTP client program based on Linux command line terminal.
Basic Requirements of the project
1. The well-known port number (TCP Port 21) of FTP server is used and there is no need to 
input it. The ftp server can be identified by IP address, such as: 
./ftpcli 10.3.255.85
./ftpcli 127.0.0.1
2. FTP client connects FTP server using TCP protocol. It can receive the FTP replies
coming from server and translate it into natural language, and change the user’s 
commands into FTP command before sending.
3. Support user’s authentication by username & password, with hidden password function.
4. Support user’s interactive operation and provide the following commands:
list (ls), directory operation (pwd, cd, mkdir), upload a file (put), download a file (get) , 
delete a file (delete), renaming a file (rename), transfer mode (ascii, binary), and exit
(quit); 
Note: for data connection, it is required to implement passive mode. l Using nc command to understand FTP control commands and replies in both passive 
mode and active mode. (See PPT)
l Using wireshark to understand FTP control commands and replies (in passive mode) 
for those user commands. (See PPT)
5. Be able to handle errors: invalid commands, missing parameters, requested file already 
existed.
6. Detailed designing document and user manual.
7. Detailed annotation of code and nice programming style.
8. Stable and friendly to users.
9. Two persons as a group.
Extension requirements
1. Active mode for data connection;
l Using nc to understand FTP active mode. (See PPT)
2. Resuming transmission from break-point;
3. Limiting transmission data rate;
Environment of the project
1. C language
2. Linux operation system
3. gcc compiler and gdb debug tool
Steps of the project
1. Connect to ftp server using ftp command in Ubuntu OS and use Wireshark to analyze the
FTP commands and replies interaction of FTP protocol.
2. Using nc command in Ubuntu OS, access the FTP server through port 21 and implement
above working flow by manual operations. l In this procedure, you should use the FTP control commands.
l You can connect ftp.mayan.cn. l Passive/active mode can be understood clearly.
3. Abstract main modules according to above operations, design the solution, and divide the 
work into concrete functions.
4. Implement basic TCP socket programming architecture.
5. Parse/encapsulate FTP commands, and provide commands for users.
