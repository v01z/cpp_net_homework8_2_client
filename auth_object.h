
#ifndef CLIENT_HOMEWORK_8_AUTHOBJECT_H
#define CLIENT_HOMEWORK_8_AUTHOBJECT_H

#include <QString>

//--------------------------------------------------------------------------

class AuthObject {
private:
    QString id_{};
    QString pass_{};
    QString name_{};
    QString auth_header_{};
public:
    explicit AuthObject(const QString&, const QString&);
    AuthObject();

    const QString &getId() const;

    const QString &getPass() const;

    const QString &getName() const;

    const QString &getAuthHeader() const;

    void changeAuth(const QString&, const QString&);
    void setName(const QString&);
    void setPassword(const QString&);

};

//--------------------------------------------------------------------------

#endif //CLIENT_HOMEWORK_8_AUTHOBJECT_H
