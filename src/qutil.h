#ifndef QUTIL_H
#define QUTIL_H
#pragma once

#include <QVector>
#include <QByteArray>
#include <QJsonDocument>
#include <QDebug>
#include "util.h"
#include "block.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

struct QKeypair {
	Block::KeyPair key;
	std::string username;
	std::string password;
	int role;
	QVector<std::string> set_hvp_vote;
	QVector<std::string> set_hvp_candidate;
	std::unordered_map<std::string, long long> registeredAsCandidate;
	std::unordered_map<std::string, long long> registeredAsVoter;
	std::unordered_map<std::string, long long> requestRegisterAsCandidate;
	std::unordered_map<std::string, long long> requestRegisterAsVoter;
	QKeypair() {
		role = 0;
	}

	void calculate_keypair() {
		uint8_t u_hash[64]{};
		uint8_t* u_string = new uint8_t[username.length()];
		memcpy(u_string, username.data(), username.length());
		uint8_t* p_string = new uint8_t[password.length()];
		memcpy(p_string, password.data(), password.length());
		crypto_generichash_blake2b(u_hash, sizeof(u_hash), u_string, username.length(), p_string, password.length());
		memcpy(key.privatekey, u_hash, 32);
		trs_keypair_from_hash(u_hash, key.privatekey, key.publickey);
		delete[] u_string;
		delete[] p_string;
	}
};

namespace QtUtility {
	struct QJsonDC {
		QString name;
		QString pk;
	};
	struct QJsonDP {
		QString pk;
	};
	struct QJsonT {
		QJsonDC* dc;
		QJsonDP* dp;
		QString status;
		QJsonT() {
			dc = nullptr;
			dp = nullptr;
		}
		~QJsonT() {
			if (dc != nullptr) {
				delete dc;
				dc = nullptr;
			}
			if (dp != nullptr) {
				delete dp;
				dp = nullptr;
			}
		}
	};

	static void parseJson(const QJsonValue& jsonValue, QVector<QJsonDP>& dp, QVector<QJsonDC>& dc) {
		QString comparec = "candidate";
		QString comparep = "partisipan";
		if (jsonValue.isObject()) {
			QJsonObject jsonObject = jsonValue.toObject();
			for (const QString& key : jsonObject.keys()) {
				QJsonValue value = jsonObject.value(key);
				if (key.contains("type")) {
					if (value.toString().compare(comparec, Qt::CaseInsensitive) == 0) {
						qDebug() << jsonObject;
						QJsonDC dci; // Move the declaration inside the loop
						if (jsonObject.contains("name")) {
							dci.name = jsonObject.value("name").toString();
						}
						else {
							continue;
						}
						if (jsonObject.contains("publickey")) {
							dci.pk = jsonObject.value("publickey").toString();
						}
						else {
							continue;
						}
						dc.push_back(dci);
					}
					else if (value.toString().compare(comparep, Qt::CaseInsensitive) == 0) {
						if (jsonObject.contains("publickey")) {
							QJsonDP dpi; // Move the declaration inside the loop
							dpi.pk = jsonObject.value("publickey").toString();
							dp.push_back(dpi);
						}
						else {
							continue;
						}
					}
					continue;
				}
				parseJson(value, dp, dc);
			}
		}
		else if (jsonValue.isArray()) {
			QJsonArray jsonArray = jsonValue.toArray();
			for (const QJsonValue& arrayValue : jsonArray) {
				parseJson(arrayValue, dp, dc);  // Parse each element of the array individually
			}
		}
	}
	static void votepoll_to_qbytearray(Block::VotePoll* vp, QByteArray& qbytearray) {
		qbytearray.append(QByteArray((char*)&vp->publickey[0], 32));
		qbytearray.append(QByteArray((char*)&vp->signature[0], 64));
		qbytearray.append(QByteArray((char*)&vp->description[0], 64));
		qbytearray.append(QByteArray((char*)&vp->candidatesize[0], 4));
		qbytearray.append(QByteArray((char*)&vp->partisipansize[0], 4));
		qbytearray.append(QByteArray((char*)&vp->startdate[0], 4));
		qbytearray.append(QByteArray((char*)&vp->enddate[0], 4));
		qbytearray.append(QByteArray((char*)&vp->ringsize[0], 2));
		qbytearray.resize(178);
	}

	static void votepoll_to_qbytearray_wt(Block::VotePoll* vp, QByteArray& qbytearray) {
		uint8_t type[4]{};
		Block::get_blockdata_type(1, type);
		qbytearray.append(QByteArray((char*)&type[0], 4));
		qbytearray.append(QByteArray((char*)&vp->publickey[0], 32));
		qbytearray.append(QByteArray((char*)&vp->signature[0], 64));
		qbytearray.append(QByteArray((char*)&vp->description[0], 64));
		qbytearray.append(QByteArray((char*)&vp->candidatesize[0], 4));
		qbytearray.append(QByteArray((char*)&vp->partisipansize[0], 4));
		qbytearray.append(QByteArray((char*)&vp->startdate[0], 4));
		qbytearray.append(QByteArray((char*)&vp->enddate[0], 4));
		qbytearray.append(QByteArray((char*)&vp->ringsize[0], 2));
		qbytearray.resize(182);
	}

