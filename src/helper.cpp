#include "helper.h"
#include <QEventLoop>
#include <QTimer>

Helper::Helper(QObject* parent) :
    QObject(parent)
{
    _abort = false;
    _pause = true;
    _reset_block = false;
    _genesis = nullptr;
    keypair = nullptr;
}

Helper::~Helper() {
    for (auto it = set_display.begin(); it != set_display.end(); it++) {
        VoteTree::display* temp = nullptr;
        std::swap(it->second, temp);
        delete temp;
    }
    set_display.clear();
}


void Helper::abort() {
    QMutexLocker locker(&mutex);
    _abort = true;
    condition.wakeOne();
}

void Helper::Method_parseBlockFromBeg(QVector<QString> path) {
    QMutexLocker locker(&mutex);
    _pause = false;
    std::swap(_queue_parse_block, path);
    condition.wakeOne();
}

void Helper::Method_parseBlock(QString path) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_parse_block.push_back(path);
    condition.wakeOne();
}

void Helper::Method_resetBlock() {
    QMutexLocker locker(&mutex);
    _pause = false;
    _reset_block = true;
    condition.wakeOne();
}

void Helper::setupGenesisVotepoll(Block::VotePoll* vp) {
    QMutexLocker locker(&mutex);
    if (_genesis != nullptr) {
        delete _genesis;
    }
    _genesis = new Block::VotePoll(*vp);
}

void Helper::Method_User(int n, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_user_method.push_back(n);
    _queue_user_data.push_back(new QByteArray(*dat));
    condition.wakeOne();
}

void Helper::main() {
    forever{

        mutex.lock();
        if (!_abort && _pause) {
            condition.wait(&mutex);
        }

        _pause = true;

        if (_abort) {
            mutex.unlock();
            emit finished();
            return;
        }

        if (_reset_block) {
            resetBlock();
        }

        int methoduser = _queue_user_method.size();
        int parseblock = _queue_parse_block.size();
        int methodadd = _queue_admin_add_method.size();
        int methodmail = _queue_admin_mail_method.size();
        int methodlist = _queue_admin_list_method.size();
        int methodresult = _queue_admin_result_method.size();
        int methodvote = _queue_user_vote_method.size();
        int tryconn = _queue_port.size();
        mutex.unlock();

        if (parseblock) {
            parseBlock();
        }

        if (methoduser) {
            handleUser();
        }

        if (methodadd) {
            handleAdminAdd();
        }
        if (methodmail) {
            handleAdminMail();
        }

        if (methodlist) {
            handleAdminList();
        }

        if (methodresult) {
            handleAdminResult();
        }

        if (methodvote) {
            handleUserVote();
        }

        if (tryconn) {
            handleTryConn();
        }
    }
}

void Helper::resetBlock() {
    for (auto it = set_display.begin(); it != set_display.end(); it++) {
        VoteTree::display* temp = nullptr;
        std::swap(it->second, temp);
        delete temp;
    }
    set_display.clear();
}

