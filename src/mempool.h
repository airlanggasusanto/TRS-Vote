#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>

#include "util.h"
#include "block.h"
#include "qutil.h"
#include "merkle.h"
#include "votetree.h"
#include "protocol.h"

class MemPool : public	QObject
{
    Q_OBJECT
public:
    explicit MemPool(QObject* parent = nullptr);
    ~MemPool();
    void abort();
    void receiveNewData(QByteArray* data);
    void receiveNewBlock(QByteArray* data);
    void Method_Validating(int method);
    void Method_AddToVotePoll(int method, QByteArray* dat);
    void Method_StartVoting(int method, QByteArray* dat);
    void breakUp(long long descriptor);
    void DoHandshake(long long descriptor);
    void receiveHandshake(long long descriptor, QByteArray dat);
    void requestBlockFork(long long descriptor, QByteArray dat);
    void receiveBlockFork(long long descriptor, QByteArray dat);
    void receiveBlockHeader(long long descriptor, QByteArray dat);
    void receiveSyncStatus(long long descriptor, QByteArray dat);
    void requestBlockByHash(long long descriptor, QByteArray dat);
    void changeFlagSync(bool con);

private:
    QMutex mutex;
    QWaitCondition condition;
    bool _abort;
    bool _pause;
    Block::VotePoll genesis_votepoll;
    std::string genesis_key;
    bool genesis_sync;
    // storing a block and new block;
    std::unordered_map<std::string, std::string> _block_path;
    QVector<Block::Header*> _blockheader;
    QVector<std::string> ordered_blockpath;
    std::unordered_map<std::string, Block::Explorer*> _block_explorer;
    QVector<std::string> _queue_blockexplorer;

    QVector<Block::Block*> _queue_new_block;
    QVector<Block::Data*> _queue_new_blockdata;

    std::unordered_map<std::string, VoteTree::confirmed*> _votetree;
    std::unordered_map<std::string, VoteTree::unconfirmed*> s_votetree;

    QVector<std::string> ordered_valid_blockdatahash;
    std::unordered_map<std::string, Block::Data*> valid_blockdata;
    void LoadBlock();
    void HandleNewBlock();
    bool ValidateBlock(Block::Block* block);
    void HandleNewBlockData();
    void UpdateQuickCount();

    // for validating
    QVector<int> method_worker;
    bool _worker_status;
    Block::Header* _worker_header;
    Merkle::Tree* _worker_root;
    long long _worker_nonce;
    long long _worker_size;
    void HandleWorker();
    void doWorker();
    void resetWorker();
    void UpdateWorker(uint8_t* hash, long long size);
    void reValidateBlockData();

    // update votepoll
    Block::KeyPair* _addtovotepoll_key;
    int _addtovotepoll_method;
    QVector<QByteArray*> _addtovotepoll_data;
    void HandleAddToVotePoll();


    // startvoting 
    Block::KeyPair* _startvoting_key;
    int _startvoting_method;
    QVector<QByteArray*> _startvoting_data;
    std::unordered_map<std::string, int> _startvoting_index;
    void HandleStartVoting();

    // new connection made variable
    QVector<long long> _new_connection;
    QVector<long long> _handshake_key;
    QVector<QByteArray*> _handshake_value;

    // connection
    std::unordered_map<long long, Protocol::Data*> _conn;
    QVector<long long> _queue_breakup;

    // blockfrom fork variable
    QVector<long long> _queue_req_block_fork_key;
    QVector<QByteArray*> _queue_req_block_fork_value;
    QVector<long long> _queue_new_block_fork_key;
    QVector<QByteArray*> _queue_new_block_fork_value;
    Protocol::Sync* _forking;
    // fork
    QVector<std::string> _queue_fork;
    QVector<long long> _queue_sync_key;
    QVector<QByteArray*> _queue_sync_value;

    // already sync , got new block? go sync the header;
    QVector<long long> _queue_new_blockheader_key;
    QVector<QByteArray*> _queue_new_blockheader_value;
    QVector<long long> _req_block_by_hash_key;
    QVector<QByteArray*> _req_block_by_hash_value;

    // Handle connection 
    void HandleNewConn();
    void ProcessHandshake();
    void HandleBreakUp();
    void HandleReqBlockFork();
    void HandleNewBlockFork();
    void HandleNewSync();
    void HandleNewBlockHeader();
    void HandleReqBlockByHash();
    bool ValidateBlockNet(Block::Block*& blocktemp);
    void BroadcastData(QByteArray);


    void updateDataFromLast();
    void updateDataFromBeg();
public slots:
    void main();

signals:
    void finished();
    void send_Info_to_Validator(const int a, const QVector<QString> msg);
    void Update_BlockHeight(const int height);
    void send_Info_to_LoadingBar(const int a, const int msg);
    void send_display(const int, const QString, const VoteTree::display);
    void send_Info_to_AddToVotePoll(const int a, const QVector<QString> msg);
    void send_Info_to_StartVoting(const int a, const QVector<QString> msg);
    void Exit_Failure(const QString msg);
    void sendMethod(long long key, QByteArray dat);
    void reset_display();

    void setupVotepollGenesis(const Block::VotePoll vp);

    void callGenesisForm();

    void loginData(const int m, const std::string hvp, const VoteTree::history h);

    void parseBlockHelper(const QString path);
    void resetBlockHelper();
    void parseBlockFromBegHelper(const QVector<QString>path);
    void setupGenesisVotepollHelper(const Block::VotePoll vp);
    void removePeer(long long);
};