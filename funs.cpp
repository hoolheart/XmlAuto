#include "funs.h"
#include <QFile>

QString endline = "\n";
QString tab="    ";
QString tab4 = tab+tab+tab+tab;
QString timeFormat = "\"yyyy.MM.dd HH:mm:ss.zzz\"";

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
    writer_cpp<<"#include <QFile>"<<endline;

    for(int i=0;i<dat.groupLength();i++)
        writer_h<<"class "<<dat.groupAt(i)->getName()<<";"<<endline;

    for(int i=0;i<dat.groupLength();i++)
        transformGroup(*dat.groupAt(i),writer_h,writer_cpp,i==0);
    //for(int i=dat.groupLength();i>0;i--)
        //transformGroup(*dat.groupAt(i-1),writer_h,writer_cpp);

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

void transformGroup(Group &p, QTextStream &writer_h, QTextStream &writer_cpp, bool mainGroup)
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

    if(mainGroup) {
        writer_h<<tab<<"bool loadFile(QString fileName);"<<endline;
        writer_h<<tab<<"void saveFile(QString fileName);"<<endline;
    }

    writer_h<<"};"<<endline;

    wrt_cpp(p,writer_cpp);

    if(mainGroup) {
        writer_cpp<<endline;
        writer_cpp<<"bool "<<p.getName()<<"::loadFile(QString fileName)"<<endline;
        writer_cpp<<"{"<<endline;
        writer_cpp<<tab<<"if(!QFile::exists(fileName)) return false;"<<endline;//check existence
        writer_cpp<<tab<<"QFile file(fileName);"<<endline;
        writer_cpp<<tab<<"if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) return false;"<<endline;
        writer_cpp<<tab<<"QXmlStreamReader reader(&file);"<<endline;
        writer_cpp<<tab<<"reader.readNext();"<<endline;
        writer_cpp<<tab<<"bool flag=false;"<<endline;
        writer_cpp<<tab<<"while(!reader.atEnd()) {"<<endline;
        writer_cpp<<tab<<tab<<"if(reader.isStartElement()) {"<<endline;
        writer_cpp<<tab<<tab<<tab<<"if(reader.name() == \""<<p.getName()<<"\") {"<<endline;
        writer_cpp<<tab4<<"flag = loadXml(reader);"<<endline;
        writer_cpp<<tab4<<"break;"<<endline;
        writer_cpp<<tab<<tab<<tab<<"}"<<endline;
        writer_cpp<<tab<<tab<<tab<<"else"<<endline;
        writer_cpp<<tab4<<"break;"<<endline;
        writer_cpp<<tab<<tab<<"}"<<endline;
        writer_cpp<<tab<<tab<<"else"<<endline;
        writer_cpp<<tab<<tab<<tab<<"reader.readNext();"<<endline;
        writer_cpp<<tab<<"}"<<endline;
        writer_cpp<<tab<<"file.close();"<<endline;
        writer_cpp<<tab<<"return flag;"<<endline;
        writer_cpp<<"}"<<endline;

        writer_cpp<<endline;
        writer_cpp<<"void "<<p.getName()<<"::saveFile(QString fileName)"<<endline;
        writer_cpp<<"{"<<endline;
        writer_cpp<<tab<<"QFile file(fileName);"<<endline;
        writer_cpp<<tab<<"if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) return;"<<endline;
        writer_cpp<<tab<<"QXmlStreamWriter writer(&file);"<<endline;
        writer_cpp<<tab<<"writer.setAutoFormatting(true);"<<endline;
        writer_cpp<<tab<<"writer.setCodec(\"UTF-8\");"<<endline;
        writer_cpp<<tab<<"writer.writeStartDocument();"<<endline;
        writer_cpp<<tab<<"writer.writeStartElement(\""<<p.getName()<<"\");"<<endline;
        writer_cpp<<tab<<"saveXml(writer);"<<endline;
        writer_cpp<<tab<<"writer.writeEndElement();"<<endline;
        writer_cpp<<tab<<"writer.writeEndDocument();"<<endline;
        writer_cpp<<tab<<"file.close();"<<endline;
        writer_cpp<<"}"<<endline;
    }
}

