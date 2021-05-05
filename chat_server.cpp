#include <iostream>
#include "server.h"
#include<signal.h>
#include <unistd.h>


using namespace std;
int server_socket;
struct sockaddr_in client_address;
chat::server s;

void signal_handler(int sig_num);
void *thread_handler(void *socket);

int main(int argc, char *argv[]){
    server_socket=s.init();
    signal(SIGINT,signal_handler);
    int sock;
    std::vector<pthread_t> pthread_id;
    int thread_num = 0;
    int ret;

    while(1){
        socklen_t len = sizeof(struct sockaddr_in);
        sock = ::accept(server_socket, (struct sockaddr*)&client_address, &len);
        if(sock < 0){
            cerr<<"Accept error"<<endl;
            continue;
        }

        cout<<"get a client, ip:"<<inet_ntoa(client_address.sin_addr)<<","<<"port:"<<ntohs(client_address.sin_port)<<endl;
        
        pthread_t temp;
        ret = pthread_create(&temp, NULL, thread_handler, &sock);
        pthread_id.push_back(temp);

        if (ret != 0){
            cerr<< "create thread failed!"<<endl;
            close(sock);
        }else{
           cout<< "create thread["<<thread_num<<"] success!"<<endl;
        }
        thread_num++;

    }
    


}

void signal_handler(int sig_num){   
    shutdown(server_socket,SHUT_RDWR);
    close(server_socket);
    cout<<endl<<"Server is terminated"<<endl;
    exit(EXIT_SUCCESS);
}

void *thread_handler(void *socket){
    int client_socket_no = *((int*)socket);
    int socket_length;
    char  receive_buff[256];
    
    while(1){
        //::bzero(receive_buff, 256);
        memset(receive_buff, 0, sizeof(receive_buff));
        socket_length=(int)::recv(client_socket_no, receive_buff, 256, 0);
        
        if(socket_length > 0){
          
            string command=receive_buff;
            //command.pop_back();
            cout<<"Receive message from client:"<<command<<endl;
            s.command(command,client_socket_no);
        }

        else if(socket_length == -1){
            cout<<"Thread terminated"<<endl;
            break;
        } 
        else{
            cout<<"socket error: "<<socket_length<<endl;
            return NULL;
        }
        //cout<<"273["<<client_socket_no<<"]"<<endl;
    }
    
    
    return NULL;
}