void Helper::parseBlock() {
    std::string path = "";
    mutex.lock();
    path = _queue_parse_block.front().toStdString();
    _queue_parse_block.pop_front();
    mutex.unlock();
    if (!std::filesystem::exists(path)) {
        return;
    }
    long long size = std::filesystem::file_size(path);
    std::fstream read(path, std::ios::in | std::ios::binary);
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
            if (set_display.find(hvp) != set_display.end()) {
                continue;
            }
            VoteTree::display* storethis = new VoteTree::display(&vp);
            storethis->vp.blocktime = blockdate;
            set_display.emplace(hvp, storethis);
            hash_set_display.push_back(hvp.c_str());
            QVector<QString> renderthis;
            if (keypair != nullptr) {
                renderthis.push_back(storethis->vp.hash);
                renderthis.push_back(storethis->vp.desc);
                renderthis.push_back(QString::number(storethis->vp.startdate));
                renderthis.push_back(QString::number(storethis->vp.enddate));
                renderthis.push_back(QString::number(storethis->vp.candidatsize));
                renderthis.push_back(QString::number(storethis->vp.partisipansize));
                renderthis.push_back(QString::number(storethis->dc.size()));
                renderthis.push_back(QString::number(storethis->dp.size()));
                renderthis.push_back(QString::number(storethis->vp.blocktime));
                renderthis.push_back(QString::number(storethis->vp.ringsize));
                renderthis.push_back(storethis->vp.pk);
                renderthis.push_back(storethis->vp.sig);
                emit loginMethod(2, renderthis);
                renderthis.clear();
            }
        }
        else if (Block::check_blockdata_type(blk->blockdata[j]->type, 2)) {
            Block::DataVote dv;
            Block::blockdata_to_datavote(blk->blockdata[j], dv);
            std::string hashhextag = VoteTree::get_hash_hex_tag(&dv.tag);
            std::string a1 = Utility::uint8_t_to_hexstring(dv.signature.A_1, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dv.tag.issue, 32);
            std::string hashc = Utility::uint8_t_to_hexstring(dv.hashdatacandidate, 32);
            int indexdc = -1;
            if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
                int indexc = -1;
                for (int a = 0; a < findhvp->second->dc.size(); a++) {
                    if (findhvp->second->dc[a].hash.toStdString() == hashc) {
                        indexc = a;
                        break;
                    }
                }

                if (indexc == -1) {
                    continue;
                }

                auto findvs = findhvp->second->vote_index.find(hashhextag);
                if (findvs != findhvp->second->vote_index.end()) {
                    if (findhvp->second->vote[findvs->second].vote_a1.find(a1) == findhvp->second->vote[findvs->second].vote_a1.end()) {
                        findhvp->second->vote[findvs->second].vote_a1.emplace(a1, indexc);
                        findhvp->second->vote[findvs->second].block_time.emplace(a1, blockdate);
                        findhvp->second->dc[indexc].totalvote++;
                    }
                }
                else {
                    VoteTree::VoteString vstemp;
                    vstemp.vote_a1.emplace(a1, indexc);
                    vstemp.block_time.emplace(a1, blockdate);
                    findhvp->second->vote.push_back(vstemp);
                    int indexvs = findhvp->second->vote.size() - 1;
                    findhvp->second->vote_index.emplace(hashhextag, indexvs);
                    findhvp->second->dc[indexc].totalvote++;
                }
                if (keypair != nullptr) {
                    std::string mypk = Utility::uint8_t_to_hexstring(keypair->publickey, 32);
                    TRS::Tag temptag;
                    if (findhvp->second->get_tag(mypk, temptag)) {
                        Block::DataVote dv;
                        Utility::hexstring_to_uint8_t(dv.hashdatacandidate, 32, findhvp->second->dc[indexdc].hash.toStdString());
                        Utility::int_to_uint8_t(temptag.publickey.size(), dv.ringsize, 2);
                        dv.tag = temptag;
                        Block::calculate_datavote_signature(&dv, dv.signature, keypair);
                        std::string mya1 = Utility::uint8_t_to_hexstring(dv.signature.A_1, 32);
                        if (mya1 == a1) {
                            emit setFlagMethod(5, hvp, true);
                        }
                        TRS::Free_Signature(dv.signature);
                    }
                }
            }
        }
        else if (Block::check_blockdata_type(blk->blockdata[j]->type, 3)) {
            Block::DataCandidate dc;
            Block::blockdata_to_datacandidate(blk->blockdata[j], dc);
            std::string pksender = Utility::uint8_t_to_hexstring(dc.publickey, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
            uint8_t dchash[32]{};
            Block::get_hash_datacandidate(&dc, dchash);
            VoteTree::DataCandidateString dcs;
            dcs.blocktime = blockdate;
            dcs.hash = Utility::uint8_t_to_hexstring(dchash, 32).c_str();
            dcs.name = Utility::uint8_t_to_ascii(dc.candidatename, 32).c_str();
            dcs.pk = pksender.c_str();
            if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
                if (findhvp->second->dc_index.find(pksender) == findhvp->second->dc_index.end()) {
                    findhvp->second->dc.push_back(dcs);
                    int indexdc = findhvp->second->dc.size() - 1;
                    findhvp->second->dc_index.emplace(pksender, indexdc);
                    emit AddMoreSize(hvp, 1);
                }
                else {
                    int indexdc = findhvp->second->dc_index.find(pksender)->second;
                    findhvp->second->dc[indexdc] = dcs;
                }
                if (keypair != nullptr) {
                    if (sodium_memcmp(keypair->publickey, dc.publickey, 32) == 0) {
                        emit setFlagMethod(2, hvp, true);
                    }
                }
            }
        }
        if (Block::check_blockdata_type(blk->blockdata[j]->type, 4)) {
            Block::DataPartisipan dp;
            Block::blockdata_to_datapartisipan(blk->blockdata[j], dp);
            std::string pksender = Utility::uint8_t_to_hexstring(dp.publickey, 32);
            std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
            uint8_t dphash[32]{};
            Block::get_hash_datapartisipan(&dp, dphash);
            VoteTree::DataPartisipanString dps;
            dps.blocktime = blockdate;
            dps.hash = Utility::uint8_t_to_hexstring(dphash, 32).c_str();
            dps.pk = pksender.c_str();
            if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
                if (findhvp->second->dp_index.find(pksender) == findhvp->second->dp_index.end()) {
                    findhvp->second->dp.push_back(dps);
                    int indexdp = findhvp->second->dp.size() - 1;
                    findhvp->second->dp_index.emplace(pksender, indexdp);
                    emit AddMoreSize(hvp, 2);
                }
                else {
                    int indexdp = findhvp->second->dp_index.find(pksender)->second;
                    findhvp->second->dp[indexdp] = dps;
                }
                if (keypair != nullptr) {
                    if (sodium_memcmp(keypair->publickey, dp.publickey, 32) == 0) {
                        emit setFlagMethod(3, hvp, true);
                    }
                }
            }
        }
        if (Block::check_blockdata_type(blk->blockdata[j]->type, 5)) {
            Block::Message msg;
            Block::blockdata_to_msg(blk->blockdata[j], msg);
            std::string hvp = Utility::uint8_t_to_hexstring(msg.hashvotepoll, 32);
            std::string pksender = Utility::uint8_t_to_hexstring(msg.publickey, 32);
            VoteTree::MessageCipher newmsg;
            newmsg.cipher.prepend(QByteArray((char*)&msg.cipher->ciphertext[0], msg.cipher->cipherlen));
            newmsg.cipher.append(QByteArray((char*)&msg.cipher->tag[0], 16));
            newmsg.cipher.append(QByteArray((char*)&msg.cipher->nonce[0], 12));
            long long size = msg.cipher->cipherlen + 16 + 12;
            newmsg.cipher.resize(size);
            newmsg.blocktime = blockdate;
            newmsg.pk = pksender.c_str();
            if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
                if (findhvp->second->msg_index.find(pksender) == findhvp->second->msg_index.end()) {
                    findhvp->second->msg.push_back(newmsg);
                    int indexmsg = findhvp->second->msg.size() - 1;
                    findhvp->second->msg_index.emplace(pksender, indexmsg);
                }
                else {
                    int indexmsg = findhvp->second->msg_index.find(pksender)->second;
                    findhvp->second->msg[indexmsg] = newmsg;
                }
                if (keypair != nullptr) {
                    if (sodium_memcmp(msg.publickey, keypair->publickey, 32) == 0) {
                        emit setFlagMethod(4, hvp, true);
                    }
                }
            }
        }
    }
    delete blk;
}

