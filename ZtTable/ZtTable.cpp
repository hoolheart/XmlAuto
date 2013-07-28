#include "ZtTable.h"

ZtTable::ZtTable()
{cleanAll();}

ZtTable::~ZtTable()
{cleanAll();}

void ZtTable::cleanAll()
{
    header.clear();
    rows.clear();
}

bool ZtTable::loadXml(QXmlStreamReader &reader)
{
    bool flag0=false;
    cleanAll();

    if(reader.isEndElement()) return false;
    reader.readNext();
    while(!reader.isEndElement()) {
        if(reader.isStartElement()) {
            if(reader.name()=="Header") {
                if(flag0) {flag0=false;break;}
                flag0=true;
                header=reader.readElementText().trimmed().split(',');
                reader.readNext();
            }
            else if(reader.name()=="Row") {
                rows.append(reader.readElementText().trimmed().split(','));
                reader.readNext();
            }
            else {cleanAll(); return false;}
        }
        else reader.readNext();
    }

    if(flag0)
        return true;
    else {cleanAll(); return false;}
}

void ZtTable::saveXml(QXmlStreamWriter &writer)
{
    //writer.writeStartElement("ZtTable");

    writer.writeTextElement("Header", header.join(','));
    for(int i=0;i<rows.length();i++)
        writer.writeTextElement("Row", rows[i].join(','));

    //writer.writeEndElement();
}
