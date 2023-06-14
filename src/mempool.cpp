#include "mempool.h"
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <filesystem>
#include <QFile>

MemPool::MemPool(QObject* parent) :
    QObject(parent)
{
    _abort = false;
    _pause = true;
    _worker_header = nullptr;
    _worker_nonce = NULL;
    _worker_root = nullptr;
    _worker_size = NULL;
    _worker_status = false;
    _addtovotepoll_key = nullptr;
    _addtovotepoll_method = NULL;
    _startvoting_method = NULL;
    _startvoting_key = nullptr;
    _forking = nullptr;
    uint8_t zero[32]{};
    genesis_key = Utility::uint8_t_to_hexstring(zero, 32);
    genesis_sync = true;
}

MemPool::~MemPool() {
    qDebug() << "MemPool is endded";
}

void MemPool::abort() {
    QMutexLocker locker(&mutex);
    _abort = true;
    condition.wakeOne();
}

void MemPool::main() {
    LoadBlock();
    resetWorker();
    forever{
        mutex.lock();

        if (!_abort && _pause && _queue_new_blockdata.isEmpty()) {
            condition.wait(&mutex);
        }
        if (!_worker_status) {
            _pause = true;
        }

        if (_abort) {
            mutex.unlock();
            emit finished();
            return;
        }

        int new_bd = _queue_new_blockdata.size();
        int new_b = _queue_new_block.size();
        int m_work = method_worker.size();
        bool dowork = _worker_status;

        int updatevotepoll = _addtovotepoll_data.size();
        int startvoting = _startvoting_data.size();

        int newconn = _new_connection.size();
        int handshake = _handshake_key.size();
        int somebodybreakup = _queue_breakup.size();
        int somebodysync = _queue_sync_key.size();
        int newblockheader = _queue_new_blockheader_key.size();
        int reqblockbyhash = _req_block_by_hash_key.size();
        int reqblockfork = _queue_req_block_fork_key.size();
        int newblockfork = _queue_new_block_fork_key.size();

        mutex.unlock();

        if (m_work) {
            HandleWorker();
            mutex.lock();
            dowork = _worker_status;
            mutex.unlock();
        }

        if (dowork || _blockheader.isEmpty()) {
            doWorker();
        }

        if (new_bd) {
            HandleNewBlockData();
        }

        if (new_b) {
            HandleNewBlock();
        }

        if (updatevotepoll) {
            HandleAddToVotePoll();
        }

        if (startvoting) {
            HandleStartVoting();
        }

        // end connectiong
        if (somebodybreakup) {
            HandleBreakUp();
        }

        // handshake
        if (newconn) {
            HandleNewConn();
        }

        if (handshake) {
            ProcessHandshake();
        }

        if (somebodysync) {
            HandleNewSync();
        }

        if (reqblockbyhash) {
            HandleReqBlockByHash();
        }

        if (reqblockfork) {
            HandleReqBlockFork();
        }

        if (newblockfork) {
            HandleNewBlockFork();
        }

        if (newblockheader) {
            HandleNewBlockHeader();
        }
    }
}

void MemPool::receiveNewData(QByteArray* data) {
    QMutexLocker locker(&mutex);
    _pause = false;
    Block::Data* bd = new Block::Data;
    QtUtility::raw_wt_to_blockdata(*&data, *bd);
    _queue_new_blockdata.push_back(*&bd);
    condition.wakeOne();
}

void MemPool::HandleNewBlockData() {
    Block::Data* value = nullptr;
    mutex.lock();
    std::swap(value, _queue_new_blockdata.front());
    _queue_new_blockdata.pop_front();
    mutex.unlock();

    if (value == nullptr) {
        return;
    }

    uint8_t uhash[32]{};
    Block::get_hash_blockdata(value, uhash);
    std::string hash = Utility::uint8_t_to_hexstring(uhash, 32);
    if (auto find = valid_blockdata.find(hash); find != valid_blockdata.end()) {
        delete value;
        return;
    }
    long long ttimestamp = QDateTime::currentSecsSinceEpoch();
    ttimestamp += 150;

    if (!VoteTree::validate_blockdata_to_votetree(value, ttimestamp, _votetree, s_votetree, genesis_votepoll)) {
        delete value;
        return;
    }
    valid_blockdata.emplace(hash, value);
    ordered_valid_blockdatahash.push_back(hash);

    if (_worker_status) {
        _pause = false;
        UpdateWorker(uhash, static_cast<long long>(value->datsize));
    }
    long long fs = static_cast<long long>(value->datsize) + 4;
    QByteArray sendthis = QByteArray((char*)&value->data[0], fs);
    BroadcastData(sendthis);
}

void MemPool::Method_Validating(int method) {
    QMutexLocker locker(&mutex);
    method_worker.push_back(method);
    _pause = false;
    condition.wakeOne();
}

void MemPool::HandleWorker() {
    mutex.lock();
    int method = method_worker.front();
    method_worker.pop_front();
    mutex.unlock();
    QVector<QString> msg;
    QVector<QString> pleasewait;
    pleasewait.push_back("Please Wait . . .");
    if (method == 2) {
        _worker_status = true;
        //emit send_Info_to_Validator(3, pleasewait);
        resetWorker();
        msg.push_back("Validating Started . . .");
        //emit send_Info_to_Validator(method, msg);
    }
    else if (method == 1) {
        _worker_status = false;
        //emit send_Info_to_Validator(3, pleasewait);
        if (_worker_header != nullptr) {
            delete _worker_header;
            _worker_header = nullptr;
        }
        if (_worker_root != nullptr) {
            delete _worker_root;
            _worker_root = nullptr;
        }
        msg.push_back("Validating Endded . . .");
        //emit send_Info_to_Validator(method, msg);
    }
    if (_worker_status) {
        _pause = false;
    }
    else {
        _pause = true;
    }
}

void MemPool::resetWorker() {
    if (_worker_header != nullptr) {
        delete _worker_header;
        _worker_header = nullptr;
    }
    if (_worker_root != nullptr) {
        delete _worker_root;
        _worker_root = nullptr;
    }
    _worker_size = 0;
    _worker_nonce = 0;
    _worker_header = new Block::Header;
    _worker_root = new Merkle::Tree;
    if (!_blockheader.isEmpty()) {
        Block::get_block_hash(*&_blockheader.last(), _worker_header->prevhash);
    }
    else {
        sodium_memzero(_worker_header->prevhash, 32);
    }
    if (!ordered_valid_blockdatahash.isEmpty()) {
        int maxhash = ordered_valid_blockdatahash.size();
        int a = 0;
        while (a < maxhash) {
            long long size = 0;
            if (auto find = valid_blockdata.find(ordered_valid_blockdatahash[a]); find != valid_blockdata.end()) {
                size = static_cast<long long>(find->second->datsize);
            }
            _worker_size += size;
            long long max = MAXIMUMBLOCKSIZE;
            if (size == 0) {
                _worker_size -= size;
                break;
            }
            if (_worker_size >= max) {
                _worker_size -= size;
                break;
            }
            Merkle::Hash ht;
            Utility::hexstring_to_uint8_t(ht.data, 32, ordered_valid_blockdatahash[a]);
            if (!Merkle::insert_tree(*_worker_root, &ht)) {
                _worker_size -= size;
                break;
            }
            a++;
        }
    }

    if (_worker_root->current != 0) {
        _worker_root->root = new Merkle::Hash;
        Merkle::calculate_hash(*_worker_root->root, _worker_root->merklehash[15][0], _worker_root->merklehash[15][1]);
        memcpy(_worker_header->merkleroot, _worker_root->root->data, 32);
        delete _worker_root->root;
        _worker_root->root = nullptr;
    }
    _pause = false;
}

void MemPool::UpdateWorker(uint8_t* hash, long long size) {
    int maxsize = MAXIMUMBLOCKSIZE;
    _worker_size += size;
    if (_worker_size > maxsize) {
        _worker_size -= size;
        return;
    }
    Merkle::Hash ht;
    memcpy(ht.data, hash, 32);
    if (_worker_root == nullptr) {
        _worker_root = new Merkle::Tree;
    }
    if (!Merkle::insert_tree(*_worker_root, &ht)) {
        _worker_size -= size;
        return;
    }
    if (_worker_header == nullptr) {
        _worker_size -= size;
        return;
    }
    Merkle::Hash temp;
    Merkle::calculate_hash(temp, _worker_root->merklehash[15][0], _worker_root->merklehash[15][1]);
    memcpy(_worker_header->merkleroot, temp.data, 32);
    if (_worker_status) {
        _pause = false;
    }
}

