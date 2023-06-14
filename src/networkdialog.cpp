#include "networkdialog.h"
#include <QSSLSocket>
#include "protocol.h"

NetworkDialog::NetworkDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("TRS Vote - Network Settings");

    // Create the tab widget
    tabWidget = new QTabWidget(this);
    pleaseWaitDialog = new PleaseWaitDialog;
    // Create the Server Settings tab
    serverSettingsTab = new QWidget();
    createServerSettingsTab(serverSettingsTab);
    tabWidget->addTab(serverSettingsTab, "Server Settings");

    // Create the Add Peers tab
    addPeersTab = new QWidget();
    createAddPeersTab(addPeersTab);
    tabWidget->addTab(addPeersTab, "Add Peers");

    QWidget* listPeersTab = new QWidget();
    QWidget* blockedPeersTab = new QWidget();
    moreSettingsTab = new QWidget;
    tabWidget->addTab(moreSettingsTab, "More Settings");
    layoutMoreSetting = new QVBoxLayout;
    protectConnCheckBox = new QCheckBox;
    protectConnCheckBox->setText("Only made connection with the same genesis");
    supportValidatorCheckBox = new QCheckBox;
    supportValidatorCheckBox->setText("Help validating block");
    layoutMoreSetting->addWidget(protectConnCheckBox);
    layoutMoreSetting->addWidget(supportValidatorCheckBox);
    moreSettingsTab->setLayout(layoutMoreSetting);
    saveLayoutMoreSettings = new QHBoxLayout;
    saveButtonMoreSettings = new QPushButton("Save");
    saveButtonMoreSettings->setStyleSheet("QPushButton {"
        "   background-color: #1DA1F2;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0D95E8;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #E1E8ED;"
        "   color: #AAB8C2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0B7BC0;"
        "}"
        "QLineEdit {"
        "   background-color: #E1E8ED;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 6px;"
        "outline:none;"
        "}"
        "QPushButton:focus { outline: none; }");
    saveLayoutMoreSettings->addStretch();
    saveLayoutMoreSettings->addWidget(saveButtonMoreSettings);
    layoutMoreSetting->addLayout(saveLayoutMoreSettings);
    tabWidget->addTab(listPeersTab, "List Peers");
    tabWidget->addTab(blockedPeersTab, "Blocked Peers");
    tabWidget->setTabVisible(tabWidget->indexOf(listPeersTab), false);
    tabWidget->setTabVisible(tabWidget->indexOf(blockedPeersTab), false);

    // Add the tab widget to the dialog layout
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    // Set the minimum size
    setMinimumSize(500, 150);
    setMaximumSize(500, 150);
    // Apply the stylesheet
    setStyleSheet("QTabWidget::tab-bar { alignment: left; }"
        "QTabWidget::tab { background-color: #1DA1F2; color: #FFFFFF; padding: 8px 16px; }"
        "QTabWidget::tab:selected { background-color: #FFFFFF; color: #1DA1F2; }"
        "QDialog {"
        "   background-color: #F5F8FA;"
        "   color: #14171A;"
        "}"
        "QGroupBox {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E1E8ED;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "}"
        "QListWidget {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E1E8ED;"
        "   border-radius: 4px;"
        "   padding: 4px;"
        "}"
        "QPushButton {"
        "   background-color: #1DA1F2;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0D95E8;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #E1E8ED;"
        "   color: #AAB8C2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0B7BC0;"
        "}"
        "QLineEdit {"
        "   background-color: #E1E8ED;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 6px;"
        "}"
        "QComboBox {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E1E8ED;"
        "   border-radius: 4px;"
        "   padding: 2px;"
        "}");
    port = 8999;
    portLineEdit->setText(QString::number(8999));
    m_server = nullptr;
    manager = new QNetworkAccessManager(this);
    setupDatabase();
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply* networkReply) {
        networkReply->deleteLater();
    QString ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
    if (!ip.isEmpty()) {
        serverLineEdit->setText(ip);
        QString s = connectServerButton->text();
        if (s.compare("Refresh", Qt::CaseInsensitive) == 0) {
            connectServerButton->setText("Connect");
        }
    }
    else {
        serverLineEdit->setText("Unable to connect internet");
        connectServerButton->setText("Refresh");
    }
        });
}

