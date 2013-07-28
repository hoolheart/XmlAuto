#include "funs.h"
#include <QFile>

QString endline = "\n";
QString tab="    ";
QString tab4 = tab+tab+tab+tab;
QString timeFormat = "\"yyyy.MM.dd hh:mm:ss.zzz\"";

bool readFile(DataStr &dat, const QString &filename, FileCode &err)
{
    if(!QFile::exists(filename)) {
        err = FILE_NOEXIST;
        return false;
    }
    QFile file(filename);
    if(!file.open(QFile::ReadOnly|QFile::Text)) {
        err = FILE_OPENFAIL;
        return false;
    }
    QXmlStreamReader reader(&file);

    FileCode code = FILE_OK;
    reader.readNext();
    while(!reader.atEnd()) {
        if(reader.isStartElement()) {
            if(reader.name() == "DataStruct") {
                if(!dat.load(reader)) {
                    code = FILE_WROCONTENT;
                    break;
                }
                reader.readNext();
            }
            else {
                code = FILE_WROCONTENT;
                break;
            }
        }
        else
            reader.readNext();
    }

    file.close();

    err = code;
    return (code == FILE_OK);
}

bool transform(DataStr &dat, const QString &filename, FileCode &err)
{
    int dot_index = filename.lastIndexOf('.');
    if (dot_index==-1) dot_index = filename.length();
    QString name = filename.left(dot_index);
    QString name_h = name+".h";
    QString name_cpp = name+".cpp";
    int lastSlash = name.lastIndexOf('\\');
    if (name.lastIndexOf('/')>lastSlash)
        lastSlash = name.lastIndexOf('/');
    name = name.right(name.length()-lastSlash-1);
    QFile file_h(name_h), file_cpp(name_cpp);
    if(!file_h.open(QFile::WriteOnly|QFile::Text|QFile::Truncate)) {
        err = FILE_OPENFAIL;
        return false;
    }
    if(!file_cpp.open(QFile::WriteOnly|QFile::Text|QFile::Truncate)) {
        err = FILE_OPENFAIL;
        return false;
    }
    QTextStream writer_h(&file_h),writer_cpp(&file_cpp);

    writer_h<<"#ifndef "+name.toUpper()+"_H"<<endline
            <<"#define "+name.toUpper()+"_H"<<endline
            <<"#include <QStringList>"<<endline
            <<"#include <QXmlStreamReader>"<<endline
            <<"#include <QXmlStreamWriter>"<<endline;
    if (dat.checkType("timestamp"))
        writer_h<<"#include <QDateTime>"<<endline;
    if (dat.checkType("table"))
        writer_h<<"#include \"ZtTable.h\""<<endline;
    writer_h<<endline;
    writer_cpp<<"#include \""<<name<<".h\""<<endline;

    for(int i=0;i<dat.groupLength();i++)
        writer_h<<"class "<<dat.groupAt(i)->getName()<<";"<<endline;

    for(int i=dat.groupLength();i>0;i--)
        transformGroup(*dat.groupAt(i-1),writer_h,writer_cpp);

    writer_h<<endline<<"#endif"<<endline;

    file_h.close();
    file_cpp.close();
    if(file_h.error()) {
        err = FILE_WRITEFAIL;
        return false;
    }
    if(file_cpp.error()) {
        err = FILE_WRITEFAIL;
        return false;
    }
    err = FILE_OK;
    return true;
}

