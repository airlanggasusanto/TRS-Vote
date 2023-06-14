#pragma once
#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H
#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QLabel>
#include <QString>
#include <QIntValidator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSet>
#include <QEventLoop>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "sqlite3.h"
#include <QCheckBox>
#include "styles.h"
#include <QThread>

class NetworkDialog : public QDialog {
    Q_OBJECT
public:
    NetworkDialog(QWidget* parent = nullptr);
    ~NetworkDialog();

    void GettingPublicConn();
    void DefaultPort();
    void setupServer();
    void removePeer(long long key);
    QCheckBox* protectConnCheckBox;
    QCheckBox* supportValidatorCheckBox;
    QPushButton* saveButtonMoreSettings;
    void tryloop();
    void ConnectionInfo(QString ip, long long port, bool con, int withdialog);
public slots:
    void newConnection();
    void readSocket();
    void discardSocket();
    void readSocketClient();
    void discardSocketClient();
    void sendMethod(long long socketdescriptor, QByteArray data);

private:
    PleaseWaitDialog* pleaseWaitDialog;
    QNetworkAccessManager* manager;
    QLineEdit* serverLineEdit;
    QLineEdit* clientLineEdit;
    QLineEdit* portLineEdit;
    QPushButton* connectServerButton;
    QTabWidget* tabWidget;
    QWidget* serverSettingsTab;
    QWidget* addPeersTab;
    QWidget* moreSettingsTab;
    QVBoxLayout* layoutMoreSetting;
    QHBoxLayout* saveLayoutMoreSettings;
    QVBoxLayout* mainLayout;
    QTcpServer* m_server;
    QSet<QTcpSocket*> connection_server_set;
    QSet<QTcpSocket*> connection_client_set;
    QPushButton* connectButton;
    QLineEdit* portCLineEdit;
    QSqlDatabase db;
    int port;
    void createServerSettingsTab(QWidget* parent);
    void createAddPeersTab(QWidget* parent);
    void setupClient(QString ip, int port, int wdialog);
    void sendingTo(QTcpSocket* socket, QByteArray dat);
    void setupDatabase();
signals:
    void changePeer(int);
    void DoHandshake(long long key);
    void DoBreakup(long long key);
    void DoReceiveHandshake(long long key, QByteArray dat);
    void DoRequestBlockFork(long long key, QByteArray dat);
    void DoReceiveBlockFork(long long key, QByteArray dat);
    void DoReceiveBlockHeader(long long key, QByteArray dat);
    void DoReceiveSyncStatus(long long key, QByteArray dat);
    void DoRequestBlockByHash(long long key, QByteArray dat);
    void DoReceiveNewBlock(QByteArray dat);
    void DoReceiveNewData(QByteArray dat);
    void Information(QString info);
    void tryConn(QString ipAddress, long long port, int con);
};
#endif // NETWORKDIALOG_H