void Helper::handleUser() {
    int n = -1;
    QByteArray* data = nullptr;
    mutex.lock();
    n = _queue_user_method.front();
    _queue_user_method.pop_front();
    std::swap(data, _queue_user_data.front());
    _queue_user_data.pop_front();
    mutex.unlock();

    if (n == -1 || data == nullptr) {
        emit Information("error while trying to login, please try again later");
        return;
    }


    if (n == 0) {
        if (data->size() != 32) {
            emit Information("error while trying to login, please try again later");
            return;
        }
        if (keypair != nullptr) {
            delete keypair;
            keypair = nullptr;
        }
        keypair = new Block::KeyPair;
        memcpy(keypair->privatekey, data->data(), 32);
        delete data;
        keypair->calculate_base();
        QVector<QString> renderthis;
        std::string mypk = Utility::uint8_t_to_hexstring(keypair->publickey, 32);
        if (sodium_memcmp(keypair->publickey, _genesis->publickey, 32) == 0) {
            for (int j = 0; j < hash_set_display.size(); j++) {
                auto it = set_display.find(hash_set_display[j].toStdString());
                if (it == set_display.end()) {
                    continue;
                }
                renderthis.push_back(it->second->vp.hash);
                renderthis.push_back(it->second->vp.desc);
                renderthis.push_back(QString::number(it->second->vp.startdate));
                renderthis.push_back(QString::number(it->second->vp.enddate));
                renderthis.push_back(QString::number(it->second->vp.candidatsize));
                renderthis.push_back(QString::number(it->second->vp.partisipansize));
                renderthis.push_back(QString::number(it->second->dc.size()));
                renderthis.push_back(QString::number(it->second->dp.size()));
                renderthis.push_back(QString::number(it->second->vp.blocktime));
                renderthis.push_back(QString::number(it->second->vp.ringsize));
                renderthis.push_back(it->second->vp.pk);
                renderthis.push_back(it->second->vp.sig);
                emit loginMethod(2, renderthis);
                renderthis.clear();
            }
            renderthis.clear();
            renderthis.push_back("q");
            emit loginMethod(1, renderthis);
        }
        else {
            for (int j = 0; j < hash_set_display.size(); j++) {
                auto it = set_display.find(hash_set_display[j].toStdString());
                if (it == set_display.end()) {
                    continue;
                }
                renderthis.push_back(it->second->vp.hash);
                renderthis.push_back(it->second->vp.desc);
                renderthis.push_back(QString::number(it->second->vp.startdate));
                renderthis.push_back(QString::number(it->second->vp.enddate));
                renderthis.push_back(QString::number(it->second->vp.candidatsize));
                renderthis.push_back(QString::number(it->second->vp.partisipansize));
                renderthis.push_back(QString::number(it->second->dc.size()));
                renderthis.push_back(QString::number(it->second->dp.size()));
                renderthis.push_back(QString::number(it->second->vp.blocktime));
                renderthis.push_back(QString::number(it->second->vp.ringsize));
                renderthis.push_back(it->second->vp.pk);
                renderthis.push_back(it->second->vp.sig);
                emit loginMethod(2, renderthis);
                renderthis.clear();
                if (auto find = it->second->msg_index.find(mypk); find != it->second->msg_index.end()) {
                    emit setFlagMethod(4, it->first, true);
                    int indexmsg = find->second;
                    long long csize = it->second->msg[indexmsg].cipher.size() - 28;
                    QByteArray ctemp = it->second->msg[indexmsg].cipher.mid(0, csize);
                    QByteArray dll = it->second->msg[indexmsg].cipher.mid(csize, 28);
                    AES_Data cipher;
                    cipher.cipherlen = csize;
                    cipher.ciphertext = new uint8_t[csize]{};
                    memcpy(cipher.ciphertext, ctemp.data(), csize);
                    memcpy(cipher.tag, dll.data(), 16);
                    memcpy(cipher.nonce, dll.data() + 16, 12);
                    uint8_t* msg = nullptr;
                    aes_decrypt(keypair->privatekey, _genesis->publickey, &cipher, msg);
                    int role = Utility::uint8_t_to_int(msg, 2);
                    if (role == 1) {
                        emit setHistory(it->second->vp.hash, "Requested as candidate at " + it->second->vp.desc, it->second->msg[indexmsg].blocktime, 1);
                    }
                    else if (role == 2) {
                        emit setHistory(it->second->vp.hash, "Requested as voter at " + it->second->vp.desc, it->second->msg[indexmsg].blocktime, 1);
                    }
                    delete[] msg;

                }
                if (auto find = it->second->dc_index.find(mypk); find != it->second->dc_index.end()) {
                    emit setHistory(it->second->vp.hash, "Registered as candidate at " + it->second->vp.desc, it->second->dc[find->second].blocktime, 2);
                    emit setFlagMethod(2, it->first, true);
                }
                if (auto find = it->second->dp_index.find(mypk); find != it->second->dp_index.end()) {
                    emit setHistory(it->second->vp.hash, "Registered as voter at " + it->second->vp.desc, it->second->dp[find->second].blocktime, 2);
                    emit setFlagMethod(3, it->first, true);
                }
                TRS::Tag temptag;
                if (it->second->get_tag(mypk, temptag)) {
                    std::string hashtag = VoteTree::get_hash_hex_tag(&temptag);
                    int indexc = -1;
                    for (int i = 0; i < it->second->dc.size(); i++) {
                        Block::DataVote dv;
                        Utility::hexstring_to_uint8_t(dv.hashdatacandidate, 32, it->second->dc[i].hash.toStdString());
                        Utility::int_to_uint8_t(temptag.publickey.size(), dv.ringsize, 2);
                        dv.tag = temptag;
                        Block::calculate_datavote_signature(&dv, dv.signature, keypair);
                        std::string a1 = Utility::uint8_t_to_hexstring(dv.signature.A_1, 32);
                        if (auto findindexv = it->second->vote_index.find(hashtag); findindexv != it->second->vote_index.end()) {
                            int indextag = findindexv->second;
                            auto finda1 = it->second->vote[indextag].vote_a1.find(a1);
                            if (finda1 == it->second->vote[indextag].vote_a1.end()) {
                                continue;
                            }
                            else {
                                
                                emit setHistory(it->second->vp.hash, "Already voting for " + it->second->dc[i].name + ", at " + it->second->vp.desc, it->second->vote[indextag].block_time.find(a1)->second, 3);
                                emit setFlagMethod(5, it->first, true);
                                break;
                            }
                        }
                        TRS::Free_Signature(dv.signature);
                    }
                }
            }
            renderthis.clear();
            renderthis.push_back("q");
            renderthis.push_back("q");
            emit loginMethod(1, renderthis);
        }
        return;
    }
    else if (n == 1) {
        delete data;
        if (keypair != nullptr) {
            delete keypair;
            keypair = nullptr;
        }
        emit Information("Logout Successfully.");
    }
}

