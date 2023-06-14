#ifndef VOTETREE_H
#define VOTETREE_H
#pragma once
#include "block.h"
#include "util.h"
#include "qutil.h"

namespace VoteTree {
	struct QuickCount {
		int count{ 0 };
		Block::DataCandidate data;
	};

	struct VoteProof {
		std::unordered_map<std::string, TRS::Signature*> sig;
		std::unordered_map<std::string, int> candidate_index;

		VoteProof() {

		}

		~VoteProof() {
			for (auto& pair : sig) {
				delete pair.second;
			}
			sig.clear();
			candidate_index.clear();
		}
	};

	static std::string get_hash_hex_tag(TRS::Tag* tag) {
		uint8_t hash[32]{};
		crypto_generichash_blake2b_state state;
		crypto_generichash_blake2b_init(&state, NULL, 0, 32);
		crypto_generichash_blake2b_update(&state, tag->issue, 32);
		for (int a = 0; a < tag->publickey.size(); a++) {
			crypto_generichash_blake2b_update(&state, tag->publickey[a].data, 32);
		}
		crypto_generichash_blake2b_final(&state, hash, 32);
		std::string string = Utility::uint8_t_to_hexstring(hash, 32);
		return string;
	}

	struct confirmed {
		Block::VotePoll* votepoll;
		std::vector<QuickCount> tally;
		std::unordered_map<std::string, int> set_pk_candidate;
		std::unordered_map<std::string, VoteProof*> proof_of_vote;
		std::unordered_map<std::string, int> set_pk_partisipan;
		std::vector<TRS::Size32> ordered_pk;
		std::vector<TRS::Size32> hash_data_partisipan;
		std::unordered_map<std::string, int> register_pk;
		std::vector<std::string> hash_pkr;
		long long wtimestamp{ 0 };

		confirmed() {
			votepoll = nullptr;
		}

		confirmed(Block::VotePoll* vp) {
			votepoll = new Block::VotePoll;
			memcpy(votepoll, vp, sizeof(Block::VotePoll));
		}

		~confirmed() {
			if (votepoll != nullptr) {
				delete votepoll;
			}
			tally.clear();
			set_pk_candidate.clear();
			proof_of_vote.clear();
			set_pk_partisipan.clear();
			hash_data_partisipan.clear();
			ordered_pk.clear();
			register_pk.clear();
			hash_pkr.clear();
		}
		void get_tag(TRS::Tag& tag, int index) {
			int totalsize = set_pk_partisipan.size();
			int ringsize = Utility::uint8_t_to_int(votepoll->ringsize, 2);
			int totalring = totalsize / ringsize;
			if (totalring == 0) {
				totalring = 1;
			}
			int ring_pos = index % totalring;
			while (ring_pos < totalsize) {
				TRS::Size32 pk;
				memcpy(pk.data, ordered_pk[ring_pos].data, 32);
				tag.publickey.push_back(pk);
				ring_pos += totalring;
			}
			Block::get_hash_votepoll(votepoll, tag.issue);
		}

		int get_index_candidate(uint8_t* hdc) {
			for (int a = 0; a < tally.size(); a++) {
				uint8_t th[32]{};
				Block::get_hash_datacandidate(&tally[a].data, th);
				if (sodium_memcmp(th, hdc, 32) == 0) {
					return a;
				}
			}
			return -1;
		}

