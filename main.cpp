#include <iostream>
#include <string>
#include <QString>
#include <QTextStream>
#include "funs.h"
using namespace std;

int main()
{
    QString filename;
    QTextStream in(stdin);
    cout<<"Input descriptive xml file: ";
    in>>filename;
    DataStr dat;
    FileCode err;
    cout<<"Read file...";
    if(readFile(dat,filename,err))
        cout<<"Succeed!\n";
    else {
        cout<<"Failed!\n"
           <<"Error code: "<<err<<"\n"
            <<"The transformation terminates.\n";
        system("pause");
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
        system("pause");
        return 3;
    }

    system("pause");
    return 0;
}
