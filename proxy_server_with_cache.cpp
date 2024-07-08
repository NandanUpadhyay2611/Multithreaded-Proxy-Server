#include<proxy_parse.h>
#include <cstdlib>
#include<iostream>
#include<time.h>
#include<string>
#include<mutex>
#include<semaphore.h>
#include<thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;


#define MAX_CLIENTS 10
#define MAX_BYTES 4096;

#include <mutex>
#include <condition_variable>

// class Semaphore {// making custom semaphore cause its not supported in older version of cpp
// public:
//     Semaphore(int count = 0) : count_(count) {}

//     void acquire() {
//         std::unique_lock<std::mutex> lock(mutex_);
//         cv_.wait(lock, [this]() { return count_ > 0; });
//         --count_;
//     }

//     void release() {
//         std::lock_guard<std::mutex> lock(mutex_);
//         ++count_;
//         cv_.notify_one();
//     }

// private:
//     int count_;
//     std::mutex mutex_;
//     std::condition_variable cv_;
// };

struct cache_element{  //structure for cache
    string data;
    int len;
    string url;
    time_t lru_time_track;
    cache_element* next;
};

// declaring functions will initiallize later
cache_element* find(string url);
int add_cache_element(string data,int size,string url);
void remove_cache_element();

int port=8080;
int proxy_socketId; //making socket for server and client
thread tid[MAX_CLIENTS];  //we are going to handle multiple requests at a time so we use different thread for different req

sem_t semaphore; //more than 1 threads are going to share the lru cache so for avoiding race condition
mutex mtxLock;  //automatically initiallized unlike in c


cache_element* head;
int cacheSize;

int connectRemoteServer(char* hostAddr,int portNo){
    int remoteSocketId=socket(AF_INET,SOCK_STREAM,0);
    if(remoteSocketId<0){
    cerr<<"Failed creating a socket\n";
}

} 


int handleRequest(int clientSocketId,ParsedRequest* request,char* tempReq){
    char *buff=(char *)malloc(sizeof(char)*MAX_BYTES);
    strcpy(buff,"GET ");
    strcat(buff,request->path);
    strcat(buff," ");
    strcat(buff,request->version);
    strcat(buff,"\r\n");  //This sequence ("\r\n") is used to terminate the HTTP request line
    size_t len=strlen(buff);

    if(ParsedHeader_set(request,"Connection","close")<0){
        cerr<<"Set header key is not working\n";
    }
    if(ParsedHeader_get(request,"Host")==NULL){
        if(ParsedHeader_set(request,"Host",request->host)<0){
            cerr<<"Set Host header key is not working\n";
        }
    }

    if(ParsedRequest_unparse_headers(request,buff+len,(size_t)MAX_BYTES-len)<0){ //attempts to serialize the HTTP headers from the request object into the buffer buff starting from the position buff + len.
        cout<<"Unparse failed\n";
    }

    int serverPort=80;
    if(request->port!=NULL){
        serverPort=atoi(request->port);
    }

    int remoteSocketId=connectRemoteServer(request->host,serverPort); //socket of the original server to where request is made to like google.com
}