void Helper::Method_Admin_Add(int con, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_admin_add_method.push_back(con);
    _queue_admin_add_data.push_back(new QByteArray(*dat));
    condition.wakeOne();
}

void Helper::handleAdminAdd() {
    QByteArray* data = nullptr;
    int method = -1;
    mutex.lock();
    std::swap(data, _queue_admin_add_data.front());
    _queue_admin_add_data.pop_front();
    method = _queue_admin_add_method.front();
    _queue_admin_add_method.pop_front();
    mutex.unlock();

    if (data == nullptr) {
        Information("Error while trying to add new member in this votepoll, Please try-again later");
        return;
    }

    if (method == -1) {
        Information("Error while trying to add new member in this votepoll, Please try-again later");
        delete data;
        return;
    }

    if (method == 1) {
        if (data->size() != BLOCK_DATACANDIDATE_BYTES) {
            Information("Error while trying to add new candidate in this votepoll, Please try-again later");
            return;
        }
        uint8_t rawdc[BLOCK_DATACANDIDATE_BYTES]{};
        memcpy(rawdc, data->data(), BLOCK_DATACANDIDATE_BYTES);
        delete data;
        Block::DataCandidate dc;
        Block::raw_to_datacandidate(rawdc, dc);
        std::string pks = Utility::uint8_t_to_hexstring(dc.publickey, 32);
        std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
        if (auto find = set_display.find(hvp); find != set_display.end()) {
            auto findpk = find->second->dc_index.find(pks);
            if (findpk != find->second->dc_index.end()) {
                Information("Can't add this candidate due to publickey already exist");
                return;
            }
            if (find->second->dc.size() >= find->second->vp.candidatsize) {
                Information("Can't add this candidate due to candidate already reached maximum");
                return;
            }
            long long currenttime = QDateTime::currentSecsSinceEpoch();
            currenttime += 150;
            if (currenttime >= find->second->vp.startdate) {
                Information("Can't add this candidate due to polling registration is already closed");
                return;
            }
            if (!find->second->dc.isEmpty()) {
                Utility::hexstring_to_uint8_t(dc.parenthash, 32, find->second->dc.last().hash.toStdString());
            }
            else {
                Utility::hexstring_to_uint8_t(dc.parenthash, 32, hvp);
            }
            Utility::hexstring_to_uint8_t(dc.hashvotepoll, 32, hvp);
            Block::calculate_datacandidate_signature(&dc, dc.signature, keypair);
            Block::Data bd;
            Block::datacandidate_to_blockdata(&dc, bd);
            size_t size = bd.datsize + 4;
            QByteArray qdat = QByteArray((char*)&bd.data[0], size);
            VoteTree::DataCandidateString dcs;
            dcs.name = Utility::uint8_t_to_ascii(dc.candidatename, 32).c_str();
            dcs.pk = Utility::uint8_t_to_hexstring(dc.publickey, 32).c_str();
            uint8_t hash[32]{};
            Block::get_hash_datacandidate(&dc, hash);
            dcs.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
            find->second->dc.push_back(dcs);
            int indexs = find->second->dc.size() - 1;
            find->second->dc_index.emplace(pks, indexs);
            HelperToMemPoolNewData(qdat);
            Information("Add candidate success");
            emit AddMoreSize(hvp, 1);
        }
        else {
            Information("Unexpected Error When trying add this candidate");
            return;
        }
    }
    else if (method == 2) {
        if (data->size() != BLOCK_DATAPARTISIPAN_BYTES) {
            Information("Error while trying to add new voter in this votepoll, Please try-again later");
            return;
        }
        uint8_t rawdp[BLOCK_DATAPARTISIPAN_BYTES]{};
        memcpy(rawdp, data->data(), BLOCK_DATAPARTISIPAN_BYTES);
        delete data;
        Block::DataPartisipan dp;
        Block::raw_to_datapartisipan(rawdp, dp);
        std::string pks = Utility::uint8_t_to_hexstring(dp.publickey, 32);
        std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
        if (auto find = set_display.find(hvp); find != set_display.end()) {
            auto findpk = find->second->dp_index.find(pks);
            if (findpk != find->second->dp_index.end()) {
                Information("Can't add this voter due to publickey already exist");
                return;
            }
            if (find->second->dp.size() >= find->second->vp.partisipansize) {
                Information("Can't add this voter due to voter already reached maximum");
                return;
            }
            long long currenttime = QDateTime::currentSecsSinceEpoch();
            currenttime += 150;
            if (currenttime >= find->second->vp.startdate) {
                Information("Can't add this voter due to polling registration is already closed");
                return;
            }
            if (!find->second->dp.isEmpty()) {
                Utility::hexstring_to_uint8_t(dp.parenthash, 32, find->second->dp.last().hash.toStdString());
            }
            else {
                Utility::hexstring_to_uint8_t(dp.parenthash, 32, hvp);
            }
            Utility::hexstring_to_uint8_t(dp.hashvotepoll, 32, hvp);
            Block::calculate_datapartisipan_signature(&dp, dp.signature, keypair);
            Block::Data bd;
            Block::datapartisipan_to_blockdata(&dp, bd);
            size_t size = bd.datsize + 4;
            QByteArray qdat = QByteArray((char*)&bd.data[0], size);
            VoteTree::DataPartisipanString dps;
            dps.pk = pks.c_str();
            uint8_t hash[32]{};
            Block::get_hash_datapartisipan(&dp, hash);
            dps.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
            find->second->dp.push_back(dps);
            int indexs = find->second->dp.size() - 1;
            find->second->dp_index.emplace(pks, indexs);
            emit HelperToMemPoolNewData(qdat);
            Information("Add voter success");
            emit AddMoreSize(hvp, 1);
        }
        else {
            Information("Unexpected Error When trying add this voter");
            return;
        }
    }
    else if (method == 3) {
        Block::DataCandidate dc;
        if (data->size() != BLOCK_DATACANDIDATE_BYTES) {
            Information("Error while trying to add new candidate in this votepoll, Please try-again later");
            return;
        }
        uint8_t rawdc[BLOCK_DATACANDIDATE_BYTES]{};
        memcpy(rawdc, data->data(), BLOCK_DATACANDIDATE_BYTES);
        delete data;
        Block::raw_to_datacandidate(rawdc, dc);
        QString pk = Utility::uint8_t_to_hexstring(dc.publickey, 32).c_str();
        std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
        QByteArray ce;
        if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
            if (auto findpk = findhvp->second->dc_index.find(pk.toStdString()); findpk != findhvp->second->dc_index.end()) {
                Information("Error while adding data, due to candidate already exist");
                emit setMail(3, pk, ce, 0);
                return;
            }
            if (auto findpk = findhvp->second->dp_index.find(pk.toStdString()); findpk != findhvp->second->dp_index.end()) {
                Information("Error while adding data, due this publickey already become voter");
                emit setMail(3, pk, ce, 0);
                return;
            }
            if (findhvp->second->dc.size() >= findhvp->second->vp.candidatsize) {
                Information("Error while adding data, due to candidate already reached maximum");
                emit setMail(4, pk, ce, 0);
                return;
            }
            long long currenttime = QDateTime::currentSecsSinceEpoch();
            currenttime += 150;
            if (currenttime >= findhvp->second->vp.startdate) {
                Information("Error while adding data, due to registration already closed");
                emit setMail(4, pk, ce, 0);
                return;
            }
            if (findhvp->second->dc.isEmpty()) {
                Utility::hexstring_to_uint8_t(dc.parenthash, 32, hvp);
            }
            else {
                Utility::hexstring_to_uint8_t(dc.parenthash, 32, findhvp->second->dc.last().hash.toStdString());
            }
            Block::calculate_datacandidate_signature(&dc, dc.signature, keypair);
            uint8_t hash[32]{};
            Block::get_hash_datacandidate(&dc, hash);
            VoteTree::DataCandidateString dcs;
            dcs.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
            dcs.blocktime = QDateTime::currentSecsSinceEpoch();
            dcs.name = Utility::uint8_t_to_ascii(dc.candidatename, 32).c_str();
            dcs.pk = pk;

            findhvp->second->dc.push_back(dcs);
            int indexdc = findhvp->second->dc.size() - 1;
            findhvp->second->dc_index.emplace(pk.toStdString(), indexdc);
            Information("Adding this candidate is success");
            long long bce = QDateTime::currentSecsSinceEpoch();
            emit setMail(5, pk, ce, bce);
            Block::Data bd;
            Block::datacandidate_to_blockdata(&dc, bd);
            int sizebd = bd.datsize + 4;
            emit AddMoreSize(hvp, 1);
            QByteArray qdat = QByteArray((char*)&bd.data[0], sizebd);
            emit HelperToMemPoolNewData(qdat);
        }
        else {
            Information("There was an error while adding this data, please try re-login");
            return;
        }
    }
    else if (method == 4) {
        Block::DataPartisipan dp;
        if (data->size() != BLOCK_DATAPARTISIPAN_BYTES) {
            Information("Error while trying to add new voter in this votepoll, Please try-again later");
            return;
        }
        uint8_t rawdp[BLOCK_DATAPARTISIPAN_BYTES]{};
        memcpy(rawdp, data->data(), BLOCK_DATAPARTISIPAN_BYTES);
        delete data;
        Block::raw_to_datapartisipan(rawdp, dp);
        QString pk = Utility::uint8_t_to_hexstring(dp.publickey, 32).c_str();
        std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
        QByteArray ce;
        if (auto findhvp = set_display.find(hvp); findhvp != set_display.end()) {
            if (auto findpk = findhvp->second->dp_index.find(pk.toStdString()); findpk != findhvp->second->dp_index.end()) {
                Information("Error while adding data, due to voter already exist");
                emit setMail(3, pk, ce, 0);
                return;
            }
            if (auto findpk = findhvp->second->dc_index.find(pk.toStdString()); findpk != findhvp->second->dc_index.end()) {
                Information("Error while adding data, due this publickey already become candidate");
                emit setMail(3, pk, ce, 0);
                return;
            }
            if (findhvp->second->dp.size() >= findhvp->second->vp.partisipansize) {
                Information("Error while adding data, due to voter already reached maximum");
                emit setMail(4, pk, ce, 0);
                return;
            }
            long long currenttime = QDateTime::currentSecsSinceEpoch();
            currenttime += 150;
            if (currenttime >= findhvp->second->vp.startdate) {
                Information("Error while adding data, due to registration already closed");
                emit setMail(4, pk, ce, 0);
                return;
            }
            if (findhvp->second->dp.isEmpty()) {
                Utility::hexstring_to_uint8_t(dp.parenthash, 32, hvp);
            }
            else {
                Utility::hexstring_to_uint8_t(dp.parenthash, 32, findhvp->second->dp.last().hash.toStdString());
            }
            Block::calculate_datapartisipan_signature(&dp, dp.signature, keypair);
            uint8_t hash[32]{};
            Block::get_hash_datapartisipan(&dp, hash);
            VoteTree::DataPartisipanString dps;
            dps.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
            dps.blocktime = QDateTime::currentSecsSinceEpoch();
            dps.pk = pk;
            findhvp->second->dp.push_back(dps);
            int indexdc = findhvp->second->dp.size() - 1;
            findhvp->second->dp_index.emplace(pk.toStdString(), indexdc);
            Information("Adding this voter is success");
            Block::Data bd;
            Block::datapartisipan_to_blockdata(&dp, bd);
            long long bce = QDateTime::currentSecsSinceEpoch();
            emit setMail(5, pk, ce, bce);
            emit AddMoreSize(hvp, 2);
            int sizebd = bd.datsize + 4;
            QByteArray qdat = QByteArray((char*)&bd.data[0], sizebd);
            emit HelperToMemPoolNewData(qdat);
        }
        else {
            Information("There was an error while adding this data, please try again later");
            return;
        }
    }
}

