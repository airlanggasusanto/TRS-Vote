#pragma once
#ifndef BLOCK_H
#define BLOCK_H

#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iomanip>
#include <sstream>
#include "trslib.h"
#include "merkle.h"
#include "util.h"
#include "sodium.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <QDebug>

#define BLOCK_VOTEPOLL_BYTES 178
#define BLOCK_DATACANDIDATE_BYTES 192
#define BLOCK_DATAPARTISIPAN_BYTES 160
#define MAXIMUMBLOCKSIZE 134217728
#define BLOCK_DATA_MINIMUM_BYTES 160
#define MAXIMUMDISPLAYSIZE 2097152
#define MAXIMUMNONCE 4294967296

namespace Block {
	struct KeyPair {
		uint8_t privatekey[32]{};
		uint8_t publickey[32]{};

		KeyPair() {

		}

		KeyPair(uint8_t* privkey) {
			memcpy(privatekey, privkey, 32);
			trs_keypair_from_seed(privatekey, publickey);
		}

		void calculate_base() {
			trs_keypair_from_seed(privatekey, publickey);
		}
	};

	struct Message {
		AES_Data* cipher;
		uint8_t publickey[32]{};
		uint8_t signature[64]{};
		uint8_t hashvotepoll[32]{};
		Message() {
			cipher = nullptr;
		}

		~Message() {
			if (cipher != nullptr) {
				delete cipher;
			}
		}
	};

	struct VotePoll {
		uint8_t publickey[32]{};
		uint8_t signature[64]{};
		uint8_t description[64]{};
		uint8_t candidatesize[4]{};
		uint8_t partisipansize[4]{};
		uint8_t startdate[4]{};
		uint8_t enddate[4]{};
		uint8_t ringsize[2]{};
	};

	struct DataCandidate {
		uint8_t signature[crypto_sign_BYTES]{};
		uint8_t parenthash[crypto_generichash_blake2b_BYTES]{};
		uint8_t hashvotepoll[crypto_generichash_blake2b_BYTES]{};
		uint8_t candidatename[32]{};
		uint8_t publickey[crypto_sign_PUBLICKEYBYTES]{};
	};

	struct DataPartisipan {
		uint8_t signature[crypto_sign_BYTES]{};
		uint8_t parenthash[crypto_generichash_blake2b_BYTES]{};
		uint8_t hashvotepoll[crypto_generichash_blake2b_BYTES]{};
		uint8_t publickey[crypto_sign_PUBLICKEYBYTES]{};
	};

	struct DataVote {
		uint8_t ringsize[2]{};
		TRS::Signature signature;
		TRS::Tag tag;
		uint8_t hashdatacandidate[32]{};
		DataVote() {

		}
		~DataVote() {
			tag.publickey.clear();
		}
	};

	struct Header {
		uint8_t prevhash[32]{};
		uint8_t merkleroot[32]{};
		uint8_t timestamp[4]{};
		uint8_t nonce[4]{};
	};

	struct Data {
		uint8_t type[4]{};
		uint8_t* data;
		size_t datsize{ 0 };

		Data() {
			data = nullptr;
		}
		Data(Data* copy) {
			memcpy(type, copy->type, 4);
			datsize = copy->datsize;
			data = new uint8_t[datsize + 4];
			memcpy_s(data, datsize + 4, copy->data, datsize + 4);
		}
		~Data() {
			if (data != nullptr) {
				delete[] data;
			}
		}
	};

	struct Block {
		Header* header = nullptr;
		std::vector<Data*> blockdata;
		Block() {
			header = nullptr;
		}

		~Block() {
			if (header != nullptr) {
				delete header;
			}
			for (int a = 0; a < blockdata.size(); a++) {
				delete blockdata[a];
			}
			blockdata.clear();
		}
	};

	struct Explorer {
		std::unordered_set<std::string> set_hash;
		long long time;
		Explorer() {
			time = 0;
		}
		~Explorer() {
			set_hash.clear();
		}
	};

	static void get_magic_bytes(uint8_t(&out)[4]) {
		char __magicbytes[8 + 1] = { 'f','7','2','6','5','6','f','6','\0' };
		sodium_hex2bin(out, 4, __magicbytes, 9, NULL, NULL, NULL);
	}

