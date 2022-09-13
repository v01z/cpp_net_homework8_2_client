#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "client.h"

//--------------------------------------------------------------------------

Client::Client(const QString& remote_host, uint remote_port, QWidget *parent)
    : QWidget(parent),
      socket_{ new QSslSocket(this) },
      txt_info_{ new QTextEdit(this) },
      txt_input_user_{ new QLineEdit(this) },
      txt_input_pass_{ new QLineEdit(this) },
      txt_input_server_{ new QLineEdit(remote_host + " " + QString::number(remote_port), this) },
      txt_input_message_{ new QLineEdit(this) },
      txt_input_recipient_{ new QLineEdit(this) },
      label_{ new QLabel("Not connected.", this) },
      disconnect_button_{ new QPushButton ("Disconnect", this)},
      push_button_{ new QPushButton ( this)},
      combo_box_{ new QComboBox (this) },
      command_list_{
          "Get incoming messages",
          "Send message to:",
          "Change name",
          "Change password",
          "Add account",
          "Delete account",
          "Shutdown server"
          },
      auth_{} //default login: idguest
{
    connect(socket_, SIGNAL(connected()),
            SLOT(slotConnected()));
    connect(socket_, SIGNAL(readyRead()),
            SLOT(slotReadyRead()));
    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(socket_, SIGNAL(disconnected()),
            SLOT(slotDisconnected()));

    txt_info_->setReadOnly(true);

    connect(disconnect_button_, SIGNAL(clicked()),
            SLOT(disconnectButtonClicked()));
    connect(push_button_, SIGNAL(clicked()),
            SLOT(pushButtonClicked()));
    connect(txt_input_server_, SIGNAL(returnPressed()), this,
            SLOT(pushButtonClicked()));

    connect(combo_box_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(comboboxIndexChanged(int)));

    auto vbox_layout = new QVBoxLayout;
    auto hbox_layout_top = new QHBoxLayout;
    auto hbox_layout_bottom = new QHBoxLayout;

    vbox_layout->addLayout(hbox_layout_top);
    hbox_layout_top->addWidget(label_);
    hbox_layout_top->addWidget(disconnect_button_);
    disconnect_button_->setVisible(false);

    vbox_layout->addWidget(txt_info_);

    vbox_layout->addLayout(hbox_layout_bottom);
    hbox_layout_bottom->addWidget(txt_input_user_);
    hbox_layout_bottom->addWidget(txt_input_pass_);

    txt_input_user_->setPlaceholderText("user");
    txt_input_pass_->setPlaceholderText("password");
    txt_input_user_->setText(auth_.getId());
    txt_input_pass_->setText(auth_.getPass());

    vbox_layout->addWidget(txt_input_server_);
    txt_input_server_->setPlaceholderText("server port");

    vbox_layout->addWidget(combo_box_);
    combo_box_->addItems(command_list_);

    combo_box_->setVisible(false);
    txt_info_->setText("Hello. Click \'Connect\' to continue.");
    push_button_->setText("Connect");

    vbox_layout->addWidget(txt_input_recipient_);
    txt_input_recipient_->setVisible(false);

    vbox_layout->addWidget(txt_input_message_);
    txt_input_message_->setPlaceholderText("Enter message/command here");
    txt_input_message_->setVisible(false);

    vbox_layout->addWidget(push_button_);

    setLayout(vbox_layout);
}

//--------------------------------------------------------------------------

bool Client::isConnected() const {
    return socket_->state() == QAbstractSocket::ConnectedState;
}

//--------------------------------------------------------------------------

void Client::connectToServer(const QString& host, uint port) {
   if (socket_ == nullptr)
       return;
   if(host.isEmpty())
       return;
   if(port == 0)
       return;

   auth_.changeAuth(txt_input_user_->text(), txt_input_pass_->text());

   socket_->setPeerVerifyMode(QSslSocket::VerifyNone);
   socket_->connectToHostEncrypted(host, port);
   if (!socket_->waitForEncrypted()) {
    qDebug() << socket_->errorString();
    return;
    }

   socket_->write(createIncMsgsGetRequest().toStdString().c_str());
}

//--------------------------------------------------------------------------