		bool check_proof(int index, Block::DataVote* dv) {
			TRS::Tag tag;
			get_tag(tag, index);
			std::string t_h = get_hash_hex_tag(&tag);
			std::string d_h = get_hash_hex_tag(&dv->tag);
			if (t_h != d_h) {
				return false;
			}
			int indexc = get_index_candidate(dv->hashdatacandidate);
			if (indexc == -1) {
				return false;
			}
			auto find = proof_of_vote.find(d_h);
			if (find == proof_of_vote.end()) {
				return true;
			}
			std::string my_a_1 = Utility::uint8_t_to_hexstring(dv->signature.A_1, 32);
			auto check_a_1 = find->second->sig.find(my_a_1);
			if (check_a_1 != find->second->sig.end()) {
				return false;
			}
			for (auto& pair : find->second->sig) {
				long long size = static_cast<long long>(pair.second->ring_size) * 32;
				std::string a_1 = Utility::uint8_t_to_hexstring(pair.second->A_1, 32);
				auto find_index = find->second->candidate_index.find(a_1);
				if (find_index == find->second->candidate_index.end()) {
					continue;
				}
				uint8_t msg[32]{};
				int hashcandidate = find_index->second;
				Block::get_hash_datacandidate(&tally[hashcandidate].data, msg);
				int line = trs_trace(tag.publickey.data(), size, tag.issue, msg, dv->hashdatacandidate, pair.second->A_1, dv->signature.A_1, pair.second->C_n, dv->signature.C_n, pair.second->Z_n, dv->signature.Z_n);
				if (line != -1) {
					return false;
				}
			}
			return true;
		}

		void insert_proof(Block::DataVote* dv) {
			int indexc = get_index_candidate(dv->hashdatacandidate);
			std::string hasht = get_hash_hex_tag(&dv->tag);
			std::string a_1 = Utility::uint8_t_to_hexstring(dv->signature.A_1, 32);
			if (tally.size() >= indexc) {
				tally[indexc].count += 1;
			}
			if (auto find = proof_of_vote.find(hasht); find != proof_of_vote.end()) {
				TRS::Signature* newsig = new TRS::Signature;
				TRS::Copy_Signature(&dv->signature, *newsig);
				find->second->sig.emplace(a_1, newsig);
				find->second->candidate_index.emplace(a_1, indexc);
			}
			else {
				VoteProof* newproof = new VoteProof;
				TRS::Signature* newsig = new TRS::Signature;
				TRS::Copy_Signature(&dv->signature, *newsig);
				newproof->sig.emplace(a_1, newsig);
				newproof->candidate_index.emplace(a_1, indexc);
				proof_of_vote.emplace(hasht, newproof);
			}
		}
	};

	struct unconfirmed {
		std::unordered_set<std::string> set_pk_candidate;
		std::unordered_set<std::string> set_pk_partisipan;
		std::unordered_map<std::string, VoteProof*> proof_of_vote;
		uint8_t lasthash_candidate[32]{};
		uint8_t lasthash_partisipan[32]{};
		std::unordered_set<std::string> register_pk;

		unconfirmed() {
		}

		unconfirmed(Block::VotePoll* vp) {
			Block::get_hash_votepoll(*&vp, lasthash_candidate);
			Block::get_hash_votepoll(*&vp, lasthash_partisipan);
		}

		unconfirmed(confirmed* in) {
			if (!in->tally.empty()) {
				Block::get_hash_datacandidate(&in->tally.back().data, lasthash_candidate);
			}
			else {
				Block::get_hash_votepoll(in->votepoll, lasthash_candidate);
			}
			if (!in->hash_data_partisipan.empty()) {
				memcpy(lasthash_partisipan, in->hash_data_partisipan.back().data, 32);
			}
			else {
				Block::get_hash_votepoll(in->votepoll, lasthash_partisipan);
			}
		}

		~unconfirmed() {
			set_pk_candidate.clear();
			set_pk_partisipan.clear();
			proof_of_vote.clear();
			register_pk.clear();
		}