void transformGroup(Group &p, QTextStream &writer_h, QTextStream &writer_cpp)
{
    writer_h<<endline;
    writer_h<<"class "<<p.getName()<<" {"<<endline;

    writer_h<<"private:"<<endline;
    for(int i=0;i<p.elementLength();i++)
        wrt_h_D(*p.elementAt(i),writer_h);

    writer_h<<"public:"<<endline;
    writer_h<<tab<<p.getName()<<"();"<<endline;
    writer_h<<tab<<"~"<<p.getName()<<"();"<<endline;
    for(int i=0;i<p.elementLength();i++) {
        wrt_h_R(*p.elementAt(i),writer_h);
        wrt_h_W(*p.elementAt(i),writer_h);
    }

    writer_h<<tab<<"void cleanAll();"<<endline;
    writer_h<<tab<<"bool loadXml(QXmlStreamReader &reader);"<<endline;
    writer_h<<tab<<"void saveXml(QXmlStreamWriter &writer);"<<endline;

    writer_h<<"};"<<endline;

    wrt_cpp(p,writer_cpp);
}

void wrt_h_D(Element &e, QTextStream &writer)
{
    int t = e.gettype();
//    if(t<0 || t>7) return;
    writer<<tab;
    switch (t) {
    case 0: // int
        writer<<"int ";
        break;
    case 1: // double
        writer<<"double ";
        break;
    case 2: // string
        writer<<"QString ";
        break;
    case 3: // intlist
        writer<<"QList<int> ";
        break;
    case 4: // doublelist
        writer<<"QList<double> ";
        break;
    case 5: // stringlist
        writer<<"QStringList ";
        break;
    case 6: // complex
        writer<<e.getName()<<" ";
        break;
    case 7: // complexlist
        writer<<"QList<"<<e.getName()<<" *> ";
        break;
    case 8: // boolean
        writer<<"bool ";
        break;
    case 9: // booleanlist
        writer<<"QList<bool> ";
        break;
    case 10: // timestamp
        writer<<"QDateTime ";
        break;
    case 11: // timestamplist
        writer<<"QList<QDateTime> ";
        break;
    case 12: // table
        writer<<"ZtTable ";
        break;
    default:
        break;
    }
    writer<<e.getname()<<";"<<endline;
}

void wrt_h_R(Element &e, QTextStream &writer)
{
    int t = e.gettype();
//    if(t<0 || t>7) return;
    writer<<tab;
    switch (t) {
    case 0: // int
        writer<<"int "<<e.getName()<<"() const "
              <<"{return "<<e.getname()<<";}";
        break;
    case 1: // double
        writer<<"double "<<e.getName()<<"() const "
              <<"{return "<<e.getname()<<";}";
        break;
    case 2: // string
        writer<<"QString "<<e.getName()<<"() const "
              <<"{return "<<e.getname()<<";}";
        break;
    case 3: // intlist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}"<<endline;
        writer<<tab;
        writer<<"int "<<e.getname()<<"At(int i) const "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return 0;}";
        break;
    case 4: // doublelist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}"<<endline;
        writer<<tab;
        writer<<"double "<<e.getname()<<"At(int i) const "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return 0;}";
        break;
    case 5: // stringlist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}"<<endline;
        writer<<tab;
        writer<<"QString "<<e.getname()<<"At(int i) const "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return \"\";}";
        break;
    case 6: // complex
        writer<<e.getName()<<"* "<<e.getName()<<"_data() "
              <<"{return &"<<e.getname()<<";}";
        break;
    case 7: // complexlist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}"<<endline;
        writer<<tab;
        writer<<e.getName()<<"* "<<e.getname()<<"At(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return 0;}";
        break;
    case 8: // boolean
        writer<<"bool "<<e.getName()<<"() const "
             <<"{return "<<e.getname()<<";}";
       break;
    case 9: // booleanlist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}"<<endline;
        writer<<tab;
        writer<<"bool "<<e.getname()<<"At(int i) const "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return false;}";
        break;
    case 10: // timestamp
        writer<<"QDateTime "<<e.getName()<<"() const "
              <<"{return "<<e.getname()<<";}";
        break;
    case 11: // timestamplist
        writer<<"int "<<e.getname()<<"Length() const "
              <<"{return "<<e.getname()<<".length();}";
        writer<<tab;
        writer<<"QDateTime "<<e.getname()<<"At(int i) const "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) return "
              <<e.getname()<<"[i]; else return QDateTime();}";
        break;
    case 12: // table
        writer<<"ZtTable* "<<e.getName()<<"() "
              <<"{return &"<<e.getname()<<";}";
        break;
    default:
        break;
    }
    writer<<endline;
}