NetworkDialog::~NetworkDialog() {
    delete tabWidget;
    delete serverSettingsTab;
    delete addPeersTab;
    delete mainLayout;
    db.close();
}

void NetworkDialog::setupDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("conn.db");
    db.open();
    // Check if the table already exists
    QSqlQuery checkTableQuery;
    checkTableQuery.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='connection'");
    bool tableExists = checkTableQuery.next();

    // If the table doesn't exist, create it
    if (!tableExists) {
        QSqlQuery createTableQuery;
        createTableQuery.exec("CREATE TABLE connection (ipAddress TEXT, port INTEGER)");
    }


    // Perform a loop for every connection in the "connection" table
    if (tableExists) {
    }
}

void NetworkDialog::tryloop() {
    QSqlQuery selectQuery;
    selectQuery.exec("SELECT * FROM connection");

    while (selectQuery.next()) {
        QString ipAddress = selectQuery.value(0).toString();
        int port = selectQuery.value(1).toInt();
        setupClient(ipAddress, port, 2);
    }
}

void NetworkDialog::ConnectionInfo(QString ip, long long port, bool con, int wdialog) {
    pleaseWaitDialog->close();
    if (con) {
        QTcpSocket* temp_socket = new QTcpSocket;
        temp_socket->connectToHost(ip, port);
        if (temp_socket->waitForConnected()) {
            // If connected, retrieve the IP address and port
            QString ipAddress = temp_socket->peerAddress().toString();
            long long portNumber = temp_socket->peerPort();

            // Check if the IP address is a local address and already exists in the database
            bool isLocalAddress = QHostAddress(ipAddress).isLoopback();
            bool existsInDatabase = false;
            if (!isLocalAddress) {
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT COUNT(*) FROM connection WHERE ipAddress = :ip");
                checkQuery.bindValue(":ip", ipAddress);
                checkQuery.exec();

                if (checkQuery.next()) {
                    int count = checkQuery.value(0).toInt();
                    existsInDatabase = (count > 0);
                }
            }

            // Insert the IP address and port into the database if it's not a local address or doesn't exist in the database
            if (!isLocalAddress && !existsInDatabase) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO connection (ipAddress, port) VALUES (:ip, :port)");
                insertQuery.bindValue(":ip", ipAddress);
                insertQuery.bindValue(":port", portNumber);
                insertQuery.exec();
            }
            connection_client_set.insert(temp_socket);
            int peernum = connection_server_set.size() + connection_client_set.size();
            emit changePeer(peernum);
            connect(temp_socket, &QTcpSocket::readyRead, this, &NetworkDialog::readSocketClient);
            connect(temp_socket, &QTcpSocket::disconnected, this, &NetworkDialog::discardSocketClient);
            emit DoHandshake(temp_socket->socketDescriptor());
            if (wdialog == 1) {
                emit Information("Connected to " + ipAddress + ":" + QString::number(portNumber));
            }
        }
        else {
            delete temp_socket;
            if (wdialog==1) {
                emit Information("Can't connect to " + ip + ":" + QString::number(port));
            }
        }
    }
    else {
        if (wdialog == 1) {
            emit Information("Can't connect to " + ip + ":" + QString::number(port));
        }
    }
}