		bool check_proof(confirmed* vt, int index, Block::DataVote* dv) {
			TRS::Tag tag;
			vt->get_tag(tag, index);
			std::string t_h = get_hash_hex_tag(&tag);
			std::string d_h = get_hash_hex_tag(&dv->tag);
			if (t_h != d_h) {
				return false;
			}
			int indexc = vt->get_index_candidate(dv->hashdatacandidate);
			if (indexc == -1) {
				return false;
			}
			auto find = proof_of_vote.find(d_h);
			if (find == proof_of_vote.end()) {
				return true;
			}
			std::string my_a_1 = Utility::uint8_t_to_hexstring(dv->signature.A_1, 32);
			auto check_a_1 = find->second->sig.find(my_a_1);
			if (check_a_1 != find->second->sig.end()) {
				return false;
			}
			for (auto& pair : find->second->sig) {
				long long size = static_cast<long long>(pair.second->ring_size) * 32;
				std::string a_1 = Utility::uint8_t_to_hexstring(pair.second->A_1, 32);
				auto find_index = find->second->candidate_index.find(a_1);
				if (find_index == find->second->candidate_index.end()) {
					continue;
				}
				uint8_t msg[32]{};
				int hashcandidate = find_index->second;
				Block::get_hash_datacandidate(&vt->tally[hashcandidate].data, msg);
				int line = trs_trace(tag.publickey.data(), size, tag.issue, msg, dv->hashdatacandidate, pair.second->A_1, dv->signature.A_1, pair.second->C_n, dv->signature.C_n, pair.second->Z_n, dv->signature.Z_n);
				if (line != -1) {
					return false;
				}
			}
			return true;
		}

		void insert_proof(Block::DataVote* dv, confirmed* vt) {
			int indexc = vt->get_index_candidate(dv->hashdatacandidate);
			std::string hasht = get_hash_hex_tag(&dv->tag);
			std::string a_1 = Utility::uint8_t_to_hexstring(dv->signature.A_1, 32);
			if (auto find = proof_of_vote.find(hasht); find != proof_of_vote.end()) {
				TRS::Signature* newsig = new TRS::Signature;
				TRS::Copy_Signature(&dv->signature, *newsig);
				find->second->sig.emplace(a_1, newsig);
				find->second->candidate_index.emplace(a_1, indexc);
			}
			else {
				VoteProof* newproof = new VoteProof;
				TRS::Signature* newsig = new TRS::Signature;
				TRS::Copy_Signature(&dv->signature, *newsig);
				newproof->sig.emplace(a_1, newsig);
				newproof->candidate_index.emplace(a_1, indexc);
				proof_of_vote.emplace(hasht, newproof);
			}
		}

		void delete_proof(Block::DataVote* dv) {
			std::string hasht = get_hash_hex_tag(&dv->tag);
			std::string a_1 = Utility::uint8_t_to_hexstring(dv->signature.A_1, 32);
			if (auto find = proof_of_vote.find(hasht); find != proof_of_vote.end()) {
				auto sig = find->second->sig.find(a_1);
				if (sig != find->second->sig.end()) {
					delete sig->second;
					find->second->sig.erase(a_1);
				}
				find->second->candidate_index.erase(a_1);
			}
		}
	};

	struct history {
		std::unordered_map<std::string, QByteArray> ciphermsg;
		std::unordered_map<std::string, long long> datemsg;
		std::unordered_map<std::string, long long> updatedatemsgaccepted;
		history() {
		}

		~history() {
			ciphermsg.clear();
			datemsg.clear();
		}
	};

	struct VotepollString {
		QString hash;
		QString sig;
		QString desc;
		QString pk;
		long long startdate;
		long long enddate;
		int candidatsize;
		int partisipansize;
		int ringsize;
		long long blocktime;
		int currentvote;
		VotepollString() {
			candidatsize = 0;
			partisipansize = 0;
			ringsize = 0;
			blocktime = 0;
			currentvote = 0;
			startdate = 0;
			enddate = 0;
		}
		~VotepollString() {

		}
	};

	struct DataCandidateString {
		QString hash;
		QString name;
		QString pk;
		long long blocktime;
		long long totalvote;
		DataCandidateString() {
			blocktime = 0;
			totalvote = 0;
		}
		~DataCandidateString() {

		}
	};

	struct DataPartisipanString {
		QString hash;
		QString pk;
		long long blocktime;
		DataPartisipanString() {
			blocktime = 0;
		}
		~DataPartisipanString() {

		}
	};

	struct MessageCipher {
		QByteArray cipher;
		long long blocktime;
		QString pk;
		MessageCipher() {
			blocktime = 0;
		}
		~MessageCipher() {

		}
	};

