#include "auth_object.h"

//--------------------------------------------------------------------------

AuthObject::AuthObject(const QString &id,  const QString &pass):
        id_{ id },
        pass_{ pass },
        name_{},
        auth_header_{" HTTP/1.1\r\nAuthorization: Basic "
            + (id + ":" + pass).toUtf8().toBase64() + "\r\n" }
{}

//--------------------------------------------------------------------------

AuthObject::AuthObject():
    id_{ "idguest" },
    pass_{ "guest" },
    name_{ "guest" },
    auth_header_{" HTTP/1.1\r\nAuthorization: Basic "
                 + (id_ + ":" + pass_).toUtf8().toBase64() + "\r\n" }
{}

//--------------------------------------------------------------------------

const QString &AuthObject::getId() const {
    return id_;
}

//--------------------------------------------------------------------------

const QString &AuthObject::getPass() const {
    return pass_;
}

//--------------------------------------------------------------------------

const QString &AuthObject::getName() const {
    return name_;
}

//--------------------------------------------------------------------------

const QString &AuthObject::getAuthHeader() const {
    return auth_header_;
}

//--------------------------------------------------------------------------

void AuthObject::changeAuth(const QString& id, const QString& pass) {
    id_ = id;
    pass_ = pass;
    name_.clear();
    auth_header_  = " HTTP/1.1\r\nAuthorization: Basic "
        + (id + ":" + pass).toUtf8().toBase64() + "\r\n";
}

//--------------------------------------------------------------------------

void AuthObject::setName(const QString &name) {
    name_ = name;
}

//--------------------------------------------------------------------------

void AuthObject::setPassword(const QString &pass) {
    pass_ = pass;
}

//--------------------------------------------------------------------------