void NetworkDialog::createServerSettingsTab(QWidget* parent) {
    // Create the layout for the Server Settings tab
    QVBoxLayout* layoutS = new QVBoxLayout(parent);

    // Create a QHBoxLayout to contain the line edits and labels
    QHBoxLayout* lineSEditLayout = new QHBoxLayout();

    // Create a QLabel for the server address
    QLabel* serverLabel = new QLabel("IP Address:", parent);
    lineSEditLayout->addWidget(serverLabel);

    // Add a disabled QLineEdit for the server address
    serverLineEdit = new QLineEdit(parent);
    serverLineEdit->setReadOnly(true);
    serverLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lineSEditLayout->addWidget(serverLineEdit);

    // Create a QLabel for the port
    QLabel* portLabel = new QLabel("Port:", parent);
    lineSEditLayout->addWidget(portLabel);

    // Add a QLineEdit for the port
    portLineEdit = new QLineEdit(parent);
    portLineEdit->setFixedWidth(80);
    QIntValidator* validator = new QIntValidator(0, 65536, portLineEdit); // Set maximum value to 100
    portLineEdit->setValidator(validator);
    lineSEditLayout->addWidget(portLineEdit);
    // Add a "Connect" button
    connectServerButton = new QPushButton("Connect", parent);
    connectServerButton->setFixedWidth(80);
    lineSEditLayout->addWidget(connectServerButton);
    connect(connectServerButton, &QPushButton::clicked, [&]() {
        if (connectServerButton->text().compare("Connect", Qt::CaseInsensitive) == 0) {
            setupServer();
        }
        else if (connectServerButton->text().compare("Disconnect", Qt::CaseInsensitive) == 0) {
            // Remove elements from connection_server_set
            foreach(QTcpSocket * socket, connection_client_set)
            {
                socket->close();
                socket->deleteLater();
            }
            connection_client_set.clear();
            foreach(QTcpSocket * socket, connection_server_set)
            {
                socket->close();
                socket->deleteLater();
            }
            connection_server_set.clear();
            if (m_server != nullptr) {
                m_server->close();
                delete m_server;
                m_server = nullptr;
            }
            connectServerButton->setText("Connect");
        }
        else if (connectServerButton->text().compare("Refresh", Qt::CaseInsensitive) == 0) {
            GettingPublicConn();
        }
        });
    // Add the lineEditLayout to the main layout
    layoutS->addLayout(lineSEditLayout);
    layoutS->setAlignment(Qt::AlignCenter);
    // Set the layout for the Server Settings tab
    parent->setLayout(layoutS);
}


void NetworkDialog::createAddPeersTab(QWidget* parent) {

    // Create the layout for the Server Settings tab
    QVBoxLayout* layout = new QVBoxLayout(parent);

    // Create a QHBoxLayout to contain the line edits and labels
    QHBoxLayout* lineEditLayout = new QHBoxLayout();

    // Create a QLabel for the server address
    QLabel* clientLabel = new QLabel("IP Address:", parent);
    lineEditLayout->addWidget(clientLabel);

    clientLineEdit = new QLineEdit(parent);
    clientLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QRegularExpressionValidator* validator = new QRegularExpressionValidator(QRegularExpression("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"), clientLineEdit);
    clientLineEdit->setValidator(validator);

    lineEditLayout->addWidget(clientLineEdit);


    // Create a QLabel for the port
    QLabel* portLabel = new QLabel("Port:", parent);
    lineEditLayout->addWidget(portLabel);

    // Add a QLineEdit for the port
    portCLineEdit = new QLineEdit(parent);
    // Create a custom signal handler for text changed
    QIntValidator* thival = new QIntValidator(0, 65536, portCLineEdit);
    portCLineEdit->setValidator(thival);
    portCLineEdit->setMaxLength(5);
    portCLineEdit->setFixedWidth(80);
    lineEditLayout->addWidget(portCLineEdit);

    // Add a "Connect" button
    connectButton = new QPushButton("Connect", parent);
    connectButton->setFixedWidth(80);
    lineEditLayout->addWidget(connectButton);
    connect(connectButton, &QPushButton::clicked, [&]() {
        if (portCLineEdit->text().toInt() > 65536) {
            emit Information("The maximum valid port number is 65535. Ports with numbers higher than 65535 are not supported.");
            return;
        }
    QString ip = clientLineEdit->text();
    setupClient(ip, portCLineEdit->text().toInt(),1);
        });
    // Add the lineEditLayout to the main layout
    layout->addLayout(lineEditLayout);
    layout->setAlignment(Qt::AlignCenter);
    // Set the layout for the Server Settings tab
    parent->setLayout(layout);
}

void NetworkDialog::GettingPublicConn() {
    manager->get(QNetworkRequest(QUrl("https://api.ipify.org?format=json")));
}

void NetworkDialog::DefaultPort() {
    portLineEdit->setText(QString::number(port));
}