void wrt_h_D(Element &e, QTextStream &writer)
{
    int t = e.gettype();
    if(t<0 || t>=Element::typelist.size()) return;
    //get type
    QString type = Element::typelist[t];
    switch (t) {
    case 2://string
        type = "QString";
        break;
    case 3://complex
        type = e.getComplexType();
        break;
    case 4://boolean
        type = "bool";
        break;
    case 5://timestamp
        type = "QDateTime";
        break;
    case 6://table
        type = "ZtTable";
        break;
    default:
        break;
    }
    writer<<tab;
    //consider list
    if(e.isMultiple()) {
        if(t==3 || t==6)//complex
            writer<<"QList<"<<type<<"* > ";
        else
            writer<<"QList<"<<type<<"> ";
        writer<<e.getname()<<"s;"<<endline;
    }
    else {
        writer<<type<<" ";
        writer<<e.getname()<<";"<<endline;
    }
}

void wrt_h_R(Element &e, QTextStream &writer)
{
    int t = e.gettype();
    if(t<0 || t>=Element::typelist.size()) return;
    //get type
    QString type = Element::typelist[t];
    switch (t) {
    case 2://string
        type = "QString";
        break;
    case 3://complex
        type = e.getComplexType();
        break;
    case 4://boolean
        type = "bool";
        break;
    case 5://timestamp
        type = "QDateTime";
        break;
    case 6://table
        type = "ZtTable";
        break;
    default:
        break;
    }
    //consider list
    if(e.isMultiple()) {
        if(t==3 || t==6) {//complex type
            writer<<tab<<"int "<<e.getname()<<"Length() const "
                  <<"{return "<<e.getname()<<"s.size();}"<<endline;//get length
            writer<<tab<<type<<"* "<<e.getname()<<"At_ptr(int i) const "
                  <<"{if(i>=0 && i<"<<e.getname()<<"s.size()) return "
                  <<e.getname()<<"s[i]; else return NULL;}"<<endline;
        }
        else {
            writer<<tab<<"int "<<e.getname()<<"Length() const "
                  <<"{return "<<e.getname()<<"s.size();}"<<endline;//get length
            writer<<tab<<type<<" "<<e.getname()<<"At(int i) const "
                  <<"{return "<<e.getname()<<"s[i];}"<<endline;
        }
    }
    else {
        if(t==3 || t==6) {//complex type
            writer<<tab<<type<<"* "<<e.getName()<<"_ptr() "
                 <<"{return &"<<e.getname()<<";}"<<endline;
        }
        else {
            writer<<tab<<type<<" "<<e.getName()<<"() const "
                 <<"{return "<<e.getname()<<";}"<<endline;
        }
    }
}

