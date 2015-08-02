#include <myunp.h>
#include <x.pb.h>
using namespace std;
using namespace x;
char buf[MAXLINE];
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    connect_to_server(sockfd, servaddr);
    person p;
    p.set_name("yx");
    p.set_id(2);
    p.set_email("xy300007@163.com");
    string str;
    p.SerializeToString(&str);
    memset(buf, 0, MAXLINE);
    sprintf(buf,"%s",str.c_str());
    cout<<buf<<endl;
    //*************发送数据*************
    write(sockfd,  buf, strlen(buf));
    //***********************************
    person x;
    x.ParseFromString(str);
//    cout<<str<<endl; // 这里的输出将是tom，说明反序列化正确
//    cout<<x.name()<<endl; // 这里的输出将是tom，说明反序列化正确
//    cout<<x.id()<<endl; // 这里的输出将是tom，说明反序列化正确
//    cout<<x.email()<<endl; // 这里的输出将是tom，说明反序列化正确
    return 0;
}
