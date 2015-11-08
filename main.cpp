#include <iostream>
#include <string>

#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"


using namespace std;
using boost::filesystem::path;
using boost::filesystem::is_directory;

// without this it wont work. I'm not sure what it does.
// it has something to do with locking the blockchain and tx pool
// during certain operations to avoid deadlocks.
unsigned int epee::g_test_dbg_lock_sleep = 0;


int main(int ac, const char* av[]) {

    // get command line options
    xmreg::CmdLineOptions opts {ac, av};

    auto help_opt = opts.get_option<bool>("help");

    // if help was chosen, display help text and finish
    if (*help_opt)
    {
        return 0;
    }

    // get other options
    auto address_opt = opts.get_option<string>("address");
    auto viewkey_opt = opts.get_option<string>("viewkey");
    auto tx_hash_opt = opts.get_option<string>("txhash");
    auto bc_path_opt = opts.get_option<string>("bc-path");


    // default path to monero folder
    // on linux this is /home/<username>/.bitmonero
    string default_monero_dir = tools::get_default_data_dir();

    // the default folder of the lmdb blockchain database
    // is therefore as follows
    string default_lmdb_dir   = default_monero_dir + "/lmdb";

    // get the program command line options, or
    // some default values for quick check
    string address_str = address_opt ? *address_opt : "48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU";
    string viewkey_str = viewkey_opt ? *viewkey_opt : "1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a";
    string tx_hash_str = tx_hash_opt ? *tx_hash_opt : "66040ad29f0d780b4d47641a67f410c28cce575b5324c43b784bb376f4e30577";
    path blockchain_path = bc_path_opt ? path(*bc_path_opt) : path(default_lmdb_dir);


    if (!is_directory(blockchain_path))
    {
        cerr << "Given path \"" << blockchain_path   << "\" "
             << "is not a folder or does not exist" << " "
             << endl;
        return 1;
    }

    blockchain_path = xmreg::remove_trailing_path_separator(blockchain_path);

    cout << "Blockchain path: " << blockchain_path << endl;

    // enable basic monero log output
    uint32_t log_level = 0;
    epee::log_space::get_set_log_detalisation_level(true, log_level);
    epee::log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL);


    // create instance of our MicroCore
    xmreg::MicroCore mcore;

    // initialize the core using the blockchain path
    if (!mcore.init(blockchain_path.string()))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }

    // get the highlevel cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage.get_current_blockchain_height();

    cout << "Current blockchain height: " << height << endl;


    // parse string representing given monero address
    cryptonote::account_public_address address;

    if (!xmreg::parse_str_address(address_str,  address))
    {
        cerr << "Cant parse string address: " << address_str << endl;
        return 1;
    }


    // parse string representing given private viewkey
    crypto::secret_key prv_view_key;
    if (!xmreg::parse_str_secret_key(viewkey_str, prv_view_key))
    {
        cerr << "Cant parse view key: " << viewkey_str << endl;
        return 1;
    }


    // we also need tx public key, but we have tx hash only.
    // to get the key, first, we obtained transaction object tx
    // and then we get its public key from tx's extras.
    cryptonote::transaction tx;

    if (!xmreg::get_tx_pub_key_from_str_hash(core_storage, tx_hash_str, tx))
    {
        cerr << "Cant find transaction with hash: " << tx_hash_str << endl;
        return 1;
    }


    crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);

    if (pub_tx_key == cryptonote::null_pkey)
    {
        cerr << "Cant get public key of tx with hash: " << tx_hash_str << endl;
        return 1;
    }


    // public transaction key is combined with our viewkey
    // to create, so called, derived key.
    crypto::key_derivation derivation;

    if (!generate_key_derivation(pub_tx_key, prv_view_key, derivation))
    {
        cerr << "Cant get dervied key for: " << "\n"
             << "pub_tx_key: " << prv_view_key << " and "
             << "prv_view_key" << prv_view_key << endl;
        return 1;
    }


    // lets check our keys
    cout << "\n"
         << "address          : <" << xmreg::print_address(address) << ">\n"
         << "private view key : "  << prv_view_key << "\n"
         << "tx hash          : <" << tx_hash_str << ">\n"
         << "public tx key    : "  << pub_tx_key << "\n"
         << "dervied key      : "  << derivation << "\n" << endl;


    // each tx that we (or the address we are checking) received
    // contains a number of outputs.
    // some of them are ours, some not. so we need to go through
    // all of them in a given tx block, to check which outputs are ours.

    // get the total number of outputs in a transaction.
    size_t output_no = tx.vout.size();

    // sum amount of xmr sent to us
    // in the given transaction
    uint64_t money_transfered {0};

    // loop through outputs in the given tx
    // to check which outputs our ours. we compare outputs'
    // public keys with the public key that would had been
    // generated for us if we had gotten the outputs.
    // not sure this is the case though, but that's my understanding.
    for (size_t i = 0; i < output_no; ++i)
    {
        // get the tx output public key
        // that normally would be generated for us,
        // if someone had sent us some xmr.
        crypto::public_key pubkey;

        crypto::derive_public_key(derivation,
                                  i,
                                  address.m_spend_public_key,
                                  pubkey);

        // get tx output public key
        const cryptonote::txout_to_key tx_out_to_key
                = boost::get<cryptonote::txout_to_key>(tx.vout[i].target);


        cout << "Output no: " << i << ", " << tx_out_to_key.key;

        // check if the output's public key is ours
        if (tx_out_to_key.key == pubkey)
        {
            // if so, than add the xmr amount to the money_transfered
            money_transfered += tx.vout[i].amount;
            cout << ", mine key: " << cryptonote::print_money(tx.vout[i].amount) << endl;
        }
        else
        {
            cout << ", not mine key " << endl;
        }
    }

    cout << "\nTotal xmr received: " << cryptonote::print_money(money_transfered) << endl;


    cout << "\nEnd of program." << endl;

    return 0;
}