#ifndef DATASTR_H
#define DATASTR_H
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * @file The header file of the data classes to store the informations fetched from descriptive xml file.
 * @author Edward Chou
 */

/**
 * @brief The Element class
 * This class is for storing the informations about 'element' objects.
 * attributes:
 *      'name', the variable name in cpp codes corresponding the xml object;
 *      'Name', the name of xml object;
 *      'type', the type of this element.
 * static attributes:
 *      'typelist', the list of all available types.
 */

class Element {
private:
    QString name;
    QString Name;
    int type;
public:
    static QStringList typelist;
    void cleanAll()
    {
        type=0; name=""; Name="";
    }
    Element() {cleanAll();}
    ~Element() {cleanAll();}
    QString getname() const {return name;}
    QString getName() const {return Name;}
    int gettype() const {return type;}
    void setname(const QString &_name) {name = _name;}
    void setName(const QString &_Name) {Name = _Name;}
    void settype(int _type) {type = _type;}
    bool load(QXmlStreamReader &reader);
    void save(QXmlStreamWriter &writer);
};

/**
 * @brief The Group class
 * This class is for storing the information of complex xml type.
 * attributes:
 *      'name', reserved;
 *      'Name', the name of the class in cpp codes corresponding to the complex xml type;
 *      'elements', the list of elements within this complex type.
 */

class Group {
private:
    QString name;
    QString Name;
    QList<Element *> elements;
public:
    void cleanAll()
    {
        for(int i=0;i<elements.length();i++) delete elements[i];
        elements.clear();
        name = ""; Name = "";
    }
    Group() {cleanAll();}
    ~Group() {cleanAll();}
    QString getname() const {return name;}
    QString getName() const {return Name;}
    int elementLength() const {return elements.length();}
    Element * elementAt(int i)
    {
        if(i<elements.length()) return elements[i];
        else return 0;
    }
    void setname(const QString &_name) {name = _name;}
    void setName(const QString &_Name) {Name = _Name;}
    void addelement(Element *_e) {elements.append(_e);}
    void delelement(int i)
    {
        if(i<elements.length()) {
            delete elements[i];
            elements.removeAt(i);
        }
    }
    bool load(QXmlStreamReader &reader);
    void save(QXmlStreamWriter &writer);
};

/**
 * @brief The DataStr class
 * This class is for store all informations fetched from the descriptive xml file.
 * attributes:
 *      'groups', list of complex xml types.
 */

class DataStr {
private:
    QList<Group *> groups;
public:
    void cleanAll()
    {
        for(int i=0;i<groups.length();i++) delete groups[i];
        groups.clear();
    }
    DataStr() {cleanAll();}
    ~DataStr() {cleanAll();}
    int groupLength() const {return groups.length();}
    Group * groupAt(int i)
    {
        if(i<groups.length()) return groups[i];
        else return 0;
    }
    void addgroup(Group *_g) {groups.append(_g);}
    void delgroup(int i)
    {
        if(i<groups.length()) {
            delete groups[i];
            groups.removeAt(i);
        }
    }
    bool load(QXmlStreamReader &reader);
    void save(QXmlStreamWriter &writer);
    bool checkType(QString _t);
};

#endif // DATASTR_H
