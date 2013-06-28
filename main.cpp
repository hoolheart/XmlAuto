#include <iostream>
#include <string>
#include <QString>
#include "funs.h"

int main(int argc, char *argv[])
{
    using namespace std;
    QString filename;
    if(argc<2)
        filename = "test.xml";
    else
        filename = argv[1];
    cout<<"File name is "<<argv[1]<<".\n";
    DataStr dat;
    FileCode err;
    cout<<"Read file...";
    if(readFile(dat,filename,err))
        cout<<"Succeed!\n";
    else {
        cout<<"Failed!\n"
            <<"The transformation terminates.\n";
        return 3;
    }
    cout<<"Creating the corresponding cpp and h files...";
    if(transform(dat,filename,err)) {
        cout<<"Succeed!\n";
        cout<<"The transformation is finished.\n";
    }
    else {
        cout<<"Failed!\n"
            <<"The transformation terminates.\n";
        return 3;
    }

    return 0;
}