void Helper::Method_Admin_Mail(int con, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_admin_mail_data.push_back(new QByteArray(*dat));
    _queue_admin_mail_method.push_back(con);
    condition.wakeOne();
}

void Helper::handleAdminMail() {
    int method = -1;
    QByteArray* data = nullptr;
    mutex.lock();
    std::swap(data, _queue_admin_mail_data.front());
    method = _queue_admin_mail_method.front();
    _queue_admin_mail_data.pop_front();
    _queue_admin_mail_method.pop_front();
    mutex.unlock();
    if (data == nullptr) {
        Information("Error while trying to parse mail");
        return;
    }

    if (method == -1) {
        delete data;
        Information("Error while trying to parse mail");
        return;
    }

    if (method == 1) {
        if (data->size() != 32) {
            Information("Error while trying to parse mail");
            delete data;
            return;
        }
        uint8_t uhash[32]{};
        memcpy(uhash, data->data(), 32);
        std::string hvp = Utility::uint8_t_to_hexstring(uhash, 32).c_str(); 
        auto find = set_display.find(hvp);
        if ( find != set_display.end()) {
            for (int i = 0; i < find->second->msg.size(); i++) {
                std::string pks = find->second->msg[i].pk.toStdString();
                emit setMail(1 ,pks.c_str(), find->second->msg[i].cipher, find->second->msg[i].blocktime);
                if (auto findpk = find->second->dc_index.find(pks); findpk != find->second->dc_index.end()) {
                    QByteArray ce;
                    long long bce = find->second->dc[findpk->second].blocktime;
                    emit setMail(5, pks.c_str(), ce, bce);
                }
                if (auto findpk = find->second->dp_index.find(pks); findpk != find->second->dp_index.end()) {
                    QByteArray ce;
                    long long bce = find->second->dp[findpk->second].blocktime;
                    emit setMail(5, pks.c_str(), ce, bce);
                }
            }
            QString pke = find->second->vp.hash;
            QByteArray ce;
            long long bte = 0;
            emit setMail(2, pke, ce, bte);
        }
    }
}

