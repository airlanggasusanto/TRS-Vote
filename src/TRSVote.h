#pragma once
#ifndef TRSVOTE_H
#define TRSVOTE_H
#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include "styles.h"
#include "ui_TRSVote.h"
#include "qutil.h"
#include <QProgressBar>
#include <QStackedWidget>
#include "mempool.h"
#include <QThread>
#include <unordered_map>
#include <QIcon>
#include <QVector>
#include <unordered_set>
#include "networkdialog.h"
#include "helper.h"

class TRSVote : public QMainWindow
{
    Q_OBJECT

public:
    TRSVote(QWidget* parent = nullptr);
    ~TRSVote();

private:
    // main menu and ui
    void SetupUI();
    QStackedWidget* stackedWidget;
    Ui::TRSVoteClass ui;
    dialogAbout* aboutTRSVote;
    QLabel* blockheightlabel;
    QLabel* peerlabel;
    QProgressBar* loadingBar;
    QLabel* statusloadingBar;
    QWidget* pleaseWaitWidget;
    void SetupPleaseWait();

    // login
    Block::VotePoll* genesisVotepoll;
    QKeypair* storedKey;
    LoginForm* loginWidget;
    LoginForm* registerWidget;
    void SetupLoginSystem();
    bool checkThatAccountValid(QString username, QString password);
    bool checkThatKeypairValid(QString privatekey);
    void Information(QString msg);

    // panel
    void SetupPanelSystem();
    PanelWidget* panelWidget;
    KeyPairDialog* keypairDialog;
    AddDialog* addDialog;
    PollingDetailDialog* detailDialog;
    RegistrationForm* registerDialog;
    VoteDialog* voteDialog;
    void handleRegister(QString hvp);
    void handleVote(QString hvp);
    void handleResult(QString hvp);
    void setupDialog(QString hvp, int m);
    void doingVote(QString hvp, QString pkcan);
    std::unordered_map<std::string, VoteTree::display*> set_display;

    // QThread
    MemPool* _mempool;
    QThread* _mempoolthread;
    Helper* _helper;
    QThread* _helperthread;
    void SetupMempool();
    void callGenesisForm();
    void setupVotepollGenesis(Block::VotePoll vp);
    void handleLogin(int m, std::string hvp, VoteTree::history h);
    void updatePolling(int m, const QString, const VoteTree::display);
    void resetPolling();
    void send_Info_to_LoadingBar(int method, int count);

    NetworkDialog* nsdialog;
    void sendMethod(long long key, QByteArray dat);

public slots:
    void HelperLogin(int a, QVector<QString> data);
    void setFlagHelper(int a, std::string hvp, bool s);
    void handleRegisterWHelper(QString hvp);
    void handleVoteWHelper(QString hvp);
    void handleResultWHelper(QString hvp);
    void handleMailWHelper(QString hvp);
    void setHistoryHelper(QString hvp, QString h, long long time, int a );
    void HelperToMemPoolNewData(QByteArray);
    void HelperAddMoreSize(std::string hvp, int m);
    void setMailHelper(int a, QString pk, QByteArray cipher, long long blocktime);
    void setListHelper(int a, QVector<QString> dat, long long blocktime);
    void setResultHelper(int a, VoteTree::DataCandidateString dat);
    void setVoteHelper(int a, VoteTree::DataCandidateString dat);
    void doingVoteWHelper(QString hvp, QString pkcan);
    void removePeer(long long key);
    void tryConn(QString ipAddress, long long port, int con);
    void ConnectionInfo(QString ip, long long port, bool con, int wdialog);
    void parseBlockHelper(QString path);
    void parseBlockFromBegHelper(QVector<QString> path);
    void Update_BlockHeight(int c);
};

#endif // TRSVote.h