void wrt_h_W(Element &e, QTextStream &writer)
{
    int t = e.gettype();
    if(t<0 || t>=Element::typelist.size()) return;
    //get type
    QString type = Element::typelist[t];
    switch (t) {
    case 2://string
        type = "QString";
        break;
    case 3://complex
        type = e.getComplexType();
        break;
    case 4://boolean
        type = "bool";
        break;
    case 5://timestamp
        type = "QDateTime";
        break;
    case 6://table
        type = "ZtTable";
        break;
    default:
        break;
    }
    //consider list
    if(e.isMultiple()) {
        if(t==3 || t==6) {//complex type
            writer<<tab<<"bool del"<<e.getName()<<"(int i) "
                  <<"{if(i>=0 && i<"<<e.getname()<<"s.size()) {"
                  <<"delete "<<e.getname()<<"s[i];"<<e.getname()<<"s.removeAt(i);return true;} "
                  <<"else return false;}"<<endline;
            writer<<tab<<"void add"<<e.getName()<<"("<<type<<" *_n) "
                  <<"{"<<e.getname()<<"s.append(_n);}"<<endline;
        }
        else {
            writer<<tab<<"bool del"<<e.getName()<<"(int i) "
                  <<"{if(i>=0 && i<"<<e.getname()<<"s.size()) {"
                  <<e.getname()<<"s.removeAt(i);return true;} "
                  <<"else return false;}"<<endline;
            writer<<tab<<"bool change"<<e.getName()<<"(int i, "<<type<<" _n) "
                  <<"{if(i>=0 && i<"<<e.getname()<<"s.size()) {"
                  <<e.getname()<<"s[i]=_n;return true;} "
                  <<"else return false;}"<<endline;
            writer<<tab<<"void add"<<e.getName()<<"("<<type<<" _n) "
                  <<"{"<<e.getname()<<"s.append(_n);}"<<endline;
        }
    }
    else {
        if(t==3 || t==6) {//complex type
            return;
        }
        else {
            writer<<tab<<"void set"<<e.getName()<<"("<<type<<" _n) "
                  <<"{"<<e.getname()<<"=_n;}"<<endline;
        }
    }
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
        int t = e->gettype();
        if(t<0 || t>=Element::typelist.size()) return;
        //get type
        QString type = Element::typelist[t];
        switch (t) {
        case 2://string
            type = "QString";
            break;
        case 3://complex
            type = e->getComplexType();
            break;
        case 4://boolean
            type = "bool";
            break;
        case 5://timestamp
            type = "QDateTime";
            break;
        case 6://table
            type = "ZtTable";
            break;
        default:
            break;
        }
        //consider multiple
        if(e->isMultiple()) {
            if(t==3 || t==6) {//complex
                writer<<tab<<"for(int i=0;i<"<<e->getname()<<"s.size();i++) "
                      <<"delete "<<e->getname()<<"s[i];"<<endline;
            }
            writer<<tab<<e->getname()<<"s.clear();"<<endline;
        }
        else if(t==3 || t==6)//single complex
            writer<<tab<<e->getname()<<".cleanAll();"<<endline;
    }
    writer<<"}"<<endline;

    writer<<endline;
    writer<<"bool "<<p.getName()<<"::loadXml(QXmlStreamReader &reader)"<<endline;
    writer<<"{"<<endline;
    QStringList tmp;
    for(int i=0;i<p.elementLength();i++) {
        //consider necessary
        if(p.elementAt(i)->isNecesssary())
            tmp<<"flag"+QString::number(i);
        if(i%3==0) writer<<tab;
        writer<<"bool "<<"flag"<<QString::number(i)<<"=false;";
        if(i==p.elementLength()-1) writer<<endline;//last one
        else if(i%3==2) writer<<endline;//every three flags in one line
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
    //writer<<tab<<"writer.writeStartElement(\""<<p.getName()<<"\");"<<endline;
    //writer<<endline;
    for(int i=0;i<p.elementLength();i++)
        wrt_cpp_S(*p.elementAt(i),writer);
    //writer<<endline;
    //writer<<tab<<"writer.writeEndElement();"<<endline;
    writer<<"}"<<endline;
}