void Helper::Method_Admin_List(int con, QString dat) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_admin_list_data.push_back(dat);
    _queue_admin_list_method.push_back(con);
    condition.wakeOne();
}

void Helper::handleAdminList() {
    mutex.lock();
    QString data = _queue_admin_list_data.front();
    _queue_admin_list_data.pop_front();
    int method = _queue_admin_list_method.front();
    _queue_admin_list_method.pop_front();
    mutex.unlock();

    if (data.isEmpty()) {
        Information("Error while parsing data, please try restarting app");
        return;
    }

    if (method == 1) {
        auto find = set_display.find(data.toStdString());
        if (find == set_display.end()) {
            Information("Error while parsing data, please try restarting app");
            return;
        }
        
        for (auto it = find->second->dc.begin(); it != find->second->dc.end(); it++) {
            QVector<QString> renderthis;
            renderthis.push_back(it->name);
            renderthis.push_back(it->pk);
            emit setList(1, renderthis, it->blocktime);
        }
        for (auto it = find->second->dp.begin(); it != find->second->dp.end(); it++) {
            QVector<QString> renderthis;
            renderthis.push_back(it->pk);
            emit setList(2, renderthis, it->blocktime);
        }
        QVector<QString> empt;
        emit setList(3, empt, 0);
    }
    else if (method == 2) {
        QVector<QString> empt;
        if (data.size() <= 64) {
            emit setList(4, empt, 0);
            return;
        }
        std::string hvp = data.mid(0, 64).toStdString();
        std::string search = data.mid(64, data.length()).toStdString();
        QString searchlow = search.c_str();
        searchlow = searchlow.toLower();

        // search candidate by name;
        auto findhvp = set_display.find(hvp);
        if (findhvp == set_display.end()) {
            emit setList(4, empt, 0);
            return;
        }

        QRegularExpression regex(QRegularExpression::escape(searchlow));
        for (int i = 0; i < findhvp->second->dc.size(); i++) {
            QString searchthis = findhvp->second->dc[i].name;
            searchthis = searchthis.toLower();
            if (regex.match(searchthis).hasMatch()) {
                QVector<QString> renderthis;
                renderthis.push_back(findhvp->second->dc[i].name);
                renderthis.push_back(findhvp->second->dc[i].pk);
                emit setList(1, renderthis, findhvp->second->dc[i].blocktime);
            }
        }
        emit setList(6, empt, findhvp->second->dc.size());
    }
    else if (method == 3) {
        QVector<QString> empt;
        if (data.size() <= 64) {
            emit setList(4, empt, 0);
            return;
        }
        // search candidate by pk;
        std::string hvp = data.mid(0, 64).toStdString();
        std::string search = data.mid(64, data.length()).toStdString();
        QString searchlow = search.c_str();
        searchlow = searchlow.toLower();

        auto findhvp = set_display.find(hvp);
        if (findhvp == set_display.end()) {
            emit setList(4, empt, 0);
            return;
        }
        QRegularExpression regex(QRegularExpression::escape(searchlow));
        for (int i = 0; i < findhvp->second->dc.size(); i++) {
            QString searchthis = findhvp->second->dc[i].pk;
            searchthis = searchthis.toLower();
            if (regex.match(searchthis).hasMatch()) {
                QVector<QString> renderthis;
                renderthis.push_back(findhvp->second->dc[i].name);
                renderthis.push_back(findhvp->second->dc[i].pk);
                emit setList(1, renderthis, findhvp->second->dc[i].blocktime);
            }
        }
        emit setList(6, empt, findhvp->second->dc.size());
    }
    else if (method == 4) {
        QVector<QString> empt;
        if (data.size() <= 64) {
            emit setList(5, empt, 0);
            return;
        }
        // search voter by pk
        std::string hvp = data.mid(0, 64).toStdString();
        std::string search = data.mid(64, data.length()).toStdString();
        QString searchlow = search.c_str();
        searchlow = searchlow.toLower();
        auto findhvp = set_display.find(hvp);
        if (findhvp == set_display.end()) {
            emit setList(5, empt, 0);
            return;
        }
        QRegularExpression regex(QRegularExpression::escape(searchlow));
        for (int i = 0; i < findhvp->second->dp.size(); i++) {
            QString searchthis = findhvp->second->dp[i].pk;
            searchthis = searchthis.toLower();
            if (regex.match(searchthis).hasMatch()) {
                QVector<QString> renderthis;
                renderthis.push_back(findhvp->second->dp[i].pk);
                emit setList(2, renderthis, findhvp->second->dp[i].blocktime);
            }
        }
        emit setList(7, empt, findhvp->second->dp.size());
    }
    else if (method == 5) {
        auto find = set_display.find(data.toStdString());
        if (find == set_display.end()) {
            Information("Error while parsing data, please try restarting app");
            return;
        }

        for (auto it = find->second->dc.begin(); it != find->second->dc.end(); it++) {
            QVector<QString> renderthis;
            renderthis.push_back(it->name);
            renderthis.push_back(it->pk);
            emit setList(1, renderthis, it->blocktime);
        }
        QVector<QString> empt;
        emit setList(8, empt, 0);
    }
    else if (method == 6) {
        auto find = set_display.find(data.toStdString());
        if (find == set_display.end()) {
            Information("Error while parsing data, please try restarting app");
            return;
        }
        for (auto it = find->second->dp.begin(); it != find->second->dp.end(); it++) {
            QVector<QString> renderthis;
            renderthis.push_back(it->pk);
            emit setList(2, renderthis, it->blocktime);
        }
        QVector<QString> empt;
        emit setList(9, empt, 0);
    }
}