void Client::slotError(QAbstractSocket::SocketError err) {
    QString str_error = "Error: " +
            (err == QAbstractSocket::HostNotFoundError ?
            "The host was not found.":
            err == QAbstractSocket::RemoteHostClosedError ?
            "The remote host is closed.":
            err == QAbstractSocket::ConnectionRefusedError ?
            "The connection was refused.":
            QString(socket_->errorString()));
}

//--------------------------------------------------------------------------

void Client::slotSendToServer() {

    QString request{};

    switch(combo_box_->currentIndex())
    {
        case(0): //"Get incomming messages"
        {
            request = createIncMsgsGetRequest();
            break;
        }
        case(1): //"Send message to:"
        {
            request = createSendMessageRequest();
            break;
        }
        case(2): //"Change name"
        {
            request = createChangeNameRequest();
            break;
        }
        case(3): //"Change password"
        {
            request = createChangePasswordRequest();
            break;
        }
        case(4): //"Add account"
        {
            request = createAddAccountRequest();
            break;
        }
        case(5): //"Delete account"
        {
            request = createDeleteAccountRequest();
            break;
        }
        case(6): //"Shutdown server"
        {
            request = createShutdownServerRequest();
            break;
        }
        default: //just in case
        {
            return;
        }
    }

    socket_->write(request.toUtf8().constData());
    txt_input_server_->setText("");
}

//--------------------------------------------------------------------------

void Client::slotConnected() {
    QString connection_info{ "Connected to " + socket_->peerName() + " : "
                             + QString::number(socket_->peerPort()) +
                                "\nYou are: \'" + auth_.getId() + "\'" };

    disconnect_button_->setVisible(true);
    txt_info_->append(connection_info);
    label_->setText(connection_info);

    txt_input_user_->setVisible(false);
    txt_input_pass_->setVisible(false);
    txt_input_server_->setVisible(false);
    combo_box_->setVisible(true);
    combo_box_->setCurrentIndex(0);
    push_button_->setText("Send");

    auto view = qobject_cast<QListView *>(combo_box_->view());
    Q_ASSERT(view != nullptr);
    if(auth_.getId() == ROOT_ACCOUNT)
    {
        view->setRowHidden(4, false); //"Add account"
        view->setRowHidden(5, false); //"Delete account"
        view->setRowHidden(6, false); //"Shutdown server"
    }
    else
    {
        view->setRowHidden(4, true);
        view->setRowHidden(5, true);
        view->setRowHidden(6, true);
    }
}

//--------------------------------------------------------------------------

void Client::slotDisconnected(){
    if(!isConnected())
    {
        QString connection_info{ "Disconnected." };

        txt_info_->append(connection_info);
        label_->setText(connection_info);

        txt_input_user_->setVisible(true);
        txt_input_pass_->setVisible(true);
        txt_input_server_->setVisible(true);
        txt_input_recipient_->setVisible(false);

        combo_box_->setVisible(false);

        txt_input_server_->setText("");

        combo_box_->setCurrentIndex(0);

        disconnect_button_->setVisible(false);
        push_button_->setText("Connect");
    }
}

//--------------------------------------------------------------------------

void Client::disconnectButtonClicked(){
    if(isConnected()) //not neccessary, but
        socket_->disconnectFromHost();
}

//--------------------------------------------------------------------------

void Client::pushButtonClicked() {
    if(isConnected())
    {
        slotSendToServer();
    }
    else
    {
        QString host{ txt_input_server_->text().section(" ", 0, 0) };
        QString port{ txt_input_server_->text().section(" ", 1, 1) };
        if (host.isEmpty() || port.isEmpty())
        {
            txt_info_->append(
                    "Enter correct values for host and port\n"
                    "Hostname/ip and port should be separated with one space.");
            return;
        }
        connectToServer(host, port.toUInt());
    }
}

//--------------------------------------------------------------------------