	struct VoteString {
		std::unordered_map<std::string, int> vote_a1;
		std::unordered_map<std::string, long long> block_time;
		VoteString() {

		}
		~VoteString() {
			vote_a1.clear();
			block_time.clear();
		}
	};

	struct display {
		VotepollString vp;
		QVector<DataPartisipanString> dp;
		std::unordered_map<std::string, int> dp_index;
		QVector<DataCandidateString> dc;
		std::unordered_map<std::string, int> dc_index;
		QVector<MessageCipher> msg;
		std::unordered_map<std::string, int> msg_index;
		QVector<VoteString> vote;
		std::unordered_map<std::string, int> vote_index;
		display() {

		}

		display(Block::VotePoll* vptemp) {
			uint8_t hash[32]{};
			Block::get_hash_votepoll(vptemp, hash);
			vp.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
			vp.desc = Utility::uint8_t_to_ascii(vptemp->description, 64).c_str();
			vp.pk = Utility::uint8_t_to_hexstring(vptemp->publickey, 32).c_str();
			vp.candidatsize = Utility::uint8_t_to_int(vptemp->candidatesize, 4);
			vp.partisipansize = Utility::uint8_t_to_int(vptemp->partisipansize, 4);
			vp.ringsize = Utility::uint8_t_to_int(vptemp->ringsize, 2);
			vp.startdate = Utility::uint8_t_to_int(vptemp->startdate, 4);
			vp.enddate = Utility::uint8_t_to_int(vptemp->enddate, 4);
			vp.sig = Utility::uint8_t_to_hexstring(vptemp->signature, 64).c_str();
		}

		display(confirmed* in) {
			uint8_t hash[32]{};
			Block::get_hash_votepoll(in->votepoll, hash);
			vp.hash = Utility::uint8_t_to_hexstring(hash, 32).c_str();
			vp.desc = Utility::uint8_t_to_ascii(in->votepoll->description, 64).c_str();
			vp.pk = Utility::uint8_t_to_hexstring(in->votepoll->publickey, 32).c_str();
			vp.candidatsize = Utility::uint8_t_to_int(in->votepoll->candidatesize, 4);
			vp.partisipansize = Utility::uint8_t_to_int(in->votepoll->partisipansize, 4);
			vp.ringsize = Utility::uint8_t_to_int(in->votepoll->ringsize, 2);
			vp.startdate = Utility::uint8_t_to_int(in->votepoll->startdate, 4);
			vp.enddate = Utility::uint8_t_to_int(in->votepoll->enddate, 4);
			vp.blocktime = in->wtimestamp;
			vp.sig = Utility::uint8_t_to_hexstring(in->votepoll->signature, 64).c_str();
		}

		~display() {
			dp.clear();
			dc.clear();
			msg.clear();
			dp_index.clear();
			dc_index.clear();
			msg_index.clear();
		}

		bool get_tag(const std::string& pk, TRS::Tag& tag) {
			auto find_i = dp_index.find(pk);
			if (find_i == dp_index.end()) {
				return false;
			}
			int index = find_i->second;
			int totalsize = dp.size();
			int ringsize = vp.ringsize;
			int totalring = totalsize / ringsize;
			if (totalring == 0) {
				totalring = 1;
			}
			int ring_pos = index % totalring;
			while (ring_pos < totalsize) {
				TRS::Size32 pk;
				Utility::hexstring_to_uint8_t(pk.data, 32, dp[ring_pos].pk.toStdString());
				tag.publickey.push_back(pk);
				ring_pos += totalring;
			}
			Utility::hexstring_to_uint8_t(tag.issue, 32, vp.hash.toStdString());
			return true;
		}
	};