void MemPool::doWorker() {
    if (_worker_size < 1) {
        _pause = true;
        return;
    }
    if (_worker_header == nullptr) {
        resetWorker();
        return;
    }
    long long currenttime = QDateTime::currentSecsSinceEpoch();
    Utility::int_to_uint8_t(currenttime, _worker_header->timestamp, 4);
    Utility::int_to_uint8_t(_worker_nonce, _worker_header->nonce, 4);
    uint8_t hash[32]{};
    Block::get_block_hash(_worker_header, hash);
    if (Block::check_block_hash(hash)) {
        int checkifvalid = _blockheader.size();
        long long blocksize = _worker_size + 76;
        Block::Block* tempblock = new Block::Block;
        std::swap(tempblock->header, _worker_header);
        int votepolltotal = 0;
        int datavotetotal = 0;
        int datacandidate = 0;
        int datapartisipan = 0;
        for (int a = 0; a < _worker_root->merklehash[0].size(); a++) {
            if (_worker_root->merklehash[0][a] == nullptr) {
                break;
            }
            std::string hasheach = Utility::uint8_t_to_hexstring(_worker_root->merklehash[0][a]->data, 32);
            if (auto find = valid_blockdata.find(hasheach); find != valid_blockdata.end()) {
                if (Block::check_blockdata_type(find->second->type, 1)) {
                    votepolltotal++;
                }
                else if (Block::check_blockdata_type(find->second->type, 2)) {
                    datavotetotal++;
                }
                else if (Block::check_blockdata_type(find->second->type, 3)) {
                    datacandidate++;
                }
                else if (Block::check_blockdata_type(find->second->type, 4)) {
                    datapartisipan++;
                }
                Block::Data* tempbd = new Block::Data(find->second);
                blocksize += 4;
                tempblock->blockdata.push_back(tempbd);
            }
            else {
                delete tempblock;
                resetWorker();
                return;
            }
        }
        _queue_new_block.push_front(tempblock);
        HandleNewBlock();
        int checkthis = _blockheader.size();
        QVector<QString> msg;
        QString newblkhash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
        msg.push_back(QString("___________________________________________________"));
        msg.push_back(QString("Successfully getting new block : "));
        msg.push_back(QString("Blockhash : " + newblkhash));
        msg.push_back(QString("At Time : " + QDateTime::fromSecsSinceEpoch(currenttime).toString("h:m:s ap - dd/MM/yyyy")));
        msg.push_back(QString("Vote Poll Tervalidasi : " + QString::number(votepolltotal)));
        msg.push_back(QString("Data Vote Tervalidasi : " + QString::number(datavotetotal)));
        msg.push_back(QString("Data Candidate Tervalidasi : " + QString::number(datacandidate)));
        msg.push_back(QString("Data Partisipan Tervalidasi : " + QString::number(datapartisipan)));
        msg.push_back(QString("Total Block Size : " + QString::number(blocksize) + " bytes"));
        msg.push_back(QString("___________________________________________________"));
        if (checkifvalid == checkthis) {
            reValidateBlockData();
            return;
        }
        else {
            //emit send_Info_to_Validator(3, msg);
        }
    }

    long long maxnonce = MAXIMUMNONCE;
    _worker_nonce += 64;
    _worker_nonce -= 1;
    if (_worker_nonce >= maxnonce) {
        _worker_nonce = 0;
    }
}

void MemPool::reValidateBlockData() {
    for (auto it = s_votetree.begin(); it != s_votetree.end(); ) {
        delete it->second;
        it = s_votetree.erase(it);
    }
    s_votetree.clear();

    for (auto& [key, value] : _votetree) {
        if (key == genesis_key) {
            continue;
        }
        VoteTree::unconfirmed* temp = new VoteTree::unconfirmed;
        if (!value->tally.empty()) {
            Block::get_hash_datacandidate(&value->tally.back().data, temp->lasthash_candidate);
        }
        else {
            Block::get_hash_votepoll(value->votepoll, temp->lasthash_candidate);
        }
        if (!value->hash_data_partisipan.empty()) {
            memcpy(temp->lasthash_partisipan, value->hash_data_partisipan.back().data, 32);
        }
        else {
            Block::get_hash_votepoll(value->votepoll, temp->lasthash_partisipan);
        }
        s_votetree.emplace(key, temp);
    }
    auto it = valid_blockdata.begin();
    while (it != valid_blockdata.end()) {
        long long timestamp = QDateTime::currentSecsSinceEpoch();
        if (!VoteTree::validate_blockdata_to_votetree(it->second, timestamp, _votetree, s_votetree, genesis_votepoll)) {
            if (int b = ordered_valid_blockdatahash.indexOf(it->first); b != -1) {
                ordered_valid_blockdatahash.removeAt(b);
            }
            delete it->second;
            it = valid_blockdata.erase(it);
        }
        else {
            ++it;
        }
    }
}

void MemPool::LoadBlock() {
    if (!std::filesystem::exists("net")) {
        std::filesystem::create_directories("net");
    }
    else {
        std::filesystem::remove_all("net");
        std::filesystem::create_directories("net");
    }
    QEventLoop waitloop;
    QTimer::singleShot(3000, &waitloop, SLOT(quit()));
    waitloop.exec();
    waitloop.deleteLater();
    QVector<std::string> _blk_path;
    const std::filesystem::path block_path{ "block" };
    if (!std::filesystem::exists(block_path)) {
        std::filesystem::create_directories(block_path);
    }
    for (const auto& entry : std::filesystem::directory_iterator(block_path)) {
        std::string something = entry.path().string();
        _blk_path.push_back(something);
    }
    for (int a = 0; a < _blk_path.size(); a++) {
        mutex.lock();
        bool statusss = _abort;
        mutex.unlock();
        if (statusss) {
            return;
        }
        long long blksize = std::filesystem::file_size(_blk_path[a]);
        std::ifstream read(_blk_path[a], std::ios::in | std::ios::binary);
        uint8_t* blkdat = new uint8_t[blksize]{};
        read.seekg(0, std::ios::beg);
        read.read((char*)&blkdat[0], blksize);
        read.close();
        if (Block::check_magic_bytes(blkdat)) {
            Block::Block* temp_block = new Block::Block;
            Block::raw_wm_to_block(blkdat, *temp_block, blksize);
            if (ValidateBlock(temp_block)) {
                Block::Header* this_header = nullptr;
                std::swap(this_header, temp_block->header);
                _blockheader.push_back(this_header);
                uint8_t uhash[32]{};
                int heightblock = _blockheader.size() - 1;
                Block::get_block_hash(this_header, uhash);
                std::string shash = Utility::uint8_t_to_hexstring(uhash, 32);
                _block_path.emplace(shash, _blk_path[a]);
                ordered_blockpath.push_back(_blk_path[a]);
                UpdateQuickCount();
                if (a != 0) {
                    emit parseBlockHelper(_blk_path[a].c_str());
                }
                emit Update_BlockHeight(heightblock);
            }
            delete temp_block;
        }
        delete[] blkdat;
    }
    if (_blockheader.isEmpty()) {
        emit callGenesisForm();
    }
}

void MemPool::receiveNewBlock(QByteArray* data) {
    QMutexLocker locker(&mutex);
    _pause = false;
    Block::Block* queue = new Block::Block;
    QtUtility::raw_wm_to_block(data, *queue, data->size());
    _queue_new_block.push_back(queue);
    condition.wakeOne();
}

void MemPool::HandleNewBlock() {
    Block::Block* value = nullptr;
    mutex.lock();
    std::swap(value, _queue_new_block.front());
    _queue_new_block.pop_front();
    mutex.unlock();

    if (value == nullptr) {
        return;
    }

    if (!ValidateBlock(value)) {
        delete value;
        return;
    }
    std::string mainpath = "block";
    int index = _blockheader.size();
    Block::block_to_file(*&value, mainpath, index);
    Block::Header* storethis = nullptr;
    std::swap(storethis, value->header);
    uint8_t uhash[32]{};
    Block::get_block_hash(storethis, uhash);
    std::string hash = Utility::uint8_t_to_hexstring(uhash, 32);
    delete value;
    _block_path.emplace(hash, mainpath);
    ordered_blockpath.push_back(mainpath);
    _blockheader.push_back(storethis);
    emit Update_BlockHeight(index);
    UpdateQuickCount();
    if (ordered_blockpath.size() > 1) {
        emit parseBlockHelper(ordered_blockpath.last().c_str());
    }
    if (_forking == nullptr) {
        uint8_t headerraw[72]{};
        Block::blockheader_to_raw(storethis, headerraw);
        QByteArray sendthis;
        uint8_t pt[4]{};
        Protocol::get_protocol_type(4, pt);
        sendthis.prepend(QByteArray((char*)&pt[0], 4), 4);
        sendthis.append(QByteArray((char*)&headerraw[0], 72), 72);
        BroadcastData(sendthis);
    }
}