void NetworkDialog::setupServer() {
    if (m_server != nullptr) {
        m_server->close();
        delete m_server;
    }
    m_server = new QTcpServer();
    int p = portLineEdit->text().toInt();
    if (p >= 65536) {
        serverLineEdit->setText("Unable to use port");
        connectServerButton->setText("Refresh");
        return;
    }
    port = p;
    if (m_server->listen(QHostAddress::AnyIPv4, port))
    {
        connect(m_server, &QTcpServer::newConnection, this, &NetworkDialog::newConnection);
        int peernum = connection_server_set.size() + connection_client_set.size();
        connectServerButton->setText("Disconnect");
        emit changePeer(peernum);
    }
    else
    {
        serverLineEdit->setText("Unable to connect internet");
        connectServerButton->setText("Refresh");
    }
}

void NetworkDialog::newConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        if (!socket) {
            continue;
        }
        connection_server_set.insert(socket);
        connect(socket, &QTcpSocket::readyRead, this, &NetworkDialog::readSocket);
        connect(socket, &QTcpSocket::disconnected, this, &NetworkDialog::discardSocket);
        QEventLoop waitloop;
        QTimer::singleShot(100, &waitloop, SLOT(quit()));
        waitloop.exec();
        waitloop.deleteLater();
        emit DoHandshake(socket->socketDescriptor());
    }
    int peernum = connection_server_set.size() + connection_client_set.size();
    emit changePeer(peernum);
}

void NetworkDialog::readSocket() {
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_4);

    socketStream.startTransaction();
    socketStream >> buffer;
    if (!socketStream.commitTransaction()) {
        return;
    }

    if (buffer.size() < 36) {
        return;
    }
    long long desc = socket->socketDescriptor();
    uint8_t type[4]{};
    memcpy(type, buffer.first(4).data(), 4);

    if (Protocol::check_protocol_type(1, type)) {
        emit DoReceiveHandshake(desc, buffer);
    }
    else if (Protocol::check_protocol_type(2, type)) {
        emit DoRequestBlockFork(desc, buffer);
    }
    else if (Protocol::check_protocol_type(3, type)) {
        emit DoReceiveBlockFork(desc, buffer);
    }
    else if (Protocol::check_protocol_type(4, type)) {
        // sending new header
        emit DoReceiveBlockHeader(desc, buffer);
    }
    else if (Protocol::check_protocol_type(5, type)) {
        // send blockheader after fork
        emit DoReceiveSyncStatus(desc, buffer);
    }
    else if (Protocol::check_protocol_type(6, type)) {
        // request block by hash
        emit DoRequestBlockByHash(desc, buffer);
    }
    else {
        if (Block::check_magic_bytes(type)) {
            emit DoReceiveNewBlock(buffer);
        }
        else {
            emit DoReceiveNewData(buffer);
        }
    }
}

void NetworkDialog::discardSocket() {
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_server_set.find(socket);
    if (it != connection_server_set.end()) {
        emit DoBreakup(socket->socketDescriptor());
        connection_server_set.remove(*it);
    }
    socket->deleteLater();
    int peernum = connection_server_set.size() + connection_client_set.size();
    emit changePeer(peernum);
}

void NetworkDialog::setupClient(QString ip, int pp, int wdialog) {
    QString myip;
    const QHostAddress& localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress& address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
            myip = address.toString();
    }
    if (myip.compare(ip, Qt::CaseInsensitive) == 0) {
        if (wdialog == 1) {
            Information("Can't connect to " + ip + ":" + portCLineEdit->text());
        }
        return;
    }
    QTcpSocket* temp_socket = new QTcpSocket;
    temp_socket->connectToHost(ip, pp);
    if (temp_socket->waitForConnected(500)) {
        // If connected, retrieve the IP address and port
        QString ipAddress = temp_socket->peerAddress().toString();
        long long portNumber = temp_socket->peerPort();

        // Check if the IP address is a local address and already exists in the database
        bool isLocalAddress = QHostAddress(ipAddress).isLoopback();
        bool existsInDatabase = false;
        if (!isLocalAddress) {
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT COUNT(*) FROM connection WHERE ipAddress = :ip");
            checkQuery.bindValue(":ip", ipAddress);
            checkQuery.exec();

            if (checkQuery.next()) {
                int count = checkQuery.value(0).toInt();
                existsInDatabase = (count > 0);
            }
        }

        // Insert the IP address and port into the database if it's not a local address or doesn't exist in the database
        if (!isLocalAddress && !existsInDatabase) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO connection (ipAddress, port) VALUES (:ip, :port)");
            insertQuery.bindValue(":ip", ipAddress);
            insertQuery.bindValue(":port", portNumber);
            insertQuery.exec();
        }
        connection_client_set.insert(temp_socket);
        int peernum = connection_server_set.size() + connection_client_set.size();
        emit changePeer(peernum);
        connect(temp_socket, &QTcpSocket::readyRead, this, &NetworkDialog::readSocketClient);
        connect(temp_socket, &QTcpSocket::disconnected, this, &NetworkDialog::discardSocketClient);
        emit DoHandshake(temp_socket->socketDescriptor());
        if (wdialog == 1) {
            emit Information("Connected to " + ipAddress + ":" + QString::number(portNumber));
        }
    }
    else {
        delete temp_socket;
        if (wdialog == 1) {
            emit Information("Can't connect to " + ip + ":" + QString::number(port));
        }
    }
}