void Client::comboboxIndexChanged(int index)
{
    switch(index)
    {
        case(0): //"Get incoming messages"
        {
            push_button_->setText("Click to get message(-s)");
            txt_input_message_->setVisible(false);
            txt_input_recipient_->setVisible(false);
            break;
        }
        case(1): //"Send message to:"
        {
            txt_input_message_->setText("");
            txt_input_message_->setPlaceholderText("Enter new message here");
            txt_input_message_->setVisible(true);
            txt_input_recipient_->setPlaceholderText("Enter recipient of ur message");
            txt_input_recipient_->setText("");
            txt_input_recipient_->setVisible(true);
            push_button_->setText("Send message");
            break;
        }
        case(2): //"Change name"
        {
            txt_input_message_->setText("");
            txt_input_message_->setPlaceholderText("Enter new name here");
            txt_input_message_->setVisible(true);
            txt_input_recipient_->setVisible(false);
            push_button_->setText("Click to append new name");
            break;
        }
        case(3): //"Change password"
        {
            txt_input_message_->setText("");
            txt_input_message_->setPlaceholderText("Enter new password here");
            txt_input_message_->setVisible(true);
            txt_input_recipient_->setVisible(false);
            push_button_->setText("Click to append new password");
            break;
        }
        case(4): //"Add account"
        {
            txt_input_message_->setText("");
            txt_input_message_->setPlaceholderText("Enter password for new id");
            txt_input_message_->setVisible(true);
            txt_input_recipient_->setPlaceholderText("Enter new id here");
            txt_input_recipient_->setText("");
            txt_input_recipient_->setVisible(true);
            push_button_->setText("Click to add new account");
            break;
        }
        case(5): //"Delete account"
        {
            txt_input_message_->setVisible(false);
            txt_input_recipient_->setPlaceholderText("Enter id to delete");
            txt_input_recipient_->setText("");
            txt_input_recipient_->setVisible(true);
            push_button_->setText("Click to delete chosen account");
            break;
        }
        case(6): //"Shutdown server"
        {
            txt_input_message_->setVisible(false);
            txt_input_recipient_->setVisible(false);
            push_button_->setText("Click to shutdown server app");
            break;
        }
        default: //just in case
        {
            return;
        }
    }
}

//--------------------------------------------------------------------------

const QString Client::createIncMsgsGetRequest()
{
    return "GET income" + auth_.getAuthHeader()+ "\r\n";
}

//--------------------------------------------------------------------------

const QString Client::createSendMessageRequest()
{
    QString message = txt_input_message_->text();
    return "POST  "+ txt_input_recipient_->text() + auth_.getAuthHeader()
            + "Content-Length: " + QString::number(message.length())
                + "\r\n\r\n" + message;
}

//--------------------------------------------------------------------------

const QString Client::createChangeNameRequest()
{
    QString new_name = txt_input_message_->text();
    return "PATCH name" + auth_.getAuthHeader()
        + "Content-Length: " + QString::number(new_name.length())
            + "\r\n\r\n" + new_name;
}

//--------------------------------------------------------------------------

const QString Client::createChangePasswordRequest()
{
    QString new_pass = txt_input_message_->text();
    return "PATCH password" + auth_.getAuthHeader()
           + "Content-Length: " + QString::number(new_pass.length())
           + "\r\n\r\n" + new_pass;
}

//--------------------------------------------------------------------------

const QString Client::createAddAccountRequest()
{
    const QString user_name = txt_input_recipient_->text();
    const QString user_pass = txt_input_message_->text();

    return "PUT " + user_name + auth_.getAuthHeader()
        + "Content-Length: " + QString::number(user_pass.length())
           + "\r\n\r\n" + user_pass;
}

//--------------------------------------------------------------------------

const QString Client::createDeleteAccountRequest()
{
    const QString user_to_remove = txt_input_recipient_->text();

    return "DELETE " + user_to_remove + auth_.getAuthHeader()
            + "\r\n";
}

//--------------------------------------------------------------------------

const QString Client::createShutdownServerRequest()
{
    return "DELETE @exit@" + auth_.getAuthHeader()
           + "\r\n";
}

//--------------------------------------------------------------------------

void Client::slotReadyRead() {

    static bool already_got_header{};
    while (socket_->bytesAvailable() > 0) {
        if (!already_got_header) {
            QString line{socket_->readLine()};

#ifdef DEBUG
            txt_info_->append(line);
#endif //DEBUG
                if (line.indexOf("\r\n") == 0)
                    already_got_header = true;

        } else {

            char temp_buff[1024]{};
            ssize_t block_size = socket_->read(temp_buff, sizeof (temp_buff));
            if(block_size == -1)
            {
                qCritical("Trying to read from closed (?) socket.");
                return;
            }
            txt_info_->append("Got from server:\n" + QString(temp_buff));
            already_got_header = false;
        }
    }
}

//--------------------------------------------------------------------------