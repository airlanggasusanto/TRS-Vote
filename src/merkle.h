#pragma once
#ifndef MERKLE_H
#define MERKLE_H
#include <vector>
#include "sodium.h"

namespace Merkle {
	struct Hash {
		uint8_t data[32]{};
		Hash* left;
		Hash* right;
		Hash() {
			left = nullptr;
			right = nullptr;
		}
		~Hash() {
			left = nullptr;
			right = nullptr;
		}
	};

	struct Tree {
		Hash* root;
		std::vector<std::vector<Hash*>> merklehash;
		int current;
		Tree() {
			root = nullptr;
			current = 0;
			merklehash.resize(16);
			int a = 65536;
			for (int b = 0; b < merklehash.size(); b++) {
				merklehash[b].resize(a);
				a = a / 2;
			}
		}
		~Tree() {
			for (int a = 0; a < 15; a++) {
				for (int b = 0; b < merklehash[a].size(); b++) {
					if (merklehash[a][b] != nullptr) {
						delete merklehash[a][b];
					}
				}
				merklehash[a].clear();
			}merklehash.clear();
		}
	};

	static void calculate_hash(Hash& hash, Hash* tleft, Hash* tright) {
		hash.left = tleft;
		hash.right = tright;
		if (tleft == nullptr) {
			sodium_memzero(hash.data, 32);
			return;
		}
		if (tright != nullptr) {
			crypto_generichash_state state;

			crypto_generichash_init(&state, NULL, 0, crypto_generichash_blake2b_BYTES);

			crypto_generichash_update(&state, hash.left->data, crypto_generichash_blake2b_BYTES);
			crypto_generichash_update(&state, hash.right->data, crypto_generichash_blake2b_BYTES);

			crypto_generichash_final(&state, hash.data, crypto_generichash_blake2b_BYTES);
		}
		else {
			memcpy(hash.data, hash.left, 32);
		}
	}

	static bool insert_tree(Tree& tree, Hash* newhash) {
		if (tree.current == 65536) {
			return false;
		}
		tree.merklehash[0][tree.current] = new Hash;
		memcpy(tree.merklehash[0][tree.current]->data, newhash->data, 32);
		int prev = tree.current;
		int bb = 0;
		int a = tree.current / 2;
		int c = tree.current % 2;
		for (int b = 1; b < 16; b++) {
			if (tree.merklehash[b][a] == nullptr) {
				if (c == 1) {
					int tprev = prev - 1;
					tree.merklehash[b][a] = new Hash;
					calculate_hash(*tree.merklehash[b][a], tree.merklehash[bb][tprev], tree.merklehash[bb][prev]);
				}
				else {
					int tprev = prev + 1;
					tree.merklehash[b][a] = new Hash;
					calculate_hash(*tree.merklehash[b][a], tree.merklehash[bb][prev], tree.merklehash[bb][tprev]);
				}
			}
			else {
				delete tree.merklehash[b][a];
				tree.merklehash[b][a] = nullptr;
				if (c == 1) {
					int tprev = prev - 1;
					tree.merklehash[b][a] = new Hash;
					calculate_hash(*tree.merklehash[b][a], tree.merklehash[bb][tprev], tree.merklehash[bb][prev]);
				}
				else {
					int tprev = prev + 1;
					tree.merklehash[b][a] = new Hash;
					calculate_hash(*tree.merklehash[b][a], tree.merklehash[bb][prev], tree.merklehash[bb][tprev]);
				}
			}
			c = a;
			if (a >= 2) {
				c = a % 2;
			}
			prev = a;
			a = a / 2;
			bb = b;

		}
		tree.current++;
		return true;
	}

	static bool verify_tree(Tree& tree, uint8_t* hash) {
		if (tree.current == 0) {
			return false;
		}
		if (tree.root != nullptr) {
			delete tree.root;
			tree.root = nullptr;
		}
		tree.root = new Hash;
		calculate_hash(*tree.root, tree.merklehash[15][0], tree.merklehash[15][1]);
		if (sodium_memcmp(tree.root->data, hash, 32) == 0) {
			return true;
		}
		return false;
	}
}

#endif