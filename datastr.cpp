#include "datastr.h"

QString types =
"int,double,string,intlist,doublelist,stringlist,complex,complexlist,boolean,booleanlist,timestamp,timestamplist,table";
QStringList Element::typelist=types.split(",");

bool Element::load(QXmlStreamReader &reader)
{
    bool flag1=false,flag2=false;

    if(reader.isEndElement())
        return false;
    reader.readNext();
    while(!reader.isEndElement()) {
        if(reader.isStartElement()) {
            if(reader.name() == "Name") {
                if(flag1) {
                    flag1=false;
                    break;
                }
                flag1 = true;
                Name = reader.readElementText().trimmed();
                name = Name.toLower();
                Name = Name.left(1).toUpper()+Name.right(Name.length()-1);
                reader.readNext();
            }
            else if(reader.name() == "Type") {
                if(!flag1)
                    break;
                if(flag2) {
                    flag2=false;
                    break;
                }
                QString tmp = reader.readElementText().trimmed();
                for(type=0;type<typelist.length();type++) {
                    if (tmp==typelist[type])
                        break;
                }
                if(type<typelist.length())
                    flag2 = true;
                else
                    break;
                reader.readNext();
            }
            else if(reader.name() == "Param") {
                if(!flag1)
                    break;
                if(!flag2)
                    break;
                QString tmp = reader.readElementText().trimmed();
                if(typelist[type].right(4)=="list")
                    name = tmp;
                else
                    break;
                reader.readNext();
            }
            else {
                cleanAll();
                return false;
            }
        }
        else
            reader.readNext();
    }

    if(flag1&&flag2)
        return true;
    else {
        cleanAll();
        return false;
    }
}

void Element::save(QXmlStreamWriter &writer)
{
    writer.writeStartElement("Element");

    writer.writeTextElement("Name",name);
    writer.writeTextElement("Type",typelist[type]);

    writer.writeEndElement();
}

bool Group::load(QXmlStreamReader &reader)
{
    bool flag1=false;

    if(reader.isEndElement())
        return false;
    reader.readNext();
    while(!reader.isEndElement()) {
        if(reader.isStartElement()) {
            if(reader.name() == "Name") {
                if(flag1) {flag1=false; break;}
                flag1 = true;
                Name = reader.readElementText().trimmed();
                name = Name.toLower();
                Name = Name.left(1).toUpper()+Name.right(Name.length()-1);
                reader.readNext();
            }
            else if(reader.name() == "Element") {
                Element *p=new Element();
                if(p->load(reader)) {
                    elements.append(p);
                }
                else {
                    delete p;
                    cleanAll();
                    return false;
                }
                reader.readNext();
            }
            else {
                cleanAll();
                return false;
            }
        }
        else
            reader.readNext();
    }

    if(flag1)
        return true;
    else {
        cleanAll();
        return false;
    }
}

void Group::save(QXmlStreamWriter &writer)
{
    writer.writeStartElement("Group");

    writer.writeTextElement("Name",name);
    for(int i=0;i<elements.length();i++)
        elements[i]->save(writer);

    writer.writeEndElement();
}

bool DataStr::load(QXmlStreamReader &reader)
{
    if(reader.isEndElement())
        return false;
    reader.readNext();
    while(!reader.isEndElement()) {
        if(reader.isStartElement()) {
            if(reader.name() == "Group") {
                Group *p = new Group();
                if(p->load(reader))
                    groups.append(p);
                else {
                    delete p;
                    cleanAll();
                    return false;
                }
                reader.readNext();
            }
            else {
                cleanAll();
                return false;
            }
        }
        else
            reader.readNext();
    }

    return true;
}

void DataStr::save(QXmlStreamWriter &writer)
{
    writer.writeStartElement("DataStruct");
    for(int i=0;i<groups.length();i++)
        groups[i]->save(writer);
    writer.writeEndElement();
}

bool DataStr::checkType(QString _t) {
    QStringList typeList = types.split(',');
    int k;
    for (k=0;k<typeList.length();k++) {
        if (typeList[k]==_t)
            break;
    }
    if (k>=typeList.length())
        return false;

    for (int i=0;i<groups.length();i++) {
        for (int j=0;j<groups[i]->elementLength();j++) {
            if (groups[i]->elementAt(j)->gettype()==k)
                return true;
        }
    }
    return false;
}