void Helper::Method_Admin_Result(int con, QString hvp) {
    QMutexLocker locker(&mutex);
    _pause = true;
    _queue_admin_result_data.push_back(hvp);
    _queue_admin_result_method.push_back(con);
    condition.wakeOne();
}

void Helper::handleAdminResult() {
    mutex.lock();
    QString hvp = _queue_admin_result_data.front();
    int con = _queue_admin_result_method.front();
    _queue_admin_result_data.pop_front();
    _queue_admin_result_method.pop_front();
    mutex.unlock();

    if (hvp.isEmpty()) {
        Information("Error while parsing data, please try restarting app");
        return;
    }
    auto find = set_display.find(hvp.toStdString());
    if (find == set_display.end()) {
        Information("Error while parsing data, please try restarting app");
        return;
    }
    if (con == 1) {
        for (int i = 0; i < find->second->dc.size(); i++) {
            emit setResult(1, find->second->dc[i]);
        }
        VoteTree::DataCandidateString dcs;
        emit setResult(2, dcs);
    }
    else {
        Information("Failed to parsing data, please try again.");
        return;
    }
}

void Helper::Method_User_Vote(int con, QByteArray* dat) {
    QMutexLocker locker(&mutex);
    _pause = true;
    _queue_user_vote_data.push_back(new QByteArray(*dat));
    _queue_user_vote_method.push_back(con);
    condition.wakeOne();
}