bool MemPool::ValidateBlock(Block::Block* block) {
    // declaring thiss block 
    uint8_t thash[32]{};
    Block::get_block_hash(block->header, thash);
    long long ttimestamp = Utility::uint8_t_to_int(block->header->timestamp, 4);
    // declaring for validating
    uint8_t uhash[32]{};
    long long utimestamp = 0;
    if (!Block::check_block_hash(thash)) {
        return false;
    }

    if (_blockheader.isEmpty()) {
        sodium_memzero(uhash, 32);
    }
    else {
        Block::get_block_hash(_blockheader.last(), uhash);
        utimestamp = Utility::uint8_t_to_int(_blockheader.last()->timestamp, 4);
    }

    if (utimestamp > ttimestamp) {
        return false;
    }

    if (sodium_memcmp(uhash, block->header->prevhash, 32) != 0) {
        return false;
    }
    // deletion
    QVector<std::string> deletethis;
    QtUtility::get_blockdata_hash_qvector(block, deletethis);

    // calculate merkleroot;
    // our implementation can't fit more than 65535 transaction;
    if (block->blockdata.size() > 65536) {
        return false;
    }
    std::unordered_map <std::string, VoteTree::unconfirmed*> _vtree;
    for (auto& [key, value] : _votetree) {
        if (key == genesis_key) {
            continue;
        }
        VoteTree::unconfirmed* tempunconf = new VoteTree::unconfirmed;
        if (!value->tally.empty()) {
            Block::get_hash_datacandidate(&value->tally.back().data, tempunconf->lasthash_candidate);
        }
        else {
            Block::get_hash_votepoll(value->votepoll, tempunconf->lasthash_candidate);
        }
        if (!value->hash_data_partisipan.empty()) {
            memcpy(tempunconf->lasthash_partisipan, value->hash_data_partisipan.back().data, 32);
        }
        else {
            Block::get_hash_votepoll(value->votepoll, tempunconf->lasthash_partisipan);
        }
        _vtree.emplace(key, tempunconf);
    }

    emit send_Info_to_LoadingBar(1, block->blockdata.size());
    Merkle::Tree* uroot = new Merkle::Tree;
    for (int a = 0; a < block->blockdata.size(); a++) {
        emit send_Info_to_LoadingBar(2, a);
        if (block->blockdata[a] == nullptr) {
            break;
        }
        // validate blockdata
        bool valid = VoteTree::validate_blockdata_to_votetree(block->blockdata[a], ttimestamp, _votetree, _vtree, genesis_votepoll);
        if (valid) {
            Merkle::Hash ublockhashtree;
            Block::get_hash_blockdata(block->blockdata[a], ublockhashtree.data);
            Merkle::insert_tree(*uroot, &ublockhashtree);
        }
        else {
            delete uroot;
            std::for_each(_vtree.begin(), _vtree.end(), [](auto& p) { delete p.second; });
            _vtree.clear();
            emit send_Info_to_LoadingBar(6, 0);
            return false;
        }
    }
    emit send_Info_to_LoadingBar(2, block->blockdata.size());
    std::for_each(_vtree.begin(), _vtree.end(), [](auto& p) { delete p.second; });
    _vtree.clear();

    // verify merklerootl
    if (!Merkle::verify_tree(*uroot, block->header->merkleroot)) {
        delete uroot;
        return false;
    }
    delete uroot;
    std::string blockhash = Utility::uint8_t_to_hexstring(thash, 32);
    Block::Explorer* newxplorer = new Block::Explorer;
    newxplorer->time = ttimestamp;
    for (int a = 0; a < block->blockdata.size(); a++) {
        if (block->blockdata[a] == nullptr) {
            break;
        }
        newxplorer->set_hash.emplace(deletethis[a]);
        VoteTree::insert_blockdata_to_votetree(block->blockdata[a], ttimestamp, _votetree, s_votetree, genesis_votepoll);
        if (auto find = valid_blockdata.find(deletethis[a]); find != valid_blockdata.end()) {
            if (find->second != nullptr) {
                delete find->second;
                find->second = nullptr;
            }
            std::string key = find->first;
            if (int b = ordered_valid_blockdatahash.indexOf(key); b != -1) {
                ordered_valid_blockdatahash.removeAt(b);
            }
            valid_blockdata.erase(find);
        }
    }
    if (auto find_explorer = _block_explorer.find(blockhash); find_explorer != _block_explorer.end()) {
        delete find_explorer->second;
        std::swap(find_explorer->second, newxplorer);
    }
    else {
        _block_explorer.emplace(blockhash, newxplorer);
    }
    emit send_Info_to_LoadingBar(3, 0);
    return true;
}

void MemPool::UpdateQuickCount() {
    if (_votetree.size() == 1) {
        emit setupGenesisVotepollHelper(genesis_votepoll);
        return;
    }
}

void MemPool::Method_AddToVotePoll(int method, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _addtovotepoll_method = method;
    _addtovotepoll_data.push_back(new QByteArray(*dat));
    condition.wakeOne();
}