void wrt_h_W(Element &e, QTextStream &writer)
{
    int t = e.gettype();
//    if(t<0 || t>7 || t==6) return;
    if(t==6 || t==12) return;
    writer<<tab;
    switch (t) {
    case 0: // int
        writer<<"void set"<<e.getName()<<"(int _n) "
              <<"{"<<e.getname()<<"=_n;}";
        break;
    case 1: // double
        writer<<"void set"<<e.getName()<<"(double _n) "
              <<"{"<<e.getname()<<"=_n;}";
        break;
    case 2: // string
        writer<<"void set"<<e.getName()<<"(const QString &_n) "
              <<"{"<<e.getname()<<"=_n;}";
        break;
    case 3: // intlist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"bool change"<<e.getName()<<"(int i, int _n) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<"[i]=_n;return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"(int _n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    case 4: // doublelist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"bool change"<<e.getName()<<"(int i, double _n) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<"[i]=_n;return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"(double _n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    case 5: // stringlist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"bool change"<<e.getName()<<"(int i, const QString &_n) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<"[i]=_n;return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"(const QString &_n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    case 7: // complexlist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<"delete "<<e.getname()<<"[i];"<<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"("<<e.getName()<<" *_n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    case 8: // boolean
        writer<<"void set"<<e.getName()<<"(bool _n) "
              <<"{"<<e.getname()<<"=_n;}";
        break;
    case 9: // booleanlist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"bool change"<<e.getName()<<"(int i, bool _n) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<"[i]=_n;return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"(bool _n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    case 10: // timestamp
        writer<<"void set"<<e.getName()<<"(const QDateTime &_n) "
              <<"{"<<e.getname()<<"=_n;}";
        break;
    case 11: // timestamplist
        writer<<"bool del"<<e.getName()<<"(int i) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<".removeAt(i);return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"bool change"<<e.getName()<<"(int i, const QDateTime &_n) "
              <<"{if(i>=0 && i<"<<e.getname()<<".length()) {"
              <<e.getname()<<"[i]=_n;return true;} "
              <<"else return false;}"<<endline;
        writer<<tab<<"void add"<<e.getName()<<"(const QDateTime &_n) "
              <<"{"<<e.getname()<<".append(_n);}";
        break;
    default:
        break;
    }
    writer<<endline;
}