void Helper::handleUserVote() {
    QByteArray* data = nullptr;
    mutex.lock();
    std::swap(data,_queue_user_vote_data.front());
    _queue_user_vote_data.pop_front();
    int con = _queue_user_vote_method.front();
    _queue_user_vote_method.pop_front();
    mutex.unlock();

    if (data == nullptr) {
        Information("Failed to parsing data, please try again.");
        return;
    }

    if (con == 1) {
        if (data->size() != 32) {
            delete data;
            Information("Failed to parsing data, please try again.");
            return;
        }
        std::string hvp = data->toHex().toStdString();
        delete data;
        auto find = set_display.find(hvp);
        if (find == set_display.end()) {
            Information("Failed to parsing data, please try again.");
            return;
        }
        for (int i = 0; i < find->second->dc.size(); i++) {
            emit setVote(1, find->second->dc[i]);
        }
        VoteTree::DataCandidateString dcs;
        emit setVote(2, dcs);
    }
    else if (con == 2) {
        if (data->size() != 64) {
            delete data;
            Information("Failed to vote, please try again.");
            return;
        }
        QString full = data->toHex();
        delete data;
        std::string hvp = full.mid(0, 64).toStdString();
        std::string pkcandidate = full.mid(64, full.length()).toStdString();
        auto find = set_display.find(hvp);
        if (find == set_display.end()) {
            Information("Failed to parsing data, please try again.");
            return;
        }
        int indexc = -1;
        if (auto findindexc = find->second->dc_index.find(pkcandidate); findindexc != find->second->dc_index.end()) {
            indexc = findindexc->second;
        }
        else {
            Information("Failed to parsing data, please try again.");
            return;
        }

        std::string mypks = Utility::uint8_t_to_hexstring(keypair->publickey, 32);
        TRS::Tag mytag;
        if (!find->second->get_tag(mypks, mytag)) {
            Information("Error when voting, please try-again later.");
            return;
        }
        std::string hashhextag = VoteTree::get_hash_hex_tag(&mytag);
        auto findtag = find->second->vote_index.find(hashhextag);
        VoteTree::VoteString* vstring = nullptr;
        if (findtag != find->second->vote_index.end()) {
            vstring = &find->second->vote[findtag->second];
            for (int a = 0; a < find->second->dc.size(); a++) {
                Block::DataVote votetemp;
                votetemp.tag = mytag;
                Utility::hexstring_to_uint8_t(votetemp.hashdatacandidate, 32, find->second->dc[a].hash.toStdString());
                Block::calculate_datavote_signature(&votetemp, votetemp.signature, keypair);
                std::string a1temp = Utility::uint8_t_to_hexstring(votetemp.signature.A_1, 32);
                if (vstring->vote_a1.find(a1temp) == vstring->vote_a1.end()) {
                    continue;
                }
                else {
                    Information("Can't vote again, due already vote before");
                    return;
                }
            }
        }

        Block::DataVote myvote;
        myvote.tag = mytag;
        Utility::hexstring_to_uint8_t(myvote.hashdatacandidate, 32, find->second->dc[indexc].hash.toStdString());
        Block::calculate_datavote_signature(&myvote, myvote.signature, keypair);
        Utility::int_to_uint8_t(mytag.publickey.size(), myvote.ringsize, 2);
        std::string a1string = Utility::uint8_t_to_hexstring(myvote.signature.A_1, 32);

        Block::Data bd;
        myvote.signature.ring_size = myvote.tag.publickey.size();
        Block::datavote_to_blockdata(&myvote, bd);
        int sizebd = bd.datsize + 4;
        QByteArray sendthis = QByteArray((char*)&bd.data[0], sizebd);
        emit HelperToMemPoolNewData(sendthis);
        if (vstring == nullptr) {
            VoteTree::VoteString newvs;
            newvs.vote_a1.emplace(a1string, indexc);
            newvs.block_time.emplace(a1string, 0);
            find->second->vote.push_back(newvs);
            int indexnewvs = find->second->vote.size() - 1;
            find->second->vote_index.emplace(hashhextag, indexnewvs);
        }
        else {
            vstring->vote_a1.emplace(a1string, indexc);
            vstring->block_time.emplace(a1string, 0);
        }
        emit setFlagMethod(5, hvp, true);
        Information("Vote success.");
    }
    else {
        delete data;
        Information("Failed to parsing data, please try again.");
        return;
    }
}

void Helper::tryConn(QString ipAddress, long long port, int con) {
    QMutexLocker locker(&mutex);
    _pause = false;
    _queue_ipAddress.push_back(ipAddress);
    _queue_port.push_back(port);
    _queue_conn_method.push_back(con);
    condition.wakeOne();
}

void Helper::handleTryConn() {
    mutex.lock();
    int conn = _queue_conn_method.front();
    _queue_conn_method.pop_front();
    int port = _queue_port.front();
    _queue_port.pop_front();
    QString ipAddress = _queue_ipAddress.front();
    _queue_ipAddress.pop_front();
    mutex.unlock();
    QTcpSocket temp_socket;
    temp_socket.connectToHost(ipAddress, port);
    if (temp_socket.waitForConnected(500)) {
        temp_socket.close();
        emit ConnectionInfo(ipAddress, port, true, conn);
    }
    else {
        emit ConnectionInfo(ipAddress, port, false, conn);
    }
}