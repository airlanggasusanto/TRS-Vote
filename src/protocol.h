#pragma once
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "block.h"
#include "votetree.h"
#include <QVector>
#include <QDebug>

namespace Protocol {
	struct Data {
		QVector<Block::Header*> header;
	};

	struct Sync {
		long long key;
		int method;
		QVector<Block::Header*> header;
		std::unordered_map<std::string, VoteTree::confirmed*> votetree;
		std::unordered_map<std::string, VoteTree::unconfirmed*> unconf;
		std::unordered_map<std::string, std::string> blockpath;
		QVector<std::string> ordered_blockpath;
		Block::VotePoll genesis_votepoll;
		std::unordered_map<std::string, Block::Explorer*> _block_explorer;
		Sync() {
			key = 0;
			method = 0;
		}
		~Sync() {
			for (auto it = header.begin(); it != header.end(); it++) {
				Block::Header* temp = nullptr;
				std::swap(temp, *it);
				delete temp;
			}
			header.clear();
			for (auto it = votetree.begin(); it != votetree.end(); it++) {
				VoteTree::confirmed* temp = nullptr;
				std::swap(temp, it->second);
				delete temp;
			}
			votetree.clear();
			for (auto it = unconf.begin(); it != unconf.end(); it++) {
				VoteTree::unconfirmed* temp = nullptr;
				std::swap(temp, it->second);
				delete temp;
			}
			unconf.clear();
			for (auto it = _block_explorer.begin(); it != _block_explorer.end(); it++) {
				Block::Explorer* temp = nullptr;
				std::swap(temp, it->second);
				delete temp;
			}
			_block_explorer.clear();
			blockpath.clear();
			ordered_blockpath.clear();
		}
	};

	static void get_protocol_type(int a, uint8_t(&out)[4]) {
		char __type0[8 + 1] = { '1','1','0','F','6','C','6','C','\0' };
		char __type1[8 + 1] = { '1','1','1','F','7','4','6','5','\0' };
		char __type2[8 + 1] = { '1','1','2','E','7','3','7','7','\0' };
		char __type3[8 + 1] = { '1','1','3','5','6','D','6','2','\0' };
		char __type4[8 + 1] = { '1','1','4','E','7','3','7','7','\0' };
		char __type5[8 + 1] = { '1','1','5','5','6','D','6','2','\0' };
		if (a == 1) {
			sodium_hex2bin(out, 4, __type0, 9, NULL, NULL, NULL);
		}
		else if (a == 2) {
			sodium_hex2bin(out, 4, __type1, 9, NULL, NULL, NULL);
		}
		else if (a == 3) {
			sodium_hex2bin(out, 4, __type2, 9, NULL, NULL, NULL);
		}
		else if (a == 4) {
			sodium_hex2bin(out, 4, __type3, 9, NULL, NULL, NULL);
		}
		else if (a == 5) {
			sodium_hex2bin(out, 4, __type4, 9, NULL, NULL, NULL);
		}
		else if (a == 6) {
			sodium_hex2bin(out, 4, __type5, 9, NULL, NULL, NULL);
		}
	}

	static bool check_protocol_type(int m, uint8_t* type) {
		uint8_t datatype[4]{};
		get_protocol_type(m, datatype);
		if (sodium_memcmp(datatype, type, 4) == 0) {
			return true;
		}
		return false;
	}

	static bool compare_header(QVector<Block::Header*>& a, QVector<Block::Header*>& b, QByteArray* sendthis, int& index) {
		if (a.size() >= b.size()) {
			return false;
		}
		if (a.isEmpty()) {
			uint8_t th[32]{};
			Block::get_block_hash(b.front(), th);
			*sendthis = QByteArray((char*)&th[0], 32);
			index = 0;
			return true;
		}
		for (int i = 0; i < a.size(); i++) {
			uint8_t ah[32]{};
			uint8_t bh[32]{};
			Block::get_block_hash(a[i], ah);
			Block::get_block_hash(b[i], bh);
			if (sodium_memcmp(ah, bh, 32) != 0) {
				*sendthis = QByteArray((char*)&bh[0], 32);
				index = i;
				return true;
			}
		}
		uint8_t lh[32]{};
		Block::get_block_hash(b[a.size()], lh);
		*sendthis = QByteArray((char*)&lh[0], 32);
		index = a.size();
		return true;
	}

	static void copy_header(QVector<Block::Header*>& in, QVector<Block::Header*>& out) {
		out.clear();
		out.resize(in.size());
		for (int a = 0; a < in.size(); a++) {
			out[a] = new Block::Header;
			memcpy(out[a], in[a], sizeof(Block::Header));
		}
	}

}
#endif