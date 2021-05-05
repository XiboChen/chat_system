#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


#include <fstream>
#include <string>
#include <vector>
#include <map>



using namespace std;


namespace chat{

class server{

public:

int init();
void loop();
void command(string command, int client_socket);


private:
void ip_address();
void port_number();
void split_string(const std::string& s, std::vector<std::string>& v, const std::string& c);
void client_config();
void login(string username,int client_socket_no);
void logout(string username,int client_socket_no);
void chat(string from_username,string to_username,string message);
void chat_all(string from_username, string message);

private:
void *thread_handler(void *socket);
int server_socket_;
struct sockaddr_in client_address_;
string ip_;
string port_;
std::map<string,int> user_list_;



};





}


/**********Initailize the socket from server to client*****************/
int chat::server::init(){
    //create the socket

    //ip address
    ip_address();

    //init socket
    int socket_server;
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr<<"Can't get socket"<<std::endl;
        exit(EXIT_FAILURE);
    }

    //get port number
    port_number();

    // bind socket and address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = stoi(port_);
    addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    if (::bind(socket_server,(struct sockaddr *)&addr,sizeof(addr))< 0) {
        std::cerr<<"Bind error"<<std::endl;
        exit(EXIT_FAILURE);
    }

    cout<<"Server IP:"<<inet_ntoa(addr.sin_addr)<<endl;
    cout<<"Server Port:"<<addr.sin_port<<endl;


    //listen for client
    if (listen(socket_server, 5) < 0) {
         std::cerr<<"Listen error"<<std::endl;
         exit(EXIT_FAILURE);
    }
    
    cout<<"Init_socket finished!"<<endl;
    client_config();

    server_socket_=socket_server;

    return socket_server;
}

//get the ip address
void chat::server::ip_address(){

    ip_="Unable to get IP Address";
    struct ifaddrs *interfaces = NULL;
    // retrieve the current interfaces - returns 0 on success
    if (!getifaddrs(&interfaces)) {
        // Loop through linked list of interfaces
        while(interfaces != NULL) {
            if(interfaces->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if(strcmp(interfaces->ifa_name, "en0")){
                    ip_=inet_ntoa(((struct sockaddr_in*)interfaces->ifa_addr)->sin_addr);
                }
            }
            interfaces = interfaces->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
}

//read port number
void chat::server::port_number(){
   
    // string str;
    // vector<string>config_result;
    ifstream config_info("./server_file/configration_file");
    if(!config_info.is_open()) {
        cout << "Cannot open confige file.\n";
        exit(EXIT_FAILURE);
    }
    
    string line;
    while(getline(config_info,line)){   
        
        vector<string> config;
        split_string(line,config,":");
        port_=config[1];
    }
    
}

//split string
void chat::server::split_string(const std::string& s, std::vector<std::string>& v, const std::string& c)
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

//write back config file
void chat::server::client_config(){

    ofstream outfile("./client_file/configration_file");
    if(!outfile.is_open()) {
        cout << "Cannot create confige file.\n";
        exit(EXIT_FAILURE);
    }
    string config_info="servhost:"+ip_+"\nservport:"+port_+"\n";
    outfile<<config_info<<endl;
    outfile.close();
}

void chat::server::command(string command,int client_socket){

    vector<string> arguments;
    split_string(command,arguments,"|*|");

    //std::cout<<arguments[0]<<endl;

    if(arguments[0]=="login"){
        login(arguments[1],client_socket);
    }
    else if(arguments[0]=="logout"){
        logout(arguments[1],client_socket);
    }

    else if(arguments[0]=="chat"){
        arguments[2].erase(arguments[2].begin());
        chat(arguments[1],arguments[2],arguments[3]);
    }

    else if(arguments[0]=="chat_all"){
        chat_all(arguments[1],arguments[2]);
    }

}

void chat::server::login(string username,int client_socket){

    //username.pop_back();
    //std::cout<<"login: "<<"user: "<<username<<" client: "<<client_socket<<std::endl;
    user_list_.emplace(username,client_socket);
    string login_info="logged in";
    ::send(client_socket,login_info.c_str(),(int)login_info.size(),0);

}

void chat::server::logout(string username,int client_socket){

    map<string,int>::iterator it;
    it=user_list_.find(username);
    user_list_.erase(it);

    string logout_info="logged out";
    ::send(client_socket,logout_info.c_str(),(int)logout_info.size(),0);
    close(client_socket);

}

void chat::server::chat(string from_username, string to_username,string message){



    map<string,int>::const_iterator it;
    
    it=user_list_.find(from_username);
    int from_client_socket=it->second;

    it=user_list_.find(to_username);
    int to_client_socket=it->second;

    string chat_to_info="@"+from_username+">>"+message;
    int sending_re=::send(to_client_socket,chat_to_info.c_str(),(int)chat_to_info.size(),0);
    
    if(sending_re>0){
       cerr<<"Sending message to users success, bytes: "<< sending_re<<endl;
    }
    else{
       cerr<<"Sending message to users error"<<endl;
    }


    string chat_back_info="send success!";
    sending_re=::send(from_client_socket,chat_back_info.c_str(),(int)chat_back_info.size(),0);
        if(sending_re>0){
       cerr<<"Sending message back to users success, bytes: "<< sending_re<<endl;
    }
    else{
       cerr<<"Sending message back to users error"<<endl;
    }

}

void chat::server::chat_all(string from_username, string message){
    
    map<string,int>::const_iterator it; 
    it=user_list_.find(from_username);
    int from_client_socket=it->second;

    string chat_to_info="@"+from_username+">>"+message;
    for(auto user: user_list_){
        if(user.first!=from_username){
            int sending_re=::send(user.second,chat_to_info.c_str(),(int)chat_to_info.size(),0);
            if(sending_re>0){
                cerr<<"Sending message to users success, bytes: "<< sending_re<<endl;
            }
            else{
                cerr<<"Sending message to users error"<<endl;
            }
    
        }
    }

    string chat_back_info="send success!";
    int sending_re=::send(from_client_socket,chat_back_info.c_str(),(int)chat_back_info.size(),0);
        if(sending_re>0){
       cerr<<"Sending message back to users success, bytes: "<< sending_re<<endl;
    }
    else{
       cerr<<"Sending message back to users error"<<endl;
    }

    
}