	static bool validate_blockdata_to_votetree(Block::Data* blkdatatemp, long long timestamp, std::unordered_map<std::string, confirmed*>& vtree, std::unordered_map<std::string, unconfirmed*>& uvtree, Block::VotePoll& genesis_votepoll) {
		if (Block::check_blockdata_type(blkdatatemp->type, 1)) {
			Block::VotePoll newvotepoll;
			Block::blockdata_to_votepoll(*&blkdatatemp, newvotepoll);
			if (!Block::check_signature_votepoll(&newvotepoll)) {
				return false;
			}
			if (vtree.empty()) {
				uint8_t zero[32]{};
				std::string genesisvtreekey = Utility::uint8_t_to_hexstring(zero, 32);
				confirmed* temp = nullptr;
				vtree.emplace(genesisvtreekey, temp);
				genesis_votepoll = newvotepoll;
				return true;
			}
			if (sodium_memcmp(newvotepoll.publickey, genesis_votepoll.publickey, 32) == -1) {
				return false;
			}
			long long startdate = Utility::uint8_t_to_int(newvotepoll.startdate, 4);
			long long enddate = Utility::uint8_t_to_int(newvotepoll.enddate, 4);
			if (startdate < timestamp && startdate > enddate) {
				return false;
			}
			uint8_t hash[32]{};
			Block::get_hash_votepoll(&newvotepoll, hash);
			std::string hvp = Utility::uint8_t_to_hexstring(hash, 32);
			if (!vtree.empty()) {
				if (auto find_vp = vtree.find(hvp); find_vp != vtree.end()) {
					return false;
				}
			}
			if (!uvtree.empty()) {
				if (auto find_vp = uvtree.find(hvp); find_vp != uvtree.end()) {
					return false;
				}
			}
			uvtree.emplace(hvp, new unconfirmed(&newvotepoll));
			return true;
		}
		else if (Block::check_blockdata_type(blkdatatemp->type, 2)) {
			Block::DataVote dv;
			Block::blockdata_to_datavote(*&blkdatatemp, dv);
			if (!Block::check_signature_datavote(&dv)) {
				return false;
			}
			std::string hvp = Utility::uint8_t_to_hexstring(dv.tag.issue, 32);
			if (auto find_vp = vtree.find(hvp); find_vp != vtree.end()) {
				long long startdate = Utility::uint8_t_to_int(find_vp->second->votepoll->startdate, 4);
				long long enddate = Utility::uint8_t_to_int(find_vp->second->votepoll->enddate, 4);
				if (timestamp < startdate || enddate < timestamp) {
					return false;
				}
				int index = 0;
				std::string onepk = Utility::uint8_t_to_hexstring(dv.tag.publickey[0].data, 32);
				if (auto find_pk = find_vp->second->set_pk_partisipan.find(onepk); find_pk == find_vp->second->set_pk_partisipan.end()) {
					return false;
				}
				else {
					index = find_pk->second;
				}
				if (!find_vp->second->check_proof(index, &dv)) {
					return false;
				}
				if (auto find_vtree = uvtree.find(hvp); find_vtree != uvtree.end()) {
					if (find_vtree->second->check_proof(find_vp->second, index, &dv)) {
						find_vtree->second->insert_proof(&dv, find_vp->second);
						return true;
					}
				}
			}
			return false;
		}
		else if (Block::check_blockdata_type(blkdatatemp->type, 3)) {
			Block::DataCandidate dc;
			Block::blockdata_to_datacandidate(*&blkdatatemp, dc);
			std::string hvp = Utility::uint8_t_to_hexstring(dc.hashvotepoll, 32);
			if (auto find_vp = vtree.find(hvp); find_vp != vtree.end()) {
				long long maxa = static_cast<long long>(find_vp->second->set_pk_candidate.size());
				long long maxb = Utility::uint8_t_to_int(find_vp->second->votepoll->candidatesize, 4);
				if (!Block::check_signature_datacandidate(&dc, find_vp->second->votepoll->publickey)) {
					return false;
				}

				long long startdate = Utility::uint8_t_to_int(find_vp->second->votepoll->startdate, 4);
				if (startdate < timestamp) {
					return false;
				}
				std::string spk = Utility::uint8_t_to_hexstring(dc.publickey, 32);
				if (auto find_pk = find_vp->second->set_pk_candidate.find(spk); find_pk != find_vp->second->set_pk_candidate.end()) {
					return false;
				}
				if (auto find_vtree = uvtree.find(hvp); find_vtree != uvtree.end()) {
					maxa += static_cast<long long>(find_vtree->second->set_pk_candidate.size());
					if (auto find_pk = find_vtree->second->set_pk_candidate.find(spk); find_pk != find_vtree->second->set_pk_candidate.end()) {
						return false;
					}
					if (sodium_memcmp(find_vtree->second->lasthash_candidate, dc.parenthash, 32)) {
						return false;
					}
					if (maxa >= maxb) {
						return false;
					}
					uint8_t hash[32]{};
					Block::get_hash_datacandidate(&dc, hash);
					find_vtree->second->set_pk_candidate.insert(spk);
					memcpy(find_vtree->second->lasthash_candidate, hash, 32);
					return true;
				}
				return false;
			}
		}
		else if (Block::check_blockdata_type(blkdatatemp->type, 4)) {
			Block::DataPartisipan dp;
			Block::blockdata_to_datapartisipan(*&blkdatatemp, dp);
			std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
			if (auto find_vp = vtree.find(hvp); find_vp != vtree.end()) {
				long long maxpartisipan = Utility::uint8_t_to_int(find_vp->second->votepoll->partisipansize, 4);
				long long currentpartisipan = find_vp->second->set_pk_partisipan.size();
				if (!Block::check_signature_datapartisipan(&dp, find_vp->second->votepoll->publickey)) {
					return false;
				}
				long long startdate = Utility::uint8_t_to_int(find_vp->second->votepoll->startdate, 4);
				if (startdate < timestamp) {
					return false;
				}
				std::string spk = Utility::uint8_t_to_hexstring(dp.publickey, 32);
				if (auto find_pk = find_vp->second->set_pk_partisipan.find(spk); find_pk != find_vp->second->set_pk_partisipan.end()) {
					return false;
				}
				if (auto find_vtree = uvtree.find(hvp); find_vtree != uvtree.end()) {
					currentpartisipan += static_cast<long long>(find_vtree->second->set_pk_partisipan.size());
					if (auto find_pk = find_vtree->second->set_pk_partisipan.find(spk); find_pk != find_vtree->second->set_pk_partisipan.end()) {
						return false;
					}
					if (sodium_memcmp(find_vtree->second->lasthash_partisipan, dp.parenthash, 32)) {
						return false;
					}
					if (currentpartisipan >= maxpartisipan) {
						return false;
					}
					uint8_t hash[32]{};
					Block::get_hash_datapartisipan(&dp, hash);
					find_vtree->second->set_pk_partisipan.insert(spk);
					memcpy(find_vtree->second->lasthash_partisipan, hash, 32);
					return true;
				}
				return false;
			}
		}
		else if (Block::check_blockdata_type(blkdatatemp->type, 5)) {
			Block::Message msg;
			Block::blockdata_to_msg(*&blkdatatemp, msg);
			if (!Block::check_signature_msg(&msg)) {
				return false;
			}
			std::string pk = Utility::uint8_t_to_hexstring(msg.publickey, 32);
			std::string hvp = Utility::uint8_t_to_hexstring(msg.hashvotepoll, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				if (find->second->register_pk.find(pk) != find->second->register_pk.end()) {
					return false;
				}
				if (auto finduv = uvtree.find(hvp); finduv != uvtree.end()) {
					if (finduv->second->register_pk.find(pk) != finduv->second->register_pk.end()) {
						return false;
					}
					finduv->second->register_pk.emplace(pk);
					return true;
				}
			}
			return false;
		}
		return false;
	}

