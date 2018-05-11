/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <evt/chain/asset.hpp>
#include <evt/chain/block.hpp>
#include <evt/chain/chain_controller.hpp>
#include <evt/chain/transaction.hpp>
#include <evt/chain/contracts/abi_serializer.hpp>
#include <evt/chain/contracts/chain_initializer.hpp>

#include <boost/container/flat_set.hpp>

namespace fc { class variant; }

namespace evt {
   using chain::chain_controller;
   using std::unique_ptr;
   using namespace appbase;
   using chain::name;
   using chain::uint128_t;
   using chain::public_key_type;
   using fc::optional;
   using boost::container::flat_set;
   using chain::asset;
   using chain::account_name;
   using chain::contracts::abi_def;
   using chain::contracts::abi_serializer;

namespace chain_apis {
struct empty{};

template<typename>
struct resolver_factory;

class read_only {
   const chain_controller& db;
   const abi_def& system_abi;

public:
   static const string KEYi64;
   static const string KEYstr;
   static const string KEYi128i128;
   static const string KEYi64i64i64;
   static const string PRIMARY;
   static const string SECONDARY;
   static const string TERTIARY;
   
   read_only(const chain_controller& db, const abi_def& system_abi)
      : db(db), system_abi(system_abi) {}

   using get_info_params = empty;

   struct get_info_results {
      string                  server_version;
      uint32_t                head_block_num = 0;
      uint32_t                last_irreversible_block_num = 0;
      chain::block_id_type    head_block_id;
      fc::time_point_sec      head_block_time;
      account_name            head_block_producer;
      string                  recent_slots;
      double                  participation_rate = 0;
   };
   get_info_results get_info(const get_info_params&) const;

   struct producer_info {
      name                       producer_name;
   };

   struct abi_json_to_bin_params {
      name         action;
      fc::variant  args;
   };
   struct abi_json_to_bin_result {
      vector<char>   binargs;
   };
      
   abi_json_to_bin_result abi_json_to_bin( const abi_json_to_bin_params& params )const;


   struct abi_bin_to_json_params {
      name         action;
      vector<char> binargs;
   };
   struct abi_bin_to_json_result {
      fc::variant    args;
   };
      
   abi_bin_to_json_result abi_bin_to_json( const abi_bin_to_json_params& params )const;


   struct get_required_keys_params {
      fc::variant transaction;
      flat_set<public_key_type> available_keys;
   };
   struct get_required_keys_result {
      flat_set<public_key_type> required_keys;
   };

   get_required_keys_result get_required_keys( const get_required_keys_params& params)const;


   struct get_block_params {
      string block_num_or_id;
   };

   fc::variant get_block(const get_block_params& params) const;

   friend struct resolver_factory<read_only>;
};

class read_write {
   chain_controller& db;
   const abi_def& system_abi;
   uint32_t skip_flags;
public:
   read_write(chain_controller& db, const abi_def& system_abi, uint32_t skip_flags) : db(db), system_abi(system_abi), skip_flags(skip_flags) {}

   using push_block_params = chain::signed_block;
   using push_block_results = empty;
   push_block_results push_block(const push_block_params& params);

   using push_transaction_params = fc::variant_object;
   struct push_transaction_results {
      chain::transaction_id_type  transaction_id;
      fc::variant                 processed;
   };
   push_transaction_results push_transaction(const push_transaction_params& params);


   using push_transactions_params  = vector<push_transaction_params>;
   using push_transactions_results = vector<push_transaction_results>;
   push_transactions_results push_transactions(const push_transactions_params& params);

   friend resolver_factory<read_write>;
};
} // namespace chain_apis

class chain_plugin : public plugin<chain_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES()

   chain_plugin();
   virtual ~chain_plugin();

   virtual void set_program_options(options_description& cli, options_description& cfg) override;

   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown();

   chain_apis::read_only get_read_only_api() const;
   chain_apis::read_write get_read_write_api();

   bool accept_block(const chain::signed_block& block, bool currently_syncing);
   void accept_transaction(const chain::packed_transaction& trx);

   bool block_is_on_preferred_chain(const chain::block_id_type& block_id);

   // return true if --skip-transaction-signatures passed to evtd
   bool is_skipping_transaction_signatures() const;

   // Only call this in plugin_initialize() to modify chain_controller constructor configuration
   chain_controller::controller_config& chain_config();
   // Only call this after plugin_startup()!
   chain_controller& chain();
   // Only call this after plugin_startup()!
   const chain_controller& chain() const;

  void get_chain_id (chain::chain_id_type &cid) const;

private:
   unique_ptr<class chain_plugin_impl> my;
};

}

FC_REFLECT(evt::chain_apis::empty, )
FC_REFLECT(evt::chain_apis::read_only::get_info_results,
  (server_version)(head_block_num)(last_irreversible_block_num)(head_block_id)(head_block_time)(head_block_producer)
  (recent_slots)(participation_rate))
FC_REFLECT(evt::chain_apis::read_only::get_block_params, (block_num_or_id))
  
FC_REFLECT( evt::chain_apis::read_write::push_transaction_results, (transaction_id)(processed) )
  
FC_REFLECT( evt::chain_apis::read_only::producer_info, (producer_name) )
FC_REFLECT( evt::chain_apis::read_only::abi_json_to_bin_params, (action)(args) )
FC_REFLECT( evt::chain_apis::read_only::abi_json_to_bin_result, (binargs) )
FC_REFLECT( evt::chain_apis::read_only::abi_bin_to_json_params, (action)(binargs) )
FC_REFLECT( evt::chain_apis::read_only::abi_bin_to_json_result, (args) )
FC_REFLECT( evt::chain_apis::read_only::get_required_keys_params, (transaction)(available_keys) )
FC_REFLECT( evt::chain_apis::read_only::get_required_keys_result, (required_keys) )