void wrt_cpp(Group &p, QTextStream &writer)
{
    writer<<endline;
    writer<<p.getName()<<"::"<<p.getName()<<"()"<<endline;
    writer<<"{cleanAll();}"<<endline<<endline;
    writer<<p.getName()<<"::"<<"~"<<p.getName()<<"()"<<endline;
    writer<<"{cleanAll();}"<<endline;

    writer<<endline;
    writer<<"void "<<p.getName()<<"::"<<"cleanAll()"<<endline;
    writer<<"{"<<endline;
    for(int i=0;i<p.elementLength();i++) {
        Element *e = p.elementAt(i);
        switch(e->gettype()) {
        case 0: // int
        case 1: // double
            writer<<tab<<e->getname()<<"=0;"<<endline;
            break;
        case 2: // string
            writer<<tab<<e->getname()<<"=\"\";"<<endline;
            break;
        case 3: // intlist
        case 4: // doublelist
        case 5: // stringlist
        case 9: // booleanlist
        case 11: // timestamplist
            writer<<tab<<e->getname()<<".clear();"<<endline;
            break;
        case 6: // complex
        case 12: // table
            writer<<tab<<e->getname()<<".cleanAll();"<<endline;
            break;
        case 7: // complexlist
            writer<<tab;
            writer<<"for(int i=0;i<"<<e->getname()<<".length();i++) "
                  <<"delete "<<e->getname()<<"[i];";
            writer<<endline;
            writer<<tab<<e->getname()<<".clear();"<<endline;
            break;
        case 8: // boolean
            writer<<tab<<e->getname()<<"=false;"<<endline;
            break;
        default:
            break;
        }
    }
    writer<<"}"<<endline;

    writer<<endline;
    writer<<"bool "<<p.getName()<<"::loadXml(QXmlStreamReader &reader)"<<endline;
    writer<<"{"<<endline;
    QStringList tmp;
    for(int i=0;i<p.elementLength();i++) {
        int t=p.elementAt(i)->gettype();
        if(Element::typelist[t].right(4)!="list")
            tmp<<"flag"+QString::number(i);
    }
    for(int i=0;i<tmp.length();i++) {
        if(i%3==0) writer<<tab;
        writer<<"bool "<<tmp[i]<<"=false;";
        if(i==tmp.length()-1) writer<<endline;
        else if(i%3==2) writer<<endline;
        else writer<<" ";
    }
    writer<<tab<<"cleanAll();"<<endline<<endline;
    writer<<tab<<"if(reader.isEndElement()) return false;"<<endline;
    QString next="reader.readNext();", fail="cleanAll(); return false;";
    writer<<tab<<next<<endline;
    writer<<tab<<"while(!reader.isEndElement()) {"<<endline;
    writer<<tab<<tab<<"if(reader.isStartElement()) {"<<endline;
    writer<<tab<<tab<<tab;
    for(int i=0;i<p.elementLength();i++) {
        writer<<"if(reader.name()==\""<<p.elementAt(i)->getName()<<"\") {"<<endline;
        wrt_cpp_L(*p.elementAt(i),i,writer);
        writer<<tab4<<next<<endline;
        writer<<tab<<tab<<tab<<"}"<<endline<<tab<<tab<<tab<<"else ";
    }
    writer<<"{"<<fail<<"}"<<endline;
    writer<<tab<<tab<<"}"<<endline;
    writer<<tab<<tab<<"else "<<next<<endline;
    writer<<tab<<"}"<<endline<<endline;
    if(tmp.length()) {
        writer<<tab<<"if("<<tmp.join("&&")<<")"<<endline;
        writer<<tab<<tab<<"return true;"<<endline;
        writer<<tab<<"else {"<<fail<<"}"<<endline;
    }
    else {
        writer<<tab<<"return true;"<<endline;
    }
    writer<<"}"<<endline;

    writer<<endline;
    writer<<"void "<<p.getName()<<"::saveXml(QXmlStreamWriter &writer)"<<endline;
    writer<<"{"<<endline;
    writer<<tab<<"writer.writeStartElement(\""<<p.getName()<<"\");"<<endline;
    writer<<endline;
    for(int i=0;i<p.elementLength();i++)
        wrt_cpp_S(*p.elementAt(i),writer);
    writer<<endline;
    writer<<tab<<"writer.writeEndElement();"<<endline;
    writer<<"}"<<endline;
}

