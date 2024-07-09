/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ZILLIQA_SRC_LIBCONSENSUS_CONSENSUSBACKUP_H_
#define ZILLIQA_SRC_LIBCONSENSUS_CONSENSUSBACKUP_H_

#include "ConsensusCommon.h"

typedef std::function<bool(const zbytes& input, unsigned int offset,
                           zbytes& errorMsg, const uint32_t consensusID,
                           const uint64_t blockNumber, const zbytes& blockHash,
                           const uint16_t leaderID, const PubKey& leaderKey,
                           zbytes& messageToCosign)>
    MsgContentValidatorFunc;

typedef std::function<bool()> CollectiveSigReadinessFunc;
typedef std::function<void()> PostPrePrepValidationFunc;

/// Implements the functionality for the consensus committee backup.
class ConsensusBackup : public ConsensusCommon {
 private:
  enum Action {
    PROCESS_ANNOUNCE = 0x00,
    PROCESS_CHALLENGE,
    PROCESS_COLLECTIVESIG,
    PROCESS_FINALCHALLENGE,
    PROCESS_FINALCOLLECTIVESIG
  };

  // Consensus session settings
  uint16_t m_leaderID;

  // Function handler for validating message content
  MsgContentValidatorFunc m_msgContentValidator;
  MsgContentValidatorFunc m_prePrepMsgContentValidator;
  PostPrePrepValidationFunc m_postPrePrepContentValidation;
  CollectiveSigReadinessFunc m_readinessFunc;

  // Internal functions
  bool CheckState(Action action);

  bool ProcessMessageAnnounce(const zbytes& announcement, unsigned int offset);
  bool GenerateCommitFailureMessage(zbytes& commitFailure, unsigned int offset,
                                    const zbytes& errorMsg);
  bool ProcessMessageConsensusFailure(
      [[gnu::unused]] const zbytes& announcement,
      [[gnu::unused]] unsigned int offset);
  bool GenerateCommitMessage(zbytes& commit, unsigned int offset);
  bool ProcessMessageChallengeCore(const zbytes& challenge, unsigned int offset,
                                   Action action,
                                   ConsensusMessageType returnmsgtype,
                                   State nextstate, std::string_view spanName);
  bool ProcessMessageChallenge(const zbytes& challenge, unsigned int offset);
  bool GenerateResponseMessage(
      zbytes& response, unsigned int offset,
      const std::vector<ResponseSubsetInfo>& subsetInfo);
  bool ProcessMessageCollectiveSigCore(const zbytes& collectivesig,
                                       unsigned int offset, Action action,
                                       State nextstate,
                                       std::string_view spanName);
  bool ProcessMessageCollectiveSig(const zbytes& collectivesig,
                                   unsigned int offset);
  bool ProcessMessageFinalChallenge(const zbytes& challenge,
                                    unsigned int offset);
  bool ProcessMessageFinalCollectiveSig(const zbytes& finalcollectivesig,
                                        unsigned int offset);

 public:
  /// Constructor.
  ConsensusBackup(
      uint32_t consensus_id,     // unique identifier for this consensus session
      uint64_t block_number,     // latest final block number
      const zbytes& block_hash,  // unique identifier for this consensus session
      uint16_t node_id,  // backup's identifier (= index in some ordered lookup
                         // table shared by all nodes)
      uint16_t leader_id,      // leader's identifier (= index in some ordered
                               // lookup table shared by all nodes)
      const PrivKey& privkey,  // backup's private key
      const DequeOfNode& committee,  // ordered lookup table of pubkeys for this
                                     // committee (includes leader)
      uint8_t class_byte,            // class byte representing Executable class
                                     // using this instance of ConsensusBackup
      uint8_t ins_byte,              // instruction byte representing consensus
                                     // messages for the Executable class
      MsgContentValidatorFunc
          msg_validator,  // function handler for validating the complete
                          // content of message for consensus (e.g., Tx block)
      MsgContentValidatorFunc preprep_msg_validator =
          nullptr,  // function handler for validating the preprep content of
                    // message for consensus (e.g., Tx block)
      PostPrePrepValidationFunc post_preprep_validation =
          nullptr,  // function handler to execute
                    // any post activity after validation of preprep
                    // message
      CollectiveSigReadinessFunc collsig_readiness_func =
          nullptr,  // function handler for waits until some cond is met
      bool isDS = false);

  /// Destructor.
  ~ConsensusBackup();

  /// Function to process any consensus message received.
  bool ProcessMessage(const zbytes& message, unsigned int offset,
                      const Peer& from);

  unsigned int GetNumForConsensusFailure() { return 0; }

 private:
  static std::map<Action, std::string> ActionStrings;
  std::string GetActionString(Action action) const;
};

#endif  // ZILLIQA_SRC_LIBCONSENSUS_CONSENSUSBACKUP_H_