void wrt_cpp_L(Element &e, int i, QTextStream &writer)
{
    int t = e.gettype();
    if(t<0 || t>=Element::typelist.size()) return;
    //get type
    QString type = Element::typelist[t];
    switch (t) {
    case 2://string
        type = "QString";
        break;
    case 3://complex
        type = e.getComplexType();
        break;
    case 4://boolean
        type = "bool";
        break;
    case 5://timestamp
        type = "QDateTime";
        break;
    case 6://table
        type = "ZtTable";
        break;
    default:
        break;
    }
    QString flag = "flag"+QString::number(i);//setup flag
    //consider multiple
    if(!e.isMultiple())
        writer<<tab4<<"if("<<flag<<") {"<<flag<<"=false;break;}"<<endline;
    //consider necessary
    if(e.isNecesssary())
        writer<<tab4<<flag<<"=true;"<<endline;
    //parse
    switch (t) {
    case 0: // int
        if(e.isMultiple())
            writer<<tab4<<e.getname()<<"s.append(reader.readElementText().trimmed().toInt());"<<endline;
        else
            writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed().toInt();"<<endline;
        break;
    case 1: // double
        if(e.isMultiple())
            writer<<tab4<<e.getname()<<"s.append(reader.readElementText().trimmed().toDouble());"<<endline;
        else
            writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed().toDouble();"<<endline;
        break;
    case 2: // string
        if(e.isMultiple())
            writer<<tab4<<e.getname()<<"s.append(reader.readElementText().trimmed());"<<endline;
        else
            writer<<tab4<<e.getname()<<"=reader.readElementText().trimmed();"<<endline;
        break;
    case 3: // complex
    case 6: // table
        if(e.isMultiple()) {
            writer<<tab4<<type<<" *p=new "<<type<<"();"<<endline;
            writer<<tab4<<"if(p->loadXml(reader))"<<endline;
            writer<<tab4<<tab<<e.getname()<<"s.append(p);"<<endline;
            writer<<tab4<<"else {"<<endline;
            writer<<tab4<<tab<<"delete p;"<<endline;
            writer<<tab4<<tab<<"cleanAll();"<<endline;
            writer<<tab4<<tab<<"return false;"<<endline;
            writer<<tab4<<"}"<<endline;
        }
        else {
            writer<<tab4<<"if(!"<<e.getname()<<".loadXml(reader)) {"<<endline;
            writer<<tab4<<tab<<"cleanAll();"<<endline;
            writer<<tab4<<tab<<"return false;"<<endline;
            writer<<tab4<<"}"<<endline;
        }
        break;
    case 4: // boolean
        if(e.isMultiple()) {
            writer<<tab4<<"QString tmp=reader.readElementText().trimmed();"<<endline;
            writer<<tab4<<"bool tmpBool=((tmp==\"true\") || (tmp==\"True\") || (tmp==\"T\") || (tmp==\"1\"));"<<endline;
            writer<<tab4<<e.getname()<<"s.append(tmpBool);"<<endline;
        }
        else {
            writer<<tab4<<"QString tmp=reader.readElementText().trimmed();"<<endline;
            writer<<tab4<<e.getname()<<"=((tmp==\"true\") || (tmp==\"True\") || (tmp==\"T\") || (tmp==\"1\"));"<<endline;
        }
        break;
    case 5: // timestamp
        if(e.isMultiple())
            writer<<tab4<<e.getname()<<"s.append(QDateTime::fromString(reader.readElementText().trimmed(),"
                 <<timeFormat<<"));"<<endline;
        else
            writer<<tab4<<e.getname()<<"=QDateTime::fromString(reader.readElementText().trimmed(),"<<timeFormat<<");"<<endline;
        break;
    default:
        break;
    }
}

void wrt_cpp_S(Element &e, QTextStream &writer)
{
    int t=e.gettype();//get type
    if(t<0 || t>=Element::typelist.size()) return;
    //consider multiple
    if(e.isMultiple()) {
        writer<<tab<<"for(int i=0;i<"<<e.getname()<<"s.size();i++) {"<<endline;
        //parse
        switch (t) {
        case 0: // int
        case 1: // double
            writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", QString::number("<<e.getname()<<"s[i]));"<<endline;
            break;
        case 2: // string
            writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", "<<e.getname()<<"s[i]);"<<endline;
            break;
        case 3: // complex
        case 6: // table
            writer<<tab<<tab<<"writer.writeStartElement(\""<<e.getName()<<"\");"<<endline;
            writer<<tab<<tab<<e.getname()<<"s[i]->saveXml(writer);"<<endline;
            writer<<tab<<tab<<"writer.writeEndElement();"<<endline;
            break;
        case 4: // boolean
            writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", "<<e.getname()<<"s[i]?\"True\":\"False\");"<<endline;
            break;
        case 5: // timestamp
            writer<<tab<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", "<<e.getname()<<"s[i].toString("<<timeFormat<<"));"<<endline;
            break;
        default:
            break;
        }
        writer<<tab<<"}"<<endline;
    }
    else {
        //parse
        switch (t) {
        case 0: // int
        case 1: // double
            writer<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", QString::number("<<e.getname()<<"));"<<endline;
            break;
        case 2: // string
            writer<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", "<<e.getname()<<");"<<endline;
            break;
        case 3: // complex
        case 6: // table
            writer<<tab<<"writer.writeStartElement(\""<<e.getName()<<"\");"<<endline;
            writer<<tab<<e.getname()<<".saveXml(writer);"<<endline;
            writer<<tab<<"writer.writeEndElement();"<<endline;
            break;
        case 4: // boolean
            writer<<tab<<"writer.writeTextElement(\""<<e.getName()
                  <<"\", "<<e.getname()<<"?\"True\":\"False\");"<<endline;
            break;
        case 5: // timestamp
            writer<<tab<<"writer.writeTextElement(\""<<e.getName()
                 <<"\", "<<e.getname()<<".toString("<<timeFormat<<"));"<<endline;
            break;
        default:
            break;
        }
    }
}
