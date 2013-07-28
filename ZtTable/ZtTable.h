#ifndef ZTTABLE_H
#define ZTTABLE_H
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class ZtTable;

class ZtTable {
private:
    QStringList header;
    QList<QStringList> rows;
public:
    ZtTable();
    ~ZtTable();
    QStringList Header() const {return header;}
    void setHeader(const QStringList &_n) {header=_n;}
    int rowsLength() const {return rows.length();}
    QStringList rowsAt(int i) const {if(i>=0 && i<rows.length()) return rows[i]; else return QStringList();}
    bool delRow(int i) {if(i>=0 && i<rows.length()) {rows.removeAt(i);return true;} else return false;}
    bool changeRow(int i, const QStringList &_n) {if(i>=0 && i<rows.length()) {rows[i]=_n;return true;} else return false;}
    void addRow(const QStringList &_n) {rows.append(_n);}
    void cleanAll();
    bool loadXml(QXmlStreamReader &reader);
    void saveXml(QXmlStreamWriter &writer);
};

#endif
