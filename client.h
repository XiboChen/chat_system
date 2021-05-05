#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>



using namespace std;


namespace chat{

class client{

public:

client();
int init();
void command(string command);
void listen_socket();
void login();
int get_socket();


private:
void server_info();
void split_string(const std::string& s, std::vector<std::string>& v, const std::string& c);
void logout();
void read_input();
void chat();


private:
int client_socket_;
string server_port_;
string server_ip_;
string command_;
string username_;
vector<string> argument_;
fd_set fds_;

};

}

int chat::client::init(){

    server_info();
    int socket_client;
    struct sockaddr_in addr;
    if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr<<"Can't get socket"<<endl;
        exit(EXIT_FAILURE);
    }
    // cout<<"Creat client socket:"<<socket_client<<endl;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port =stoi(server_port_);
    addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());
    
    if (connect(socket_client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr<<"Error in connect"<<endl;
        exit(EXIT_FAILURE);
    }

    client_socket_=socket_client;

    return socket_client;

}

void chat::client::server_info(){

    
    // vector<string>config_result;
    ifstream config_info("./client_file/configration_file");
    if(!config_info.is_open()) {
        cerr << "Cannot open input file."<<endl;
        exit(EXIT_FAILURE);
    }
    string line;
    while(getline(config_info,line)) {
        if(line!=""){
            vector<string> config_result;
            split_string(line,config_result,":");
            if(config_result[0]=="servhost")
                server_ip_=config_result[1];
            else if(config_result[0]=="servport")
                server_port_=config_result[1];
        }
    }
}

void chat::client::split_string(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));
        
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

void chat::client::command(string command){

    
    command.pop_back();
    //cout<<"command: "<<command<<endl;
    command_=command;
    argument_.clear();
    split_string(command_,argument_," ");
    if(argument_[0]=="exit"){
        shutdown(client_socket_,SHUT_RDWR);
        close(client_socket_);
        cout<<endl<<"Client is exit"<<endl;
        exit(EXIT_SUCCESS);
    }
    else if(argument_[0]=="login")
        login();
    else if(argument_[0]=="logout")
        logout();
    else if(argument_[0]=="chat")
        chat();


}

void chat::client::login(){


    init();
    
    username_=argument_[1];

    string login_info="login|*|"+username_;
    int sending_re = (int)::send(client_socket_,login_info.c_str(),(int)login_info.size(),0);

    if(sending_re > 0)
        cerr<<"Sending message success, bytes: "<< sending_re<<endl;
    else{
       cerr<<"Sending message error"<<std::endl;
    }

    char receive_buff[256];
    memset(receive_buff, 0, sizeof(receive_buff));
    int socket_length = (int)::recv(client_socket_, receive_buff, 256, 0);

    if(socket_length > 0){
        cout<<"Login successfully!"<<endl;
    }

    //listen_socket();



}

void chat::client::logout(){
    
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    string logout_info="logout|*|"+username_;
    int sending_re = (int)::send(client_socket_,logout_info.c_str(),(int)logout_info.size(),0);
    if(sending_re > 0)
        cerr<<"Sending message success, bytes: "<< sending_re<<endl;
    else{
       cerr<<"Sending message error"<<endl;
    }

    char receive_buff[256];
    memset(receive_buff, 0, sizeof(receive_buff));
    int socket_length = (int)::recv(client_socket_, receive_buff, 256, 0);

    if(socket_length > 0){
        cout<<"Logout successfully!"<<endl;
    }
    
    close(client_socket_);
    client_socket_=-1;
    //FD_ZERO(&fds);
    
    
}

void chat::client::chat(){
    string chat_info;

    if(argument_[1][0]!='@'){
        string message="";
        for(int i=1;i<argument_.size();i++){
            message+=argument_[i]+" ";
        }
        message.pop_back();
        chat_info="chat_all|*|"+username_+"|*|"+message;
    }

    else{

        string message="";
        for(int i=2;i<argument_.size();i++){
            message+=argument_[i]+" ";
        }
        message.pop_back();

        chat_info="chat|*|"+username_+"|*|"+argument_[1]+"|*|"+message;
    }

    int sending_re = (int)::send(client_socket_,chat_info.c_str(),(int)chat_info.size(),0);
    if(sending_re>0){
       cerr<<"Sending message success, bytes: "<< sending_re<<endl;
    }
    else{
       cerr<<"Sending message error"<<endl;
    }
}

void chat::client::read_input(){

    command_.clear();
    int temp=0;   

    while (temp!='\n') {
        // Read in the char
        temp=getchar();
        command_.push_back(temp);    
    }
    command_.pop_back();
}

void chat::client::listen_socket(){

  
    while(1){


    }

}

int chat::client::get_socket(){
   
    return client_socket_;
}

chat::client::client(){
    client_socket_=-1;
}