	static void insert_blockdata_to_votetree(Block::Data* blk, long long timestamp, std::unordered_map<std::string, confirmed*>& vtree, std::unordered_map<std::string, unconfirmed*>& uvtree, Block::VotePoll& genesisblock) {
		if (Block::check_blockdata_type(blk->type, 1)) {
			Block::VotePoll newvotepoll;
			Block::blockdata_to_votepoll(*&blk, newvotepoll);
			if (sodium_memcmp(&newvotepoll, &genesisblock, sizeof(Block::VotePoll)) == 0) {
				return;
			}
			uint8_t hash[32]{};
			Block::get_hash_votepoll(&newvotepoll, hash);
			std::string hvp = Utility::uint8_t_to_hexstring(hash, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				return;
			}
			VoteTree::confirmed* newvtree = new VoteTree::confirmed(&newvotepoll);
			newvtree->wtimestamp = timestamp;
			vtree.emplace(hvp, newvtree);
			if (auto find = uvtree.find(hvp); find != uvtree.end()) {
				delete find->second;
				find->second = new VoteTree::unconfirmed(newvtree);
			}
			else {
				VoteTree::unconfirmed* newuvtree = new VoteTree::unconfirmed(&newvotepoll);
				uvtree.emplace(hvp, newuvtree);
			}
			return;
		}
		else if (Block::check_blockdata_type(blk->type, 2)) {
			Block::DataVote dv;
			Block::blockdata_to_datavote(*&blk, dv);
			std::string hvp = Utility::uint8_t_to_hexstring(dv.tag.issue, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				find->second->insert_proof(&dv);
				if (auto uv = uvtree.find(hvp); uv != uvtree.end()) {
					uv->second->delete_proof(&dv);
				}
			}
			return;
		}
		else if (Block::check_blockdata_type(blk->type, 3)) {
			VoteTree::QuickCount dc;
			Block::blockdata_to_datacandidate(*&blk, dc.data);
			std::string hvp = Utility::uint8_t_to_hexstring(dc.data.hashvotepoll, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				std::string pk_c = Utility::uint8_t_to_hexstring(dc.data.publickey, 32);
				find->second->tally.push_back(dc);
				int index = find->second->tally.size() - 1;
				find->second->set_pk_candidate.emplace(pk_c, index);
				if (auto uv = uvtree.find(hvp); uv != uvtree.end()) {
					Block::get_hash_datacandidate(&dc.data, uv->second->lasthash_candidate);
					uv->second->set_pk_candidate.erase(pk_c);
				}
			}
			return;
		}
		else if (Block::check_blockdata_type(blk->type, 4)) {
			Block::DataPartisipan dp;
			Block::blockdata_to_datapartisipan(*&blk, dp);
			std::string hvp = Utility::uint8_t_to_hexstring(dp.hashvotepoll, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				std::string pk_p = Utility::uint8_t_to_hexstring(dp.publickey, 32);
				TRS::Size32 hashdp;
				TRS::Size32 pk_trs;
				memcpy(pk_trs.data, dp.publickey, 32);
				Block::get_hash_datapartisipan(&dp, hashdp.data);
				find->second->hash_data_partisipan.push_back(hashdp);
				int index = find->second->hash_data_partisipan.size() - 1;
				find->second->ordered_pk.push_back(pk_trs);
				find->second->set_pk_partisipan.emplace(pk_p, index);
				if (auto uv = uvtree.find(hvp); uv != uvtree.end()) {
					memcpy(uv->second->lasthash_partisipan, hashdp.data, 32);
					uv->second->set_pk_partisipan.erase(pk_p);
				}
			}
			return;
		}
		else if (Block::check_blockdata_type(blk->type, 5)) {
			Block::Message msg;
			Block::blockdata_to_msg(*&blk, msg);
			std::string hvp = Utility::uint8_t_to_hexstring(msg.hashvotepoll, 32);
			if (auto find = vtree.find(hvp); find != vtree.end()) {
				std::string pk = Utility::uint8_t_to_hexstring(msg.publickey, 32);
				uint8_t hashmsg[32]{};
				Block::get_hash_blockdata(*&blk, hashmsg);
				std::string hash = Utility::uint8_t_to_hexstring(hashmsg, 32);
				find->second->hash_pkr.push_back(hash);
				int index = find->second->hash_pkr.size();
				index--;
				find->second->register_pk.emplace(pk, index);
				if (auto uv = uvtree.find(hvp); uv != uvtree.end()) {
					uv->second->register_pk.erase(pk);
				}
			}
		}
	}

	static void remove_blockdata_to_votetree() {

	}
}

#endif