void wrt_cpp_L(Element &e, int i, QTextStream &writer)
{
    QString flag = "flag"+QString::number(i);
    int t = e.gettype();
    switch (t) {
    case 0: // int
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed().toInt();"<<endline;
        break;
    case 1: // double
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed().toDouble();"<<endline;
        break;
    case 2: // string
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed();"<<endline;
        break;
    case 3: // intlist
        writer<<tab4<<e.getname()<<".append(reader.readElementText().trimmed().toInt());"<<endline;
        break;
    case 4: // doublelist
        writer<<tab4<<e.getname()<<".append(reader.readElementText().trimmed().toDouble());"<<endline;
        break;
    case 5: // stringlist
        writer<<tab4<<e.getname()<<".append(reader.readElementText().trimmed());"<<endline;
        break;
    case 6: // complex
    case 12: // table
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<"if(!"<<e.getname()<<".loadXml(reader)) {"<<endline;
        writer<<tab4<<tab<<"cleanAll();"<<endline;
        writer<<tab4<<tab<<"return false;"<<endline;
        writer<<tab4<<"}"<<endline;
        break;
    case 7: // complexlist
        writer<<tab4<<e.getName()<<" *p=new "<<e.getName()<<"();"<<endline;
        writer<<tab4<<"if(p->loadXml(reader))"<<endline;
        writer<<tab4<<tab<<e.getname()<<".append(p);"<<endline;
        writer<<tab4<<"else {"<<endline;
        writer<<tab4<<tab<<"delete p;"<<endline;
        writer<<tab4<<tab<<"cleanAll();"<<endline;
        writer<<tab4<<tab<<"return false;"<<endline;
        writer<<tab4<<"}"<<endline;
        break;
    case 8: // boolean
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<"QString tmp=reader.readElementText().trimmed();"<<endline;
        writer<<tab4<<e.getname()<<"=((tmp==\"true\") || (tmp==\"True\") || (tmp==\"T\") || (tmp==\"1\"));"<<endline;
        break;
    case 9: //booleanlist
        writer<<tab4<<"QString tmp=reader.readElementText().trimmed();"<<endline;
        writer<<tab4<<"bool tmpBool=((tmp==\"true\") || (tmp==\"True\") || (tmp==\"T\") || (tmp==\"1\"));"<<endline;
        writer<<tab4<<e.getname()<<".append(tmpBool);"<<endline;
        break;
    case 10: // timestamp
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
        writer<<tab4<<flag<<"=true;"<<endline;
        writer<<tab4<<e.getname()<<"=QDateTime::fromString(reader.readElementText().trimmed(),"<<timeFormat<<");"<<endline;
        break;
    case 11: // timestamplist
        writer<<tab4<<e.getname()<<".append(QDateTime::fromString(reader.readElementText().trimmed(),"
             <<timeFormat<<"));"<<endline;
        break;
    default:
        break;
    }
}

void wrt_cpp_S(Element &e, QTextStream &writer)
{
    int t=e.gettype();
    switch(t) {
    case 0: // int
    case 1: // double
        writer<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", QString::number("<<e.getname()<<"));"<<endline;
        break;
    case 2: // string
        writer<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", "<<e.getname()<<");"<<endline;
        break;
    case 3: // intlist
    case 4: // doublelist
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<".length();i++)"<<endline;
        writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", QString::number("<<e.getname()<<"[i]));"<<endline;
        break;
    case 5: //stringlist
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<".length();i++)"<<endline;
        writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", "<<e.getname()<<"[i]);"<<endline;
        break;
    case 6: // complex
        writer<<tab<<e.getname()<<".saveXml(writer);"<<endline;
        break;
    case 7: // complexlist
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<".length();i++)"<<endline;
        writer<<tab<<tab<<e.getname()<<"[i]->saveXml(writer);"<<endline;
        break;
    case 8: //boolean
        writer<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", "<<e.getname()<<"?\"True\":\"False\");"<<endline;
        break;
    case 9: // booleanlist
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<".length();i++)"<<endline;
        writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", "<<e.getname()<<"[i]?\"True\":\"False\");"<<endline;
        break;
    case 10: // timestamp
        writer<<tab<<"writer.writeTextElement(\""<<e.getName()
             <<"\", "<<e.getname()<<".toString("<<timeFormat<<"));"<<endline;
        break;
    case 11: //timestamplist
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<".length();i++)"<<endline;
        writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
              <<"\", "<<e.getname()<<"[i].toString("<<timeFormat<<"));"<<endline;
        break;
    case 12: // table
        writer<<tab<<"writer.writeStartElement(\""<<e.getName()<<"\");"<<endline;
        writer<<tab<<e.getname()<<".saveXml(writer);"<<endline;
        writer<<tab<<"writer.writeEndElement();"<<endline;
        break;
    default:
        break;
    }
}
