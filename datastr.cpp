#include "datastr.h"

QString types =
"int,double,string,complex,boolean,timestamp,table";
QStringList Element::typelist=types.split(",");

bool Element::load(QXmlStreamReader &reader)
{
    bool flag1=false,flag2=false;
    bool flag3=false,flag4=false;
    necessary = true; multiple = false; complexType.clear();

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
                if(flag2) {
                    flag2=false;
                    break;
                }
                QString tmp = reader.readElementText().trimmed();
                if(tmp.startsWith("complex"))
                {
                    QStringList tmpList = tmp.split(":");
                    if(tmpList.size()==2 && tmpList[0]=="complex")
                    {
                        type = 3;//complex
                        complexType = tmpList[1];//complex class type
                        flag2 = true;
                        reader.readNext();
                        continue;
                    }
                    else
                        break;
                }
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
            else if(reader.name() == "Necessary") {
                if(flag3) {
                    flag1=false;
                    break;
                }
                flag3 = true;
                QString tmp = reader.readElementText().trimmed();
                necessary = (tmp=="true")||(tmp=="True")||(tmp=="T")||(tmp=="1");
                reader.readNext();
            }
            else if(reader.name() == "Multiple") {
                if(flag4) {
                    flag1=false;
                    break;
                }
                flag4 = true;
                QString tmp = reader.readElementText().trimmed();
                multiple = (tmp=="true")||(tmp=="True")||(tmp=="T")||(tmp=="1");
                reader.readNext();
            }
            else {
                flag1=false;
                break;
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
    if(type<typelist.size())
        writer.writeTextElement("Type",typelist[type]);
    writer.writeTextElement("Necessary",necessary?"T":"F");
    writer.writeTextElement("Multiple",multiple?"T":"F");

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

QStringList DataStr::getComplexTypes()
{
    QStringList types;
    for (int i=0;i<groups.length();i++) {
        for (int j=0;j<groups[i]->elementLength();j++) {
            Element * pElement = groups[i]->elementAt(j);//get element
            if(pElement->gettype()==3)//complex
                types.append(pElement->getComplexType());
        }
    }
    return types;
}