void NetworkDialog::readSocketClient() {
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_4);

    socketStream.startTransaction();
    socketStream >> buffer;
    if (!socketStream.commitTransaction()) {
        return;
    }

    if (buffer.size() < 36) {
        return;
    }
    long long desc = socket->socketDescriptor();
    uint8_t type[4]{};
    memcpy(type, buffer.first(4).data(), 4);
    if (Protocol::check_protocol_type(1, type)) {   
        emit DoReceiveHandshake(desc, buffer);
    }
    else if (Protocol::check_protocol_type(2, type)) {
        emit DoRequestBlockFork(desc, buffer);
    }
    else if (Protocol::check_protocol_type(3, type)) {
        emit DoReceiveBlockFork(desc, buffer);
    }
    else if (Protocol::check_protocol_type(4, type)) {
        // sending new header
        emit DoReceiveBlockHeader(desc, buffer);
    }
    else if (Protocol::check_protocol_type(5, type)) {
        // send blockheader after fork
        emit DoReceiveSyncStatus(desc, buffer);
    }
    else if (Protocol::check_protocol_type(6, type)) {
        // request block by hash
        emit DoRequestBlockByHash(desc, buffer);
    }
    else {
        if (Block::check_magic_bytes(type)) {
            emit DoReceiveNewBlock(buffer);
        }
        else {
            emit DoReceiveNewData(buffer);
        }
    }
}

void NetworkDialog::discardSocketClient() {
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_client_set.find(socket);
    if (it != connection_client_set.end()) {
        emit DoBreakup(socket->socketDescriptor());
        connection_client_set.remove(*it);
    }
    socket->deleteLater();
    int peernum = connection_server_set.size() + connection_client_set.size();
    emit changePeer(peernum);
}

void NetworkDialog::sendMethod(long long socketdescriptor, QByteArray data) {
    int a = 0;
    foreach(QTcpSocket * socket, connection_server_set)
    {
        long long thissocket = socket->socketDescriptor();
        if (thissocket == socketdescriptor)
        {
            sendingTo(socket, data);
            a++;
            break;
        }
    }
    if (a != 0) {
        return;
    }
    foreach(QTcpSocket * socket, connection_client_set)
    {
        long long thissocket = socket->socketDescriptor();
        if (thissocket == socketdescriptor)
        {
            sendingTo(socket, data);
            break;
        }
    }
}

void NetworkDialog::sendingTo(QTcpSocket* socket, QByteArray dat) {
    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_4);
    socketStream << dat;
}

void NetworkDialog::removePeer(long long key) {
    QString ip;
    foreach(QTcpSocket * socket, connection_server_set)
    {
        if (socket->socketDescriptor() == key) {
            QSet<QTcpSocket*>::iterator it = connection_server_set.find(socket);
            connection_server_set.erase(it);
            socket->close();
            socket->deleteLater();
            break;
        }
    }
    foreach(QTcpSocket* socket, connection_client_set)
    {
        if (socket->socketDescriptor() == key) {
            QSet<QTcpSocket*>::iterator it = connection_client_set.find(socket);
            connection_client_set.erase(it);
            socket->close();
            socket->deleteLater();
            break;
        }
    }
    int peer = connection_client_set.size() + connection_server_set.size();
    emit changePeer(peer);
}