	static void get_blockdata_hash_qvector(Block::Block* block, QVector<std::string>& hash) {
		hash.clear();
		hash.resize(block->blockdata.size());
		for (int a = 0; a < hash.size(); a++) {
			uint8_t bhash[32]{};
			Block::get_hash_blockdata(*&block->blockdata[a], bhash);
			std::string shash = Utility::uint8_t_to_hexstring(bhash, 32);
			hash[a] = shash;
		}
	}

	static void raw_wt_to_blockdata(QByteArray* raw, Block::Data& bd) {
		memcpy(bd.type, raw->data(), 4);
		size_t size = 0;
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		if (Block::check_blockdata_type(bd.type, 1)) {
			size = BLOCK_VOTEPOLL_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw->constData(), size);
		}
		else if (Block::check_blockdata_type(bd.type, 2)) {
			uint8_t ringsize[2]{};
			memcpy(ringsize, raw->data() + 4, 2);
			size = static_cast<size_t>(Utility::uint8_t_to_int(ringsize, 2));
			size = (size * 96) + 98;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw->data(), size);
		}
		else if (Block::check_blockdata_type(bd.type, 3)) {
			size = BLOCK_DATACANDIDATE_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw->data(), size);
		}
		else if (Block::check_blockdata_type(bd.type, 4)) {
			size = BLOCK_DATAPARTISIPAN_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw->data(), size);
		}
		else if (Block::check_blockdata_type(bd.type, 5)) {
			uint8_t aessize[4]{};
			memcpy(aessize, raw->data() + 4, 4);
			size = static_cast<size_t>(Utility::uint8_t_to_int(aessize, 4));
			size += 132;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw->data(), size);
		}
	}

	// size is assumed to alread with magic bytes + 4;
	static void raw_wm_to_block(QByteArray* raw, Block::Block& block, size_t len) {
		if (block.header != nullptr) {
			delete block.header;
		}
		block.header = new Block::Header;
		memcpy(block.header->prevhash, raw->data() + 4, 32);
		memcpy(block.header->merkleroot, raw->data() + 36, 32);
		memcpy(block.header->timestamp, raw->data() + 68, 4);
		memcpy(block.header->nonce, raw->data() + 72, 4);
		size_t pos = 76;
		size_t sizebdwt = 0;
		block.blockdata.reserve(65536);
		while (pos < len) {
			uint8_t bdtype[4]{};
			memcpy(bdtype, raw->data() + pos, 4);
			if (Block::check_blockdata_type(bdtype, 1)) {
				sizebdwt = BLOCK_VOTEPOLL_BYTES;
			}
			else if (Block::check_blockdata_type(bdtype, 2)) {
				uint8_t rs[2]{};
				memcpy(rs, raw->data() + pos + 4, 2);
				long long ringsize = static_cast<size_t>(Utility::uint8_t_to_int(rs, 2));
				ringsize *= 96;
				sizebdwt = ringsize + 98;
			}
			else if (Block::check_blockdata_type(bdtype, 3)) {
				sizebdwt = BLOCK_DATACANDIDATE_BYTES;
			}
			else if (Block::check_blockdata_type(bdtype, 4)) {
				sizebdwt = BLOCK_DATAPARTISIPAN_BYTES;
			}
			else if (Block::check_blockdata_type(bdtype, 5)) {
				uint8_t as[4]{};
				memcpy(as, raw->data() + pos + 4, 4);
				long long aessize = Utility::uint8_t_to_int(as, 4);
				sizebdwt = aessize + 132;
			}
			else {
				break;
			}
			Block::Data* bd = new Block::Data;
			bd->datsize = sizebdwt;
			sizebdwt += 4;
			memcpy(bd->type, bdtype, 4);
			bd->data = new uint8_t[sizebdwt]{};
			memcpy_s(bd->data, sizebdwt, raw->data() + pos, sizebdwt);
			pos += sizebdwt;
			sizebdwt = 0;
			block.blockdata.push_back(bd);
		}
	}

	static void block_to_raw_wm(Block::Block* block, QByteArray* raw) {
		size_t len = Block::get_block_size(block);
		len += 4;
		uint8_t mb[4]{};
		Block::get_magic_bytes(mb);
		raw->prepend(QByteArray((char*)&mb[0], 4), 4);
		uint8_t header[72]{};
		Block::blockheader_to_raw(*&block->header, header);
		raw->append(QByteArray((char*)&header[0], 72), 72);
		int pos = 4;
		for (int a = 0; a < block->blockdata.size(); a++) {
			size_t bdsize = block->blockdata[a]->datsize;
			bdsize += 4;
			raw->append(QByteArray((char*)&block->blockdata[a]->data[0], bdsize), bdsize);
			pos += static_cast<int>(bdsize);
		}
	}
}

#endif