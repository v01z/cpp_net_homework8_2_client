#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QSslSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QComboBox>
#include "auth_object.h"
#include <QListView>

//--------------------------------------------------------------------------

class Client : public QWidget
{
    Q_OBJECT

private:
    QSslSocket* socket_;
    QTextEdit* txt_info_;
    QLineEdit* txt_input_user_;
    QLineEdit* txt_input_pass_;
    QLineEdit* txt_input_server_;
    QLineEdit* txt_input_message_;
    QLineEdit* txt_input_recipient_;
    QLabel* label_;
    QPushButton* disconnect_button_;
    QPushButton* push_button_;
    QComboBox* combo_box_;
    QStringList command_list_;
    AuthObject auth_;

    bool isConnected() const;
    void connectToServer(const QString&, uint);

    const QString createIncMsgsGetRequest();
    const QString createSendMessageRequest();
    const QString createChangeNameRequest();
    const QString createChangePasswordRequest();
    const QString createAddAccountRequest();
    const QString createDeleteAccountRequest();
    const QString createShutdownServerRequest();

private slots:
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected();
    void slotDisconnected();
    void disconnectButtonClicked();
    void pushButtonClicked();
    void comboboxIndexChanged(int);

public:
    Client(const QString&, uint, QWidget *parent = nullptr);
    Client() = delete;
    ~Client() override = default;
};

//--------------------------------------------------------------------------

static const QString ROOT_ACCOUNT{ "idroot"};

//--------------------------------------------------------------------------

#endif // CLIENT_H