void MemPool::HandleAddToVotePoll() {
    mutex.lock();
    int method = _addtovotepoll_method;
    QByteArray* data = _addtovotepoll_data.front();
    _addtovotepoll_data.pop_front();
    mutex.unlock();
    QVector<QString> msg;
    if (method == 1) {
        if (_addtovotepoll_key != nullptr) {
            delete _addtovotepoll_key;
            _addtovotepoll_key = nullptr;
        }
        if (data->size() == 32) {
            _addtovotepoll_key = new Block::KeyPair;
            memcpy(_addtovotepoll_key->privatekey, data->data(), 32);
            _addtovotepoll_key->calculate_base();
        }
        else {
            msg.push_back("Login Failed . . .");
            emit send_Info_to_AddToVotePoll(method, msg);
            delete data;
            return;
        }
        delete data;
        method = 0;
        for (const auto& [key, value] : _votetree) {
            if (key == genesis_key) {
                continue;
            }
            msg.clear();
            if (sodium_memcmp(value->votepoll->publickey, _addtovotepoll_key->publickey, 32) == 0) {
                method = 4;
                msg.push_back(QString::fromStdString(key));
                emit send_Info_to_AddToVotePoll(method, msg);
            }
        }
        if (method != 4) {
            method = 6;
            emit send_Info_to_AddToVotePoll(method, msg);
            return;
        }
        method = 1;
        msg.clear();
        emit send_Info_to_AddToVotePoll(method, msg);
    }
    else if (method == 2) {
        delete data;
        if (_addtovotepoll_key != nullptr) {
            delete _addtovotepoll_key;
            _addtovotepoll_key = nullptr;
        }
        _addtovotepoll_data.clear();
        emit send_Info_to_AddToVotePoll(method, msg);
    }
    else if (method == 3) {
        if (data->size() != 96 || _addtovotepoll_key == nullptr) {
            emit Exit_Failure("Error while trying create Data Candidate");
            return;
        }
        Block::DataCandidate dc;
        memcpy(dc.hashvotepoll, data->data(), 32);
        memcpy(dc.candidatename, data->data() + 32, 32);
        memcpy(dc.publickey, data->data() + 64, 32);
        delete data;
        std::string s_hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
        std::string s_pk = Utility::uint8_t_to_hexstring(dc.publickey, 32);
        auto find_hvp = _votetree.find(s_hvp);
        if (find_hvp == _votetree.end()) {
            emit Exit_Failure("Error while trying create Data Candidate");
            return;
        }
        long long startdate = Utility::uint8_t_to_int(find_hvp->second->votepoll->startdate, 4);
        long long timenow = QDateTime::currentSecsSinceEpoch();
        timenow += 150;
        if (startdate < timenow) {
            msg.push_back("Failed to Create Data due Votepoll already started");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        long long maxc = Utility::uint8_t_to_int(find_hvp->second->votepoll->candidatesize, 4);
        long long currentc = find_hvp->second->tally.size();
        auto find_s_tree = s_votetree.find(s_hvp);
        if (find_s_tree != s_votetree.end()) {
            currentc += find_s_tree->second->set_pk_candidate.size();
            auto findthispk_s = find_s_tree->second->set_pk_candidate.find(s_pk);
            if (findthispk_s != find_s_tree->second->set_pk_candidate.end()) {
                msg.push_back("Failed to Create Data, due Publickey already written");
                emit send_Info_to_AddToVotePoll(5, msg);
                return;
            }
        }
        if (currentc >= maxc) {
            msg.push_back("Failed to Create Data, due Votepoll Candidate has already reached maximum");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        auto findthispk = find_hvp->second->set_pk_candidate.find(s_pk);
        if (findthispk != find_hvp->second->set_pk_candidate.end()) {
            msg.push_back("Failed to Create Data, due Publickey already written");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        if (find_s_tree != s_votetree.end()) {
            memcpy(dc.parenthash, find_s_tree->second->lasthash_candidate, 32);
        }
        else {
            if (!find_hvp->second->tally.empty()) {
                Block::get_hash_datacandidate(&find_hvp->second->tally.back().data, dc.parenthash);
            }
            else {
                Block::get_hash_votepoll(find_hvp->second->votepoll, dc.parenthash);
            }
        }
        Block::calculate_datacandidate_signature(&dc, dc.signature, _addtovotepoll_key);
        uint8_t hash[32]{};
        Block::get_hash_datacandidate(&dc, hash);
        std::string shash = Utility::uint8_t_to_hexstring(hash, 32);
        msg.push_back(shash.c_str());
        emit send_Info_to_AddToVotePoll(method, msg);
        Block::Data* blkdat = new Block::Data;
        Block::datacandidate_to_blockdata(&dc, *blkdat);
        _queue_new_blockdata.push_back(blkdat);
        return;

    }
    else if (method == 4) {
        if (data->size() != 64 || _addtovotepoll_key == nullptr) {
            emit Exit_Failure("Error while trying create Data Partisipan");
            return;
        }
        Block::DataPartisipan dp;
        memcpy(dp.hashvotepoll, data->data(), 32);
        memcpy(dp.publickey, data->data() + 32, 32);
        std::string s_hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
        std::string s_pk = Utility::uint8_t_to_hexstring(dp.publickey, 32);
        auto find_hvp = _votetree.find(s_hvp);
        if (find_hvp == _votetree.end()) {
            emit Exit_Failure("Error while trying create Data Partisipan");
            return;
        }
        long long startdate = Utility::uint8_t_to_int(find_hvp->second->votepoll->startdate, 4);
        long long timenow = QDateTime::currentSecsSinceEpoch();
        timenow += 150;
        if (startdate < timenow) {
            msg.push_back("Failed to Create Data due Votepoll already started");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        long long maxc = Utility::uint8_t_to_int(find_hvp->second->votepoll->partisipansize, 4);
        long long currentc = find_hvp->second->hash_data_partisipan.size();
        auto find_s_tree = s_votetree.find(s_hvp);
        if (find_s_tree != s_votetree.end()) {
            currentc += find_s_tree->second->set_pk_partisipan.size();
            auto findthispk_s = find_s_tree->second->set_pk_partisipan.find(s_pk);
            if (findthispk_s != find_s_tree->second->set_pk_partisipan.end()) {
                msg.push_back("Failed to Create Data, due Publickey already written");
                emit send_Info_to_AddToVotePoll(5, msg);
                return;
            }
        }
        if (currentc >= maxc) {
            msg.push_back("Failed to Create Data, due Votepoll Partisipan has already reached maximum");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        auto findthispk = find_hvp->second->set_pk_partisipan.find(s_pk);
        if (findthispk != find_hvp->second->set_pk_partisipan.end()) {
            msg.push_back("Failed to Create Data, due Publickey already written");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        if (find_s_tree != s_votetree.end()) {
            memcpy(dp.parenthash, find_s_tree->second->lasthash_partisipan, 32);
        }
        else {
            if (!find_hvp->second->hash_data_partisipan.empty()) {
                memcpy(dp.parenthash, find_hvp->second->hash_data_partisipan.back().data, 32);
            }
            else {
                Block::get_hash_votepoll(find_hvp->second->votepoll, dp.parenthash);
            }
        }
        Block::calculate_datapartisipan_signature(&dp, dp.signature, _addtovotepoll_key);
        uint8_t hash[32]{};
        Block::get_hash_datapartisipan(&dp, hash);
        std::string shash = Utility::uint8_t_to_hexstring(hash, 32);
        msg.push_back(shash.c_str());
        emit send_Info_to_AddToVotePoll(3, msg);
        Block::Data* blkdat = new Block::Data;
        Block::datapartisipan_to_blockdata(&dp, *blkdat);
        _queue_new_blockdata.push_back(blkdat);
    }
    else if (method == 5) {
        uint8_t hvp[32]{};
        memcpy(hvp, data->data(), 32);
        int jsonsize = data->size() - 32;
        QByteArray jsonraw;
        jsonraw.resize(jsonsize);
        memcpy(jsonraw.data(), data->data() + 32, jsonsize);
        delete data;
        std::string s_hvp = Utility::uint8_t_to_hexstring(hvp, 32);
        auto find_hvp = _votetree.find(s_hvp);
        if (find_hvp == _votetree.end()) {
            emit Exit_Failure("Error while trying create Data Candidate");
            return;
        }
        long long startdate = Utility::uint8_t_to_int(find_hvp->second->votepoll->startdate, 4);
        long long timenow = QDateTime::currentSecsSinceEpoch();
        timenow += 150;
        if (startdate < timenow) {
            msg.push_back("Failed to Create Data due Votepoll already started");
            emit send_Info_to_AddToVotePoll(5, msg);
            return;
        }
        // Parse the QByteArray as a JSON document
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonraw);

        if (jsonDoc.isNull()) {
            msg.push_back("Failed to parse JSON");
            emit send_Info_to_AddToVotePoll(method, msg);
            return;
        }
        QVector<QtUtility::QJsonDC> dc;
        QVector<QtUtility::QJsonDP>dp;

        if (jsonDoc.isArray()) {
            QtUtility::parseJson(jsonDoc.array(), dp, dc);
        }
        else if (jsonDoc.isObject()) {
            QtUtility::parseJson(jsonDoc.object(), dp, dc);
        }
        int contains = dp.size() + dc.size();
        if (contains == 0) {
            msg.push_back("Failed to parse JSON");
            emit send_Info_to_AddToVotePoll(method, msg);
            return;
        }
        msg.push_back(QString::number(contains));
        emit send_Info_to_AddToVotePoll(7, msg);
        msg.clear();
        // Step 2: Create an empty QJsonArray
        QJsonArray jsonArray;

        int success = 0;
        for (int i = 0; i < dc.size(); i++) {
            QJsonObject jsonObject;
            jsonObject["_hash votepoll"] = s_hvp.c_str();
            jsonObject["type"] = "candidate";
            jsonObject["name"] = dc[i].name;
            jsonObject["publickey"] = dc[i].pk;
            Block::DataCandidate dci;
            std::string s_pk = dc[i].pk.toStdString();
            if (dc[i].name.length() > 32) {
                jsonObject.insert("_status", "Fail, The name length exceeds 32 bytes. Please enter a shorter name.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            if (!std::all_of(s_pk.begin(), s_pk.end(), ::isxdigit)) {
                jsonObject.insert("_status", "Fail, The public key must be a 32-byte hexadecimal value.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            long long maxc = Utility::uint8_t_to_int(find_hvp->second->votepoll->candidatesize, 4);
            long long currentc = find_hvp->second->tally.size();
            auto find_s_tree = s_votetree.find(s_hvp);
            if (find_s_tree != s_votetree.end()) {
                currentc += find_s_tree->second->set_pk_candidate.size();
                auto findthispk_s = find_s_tree->second->set_pk_candidate.find(s_pk);
                if (findthispk_s != find_s_tree->second->set_pk_candidate.end()) {
                    jsonObject.insert("_status", "Fail, The public key already exists.");
                    jsonArray.push_back(jsonObject);
                    continue;
                }
            }
            if (currentc >= maxc) {
                jsonObject.insert("_status", "Fail, The data candidate has reached the maximum limit.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            auto findthispk = find_hvp->second->set_pk_candidate.find(s_pk);
            if (findthispk != find_hvp->second->set_pk_candidate.end()) {
                jsonObject.insert("_status", "Fail, The public key already exists.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            if (find_s_tree != s_votetree.end()) {
                memcpy(dci.parenthash, find_s_tree->second->lasthash_candidate, 32);
            }
            else {
                if (!find_hvp->second->tally.empty()) {
                    Block::get_hash_datacandidate(&find_hvp->second->tally.back().data, dci.parenthash);
                }
                else {
                    Block::get_hash_votepoll(find_hvp->second->votepoll, dci.parenthash);
                }
            }
            memcpy(dci.hashvotepoll, hvp, 32);
            Utility::hexstring_to_uint8_t(dci.publickey, 32, s_pk);
            Utility::ascii_to_uint8_t(dc[i].name.toStdString(), dci.candidatename, 32);
            Block::calculate_datacandidate_signature(&dci, dci.signature, _addtovotepoll_key);
            uint8_t hash[32]{};
            Block::get_hash_datacandidate(&dci, hash);
            std::string shash = Utility::uint8_t_to_hexstring(hash, 32);
            Block::Data* blkdat = new Block::Data;
            Block::datacandidate_to_blockdata(&dci, *blkdat);
            _queue_new_blockdata.push_front(blkdat);
            HandleNewBlockData();
            QString ms = "Success, " + QString::fromStdString(shash);
            jsonObject.insert("_status", ms);
            success++;
            emit send_Info_to_AddToVotePoll(8, msg);
            jsonArray.push_back(jsonObject);
        }
        for (int i = 0; i < dp.size(); i++) {
            QJsonObject jsonObject;
            jsonObject["_hash votepoll"] = s_hvp.c_str();
            jsonObject["type"] = "partisipan";
            jsonObject["publickey"] = dp[i].pk;
            Block::DataPartisipan dpi;
            std::string spk = dp[i].pk.toStdString();
            if (!std::all_of(spk.begin(), spk.end(), ::isxdigit)) {
                jsonObject.insert("_status", "Fail, The public key must be a 32-byte hexadecimal value.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            long long maxc = Utility::uint8_t_to_int(find_hvp->second->votepoll->partisipansize, 4);
            long long currentc = find_hvp->second->hash_data_partisipan.size();
            auto find_s_tree = s_votetree.find(s_hvp);
            if (find_s_tree != s_votetree.end()) {
                currentc += find_s_tree->second->set_pk_partisipan.size();
                auto findthispk_s = find_s_tree->second->set_pk_partisipan.find(spk);
                if (findthispk_s != find_s_tree->second->set_pk_partisipan.end()) {
                    jsonObject.insert("_status", "Fail, The public key already exists.");
                    jsonArray.push_back(jsonObject);
                    continue;
                }
            }
            if (currentc >= maxc) {
                jsonObject.insert("_status", "Fail, The data partisipan has reached the maximum limit.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            auto findthispk = find_hvp->second->set_pk_partisipan.find(spk);
            if (findthispk != find_hvp->second->set_pk_partisipan.end()) {
                jsonObject.insert("_status", "Fail, The public key already exists.");
                jsonArray.push_back(jsonObject);
                continue;
            }
            if (find_s_tree != s_votetree.end()) {
                memcpy(dpi.parenthash, find_s_tree->second->lasthash_partisipan, 32);
            }
            else {
                if (!find_hvp->second->hash_data_partisipan.empty()) {
                    memcpy(dpi.parenthash, find_hvp->second->hash_data_partisipan.back().data, 32);
                }
                else {
                    Block::get_hash_votepoll(find_hvp->second->votepoll, dpi.parenthash);
                }
            }
            memcpy(dpi.hashvotepoll, hvp, 32);
            Utility::hexstring_to_uint8_t(dpi.publickey, 32, spk);
            Block::calculate_datapartisipan_signature(&dpi, dpi.signature, _addtovotepoll_key);
            uint8_t hash[32]{};
            Block::get_hash_datapartisipan(&dpi, hash);
            std::string shash = Utility::uint8_t_to_hexstring(hash, 32);
            Block::Data* blkdat = new Block::Data;
            Block::datapartisipan_to_blockdata(&dpi, *blkdat);
            _queue_new_blockdata.push_front(blkdat);
            HandleNewBlockData();
            QString ms = "Success, " + QString::fromStdString(shash);
            jsonObject.insert("_status", ms);
            success++;
            emit send_Info_to_AddToVotePoll(8, msg);
            jsonArray.push_back(jsonObject);
        }
        std::string filepath = "output.json";
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
        }
        QJsonDocument outputdoc(jsonArray);
        QString output = outputdoc.toJson(QJsonDocument::Indented);
        QFile file(filepath.c_str());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << output;
            file.close();
        }
        msg.clear();
        msg.push_back(QString::number(success));
        emit send_Info_to_AddToVotePoll(9, msg);
        return;
    }
}

void MemPool::Method_StartVoting(int method, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _startvoting_method = method;
    _startvoting_data.push_back(new QByteArray(*dat));
    if (_pause) {
        _pause = false;
        condition.wakeOne();
    }
}

void MemPool::HandleStartVoting() {
    mutex.lock();
    int method = _startvoting_method;
    QByteArray* data = _startvoting_data.front();
    _startvoting_data.pop_front();
    mutex.unlock();
    QVector<QString> msg;
    if (method == 1) {
        if (_startvoting_key != nullptr) {
            delete _startvoting_key;
            _startvoting_key = nullptr;
        }
        if (data->size() == 32) {
            _startvoting_key = new Block::KeyPair;
            memcpy(_startvoting_key->privatekey, data->data(), 32);
            _startvoting_key->calculate_base();
        }
        else {
            msg.push_back("Login Failed . . .");
            emit send_Info_to_StartVoting(method, msg);
            delete data;
            return;
        }
        delete data;
        std::string spk = Utility::uint8_t_to_hexstring(_startvoting_key->publickey, 32);
        method = 0;
        for (const auto& [key, value] : _votetree) {
            msg.clear();
            auto findpk = value->set_pk_partisipan.find(spk);
            if (findpk != value->set_pk_partisipan.end()) {
                msg.push_back(QString::fromStdString(key));
                method = 4;
                emit send_Info_to_StartVoting(method, msg);
                _startvoting_index.emplace(key, findpk->second);
            }
        }
        if (method != 4) {
            method = 6;
            emit send_Info_to_StartVoting(method, msg);
            return;
        }
        method = 1;
        msg.clear();
        emit send_Info_to_StartVoting(method, msg);
    }
    else if (method == 2) {
        delete data;
        if (_startvoting_key != nullptr) {
            delete _startvoting_key;
            _startvoting_key = nullptr;
        }
        _startvoting_index.clear();
        emit send_Info_to_StartVoting(method, msg);
    }
    else if (method == 3) {
        if (_startvoting_key == nullptr) {
            emit Exit_Failure("Unexpected error when trying Create Vote");
            return;
        }
        if (data->size() != 64) {
            emit Exit_Failure("Unexpected error when trying Create Vote");
            return;
        }
        Block::DataVote dv;
        uint8_t cpk[32]{};
        memcpy(dv.tag.issue, data->data(), 32);
        memcpy(cpk, data->data() + 32, 32);
        delete data;
        std::string s_issue = Utility::uint8_t_to_hexstring(dv.tag.issue, 32);
        auto find = _votetree.find(Utility::uint8_t_to_hexstring(dv.tag.issue, 32));
        if (find == _votetree.end()) {
            emit Exit_Failure("Unexpected error when trying Create Vote");
            return;
        }
        if (auto findpkc = find->second->set_pk_candidate.find(Utility::uint8_t_to_hexstring(cpk, 32)); findpkc == find->second->set_pk_candidate.end()) {
            emit Exit_Failure("Unexpected error when trying Create Vote");
            return;
        }
        else {
            Block::get_hash_datacandidate(&find->second->tally[findpkc->second].data, dv.hashdatacandidate);
        }
        long long end = Utility::uint8_t_to_int(find->second->votepoll->enddate, 4);
        long long start = Utility::uint8_t_to_int(find->second->votepoll->startdate, 4);
        long long current = QDateTime::currentSecsSinceEpoch();
        current += 150;
        if (current < start) {
            msg.push_back("Failed to create Vote, The VotePoll has not started yet");
            emit send_Info_to_StartVoting(5, msg);
            return;
        }
        if (current > end) {
            msg.push_back("Failed to create Vote, The Votepoll has ended");
            emit send_Info_to_StartVoting(5, msg);
            return;
        }
        int indexpk = 0;
        if (auto find_index = _startvoting_index.find(s_issue); find_index != _startvoting_index.end()) {
            indexpk = find_index->second;
        }
        else {
            emit Exit_Failure("Unexpected error when trying Create Vote");
            return;
        }
        find->second->get_tag(dv.tag, indexpk);
        Block::calculate_datavote_signature(&dv, dv.signature, _startvoting_key);
        if (!find->second->check_proof(indexpk, &dv)) {
            msg.push_back("Failed to create Vote, User Already Vote");
            emit send_Info_to_StartVoting(5, msg);
            return;
        }

        auto findunconf = s_votetree.find(s_issue);
        if (findunconf != s_votetree.end()) {
            if (!findunconf->second->check_proof(find->second, indexpk, &dv)) {
                msg.push_back("Failed to create Vote, User Aleady Vote");
                emit send_Info_to_StartVoting(5, msg);
                return;
            }
        }
        uint8_t uhdv[32]{};
        Block::get_hash_datavote(&dv, uhdv);
        std::string hdv = Utility::uint8_t_to_hexstring(uhdv, 32);
        msg.push_back(hdv.c_str());
        emit send_Info_to_StartVoting(method, msg);
        Block::Data* blkdat = new Block::Data;
        Block::datavote_to_blockdata(&dv, *blkdat);
        _queue_new_blockdata.push_back(blkdat);
        _pause = false;
    }

}

// a brief case to connection
void MemPool::DoHandshake(long long descriptor) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _new_connection.push_back(descriptor);
    condition.wakeOne();
}

void MemPool::receiveHandshake(long long descriptor, QByteArray dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _handshake_key.push_back(descriptor);
    _handshake_value.push_back(new QByteArray(dat));
    condition.wakeOne();
}

void MemPool::breakUp(long long descriptor) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_breakup.push_back(descriptor);
    condition.wakeOne();
}

void MemPool::requestBlockFork(long long descriptor, QByteArray dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_req_block_fork_key.push_back(descriptor);
    _queue_req_block_fork_value.push_back(new QByteArray(dat));
    condition.wakeOne();
}

void MemPool::receiveBlockFork(long long descriptor, QByteArray block) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_new_block_fork_key.push_back(descriptor);
    _queue_new_block_fork_value.push_back(new QByteArray(block));
    condition.wakeOne();
}

void MemPool::receiveSyncStatus(long long descriptor, QByteArray dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_sync_key.push_back(descriptor);
    _queue_sync_value.push_back(new QByteArray(dat));
    condition.wakeOne();
}

void MemPool::receiveBlockHeader(long long descriptor, QByteArray dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_new_blockheader_key.push_back(descriptor);
    _queue_new_blockheader_value.push_back(new QByteArray(dat));
    condition.wakeOne();
}

void MemPool::requestBlockByHash(long long descriptor, QByteArray dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _req_block_by_hash_key.push_back(descriptor);
    _req_block_by_hash_value.push_back(new QByteArray(dat));
    condition.wakeOne();
}

void MemPool::HandleBreakUp() {
    mutex.lock();
    long long key = _queue_breakup.front();
    _queue_breakup.pop_front();
    mutex.unlock();

    if (auto findthis = _conn.find(key); findthis != _conn.end()) {
        delete findthis->second;
        _conn.erase(findthis);
    }

    if (_forking != nullptr) {
        if (_forking->key == key) {
            delete  _forking;
            _forking = nullptr;
        }
    }
}

void MemPool::HandleNewConn() {
    mutex.lock();
    if (_new_connection.isEmpty()) {
        mutex.unlock();
        return;
    }
    long long key = _new_connection.front();
    _new_connection.pop_front();
    mutex.unlock();
    if (auto find = _conn.find(key); find != _conn.end()) {
        QEventLoop waitloop;
        QTimer::singleShot(100, &waitloop, SLOT(quit()));
        waitloop.exec();
        waitloop.deleteLater();
    }
    else {
        Protocol::Data* something = new Protocol::Data;
        _conn.emplace(key, something);
    }
    QByteArray sendthis;
    uint8_t type[4]{};
    Protocol::get_protocol_type(1, type);
    sendthis.prepend(QByteArray((char*)&type[0], 4));
    for (int a = 0; a < _blockheader.size(); a++) {
        uint8_t thisheader[72]{};
        Block::blockheader_to_raw(_blockheader[a], thisheader);
        sendthis.append(QByteArray((char*)&thisheader[0], 72));
    }

    if (sendthis.size() < 76) {
        return;
    }
    emit sendMethod(key, sendthis);
}

void MemPool::ProcessHandshake() {
    QByteArray* temp = nullptr;
    mutex.lock();
    long long key = _handshake_key.front();
    temp = _handshake_value.front();
    _handshake_key.pop_front();
    _handshake_value.pop_front();
    mutex.unlock();

    if (temp == nullptr) {
        return;
    }
    if (temp->size() < 76) {
        delete temp;
        return;
    }
    Protocol::Data* newprotocol = nullptr;
    newprotocol = new Protocol::Data;

    int pos = 4;
    int bufsize = temp->size();
    while (pos < bufsize) {
        uint8_t rawh[72]{};
        memcpy(rawh, temp->mid(pos, 72), 72);
        pos += 72;
        uint8_t prevhash[32]{};
        memcpy(prevhash, rawh, 32);
        uint8_t lasthash[32]{};
        if (!newprotocol->header.isEmpty()) {
            Block::get_block_hash(newprotocol->header.last(), lasthash);
        }
        if (sodium_memcmp(prevhash, lasthash, 32) != 0) {
            break;
        }
        Block::Header* newheader = new Block::Header;
        Block::raw_to_blockheader(rawh, *newheader);
        newprotocol->header.push_back(newheader);
    }
    delete temp;
    int mysize = _blockheader.size();
    int thissize = newprotocol->header.size();
    if (auto find = _conn.find(key); find != _conn.end()) {
        delete find->second;
        find->second = newprotocol;
    }
    else {
        _conn.emplace(key, newprotocol);
        _new_connection.push_back(key);
        HandleNewConn();
        QEventLoop waitloop;
        QTimer::singleShot(100, &waitloop, SLOT(quit()));
        waitloop.exec();
        waitloop.deleteLater();
    }

    /*const auto p1 = std::chrono::system_clock::now();
    qDebug() << "sync start at : " << std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count();*/

    QByteArray sendthis;
    int index = 0;
    uint8_t type[4]{};

    if (_forking != nullptr) {
        int forkingsize = _forking->header.size();
        if (forkingsize < thissize) {
            delete _forking;
        }
        else {
            return;
        }
    }

    if (mysize < thissize) {
        if (genesis_sync && !_blockheader.isEmpty()) {
            uint8_t mygenesis[32]{};
            Block::get_block_hash(_blockheader.front(), mygenesis);
            uint8_t thisgenesis[32]{};
            Block::get_block_hash(newprotocol->header.front(), thisgenesis);
            if (sodium_memcmp(mygenesis, thisgenesis, 32) != 0) {
                _queue_breakup.push_front(key);
                HandleBreakUp();
                emit removePeer(key);
                return;
            }
        }
        if (Protocol::compare_header(_blockheader, newprotocol->header, &sendthis, index)) {
            _forking = new Protocol::Sync;
            _forking->key = key;
            for (int a = index; a < newprotocol->header.size(); a++) {
                Block::Header* nh = new Block::Header;
                memcpy(nh, newprotocol->header[a], sizeof(Block::Header));
                _forking->header.push_back(nh);
            }
            std::string forkpath = "net/";
            forkpath += std::to_string(key);
            if (std::filesystem::exists(forkpath)) {
                std::filesystem::remove_all(forkpath);
            }
            std::filesystem::create_directories(forkpath);
            forkpath += "/";
            for (int a = 0; a < ordered_blockpath.size(); a++) {
                if (_forking->ordered_blockpath.size() == index) {
                    break;
                }
                int readsize = std::filesystem::file_size(ordered_blockpath[a]);
                std::fstream read(ordered_blockpath[a], std::ios::in | std::ios::binary);
                uint8_t* temp_raw = new uint8_t[readsize]{};
                read.read((char*)&temp_raw[0], readsize);
                read.close();
                Block::Block* copy_block = new Block::Block;
                Block::raw_wm_to_block(temp_raw, *copy_block, readsize);
                Block::Header* compareheader = nullptr;
                if (a != 0) {
                    int b = a - 1;
                    compareheader = newprotocol->header[b];
                }
                if (ValidateBlockNet(copy_block)) {
                    std::string patheach = forkpath;
                    Block::block_to_file(copy_block, patheach, a);
                    uint8_t ukeypath[32]{};
                    Block::get_block_hash(copy_block->header, ukeypath);
                    std::string skeypath = Utility::uint8_t_to_hexstring(ukeypath, 32);
                    _forking->blockpath.emplace(skeypath, patheach);
                    _forking->ordered_blockpath.push_back(patheach);
                }
                delete copy_block;
                delete[] temp_raw;
            }
            Protocol::get_protocol_type(2, type);
            sendthis.prepend(QByteArray((char*)&type[0], 4), 4);
            emit sendMethod(key, sendthis);
        }
    }
}

void MemPool::HandleReqBlockFork() {
    QByteArray* value = nullptr;
    mutex.lock();
    long long key = _queue_req_block_fork_key.front();
    value = _queue_req_block_fork_value.front();
    _queue_req_block_fork_key.pop_front();
    _queue_req_block_fork_value.pop_front();
    mutex.unlock();

    if (value == nullptr) {
        return;
    }
    if (value->size() != 36) {
        delete value;
        return;
    }
    QEventLoop waitloop;
    QTimer::singleShot(100, &waitloop, SLOT(quit()));
    waitloop.exec();
    waitloop.deleteLater();
    QByteArray temp = value->mid(4, value->size() - 4);
    QString qhash = temp.toHex();
    std::string shash = qhash.toStdString();
    delete value;
    if (auto find = _block_path.find(shash); find != _block_path.end()) {
        std::string tpath = find->second;
        if (!std::filesystem::exists(tpath)) {
            return;
        }
        int thisblocksize = std::filesystem::file_size(tpath);
        thisblocksize = thisblocksize - 4;
        uint8_t* thisraw = new unsigned char[thisblocksize] {};
        std::fstream read(tpath, std::ios::in | std::ios::binary);
        read.seekg(4, std::ios::beg);
        read.read((char*)&thisraw[0], thisblocksize);
        read.close();
        QByteArray sendthis = QByteArray((char*)&thisraw[0], thisblocksize);
        uint8_t type[4]{};
        Protocol::get_protocol_type(3, type);
        sendthis.prepend(QByteArray((char*)&type[0], 4));
        emit sendMethod(key, sendthis);
        delete[] thisraw;
    }
    else {
    }

}

void MemPool::HandleNewBlockFork() {
    QByteArray* value = nullptr;
    mutex.lock();
    long long key = _queue_new_block_fork_key.front();
    std::swap(value, _queue_new_block_fork_value.front());
    _queue_new_block_fork_value.pop_front();
    _queue_new_block_fork_key.pop_front();
    mutex.unlock();
    if (value == nullptr) {
        return;
    }
    if (value->size() < 76) {
        delete value;
        return;
    }

    if (_forking == nullptr) {
        delete value;
        return;
    }

    if (_forking->key != key) {
        delete value;
        return;
    }

    Block::Block* tempblock = new Block::Block;
    QtUtility::raw_wm_to_block(value, *tempblock, value->size());
    delete value;
    int index = 0;
    index = _forking->blockpath.size();
    if (ValidateBlockNet(tempblock)) {
        std::string patheach = "net/";
        patheach += std::to_string(key);
        if (!std::filesystem::exists(patheach)) {
            std::filesystem::create_directories(patheach);
        }
        Block::block_to_file(tempblock, patheach, index);
        uint8_t ukeypath[32]{};
        Block::get_block_hash(tempblock->header, ukeypath);
        std::string skeypath = Utility::uint8_t_to_hexstring(ukeypath, 32);
        _forking->blockpath.emplace(skeypath, patheach);
        _forking->ordered_blockpath.push_back(patheach);
        if (!_forking->header.isEmpty()) {
            delete _forking->header.front();
            _forking->header.pop_front();
        }
        QByteArray sendthis;
        if (!_forking->header.isEmpty()) {
            uint8_t type[4]{};
            Protocol::get_protocol_type(2, type);
            sendthis.prepend(QByteArray((char*)&type[0], 4), 4);
            uint8_t reqthishash[32]{};
            Block::get_block_hash(_forking->header.front(), reqthishash);
            sendthis.append(QByteArray((char*)&reqthishash[0], 32), 32);
            emit sendMethod(key, sendthis);
        }
        else {
            std::string path = "net/";
            path += std::to_string(key);
            if (std::filesystem::exists(path)) {
                for (int it = 0; it< _blockheader.size(); it++) {
                    Block::Header* ht = nullptr;
                    std::swap(_blockheader[it], ht);
                    delete ht;
                }
                _blockheader.clear();
                std::string mainpath = "block";
                _block_path.clear();
                ordered_blockpath.clear();
                std::filesystem::copy(path, mainpath, std::filesystem::copy_options::overwrite_existing);
                memcpy(&genesis_votepoll, &_forking->genesis_votepoll, sizeof(Block::VotePoll)); 
                emit setupVotepollGenesis(genesis_votepoll);
                for (const auto& entry : std::filesystem::directory_iterator(mainpath)) {
                    std::string something = entry.path().string();
                    std::fstream read(something, std::ios::in | std::ios::binary);
                    uint8_t magicbytes[4]{};
                    uint8_t s_hash[32]{};
                    uint8_t s_header[72]{};
                    read.seekg(0, std::ios::beg);
                    read.read((char*)&magicbytes[0], 4);
                    read.seekg(4, std::ios::beg);
                    read.read((char*)&s_header[0], 72);
                    Block::Header bh;
                    Block::raw_to_blockheader(s_header, bh);
                    Block::get_block_hash(&bh, s_hash);
                    read.close();
                    if (!Block::check_magic_bytes(magicbytes)) {
                        continue;
                    }
                    if (!Block::check_block_hash(s_hash)) {
                        continue;
                    }
                    std::string t_hash = Utility::uint8_t_to_hexstring(s_hash, 32);
                    ordered_blockpath.push_back(something);
                    _block_path.emplace(t_hash, something);
                    _blockheader.push_back(new Block::Header(bh));
                }
                uint8_t type[4]{};
                Protocol::get_protocol_type(5, type);
                std::swap(_block_explorer, _forking->_block_explorer);
                std::swap(_votetree, _forking->votetree);
                std::swap(s_votetree, _forking->unconf);
                sendthis.prepend(QByteArray((char*)&type[0], 4), 4);
                VoteTree::history empty;
                //emit reset_display();
                //emit loginData(7, "", empty);
                emit resetBlockHelper();
                resetWorker();
                UpdateQuickCount();
                QVector<QString> qblockpath;
                qblockpath.resize(ordered_blockpath.size());
                for (int i = 0; i < ordered_blockpath.size(); i++) {
                    if (i == 0) {
                        continue;
                    }
                    qblockpath[i] = ordered_blockpath[i].c_str();
                }
                emit parseBlockFromBegHelper(qblockpath);
                int height = _blockheader.size() - 1;
                emit Update_BlockHeight(height);
                _forking->votetree.clear();
                for (int a = 0; a < _blockheader.size(); a++) {
                    uint8_t headerraw[72]{};
                    Block::blockheader_to_raw(_blockheader[a], headerraw);
                    sendthis.append(QByteArray((char*)&headerraw[0], 72), 72);
                }
                delete _forking;
                _forking = nullptr;
                BroadcastData(sendthis);
            }
        }
    }
    delete tempblock;
}

void MemPool::HandleNewSync() {
    QByteArray* value = nullptr;
    mutex.lock();
    long long key = _queue_sync_key.front();
    _queue_sync_key.pop_front();
    std::swap(value, _queue_sync_value.front());
    mutex.unlock();

    if (value == nullptr) {
        return;
    }

    if (value->size() < 76) {
        return;
    }
    Protocol::Data* newprotocol = nullptr;
    if (auto find = _conn.find(key); find != _conn.end()) {
        delete find->second;
        std::swap(find->second, newprotocol);
    }
    else {
        _conn.emplace(key, newprotocol);
    }
    newprotocol = new Protocol::Data;
    int pos = 4;
    int bufsize = value->size();

    while (pos < bufsize) {
        QByteArray thisdata = value->mid(pos, 72);
        pos += 72;
        uint8_t prevhash[32]{};
        memcpy(prevhash, thisdata.data(), 32);
        uint8_t lasthash[32]{};
        if (!newprotocol->header.isEmpty()) {
            Block::get_block_hash(newprotocol->header.last(), lasthash);
        }

        if (sodium_memcmp(prevhash, lasthash, 32) != 0) {
            break;
        }
        Block::Header* newheader = new Block::Header;
        memcpy(newheader->prevhash, thisdata.data(), 32);
        memcpy(newheader->merkleroot, thisdata.data() + 32, 32);
        memcpy(newheader->timestamp, thisdata.data() + 32 + 32, 4);
        memcpy(newheader->nonce, thisdata.data() + 32 + 32 + 4, 4);
        newprotocol->header.push_back(newheader);
    }
    int mysize = _blockheader.size();
    int thissize = newprotocol->header.size();

    if (mysize < thissize) {
        QByteArray sendthis;
        int index = 0;
        if (Protocol::compare_header(_blockheader, newprotocol->header, &sendthis, index)) {
            index--;
            if (index != -1) {
            }
            uint8_t type[4]{};
            Protocol::get_protocol_type(6, type);
            sendthis.prepend(QByteArray((char*)&type[0], 4), 4);
            emit sendMethod(key, sendthis);
        }
    }
    delete value;
}

void MemPool::HandleNewBlockHeader() {
    QByteArray* value = nullptr;
    mutex.lock();
    long long key = _queue_new_blockheader_key.front();
    std::swap(value, _queue_new_blockheader_value.front());
    _queue_new_blockheader_key.pop_front();
    _queue_new_blockheader_value.pop_front();
    mutex.unlock();
    if (value == nullptr) {
        return;
    }
    if (value->size() != 76) {
        return;
    }
    Protocol::Data* netproto = nullptr;
    if (auto find_key = _conn.find(key); find_key != _conn.end()) {
        netproto = find_key->second;
    }
    else {
        _conn.emplace(key, netproto);
    }

    if (netproto == nullptr) {
        netproto = new Protocol::Data;
    }
    Block::Header* thisnewheader = new Block::Header;
    memcpy(thisnewheader->prevhash, value->data() + 4, 32);
    memcpy(thisnewheader->merkleroot, value->data() + 4 + 32, 32);
    memcpy(thisnewheader->timestamp, value->data() + 4 + 32 + 32, 4);
    memcpy(thisnewheader->nonce, value->data() + 4 + 32 + 32 + 4, 4);
    uint8_t lasthash[32]{};
    uint8_t prevhash[32]{};
    memcpy(lasthash, thisnewheader->prevhash, 32);
    if (!netproto->header.isEmpty()) {
        Block::get_block_hash(netproto->header.last(), prevhash);
    }

    if (sodium_memcmp(prevhash, lasthash, 32) != 0) {
        delete value;
        return;
    }

    netproto->header.push_back(thisnewheader);

    int mysize = _blockheader.size();
    int thissize = netproto->header.size();
    if (mysize < thissize) {
        if (_forking == nullptr) {
            QByteArray sendthis;
            int index = 0;
            if (Protocol::compare_header(_blockheader, netproto->header, &sendthis, index)) {
                uint8_t type[4]{};
                Protocol::get_protocol_type(6, type);
                sendthis.prepend(QByteArray((char*)&type[0], 4), 4);
                emit sendMethod(key, sendthis);
            }
        }
    }
}

void MemPool::HandleReqBlockByHash() {
    QByteArray* value = nullptr;
    mutex.lock();
    long long key = _req_block_by_hash_key.front();
    _req_block_by_hash_key.pop_front();
    std::swap(value, _req_block_by_hash_value.front());
    _req_block_by_hash_value.pop_front();
    mutex.unlock();

    if (value == nullptr) {
        return;
    }

    if (value->size() != 36) {
        delete value;
        return;
    }

    QString qhash = value->mid(4, 32).toHex();
    std::string shash = qhash.toStdString();
    QEventLoop waitloop;
    QTimer::singleShot(100, &waitloop, SLOT(quit()));
    waitloop.exec();
    waitloop.deleteLater();
    if (auto rfind = _block_path.find(shash); rfind != _block_path.end()) {
        int size = std::filesystem::file_size(rfind->second);
        std::fstream read(rfind->second, std::ios::in | std::ios::binary);
        unsigned char* uraw = new unsigned char[size] {};
        read.seekg(0, std::ios::beg);
        read.read((char*)&uraw[0], size);
        read.close();
        QByteArray sendthisblock = QByteArray((char*)&uraw[0], size);
        if (size < 76) {
            delete[] uraw;
            delete value;
            return;
        }
        if (sendthisblock.size() < 76) {
            delete[] uraw;
            delete value;
            return;
        }
        emit sendMethod(key, sendthisblock);
        delete[] uraw;
    }

    delete value;
}

bool MemPool::ValidateBlockNet(Block::Block*& blocktemp) {
    // declaring thiss block 
    uint8_t thash[32]{};
    Block::get_block_hash(blocktemp->header, thash);
    long long ttimestamp = Utility::uint8_t_to_int(blocktemp->header->timestamp, 4);
    // declaring for validating
    uint8_t uhash[32]{};
    long long utimestamp = 0;
    if (!Block::check_block_hash(thash)) {
        return false;
    }

    if (!_forking->ordered_blockpath.isEmpty()) {
        std::fstream readheader(_forking->ordered_blockpath.last(), std::ios::in | std::ios::binary);
        uint8_t rwh[72]{};
        readheader.seekg(4, std::ios::beg);
        readheader.read((char*)&rwh[0], 72);
        readheader.close();
        Block::Header rh;
        Block::raw_to_blockheader(rwh, rh);
        Block::get_block_hash(&rh, uhash);
        utimestamp = Utility::uint8_t_to_int(rh.timestamp, 4);
    }
    if (utimestamp > ttimestamp) {
        return false;
    }
    if (sodium_memcmp(uhash, blocktemp->header->prevhash, 32) != 0) {
        return false;
    }

    // deletion
    QVector<std::string> deletethis;
    QtUtility::get_blockdata_hash_qvector(blocktemp, deletethis);

    // calculate merkleroot;
    // our implementation can't fit more than 65535 transaction;
    if (blocktemp->blockdata.size() > 65536) {
        return false;
    }
    emit send_Info_to_LoadingBar(7, blocktemp->blockdata.size());
    Merkle::Tree* uroot = new Merkle::Tree;
    for (int a = 0; a < blocktemp->blockdata.size(); a++) {
        emit send_Info_to_LoadingBar(2, a);
        if (blocktemp->blockdata[a] == nullptr) {
            break;
        }
        // validate blockdata
        bool valid = VoteTree::validate_blockdata_to_votetree(blocktemp->blockdata[a], ttimestamp, _forking->votetree, _forking->unconf, _forking->genesis_votepoll);
        if (valid) {
            Merkle::Hash ublockhashtree;
            Block::get_hash_blockdata(blocktemp->blockdata[a], ublockhashtree.data);
            Merkle::insert_tree(*uroot, &ublockhashtree);
        }
        else {
            delete uroot;
            emit send_Info_to_LoadingBar(6, 0);
            return false;
        }
    }
    emit send_Info_to_LoadingBar(2, blocktemp->blockdata.size());
    // verify merklerootl
    if (!Merkle::verify_tree(*uroot, blocktemp->header->merkleroot)) {
        delete uroot;
        return false;
    }
    delete uroot;
    std::string blockhash = Utility::uint8_t_to_hexstring(thash, 32);
    Block::Explorer* newxplorer = new Block::Explorer;
    for (int a = 0; a < blocktemp->blockdata.size(); a++) {
        if (blocktemp->blockdata[a] == nullptr) {
            break;
        }
        newxplorer->set_hash.emplace(deletethis[a]);
        VoteTree::insert_blockdata_to_votetree(blocktemp->blockdata[a], ttimestamp, _forking->votetree, _forking->unconf, genesis_votepoll);
    }
    if (auto find_explorer = _forking->_block_explorer.find(blockhash); find_explorer != _forking->_block_explorer.end()) {
        delete find_explorer->second;
        std::swap(find_explorer->second, newxplorer);
    }
    else {
        _forking->_block_explorer.emplace(blockhash, newxplorer);
    }
    emit send_Info_to_LoadingBar(3, 0);
    return true;
}

void MemPool::BroadcastData(QByteArray dat) {
    for (auto& [key, value] : _conn) {
        emit sendMethod(key, dat);
    }
}

void MemPool::updateDataFromBeg() {
    if (ordered_blockpath.size() < 1) {
        return;
    }
    for (auto path = ordered_blockpath.begin(); path != ordered_blockpath.end(); path++) {
        if (path == ordered_blockpath.begin()) {
            continue;
        }
        long long size = std::filesystem::file_size(path->c_str());
        std::fstream read(path->c_str(), std::ios::in | std::ios::binary);
        uint8_t* block = new uint8_t[size]{};
        read.seekg(0, std::ios::beg);
        read.read((char*)&block[0], size);
        read.close();
        Block::Block* blk = new Block::Block;
        Block::raw_wm_to_block(block, *blk, size);
        long long blockdate = Utility::uint8_t_to_int(blk->header->timestamp, 4);
        delete[] block;
        for (int j = 0; j < blk->blockdata.size(); j++) {
            if (Block::check_blockdata_type(blk->blockdata[j]->type, 1)) {
                Block::VotePoll vp;
                Block::blockdata_to_votepoll(blk->blockdata[j], vp);
                uint8_t hash[32]{};
                Block::get_hash_votepoll(&vp, hash);
                std::string hvp = Utility::uint8_t_to_hexstring(hash, 32);
                if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                    VoteTree::display sendthis(findhvp->second);
                    emit send_display(1, hvp.c_str(), sendthis);
                }
            }
            else if (Block::check_blockdata_type(blk->blockdata[j]->type, 2)) {

            }if (Block::check_blockdata_type(blk->blockdata[j]->type, 3)) {
                Block::DataCandidate dc;
                Block::blockdata_to_datacandidate(blk->blockdata[j], dc);
                std::string pksender = Utility::uint8_t_to_hexstring(dc.publickey, 32);
                std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
                if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                    auto findpkm = findhvp->second->set_pk_candidate.find(pksender);
                    auto findpkr = findhvp->second->register_pk.find(pksender);
                    if (findpkm != findhvp->second->set_pk_candidate.end() && findpkr != findhvp->second->register_pk.end()) {
                        VoteTree::history sendthis;
                        sendthis.updatedatemsgaccepted.emplace(pksender, blockdate);
                        emit loginData(5, hvp, sendthis);
                    }
                }
                uint8_t dchash[32]{};
                Block::get_hash_datacandidate(&dc, dchash);
                VoteTree::DataCandidateString dcs;
                VoteTree::display senddisplay;
                dcs.blocktime = blockdate;
                dcs.hash = Utility::uint8_t_to_hexstring(dchash, 32).c_str();
                dcs.name = Utility::uint8_t_to_ascii(dc.candidatename, 32).c_str();
                dcs.pk = Utility::uint8_t_to_hexstring(dc.publickey, 32).c_str();
                senddisplay.dc.push_back(dcs);
                emit send_display(2, hvp.c_str(), senddisplay);
            }
            if (Block::check_blockdata_type(blk->blockdata[j]->type, 4)) {
                Block::DataPartisipan dp;
                Block::blockdata_to_datapartisipan(blk->blockdata[j], dp);
                std::string pksender = Utility::uint8_t_to_hexstring(dp.publickey, 32);
                std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
                if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                    auto findpkm = findhvp->second->set_pk_partisipan.find(pksender);
                    auto findpkr = findhvp->second->register_pk.find(pksender);
                    if (findpkm != findhvp->second->set_pk_partisipan.end() && findpkr != findhvp->second->register_pk.end()) {
                        VoteTree::history sendthis;
                        sendthis.updatedatemsgaccepted.emplace(pksender, blockdate);
                        emit loginData(5, hvp, sendthis);
                    }
                }
            }
            if (Block::check_blockdata_type(blk->blockdata[j]->type, 5)) {
                Block::Message msg;
                Block::blockdata_to_msg(blk->blockdata[j], msg);
                std::string hvp = Utility::uint8_t_to_hexstring(msg.hashvotepoll, 32);
                std::string pksender = Utility::uint8_t_to_hexstring(msg.publickey, 32);
                QByteArray st;
                st.prepend(QByteArray((char*)&msg.cipher->ciphertext[0], msg.cipher->cipherlen));
                st.append(QByteArray((char*)&msg.cipher->tag[0], 16));
                st.append(QByteArray((char*)&msg.cipher->nonce[0], 12));
                long long size = msg.cipher->cipherlen + 16 + 12;
                st.resize(size);
                VoteTree::history sendthis;
                sendthis.ciphermsg.emplace(pksender, st);
                sendthis.datemsg.emplace(pksender, blockdate);
                emit loginData(2, hvp, sendthis);
            }
        }
        delete blk;
    }
}

void MemPool::updateDataFromLast() {
    if (ordered_blockpath.size() < 1) {
        return;
    }
    long long size = std::filesystem::file_size(ordered_blockpath.last());
    std::fstream read(ordered_blockpath.last(), std::ios::in | std::ios::binary);
    uint8_t* block = new uint8_t[size]{};
    read.seekg(0, std::ios::beg);
    read.read((char*)&block[0], size);
    read.close();
    Block::Block* blk = new Block::Block;
    Block::raw_wm_to_block(block, *blk, size);
    long long blockdate = Utility::uint8_t_to_int(blk->header->timestamp, 4);
    delete[]block;
    for (int j = 0; j < blk->blockdata.size(); j++) {
        if (Block::check_blockdata_type(blk->blockdata[j]->type, 1)) {
            Block::VotePoll vp;
            Block::blockdata_to_votepoll(blk->blockdata[j], vp);
            uint8_t hash[32]{};
            Block::get_hash_votepoll(&vp, hash);
            std::string hvp = Utility::uint8_t_to_hexstring(hash, 32);
            if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                VoteTree::display sendthis(findhvp->second);
                emit send_display(1, hvp.c_str(), sendthis);
            }
        }
        else if (Block::check_blockdata_type(blk->blockdata[j]->type, 2)) {
            Block::DataVote dv;
            Block::blockdata_to_datavote(blk->blockdata[j], dv);
            std::string hashhextag = VoteTree::get_hash_hex_tag(&dv.tag);
            std::string a1 = Utility::uint8_t_to_hexstring(dv.signature.A_1, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dv.tag.issue, 32);
            VoteTree::VoteString vs;
            vs.block_time.emplace(a1, blockdate);
            int indexdc = -1;
            if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                indexdc = findhvp->second->get_index_candidate(dv.hashdatacandidate);
                if (indexdc != -1) {
                    VoteTree::display sendthis;
                    vs.vote_a1.emplace(a1, indexdc);
                    sendthis.vote.push_back(vs);
                    sendthis.vote_index.emplace(hashhextag, 0);
                    emit send_display(4, hvp.c_str(), sendthis);
                }
            }
        }else if (Block::check_blockdata_type(blk->blockdata[j]->type, 3)) {
            Block::DataCandidate dc;
            Block::blockdata_to_datacandidate(blk->blockdata[j], dc);
            std::string pksender = Utility::uint8_t_to_hexstring(dc.publickey, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
            if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                auto findpkm = findhvp->second->set_pk_candidate.find(pksender);
                auto findpkr = findhvp->second->register_pk.find(pksender);
                if (findpkm != findhvp->second->set_pk_candidate.end() && findpkr != findhvp->second->register_pk.end()) {
                    VoteTree::history sendthis;
                    sendthis.updatedatemsgaccepted.emplace(pksender, blockdate);
                    emit loginData(5, hvp, sendthis);
                }
            }
            uint8_t dchash[32]{};
            Block::get_hash_datacandidate(&dc, dchash);
            VoteTree::DataCandidateString dcs;
            VoteTree::display senddisplay;
            dcs.blocktime = blockdate;
            dcs.hash = Utility::uint8_t_to_hexstring(dchash, 32).c_str();
            dcs.name = Utility::uint8_t_to_ascii(dc.candidatename, 32).c_str();
            dcs.pk = Utility::uint8_t_to_hexstring(dc.publickey, 32).c_str();
            senddisplay.dc.push_back(dcs);
            emit send_display(2, hvp.c_str(), senddisplay);
        }
        if (Block::check_blockdata_type(blk->blockdata[j]->type, 4)) {
            Block::DataPartisipan dp;
            Block::blockdata_to_datapartisipan(blk->blockdata[j], dp);
            std::string pksender = Utility::uint8_t_to_hexstring(dp.publickey, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
            if (auto findhvp = _votetree.find(hvp); findhvp != _votetree.end()) {
                auto findpkm = findhvp->second->set_pk_partisipan.find(pksender);
                auto findpkr = findhvp->second->register_pk.find(pksender);
                if (findpkm != findhvp->second->set_pk_partisipan.end() && findpkr != findhvp->second->register_pk.end()) {
                    VoteTree::history sendthis;
                    sendthis.updatedatemsgaccepted.emplace(pksender, blockdate);
                    emit loginData(5, hvp, sendthis);
                }
            }
            uint8_t dphash[32]{};
            Block::get_hash_datapartisipan(&dp, dphash);
            VoteTree::DataPartisipanString dps;
            VoteTree::display senddisplay;
            dps.blocktime = blockdate;
            dps.hash = Utility::uint8_t_to_hexstring(dphash, 32).c_str();
            dps.pk = Utility::uint8_t_to_hexstring(dp.publickey, 32).c_str();
            senddisplay.dp.push_back(dps);
            emit send_display(3, hvp.c_str(), senddisplay);
        }
        if (Block::check_blockdata_type(blk->blockdata[j]->type, 5)) {
            Block::Message msg;
            Block::blockdata_to_msg(blk->blockdata[j], msg);
            std::string hvp = Utility::uint8_t_to_hexstring(msg.hashvotepoll, 32);
            std::string pksender = Utility::uint8_t_to_hexstring(msg.publickey, 32);
            QByteArray st;
            st.prepend(QByteArray((char*)&msg.cipher->ciphertext[0], msg.cipher->cipherlen));
            st.append(QByteArray((char*)&msg.cipher->tag[0], 16));
            st.append(QByteArray((char*)&msg.cipher->nonce[0], 12));
            long long size = msg.cipher->cipherlen + 16 + 12;
            st.resize(size);
            VoteTree::history sendthis;
            sendthis.ciphermsg.emplace(pksender, st);
            sendthis.datemsg.emplace(pksender, blockdate);
            emit loginData(2, hvp, sendthis);
        }
    }
    delete blk;
}

void MemPool::changeFlagSync(bool con) {
    QMutexLocker locker(&mutex);
    genesis_sync = con;
}