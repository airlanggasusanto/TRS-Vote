#pragma once
#ifndef HELPER_H
#define HELPER_H

#include <QThread>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

#include <string>
#include "block.h"
#include <filesystem>
#include <fstream>
#include "util.h"
#include "votetree.h"
#include <unordered_map>
#include <QVector>
#include <QString>
#include <QTcpSocket>
#include <QTcpServer>

class Helper : public QObject
{
	Q_OBJECT

public:
	explicit Helper(QObject* parent = nullptr);
	~Helper();
	void abort();
	void setupGenesisVotepoll(Block::VotePoll* vp);
	void Method_parseBlockFromBeg(QVector<QString> path);
	void Method_parseBlock(QString path);
	void Method_resetBlock();
	void Method_User(int con, QByteArray* dat);
	void Method_Admin_Add(int con, QByteArray* dat);
	void Method_Admin_Mail(int con, QByteArray* dat);
	void Method_Admin_List(int con, QString hvp);
	void Method_Admin_Result(int con, QString hvp);
	void Method_User_Vote(int con, QByteArray* dat);
	void tryConn(QString ipaddress, long long port, int);
private:
	QMutex mutex;
	QWaitCondition condition;
	bool _abort;
	bool _pause;
	bool _reset_block;
	Block::VotePoll* _genesis;

	std::unordered_map<std::string, VoteTree::display*> set_display;
	QVector<QString> hash_set_display;
	
	QVector<QString> _queue_parse_block;
	void parseBlock();
	void resetBlock();
	
	Block::KeyPair* keypair;
	QVector<int> _queue_user_method;
	QVector<QByteArray*> _queue_user_data;
	void handleUser();

	QVector<int> _queue_admin_add_method;
	QVector<QByteArray*> _queue_admin_add_data;
	void handleAdminAdd();

	QVector<QByteArray*> _queue_admin_mail_data;
	QVector<int> _queue_admin_mail_method;
	void handleAdminMail();

	QVector<QString> _queue_admin_list_data;
	QVector<int> _queue_admin_list_method;
	void handleAdminList();

	QVector<QString> _queue_admin_result_data;
	QVector<int> _queue_admin_result_method;
	void handleAdminResult();

	QVector<QByteArray*> _queue_user_vote_data;
	QVector<int> _queue_user_vote_method;
	void handleUserVote();

	QVector<QString> _queue_ipAddress;
	QVector<long long> _queue_port;
	QVector<int> _queue_conn_method;
	void handleTryConn();

public slots:
	void main();

signals:
	void finished();
	void Information(QString);
	void loginMethod(int a, QVector<QString> data);
	void setFlagMethod(int a, std::string hvp, bool b);
	void setHistory(QString, QString, long long, int);
	void HelperToMemPoolNewData(QByteArray dat);
	void AddMoreSize(std::string hvp, int);
	void setMail(int a, QString pk, QByteArray cipher, long long blocktime);
	void setList(int a, QVector<QString> dat, long long blocktime);
	void setResult(int a, VoteTree::DataCandidateString dat);
	void setVote(int a, VoteTree::DataCandidateString dat);
	void ConnectionInfo(QString ip, long long port, bool con, int withdialog);
};

#endif