	static bool check_magic_bytes(uint8_t* mb) {
		uint8_t temp[4]{};
		uint8_t magicbytes[4]{};
		memcpy(magicbytes, mb, 4);
		get_magic_bytes(temp);
		if (sodium_memcmp(temp, magicbytes, 4) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	static void get_blockdata_type(int a, uint8_t(&out)[4]) {
		char __typevp[8 + 1] = { '7','0','6','F','6','C','6','C','\0' };
		char __typedv[8 + 1] = { '7','6','6','F','7','4','6','5','\0' };
		char __typedc[8 + 1] = { '6','1','6','E','7','3','7','7','\0' };
		char __typedp[8 + 1] = { '6','D','6','5','6','D','6','2','\0' };
		char __typemsg[8 + 1] = { '6','3','6','9','7','0','6','8','\0' };
		if (a == 1) {
			sodium_hex2bin(out, 4, __typevp, 9, NULL, NULL, NULL);
		}
		else if (a == 2) {
			sodium_hex2bin(out, 4, __typedv, 9, NULL, NULL, NULL);
		}
		else if (a == 3) {
			sodium_hex2bin(out, 4, __typedc, 9, NULL, NULL, NULL);
		}
		else if (a == 4) {
			sodium_hex2bin(out, 4, __typedp, 9, NULL, NULL, NULL);
		}
		else if (a == 5) {
			sodium_hex2bin(out, 4, __typemsg, 9, NULL, NULL, NULL);
		}
	}

	static const boost::multiprecision::uint256_t get_difficulty() {
		std::istringstream targetmax{ "000000ffff000000000000000000000000000000000000000000000000000000" };
		boost::multiprecision::uint256_t u_int;
		targetmax >> std::hex >> u_int;
		return u_int;
	}

	static bool check_block_hash(uint8_t* blkhash) {
		boost::multiprecision::uint256_t target = get_difficulty();
		boost::multiprecision::uint256_t hashdecimal;
		char hex[65]{};
		sodium_bin2hex(hex, 65, blkhash, 32);
		std::istringstream hash{ hex };
		hash >> std::hex >> hashdecimal;
		if (hashdecimal < target) {
			return true;
		}
		return false;
	}

	static bool check_blockdata_type(uint8_t* datype, int numtype) {
		uint8_t datatype[4]{};
		get_blockdata_type(numtype, datatype);
		uint8_t type[4]{};
		memcpy(type, datype, 4);
		if (sodium_memcmp(datatype, type, 4) == 0) {
			return true;
		}
		return false;
	}

	static void raw_to_votepoll(const uint8_t* raw, VotePoll& vp) {
		uint8_t temp[BLOCK_VOTEPOLL_BYTES]{};
		memcpy(temp, raw, BLOCK_VOTEPOLL_BYTES);
		memcpy(vp.publickey, temp, sizeof vp.publickey);
		memcpy(vp.signature, temp + sizeof vp.publickey, sizeof vp.signature);
		memcpy(vp.description, temp + sizeof vp.publickey + sizeof vp.signature, 64);
		memcpy(vp.candidatesize, temp + sizeof vp.publickey + sizeof vp.signature + 64, 4);
		memcpy(vp.partisipansize, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4, 4);
		memcpy(vp.startdate, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4, 4);
		memcpy(vp.enddate, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4, 4);
		memcpy(vp.ringsize, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4 + 4, 2);
	}

	static void raw_to_datacandidate(const uint8_t* raw, DataCandidate& dc) {
		uint8_t temp[192]{};
		memcpy(temp, raw, 192);
		memcpy(dc.signature, temp, crypto_sign_BYTES);
		memcpy(dc.parenthash, temp + crypto_sign_BYTES, crypto_generichash_BYTES);
		memcpy(dc.hashvotepoll, temp + crypto_sign_BYTES + crypto_generichash_BYTES, crypto_generichash_BYTES);
		memcpy(dc.candidatename, temp + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
		memcpy(dc.publickey, temp + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
	}

	static void raw_to_datapartisipan(const uint8_t* raw, DataPartisipan& dp) {
		uint8_t temp[160]{};
		memcpy(temp, raw, 160);
		memcpy(dp.signature, temp, crypto_sign_BYTES);
		memcpy(dp.parenthash, temp + crypto_sign_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.hashvotepoll, temp + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.publickey, temp + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES + crypto_generichash_blake2b_BYTES, crypto_sign_PUBLICKEYBYTES);
	}

	static void raw_to_datavote(const uint8_t* raw, DataVote& dv) {
		memcpy(dv.ringsize, raw, 2);
		size_t integerringsize = size_t(dv.ringsize[1]);
		dv.signature.ring_size = integerringsize;
		size_t datasize = (integerringsize * 64) + 32 + 98;
		uint8_t* memtemp = new uint8_t[datasize]{};
		memcpy(memtemp, raw, datasize);
		size_t sigpos = 2;
		memcpy(dv.signature.A_1, memtemp + sigpos, 32);
		size_t czsize = (32 * integerringsize);
		size_t c_n = sigpos + 32;
		size_t z_n = sigpos + 32 + czsize;
		if (dv.signature.C_n != nullptr) {
			delete[]dv.signature.C_n;
		}
		if (dv.signature.Z_n != nullptr) {
			delete[]dv.signature.Z_n;
		}
		dv.signature.C_n = new uint8_t[czsize]{};
		dv.signature.Z_n = new uint8_t[czsize]{};
		memcpy(dv.signature.C_n, memtemp + c_n, czsize);
		memcpy(dv.signature.Z_n, memtemp + z_n, czsize);
		size_t pos = z_n + czsize;
		dv.tag.publickey.resize(integerringsize);
		memcpy(dv.tag.publickey.data(), memtemp + pos, czsize);
		pos += czsize;
		memcpy(dv.tag.issue, memtemp + pos, 32);
		pos += 32;
		memcpy(dv.hashdatacandidate, memtemp + pos, 32);
		delete[] memtemp;
	}

	static void raw_to_msg(uint8_t* raw, Message& msg) {
		int size = static_cast<int>(Utility::uint8_t_to_int(raw, 4));
		int aessize = size;
		if (aessize < 28) {
			return;
		}
		int ciphersize = aessize - 28;
		if (ciphersize < 0) {
			return;
		}
		memcpy(msg.signature, raw + 4, 64);
		memcpy(msg.publickey, raw + 68, 32);
		memcpy(msg.hashvotepoll, raw + 100, 32);
		if (msg.cipher != nullptr) {
			delete msg.cipher;
		}
		msg.cipher = new AES_Data;
		msg.cipher->ciphertext = new uint8_t[ciphersize]{};
		msg.cipher->cipherlen = static_cast<size_t>(ciphersize);
		int pos = 132;
		memcpy(msg.cipher->ciphertext, raw + pos, ciphersize);
		pos += ciphersize;
		memcpy(msg.cipher->tag, raw + pos, 16);
		pos += 16;
		memcpy(msg.cipher->nonce, raw + pos, 12);
	}

	static void votepoll_to_raw(VotePoll* vp, uint8_t(&raw)[BLOCK_VOTEPOLL_BYTES]) {
		memcpy(raw, vp->publickey, sizeof vp->publickey);
		memcpy(raw + sizeof vp->publickey, vp->signature, sizeof vp->signature);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature, vp->description, sizeof vp->description);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description, vp->candidatesize, sizeof vp->candidatesize);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize, vp->partisipansize, sizeof vp->partisipansize);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize, vp->startdate, sizeof vp->startdate);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate, vp->enddate, sizeof vp->enddate);
		memcpy(raw + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate + sizeof vp->enddate, vp->ringsize, sizeof vp->ringsize);
	}

	static void datacandidate_to_raw(DataCandidate* dc, uint8_t(&raw)[BLOCK_DATACANDIDATE_BYTES]) {
		memcpy(raw, dc->signature, sizeof dc->signature);
		memcpy(raw + sizeof dc->signature, dc->parenthash, sizeof dc->parenthash);
		memcpy(raw + sizeof dc->signature + sizeof dc->parenthash, dc->hashvotepoll, sizeof dc->hashvotepoll);
		memcpy(raw + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll, dc->candidatename, sizeof dc->candidatename);
		memcpy(raw + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll + sizeof dc->candidatename, dc->publickey, sizeof dc->publickey);
	}

	static void datapartisipan_to_raw(DataPartisipan* dp, uint8_t(&raw)[BLOCK_DATAPARTISIPAN_BYTES]) {
		memcpy(raw, dp->signature, sizeof dp->signature);
		memcpy(raw + sizeof dp->signature, dp->parenthash, sizeof dp->parenthash);
		memcpy(raw + sizeof dp->signature + sizeof dp->parenthash, dp->hashvotepoll, sizeof dp->hashvotepoll);
		memcpy(raw + sizeof dp->signature + sizeof dp->parenthash + sizeof dp->hashvotepoll, dp->publickey, sizeof dp->publickey);
	}

	static void datavote_to_raw(DataVote* dp, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[] raw;
		}
		int datasize = Utility::uint8_t_to_int(*&dp->ringsize, 2);
		datasize = (datasize * 96) + 98;
		raw = new uint8_t[datasize]{};
		size_t sizesig = dp->signature.ring_size * 32;
		memcpy_s(raw, datasize, dp->ringsize, sizeof(dp->ringsize));
		memcpy_s(raw + 2, datasize, dp->signature.A_1, sizeof(dp->signature.A_1));
		memcpy_s(raw + 2 + 32, datasize, dp->signature.C_n, sizesig);
		memcpy_s(raw + 2 + 32 + sizesig, datasize, dp->signature.Z_n, sizesig);
		memcpy_s(raw + 2 + 32 + sizesig + sizesig, datasize, dp->tag.publickey.data(), sizesig);
		memcpy_s(raw + 2 + 32 + sizesig + sizesig + sizesig, datasize, dp->tag.issue, sizeof(dp->tag.issue));
		memcpy_s(raw + 2 + 32 + sizesig + sizesig + sizesig + sizeof(dp->tag.issue), datasize, dp->hashdatacandidate, sizeof(dp->hashdatacandidate));
	}

	static void msg_to_raw(Message* msg, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[]raw;
		}
		int datasize = 132 + 28 + static_cast<int>(msg->cipher->cipherlen);
		int aessize = datasize - 132;
		raw = new uint8_t[datasize]{};
		uint8_t size[4]{};
		Utility::int_to_uint8_t(aessize, size, 4);
		memcpy_s(raw,datasize, size, 4);
		memcpy_s(raw + 4, datasize, msg->signature, 64);
		memcpy_s(raw + 68, datasize, msg->publickey, 32);
		memcpy_s(raw + 100, datasize, msg->hashvotepoll, 32);
		memcpy_s(raw + 132, datasize, msg->cipher->ciphertext, msg->cipher->cipherlen);
		memcpy_s(raw + 132 + msg->cipher->cipherlen, datasize, msg->cipher->tag, 16);
		memcpy_s(raw + 132 + msg->cipher->cipherlen + 16, datasize, msg->cipher->nonce, 12);
	}

	static void raw_wt_to_votepoll(const uint8_t* raw, VotePoll& vp) {
		uint8_t temp[BLOCK_VOTEPOLL_BYTES]{};
		memcpy(temp, raw + 4, BLOCK_VOTEPOLL_BYTES);
		memcpy(vp.publickey, temp, sizeof vp.publickey);
		memcpy(vp.signature, temp + sizeof vp.publickey, sizeof vp.signature);
		memcpy(vp.description, temp + sizeof vp.publickey + sizeof vp.signature, 64);
		memcpy(vp.candidatesize, temp + sizeof vp.publickey + sizeof vp.signature + 64, 4);
		memcpy(vp.partisipansize, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4, 4);
		memcpy(vp.startdate, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4, 4);
		memcpy(vp.enddate, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4, 4);
		memcpy(vp.ringsize, temp + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4 + 4, 2);
	}

	static void raw_wt_to_datacandidate(const uint8_t* raw, DataCandidate& dc) {
		uint8_t temp[192]{};
		memcpy(temp, raw + 4, 192);
		memcpy(dc.signature, temp, crypto_sign_BYTES);
		memcpy(dc.parenthash, temp + crypto_sign_BYTES, crypto_generichash_BYTES);
		memcpy(dc.hashvotepoll, temp + crypto_sign_BYTES + crypto_generichash_BYTES, crypto_generichash_BYTES);
		memcpy(dc.candidatename, temp + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
		memcpy(dc.publickey, temp + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
	}

	static void raw_wt_to_datapartisipan(const uint8_t* raw, DataPartisipan& dp) {
		uint8_t temp[160]{};
		memcpy(temp, raw + 4, 160);
		memcpy(dp.signature, temp, crypto_sign_BYTES);
		memcpy(dp.parenthash, temp + crypto_sign_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.hashvotepoll, temp + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.publickey, temp + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES + crypto_generichash_blake2b_BYTES, crypto_sign_PUBLICKEYBYTES);
	}

	static void raw_wt_to_msg(uint8_t* raw, Message& msg) {
		long long size = static_cast<long long>(Utility::uint8_t_to_int(raw + 4, 4));
		long long aessize = size;
		if (aessize < 28) {
			return;
		}
		int ciphersize = aessize - 28;
		if (ciphersize < 0) {
			return;
		}
		memcpy(msg.signature, raw + 4 + 4, 64);
		memcpy(msg.publickey, raw + 4 + 68, 32);
		memcpy(msg.hashvotepoll, raw + 4 + 68 + 32, 32);
		if (msg.cipher != nullptr) {
			delete msg.cipher;
		}
		msg.cipher = new AES_Data;
		msg.cipher->cipherlen = static_cast<size_t>(ciphersize);
		int pos = 132;
		msg.cipher->ciphertext = new uint8_t[ciphersize]{};
		memcpy(msg.cipher->ciphertext, raw + 4 + pos, ciphersize);
		pos += ciphersize;
		memcpy(msg.cipher->tag, raw + 4 + pos, 16);
		pos += 16;
		memcpy(msg.cipher->nonce, raw + 4 + pos, 12);
	}

	static void raw_wt_to_datavote(const uint8_t* raw, DataVote& dv) {
		memcpy(dv.ringsize, raw + 4, 2);
		size_t integerringsize = static_cast<size_t>(Utility::uint8_t_to_int(dv.ringsize, 2));
		dv.signature.ring_size = integerringsize;
		size_t datasize = (integerringsize * 64) + 32 + 98;
		uint8_t* memtemp = new uint8_t[datasize]{};
		memcpy(memtemp, raw + 6, datasize);
		size_t sigpos = 6;
		memcpy(dv.signature.A_1, memtemp + sigpos, 32);
		size_t czsize = (32 * integerringsize);
		size_t c_n = sigpos + 32;
		size_t z_n = sigpos + 32 + czsize;
		if (dv.signature.C_n != nullptr) {
			delete[]dv.signature.C_n;
		}

		if (dv.signature.Z_n != nullptr) {
			delete[]dv.signature.Z_n;
		}
		dv.signature.C_n = new uint8_t[czsize]{};
		dv.signature.Z_n = new uint8_t[czsize]{};
		memcpy(dv.signature.C_n, memtemp + c_n, czsize);
		memcpy(dv.signature.Z_n, memtemp + z_n, czsize);
		size_t pos = z_n + czsize;
		dv.tag.publickey.resize(integerringsize);
		memcpy(dv.tag.publickey.data(), memtemp + pos, czsize);
		pos += czsize;
		memcpy(dv.tag.issue, memtemp + pos, 32);
		pos += 32;
		memcpy(dv.hashdatacandidate, memtemp + pos, 32);
		delete[] memtemp;
	}

	static void votepoll_to_raw_wt(VotePoll* vp, uint8_t(&raw)[BLOCK_VOTEPOLL_BYTES + 4]) {
		uint8_t type[4]{};
		get_blockdata_type(1, type);
		memcpy(raw, type, 4);
		memcpy(raw + 4, vp->publickey, sizeof vp->publickey);
		memcpy(raw + 4 + sizeof vp->publickey, vp->signature, sizeof vp->signature);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature, vp->description, sizeof vp->description);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description, vp->candidatesize, sizeof vp->candidatesize);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize, vp->partisipansize, sizeof vp->partisipansize);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize, vp->startdate, sizeof vp->startdate);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate, vp->enddate, sizeof vp->enddate);
		memcpy(raw + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate + sizeof vp->enddate, vp->ringsize, sizeof vp->ringsize);
	}

	static void datavote_to_raw_wt(DataVote* dv, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[] raw;
		}
		size_t datasize = static_cast<size_t>(Utility::uint8_t_to_int(dv->ringsize, 2));
		datasize = (datasize * 96) + 98 + 4;
		raw = new uint8_t[datasize]{};
		size_t sizesig = dv->signature.ring_size * 32;
		uint8_t type[4]{};
		get_blockdata_type(2, type);
		memcpy_s(raw, datasize, type, 4);
		memcpy_s(raw + 4, datasize, dv->ringsize, sizeof(dv->ringsize));
		memcpy_s(raw + 4 + 2, datasize, dv->signature.A_1, sizeof(dv->signature.A_1));
		memcpy_s(raw + 4 + 2 + 32, datasize, dv->signature.C_n, sizesig);
		memcpy_s(raw + 4 + 2 + 32 + sizesig, datasize, dv->signature.Z_n, sizesig);
		memcpy_s(raw + 4 + 2 + 32 + sizesig + sizesig, datasize, dv->tag.publickey.data(), sizesig);
		memcpy_s(raw + 4 + 2 + 32 + sizesig + sizesig + sizesig, datasize, dv->tag.issue, sizeof(dv->tag.issue));
		memcpy_s(raw + 4 + 2 + 32 + sizesig + sizesig + sizesig + sizeof(dv->tag.issue), datasize, dv->hashdatacandidate, sizeof(dv->hashdatacandidate));
	}

	static void datacandidate_to_raw_wt(DataCandidate* dc, uint8_t(&raw)[BLOCK_DATACANDIDATE_BYTES + 4]) {
		uint8_t type[4]{};
		get_blockdata_type(3, type);
		memcpy(raw, type, 4);
		memcpy(raw + 4, dc->signature, sizeof dc->signature);
		memcpy(raw + 4 + sizeof dc->signature, dc->parenthash, sizeof dc->parenthash);
		memcpy(raw + 4 + sizeof dc->signature + sizeof dc->parenthash, dc->hashvotepoll, sizeof dc->hashvotepoll);
		memcpy(raw + 4 + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll, dc->candidatename, sizeof dc->candidatename);
		memcpy(raw + 4 + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll + sizeof dc->candidatename, dc->publickey, sizeof dc->publickey);
	}

	static void datapartisipan_to_raw_wt(DataPartisipan* dp, uint8_t(&raw)[BLOCK_DATAPARTISIPAN_BYTES + 4]) {
		uint8_t type[4]{};
		get_blockdata_type(4, type);
		memcpy(raw, type, 4);
		memcpy(raw + 4, dp->signature, sizeof dp->signature);
		memcpy(raw + 4 + sizeof dp->signature, dp->parenthash, sizeof dp->parenthash);
		memcpy(raw + 4 + sizeof dp->signature + sizeof dp->parenthash, dp->hashvotepoll, sizeof dp->hashvotepoll);
		memcpy(raw + 4 + sizeof dp->signature + sizeof dp->parenthash + sizeof dp->hashvotepoll, dp->publickey, sizeof dp->publickey);
	}

	static void msg_to_raw_wt(Message* msg, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[] raw;
		}
		int datasize = 136 + 28 + static_cast<int>(msg->cipher->cipherlen);
		int aessize = datasize - 136;
		raw = new uint8_t[datasize]{};
		uint8_t size[4]{};
		uint8_t type[4]{};
		Utility::int_to_uint8_t(aessize, size, 4);
		get_blockdata_type(5, type);
		memcpy_s(raw, datasize, type, 4);
		memcpy_s(raw + 4, datasize, size, 4);
		memcpy_s(raw + 8, datasize, msg->signature, 64);
		memcpy_s(raw + 72, datasize, msg->publickey, 32);
		memcpy_s(raw + 104, datasize, msg->hashvotepoll, 32);
		memcpy_s(raw + 136, datasize, msg->cipher->ciphertext, msg->cipher->cipherlen);
		memcpy_s(raw + 136 + msg->cipher->cipherlen, datasize, msg->cipher->tag, 16);
		memcpy_s(raw + 136 + msg->cipher->cipherlen + 16, datasize, msg->cipher->nonce, 12);
	}

	static void raw_wt_to_blockdata(uint8_t* raw, Data& bd) {
		memcpy(bd.type, raw, 4);
		size_t size = 0;
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		if (check_blockdata_type(bd.type, 1)) {
			size = BLOCK_VOTEPOLL_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw, size);
		}
		else if (check_blockdata_type(bd.type, 2)) {
			size = static_cast<size_t>(Utility::uint8_t_to_int(raw + 4, 2));
			size = (size * 96) + 32 + 66;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw, size);
		}
		else if (check_blockdata_type(bd.type, 3)) {
			size = BLOCK_DATACANDIDATE_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw, size);
		}
		else if (check_blockdata_type(bd.type, 4)) {
			size = BLOCK_DATAPARTISIPAN_BYTES;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw, size);
		}
		else if (check_blockdata_type(bd.type, 5)) {
			size = static_cast<size_t>(Utility::uint8_t_to_int(raw + 4, 4));
			size += 132;
			bd.datsize = size;
			size += 4;
			bd.data = new uint8_t[size]{};
			memcpy(bd.data, raw, size);
		}
	}

	static void blockdata_to_raw_wt(Data* bd, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[] raw;
		}
		size_t size = bd->datsize + 4;
		raw = new uint8_t[size]{};
		memcpy_s(raw, size, bd->data, size);
	}

	static void blockdata_to_votepoll(Data* bd, VotePoll& vp) {
		memcpy(vp.publickey, bd->data + 4, sizeof vp.publickey);
		memcpy(vp.signature, bd->data + 4 + sizeof vp.publickey, sizeof vp.signature);
		memcpy(vp.description, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature, 64);
		memcpy(vp.candidatesize, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature + 64, 4);
		memcpy(vp.partisipansize, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature + 64 + 4, 4);
		memcpy(vp.startdate, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4, 4);
		memcpy(vp.enddate, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4, 4);
		memcpy(vp.ringsize, bd->data + 4 + sizeof vp.publickey + sizeof vp.signature + 64 + 4 + 4 + 4 + 4, 2);
	}

	static void blockdata_to_datavote(Data* bd, DataVote& dv) {
		memcpy(dv.ringsize, bd->data + 4, 2);
		size_t integerringsize = static_cast<size_t>(Utility::uint8_t_to_int(dv.ringsize, 2));
		dv.signature.ring_size = integerringsize;
		size_t sigpos = 6;
		memcpy(dv.signature.A_1, bd->data + sigpos, 32);
		size_t czsize = (32 * integerringsize);
		size_t c_n = sigpos + 32;
		size_t z_n = sigpos + 32 + czsize;
		if (dv.signature.C_n != nullptr) {
			delete[]dv.signature.C_n;
		}

		if (dv.signature.Z_n != nullptr) {
			delete[]dv.signature.Z_n;
		}
		dv.signature.C_n = new uint8_t[czsize]{};
		dv.signature.Z_n = new uint8_t[czsize]{};
		memcpy(dv.signature.C_n, bd->data + c_n, czsize);
		memcpy(dv.signature.Z_n, bd->data + z_n, czsize);
		size_t pos = z_n + czsize;
		dv.tag.publickey.resize(integerringsize);
		memcpy(dv.tag.publickey.data(), bd->data + pos, czsize);
		pos += czsize;
		memcpy(dv.tag.issue, bd->data + pos, 32);
		pos += 32;
		memcpy(dv.hashdatacandidate, bd->data + pos, 32);
	}

	static void blockdata_to_datacandidate(Data* bd, DataCandidate& dc) {
		memcpy(dc.signature, bd->data + 4, crypto_sign_BYTES);
		memcpy(dc.parenthash, bd->data + 4 + crypto_sign_BYTES, crypto_generichash_BYTES);
		memcpy(dc.hashvotepoll, bd->data + 4 + crypto_sign_BYTES + crypto_generichash_BYTES, crypto_generichash_BYTES);
		memcpy(dc.candidatename, bd->data + 4 + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
		memcpy(dc.publickey, bd->data + 4 + crypto_sign_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES + crypto_generichash_BYTES, 32);
	}

	static void blockdata_to_datapartisipan(Data* bd, DataPartisipan& dp) {
		memcpy(dp.signature, bd->data + 4, crypto_sign_BYTES);
		memcpy(dp.parenthash, bd->data + 4 + crypto_sign_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.hashvotepoll, bd->data + 4 + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES, crypto_generichash_blake2b_BYTES);
		memcpy(dp.publickey, bd->data + 4 + crypto_sign_BYTES + crypto_generichash_blake2b_BYTES + crypto_generichash_blake2b_BYTES, crypto_sign_PUBLICKEYBYTES);
	}

	static void blockdata_to_msg(Data* bd, Message& msg) {
		raw_wt_to_msg(bd->data, msg);
	}

	static void votepoll_to_blockdata(VotePoll* vp, Data& bd) {
		get_blockdata_type(1, bd.type);
		bd.datsize = 0;
		bd.datsize = BLOCK_VOTEPOLL_BYTES;
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		size_t size = bd.datsize + 4;
		bd.data = new uint8_t[size]{};
		memcpy_s(bd.data, size, bd.type, 4);
		memcpy_s(bd.data + 4, size, vp->publickey, sizeof vp->publickey);
		memcpy_s(bd.data + 4 + sizeof vp->publickey, size, vp->signature, sizeof vp->signature);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature, size, vp->description, sizeof vp->description);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description, size, vp->candidatesize, sizeof vp->candidatesize);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize, size, vp->partisipansize, sizeof vp->partisipansize);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize, size, vp->startdate, sizeof vp->startdate);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate, size, vp->enddate, sizeof vp->enddate);
		memcpy_s(bd.data + 4 + sizeof vp->publickey + sizeof vp->signature + sizeof vp->description + sizeof vp->candidatesize + sizeof vp->partisipansize + sizeof vp->startdate + sizeof vp->enddate, size, vp->ringsize, sizeof vp->ringsize);
	}

	static void datavote_to_blockdata(DataVote* dv, Data& bd) {
		get_blockdata_type(2, bd.type);
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		bd.datsize = 0;
		bd.datsize = static_cast<size_t>(Utility::uint8_t_to_int(dv->ringsize, 2));
		bd.datsize = (bd.datsize * 96) + 66 + 32;
		size_t size = bd.datsize + 4;
		bd.data = new uint8_t[size]{};
		size_t sizesig = dv->signature.ring_size * 32;
		memcpy_s(bd.data, size, bd.type, 4);
		memcpy_s(bd.data + 4, size, dv->ringsize, sizeof(dv->ringsize));
		memcpy_s(bd.data + 4 + 2, size, dv->signature.A_1, sizeof(dv->signature.A_1));
		memcpy_s(bd.data + 4 + 2 + 32, size, dv->signature.C_n, sizesig);
		memcpy_s(bd.data + 4 + 2 + 32 + sizesig, size, dv->signature.Z_n, sizesig);
		memcpy_s(bd.data + 4 + 2 + 32 + sizesig + sizesig, size, dv->tag.publickey.data(), sizesig);
		memcpy_s(bd.data + 4 + 2 + 32 + sizesig + sizesig + sizesig, size, dv->tag.issue, sizeof(dv->tag.issue));
		memcpy_s(bd.data + 4 + 2 + 32 + sizesig + sizesig + sizesig + sizeof(dv->tag.issue), size, dv->hashdatacandidate, sizeof(dv->hashdatacandidate));
	}

	static void datacandidate_to_blockdata(DataCandidate* dc, Data& bd) {
		get_blockdata_type(3, bd.type);
		bd.datsize = 0;
		bd.datsize = BLOCK_DATACANDIDATE_BYTES;
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		size_t sizewt = bd.datsize + 4;
		bd.data = new uint8_t[sizewt]{};
		memcpy_s(bd.data, sizewt, bd.type, 4);
		memcpy_s(bd.data + 4, sizewt, dc->signature, sizeof dc->signature);
		memcpy_s(bd.data + 4 + sizeof dc->signature, sizewt, dc->parenthash, sizeof dc->parenthash);
		memcpy_s(bd.data + 4 + sizeof dc->signature + sizeof dc->parenthash, sizewt, dc->hashvotepoll, sizeof dc->hashvotepoll);
		memcpy_s(bd.data + 4 + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll, sizewt, dc->candidatename, sizeof dc->candidatename);
		memcpy_s(bd.data + 4 + sizeof dc->signature + sizeof dc->parenthash + sizeof dc->hashvotepoll + sizeof dc->candidatename, sizewt, dc->publickey, sizeof dc->publickey);
	}

	static void datapartisipan_to_blockdata(DataPartisipan* dp, Data& bd) {
		get_blockdata_type(4, bd.type);
		bd.datsize = 0;
		bd.datsize = BLOCK_DATAPARTISIPAN_BYTES;
		if (bd.data != nullptr) {
			delete[] bd.data;
		}
		size_t sizewt = bd.datsize + 4;
		bd.data = new uint8_t[sizewt]{};
		memcpy_s(bd.data, sizewt, bd.type, 4);
		memcpy_s(bd.data + 4, sizewt, dp->signature, sizeof dp->signature);
		memcpy_s(bd.data + 4 + sizeof dp->signature, sizewt, dp->parenthash, sizeof dp->parenthash);
		memcpy_s(bd.data + 4 + sizeof dp->signature + sizeof dp->parenthash, sizewt, dp->hashvotepoll, sizeof dp->hashvotepoll);
		memcpy_s(bd.data + 4 + sizeof dp->signature + sizeof dp->parenthash + sizeof dp->hashvotepoll, sizewt, dp->publickey, sizeof dp->publickey);
	}

	static void msg_to_blockdata(Message* msg, Data& bd) {
		msg_to_raw_wt(msg, bd.data);
		int sizemsg = 132 + static_cast<int>(msg->cipher->cipherlen) + 28;
		bd.datsize = sizemsg;
		memcpy(bd.type, bd.data, 4);
	}

	static void get_hash_votepoll(VotePoll* vp, uint8_t(&hash)[32]) {
		uint8_t dat[BLOCK_VOTEPOLL_BYTES]{};
		votepoll_to_raw(*&vp, dat);
		crypto_generichash_blake2b(hash, crypto_generichash_blake2b_BYTES, dat, BLOCK_VOTEPOLL_BYTES, NULL, 0);
	}

	static void get_hash_datavote(DataVote* dv, uint8_t(&hash)[32]) {
		uint8_t* raw = nullptr;
		datavote_to_raw(dv, raw);
		long long datasize = Utility::uint8_t_to_int(*&dv->ringsize, 2);
		datasize = (datasize * 96) + 98 + 32;
		crypto_generichash_blake2b(hash, crypto_generichash_blake2b_BYTES, raw, datasize, NULL, 0);
	}

	static void get_hash_datacandidate(DataCandidate* dc, uint8_t(&hash)[32]) {
		uint8_t dat[BLOCK_DATACANDIDATE_BYTES]{};
		datacandidate_to_raw(*&dc, dat);
		crypto_generichash_blake2b(hash, crypto_generichash_blake2b_BYTES, dat, BLOCK_DATACANDIDATE_BYTES, NULL, 0);
	}

	static void get_hash_datapartisipan(DataPartisipan* dp, uint8_t(&hash)[32]) {
		uint8_t dat[BLOCK_DATAPARTISIPAN_BYTES]{};
		datapartisipan_to_raw(*&dp, dat);
		crypto_generichash_blake2b(hash, crypto_generichash_blake2b_BYTES, dat, BLOCK_DATAPARTISIPAN_BYTES, NULL, 0);
	}

	static void get_hash_blockdata(Data* bd, uint8_t(&hash)[32]) {
		crypto_generichash(hash, crypto_generichash_blake2b_BYTES, bd->data + 4, bd->datsize, NULL, 0);
	}

	static void raw_to_blockheader(uint8_t* raw, Header& header) {
		memcpy(header.prevhash, raw, 32);
		memcpy(header.merkleroot, raw + 32, 32);
		memcpy(header.timestamp, raw + 64, 4);
		memcpy(header.nonce, raw + 68, 4);
	}

	static void blockheader_to_raw(Header* header, uint8_t(&raw)[72]) {
		memcpy(raw, header->prevhash, 32);
		memcpy(raw + 32, header->merkleroot, 32);
		memcpy(raw + 64, header->timestamp, 4);
		memcpy(raw + 68, header->nonce, 4);
	}

	static size_t get_block_size(Block* block) {
		size_t size = 72;
		for (int a = 0; a < block->blockdata.size(); a++) {
			size += block->blockdata[a]->datsize;
		}
		return size;
	}

	static size_t get_block_size_wt(Block* block) {
		size_t size = 72;
		for (int a = 0; a < block->blockdata.size(); a++) {
			size += block->blockdata[a]->datsize;
			size += 4;
		}
		return size;
	}
	// size is assumed to alread with magic bytes + 4;
	static void raw_wm_to_block(uint8_t* raw, Block& block, size_t len) {
		if (block.header != nullptr) {
			delete block.header;
		}
		block.header = new Header;
		memcpy(block.header->prevhash, raw + 4, 32);
		memcpy(block.header->merkleroot, raw + 36, 32);
		memcpy(block.header->timestamp, raw + 68, 4);
		memcpy(block.header->nonce, raw + 72, 4);
		size_t pos = 76;
		size_t sizebdwt = 0;
		block.blockdata.reserve(65536);
		while (pos < len) {
			uint8_t bdtype[4]{};
			memcpy(bdtype, raw + pos, 4);
			if (check_blockdata_type(bdtype, 1)) {
				sizebdwt = BLOCK_VOTEPOLL_BYTES;
			}
			else if (check_blockdata_type(bdtype, 2)) {
				uint8_t rs[2]{};
				memcpy(rs, raw + pos + 4, 2);
				long long ringsize = Utility::uint8_t_to_int(rs, 2);
				ringsize *= 96;
				sizebdwt = ringsize + 98;
			}
			else if (check_blockdata_type(bdtype, 3)) {
				sizebdwt = BLOCK_DATACANDIDATE_BYTES;
			}
			else if (check_blockdata_type(bdtype, 4)) {
				sizebdwt = BLOCK_DATAPARTISIPAN_BYTES;
			}
			else if(check_blockdata_type(bdtype, 5)){
				uint8_t as[4]{};
				memcpy(as, raw + pos + 4, 4);
				long long aessize = Utility::uint8_t_to_int(as, 4);
				sizebdwt = aessize + 132;
			}
			else {
				break;
			}
			Data* bd = new Data;
			bd->datsize = sizebdwt;
			sizebdwt += 4;
			memcpy(bd->type, bdtype, 4);
			bd->data = new uint8_t[sizebdwt]{};
			memcpy_s(bd->data, sizebdwt, raw + pos, sizebdwt);
			pos += sizebdwt;
			sizebdwt = 0;
			block.blockdata.push_back(bd);
		}
	}

	static void block_to_raw_wm(Block* block, uint8_t*& raw) {
		if (raw != nullptr) {
			delete[] raw;
		}
		size_t len = get_block_size_wt(*&block);
		len += 4;
		raw = new uint8_t[len]{};
		uint8_t mb[4]{};
		get_magic_bytes(mb);
		memcpy_s(raw, len, mb, 4);
		uint8_t header[72]{};
		blockheader_to_raw(*&block->header, header);
		memcpy_s(raw + 4, len, header, 72);
		int pos = 76;
		for (int a = 0; a < block->blockdata.size(); a++) {
			size_t bdsize = block->blockdata[a]->datsize;
			bdsize += 4;
			memcpy_s(raw + pos, len, block->blockdata[a]->data, bdsize);
			pos += static_cast<int>(bdsize);
		}
	}

	static bool check_signature_votepoll(VotePoll* vp) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, vp->description, sizeof(vp->description));
		crypto_generichash_update(&state, vp->candidatesize, sizeof(vp->candidatesize));
		crypto_generichash_update(&state, vp->partisipansize, sizeof(vp->partisipansize));
		crypto_generichash_update(&state, vp->startdate, sizeof(vp->startdate));
		crypto_generichash_update(&state, vp->enddate, sizeof(vp->enddate));
		crypto_generichash_update(&state, vp->ringsize, sizeof(vp->ringsize));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);

		bool v = ed25519_verify_rust(vp->publickey, hash, crypto_generichash_BYTES, vp->signature);
		return v;
	}

	static bool check_signature_datavote(DataVote* dv) {
		size_t len = Utility::uint8_t_to_int(dv->ringsize, 2);
		len *= 32;
		if (trs_verify(dv->tag.publickey.data(), len, dv->tag.issue, dv->hashdatacandidate, dv->signature.A_1, dv->signature.C_n, dv->signature.Z_n)) {
			return true;
		}
		return false;
	}

	static bool check_signature_datacandidate(DataCandidate* dc, uint8_t* publickey) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, dc->parenthash, sizeof(dc->parenthash));
		crypto_generichash_update(&state, dc->hashvotepoll, sizeof(dc->hashvotepoll));
		crypto_generichash_update(&state, dc->candidatename, sizeof(dc->candidatename));
		crypto_generichash_update(&state, dc->publickey, sizeof(dc->publickey));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		bool v = ed25519_verify_rust(publickey, hash, crypto_generichash_BYTES, dc->signature);
		return v;
	}

	static bool check_signature_datapartisipan(DataPartisipan* dp, uint8_t* publickey) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, dp->parenthash, sizeof(dp->parenthash));
		crypto_generichash_update(&state, dp->hashvotepoll, sizeof(dp->hashvotepoll));
		crypto_generichash_update(&state, dp->publickey, sizeof(dp->publickey));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		bool v = ed25519_verify_rust(publickey, hash, crypto_generichash_BYTES, dp->signature);
		return v;
	}

	static bool check_signature_msg(Message* msg) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, msg->hashvotepoll, 32);
		crypto_generichash_update(&state, msg->cipher->ciphertext, msg->cipher->cipherlen);
		crypto_generichash_update(&state, msg->cipher->tag, sizeof(msg->cipher->tag));
		crypto_generichash_update(&state, msg->cipher->nonce, sizeof(msg->cipher->nonce));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		bool v = ed25519_verify_rust(msg->publickey, hash, crypto_generichash_BYTES, msg->signature);
		return v;
	}

	static void calculate_votepoll_signature(VotePoll* vp, uint8_t(&signature)[64], KeyPair* sk) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, vp->description, sizeof(vp->description));
		crypto_generichash_update(&state, vp->candidatesize, sizeof(vp->candidatesize));
		crypto_generichash_update(&state, vp->partisipansize, sizeof(vp->partisipansize));
		crypto_generichash_update(&state, vp->startdate, sizeof(vp->startdate));
		crypto_generichash_update(&state, vp->enddate, sizeof(vp->enddate));
		crypto_generichash_update(&state, vp->ringsize, sizeof(vp->ringsize));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		ed25519_sign_rust(sk->privatekey, hash, crypto_generichash_BYTES, signature);
	}

	static void calculate_datacandidate_signature(DataCandidate* dc, uint8_t(&signature)[64], KeyPair* sk) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, dc->parenthash, sizeof(dc->parenthash));
		crypto_generichash_update(&state, dc->hashvotepoll, sizeof(dc->hashvotepoll));
		crypto_generichash_update(&state, dc->candidatename, sizeof(dc->candidatename));
		crypto_generichash_update(&state, dc->publickey, sizeof(dc->publickey));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		ed25519_sign_rust(sk->privatekey, hash, crypto_generichash_BYTES, signature);
	}

	static void calculate_datapartisipan_signature(DataPartisipan* dp, uint8_t(&signature)[64], KeyPair* sk) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, dp->parenthash, sizeof(dp->parenthash));
		crypto_generichash_update(&state, dp->hashvotepoll, sizeof(dp->hashvotepoll));
		crypto_generichash_update(&state, dp->publickey, sizeof(dp->publickey));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		ed25519_sign_rust(sk->privatekey, hash, crypto_generichash_BYTES, signature);
	}

	static void calculate_msg_signature(Message* msg, uint8_t(&signature)[64], KeyPair* sk) {
		uint8_t hash[crypto_generichash_BYTES]{};
		crypto_generichash_state state;
		crypto_generichash_init(&state, NULL, 0, crypto_generichash_BYTES);
		crypto_generichash_update(&state, msg->hashvotepoll, 32);
		crypto_generichash_update(&state, msg->cipher->ciphertext, msg->cipher->cipherlen);
		crypto_generichash_update(&state, msg->cipher->tag, sizeof(msg->cipher->tag));
		crypto_generichash_update(&state, msg->cipher->nonce, sizeof(msg->cipher->nonce));
		crypto_generichash_final(&state, hash, crypto_generichash_BYTES);
		ed25519_sign_rust(sk->privatekey, hash, crypto_generichash_BYTES, signature);
	}

	static void calculate_datavote_signature(DataVote* dv, TRS::Signature& sig, KeyPair* sk) {
		dv->signature.ring_size = dv->tag.publickey.size();
		size_t len = dv->signature.ring_size * 32;
		dv->signature.C_n = new uint8_t[len]{};
		dv->signature.Z_n = new uint8_t[len]{};
		sk->calculate_base();
		uint8_t s[64]{};
		memcpy(s, sk->privatekey, 32);
		memcpy(s + 32, sk->publickey, 32);
		trs_sign(dv->tag.publickey.data(), len, s, dv->tag.issue, dv->hashdatacandidate, dv->signature.A_1, dv->signature.C_n, dv->signature.Z_n);
		Utility::int_to_uint8_t(dv->signature.ring_size, dv->ringsize, 2);
	}

	static void get_block_hash(Header* header, uint8_t(&hash)[32]) {
		uint8_t headerwoutnonce[68]{};
		memcpy(headerwoutnonce, header->prevhash, 32);
		memcpy(headerwoutnonce + 32, header->merkleroot, 32);
		memcpy(headerwoutnonce + 32 + 32, header->timestamp, 4);
		crypto_generichash_blake2b(hash, 32, headerwoutnonce, 68, header->nonce, 4);
	}

	static bool block_to_file(Block* block, std::string& path, int newblockheight) {
		std::string newblockpath = path;
		newblockpath += "/";
		newblockpath += "blk";
		if (newblockheight < 10) {
			newblockpath += "000";
			newblockpath += std::to_string(newblockheight);
		}
		else if (newblockheight < 100) {
			newblockpath += "00";
			newblockpath += std::to_string(newblockheight);
		}
		else if (newblockheight < 1000) {
			newblockpath += "0";
			newblockpath += std::to_string(newblockheight);
		}
		newblockpath += ".dat";
		std::fstream write(newblockpath, std::ios::out | std::ios::binary);
		if (!write.is_open()) {
			return false;
		}
		path = newblockpath;
		uint8_t* buffer = nullptr;
		size_t len = get_block_size_wt(*&block);
		block_to_raw_wm(*&block, buffer);
		len += 4;
		write.write((char*)&buffer[0], len);
		write.close();
		delete[] buffer;
		return true;
	}

	static void block_to_blockexplorer(Block* block, Explorer& out) {
		for (int a = 0; a < block->blockdata.size(); a++) {
			uint8_t hash[32]{};
			get_hash_blockdata(*&block->blockdata[a], hash);
			std::string shash = Utility::uint8_t_to_hexstring(hash, 32);
			out.set_hash.insert(shash);
		}
	}
}
#endif