void* threadFn(void* socketNew){
    sem_wait(&semaphore);
    int p;
    sem_getvalue(&semaphore,p);
    cout<<"Semaphore value is: "<<p;

//we might get socketNew of  any type so we are type casting it in integer pointer
    int* t=(int*) socketNew;

    //then dereferencing;
    int socket=*t; // represents socket file descriptor

    int bytesSendClient,len;

    char* buffer=(char*)calloc(MAX_BYTES,sizeof(char)); //initiallize buffer to store data recieved from the client
    bytesSendClient=recv(socket,buffer,MAX_BYTES,0);  // recieving data from socket(client) to buffer //recv is a blocking call that waits for data to be received.


//while loop to read data
    while(bytesSendClient>0){
        len=strlen(buffer);
        if(strstr(buffer,"\r\n\r\n")==NULL){ //his loop continues receiving data until the end of the HTTP header is found (\r\n\r\n).
            bytesSendClient=recv(socket,buffer+len,MAX_BYTES-len,0);
        }
        else{
            break;
        }
    }

    char* tempReq=(char *)malloc(strlen(buffer)*sizeof(char)+1);
//Copies the received data into tempReq for further processing.
    for(int i=0;i<strlen(buffer);i++){
        tempReq[i]=buffer[i];
    }

    cache_element* temp=find(tempReq); //search for the request in linked list of lru
    if(temp!=NULL){//if found
        int size=temp->len/sizeof(char);
        int pos=0;
        char response[MAX_BYTES];

        while(pos<size){
            for(int i=0;i<MAX_BYTES;i++){
                response[i]=temp->data[i];
                pos++;

            }
            
            send(socket,response,MAX_BYTES,0);
        }

        cout<<"Data retrieved from the cache\n";
        cout<<"\n\n"<<response;
    }

    // if not present in cache
    else if(bytesSendClient>0){
        len=strlen(buffer);
        ParsedRequest* request=ParsedRequest_create();  //using library to get header ,method,host,protocol etc

        if(ParsedRequest_parse(request,buffer,len)<0){
            cerr<<"Parsing failed\n";
        }
        else{
            if(!strcmp(request->method,"GET")){
                if(request->host && request->path && checkHTTPversion(request->version)==1){
                    bytesSendClient=handleRequest(socket,request,tempReq);
                
                if(bytesSendClient==-1){
                    sendErrorMessage(socket,500);
                }
            }
            else{
                sendErrorMessage(socket,500);
            }
        }
        else{
            cout<<"This code doesnt support any method apart from GET\n";
        }
    }
    ParsedRequest_destroy(request);
    
    }
    else if(bytesSendClient==0){
        cout<<"Client is disconnected\n";
    }
    shutdown(socket,SHUT_RDWR);
    close(socket);
    free(buffer);
    sem_post(&semaphore);
    sem_getvalue(&semaphore,p)
    cout<<"Semaphore post value is "<<p<<endl;
    free(tempReq);
    return NULL;
}

int main(int argc,char* argv[]){

 sem_init(&semaphore, 0, MAX_CLIENTS);
    int clientSocketId,clientLen;

     struct sockaddr_in serverAddr, clientAddr;

if(argv==2){
    port=atoi(argv[i]);  // ./proxy 8080 when this command will be entered in terminal
}
else{
    cout<<"To few arguments\n";
    exit(1);
}

cout<<"Starting proxy server at port"<<port<<"\n";

//create a socket (for server)
proxy_socketId=socket(AF_NET,SOCK_STREAM,0);

if(proxy_socketId<0){
    cerr<<"Failed creating a socket\n";
}

int reuse=1;  //we will be reusing the same global socket
if(setsockopt(proxy_socketId,SOL_SOCKET,SO_REUSEADDR,(const char*)&reuse,sizeof(reuse))<0){
    cerr<<"SetSockOpt failed\n";
};



    // Bind the socket to an ip address and port
   
    serverAddr.sin_family= AF_INET; //IPV4
    serverAddr.sin_port= htons(8080); //port number
    serverAddr.sin_addr.s_addr=INADDR_ANY; // Bind to any available interface

    if(bind(proxy_socketId, (struct sockaddr *)&serverAddr,sizeof(serverAddr))==-1){
        std::cerr << "Error binding socket\n";
        close(proxy_socketId);
        exit(1);
    }

    cout<<"Binding on port "<<port<<endl;

       // Listen for incoming connections

       int listenStatus=listen(proxy_socketId,MAX_CLIENTS);
       if(listenStatus<0){
        cerr<<"error listening\n";
        exit(1);
       }

       cout << "Server is listening on port 8080" <<endl;


    // forr keeping ann account of how many sockets connected to your proxy
        int i=0;
        //each socket connected have their int returning just like fd
        int connectedSocketId[MAX_CLIENTS];

//opening socket for client
        while(1){
            clientLen=sizeof(clientAddr);
            //accepting data or req from client
            clientSocketId=accept(proxy_socketId,(struct sockaddr *)&clientAddr,(socklen_t*)&clientLen);

            if(clientSocketId<0){
                cerr<<"Error connecting client to server\n";
                exit(1);
            }
            else{
                connectedSocketId[i]=clientSocketId;
            }

        // converting the client's IP address from its internal binary format to a human-readable string and printing it along with the port number.
            struct sockaddr_in * clientPtr=(struct sockaddr_in *)&clientAddr;
            struct in_addr ipAddr=clientPtr->sin_addr;
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&ipAddr,str,INET_ADDRSTRLEN); //converts the IP address from its binary form (network byte order) to a human-readable string form.
            cout<<"Client connected with port number: "<<ntohs(clientAddr.sin_port)<<" and I.P address is: "<<str;


// creates a new thread for each connected client and runs thread fun withe connectedSocketId parameter
        tid[i]=thread(threadFn,(void *)connectedSocketId[i]);

            i++;
        }

        close(proxy_socketId);
        return 0;

}