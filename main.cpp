#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "ext/minicsv.h"
#include "ext/format.h"

#include <iostream>
#include <string>
#include <vector>

using boost::filesystem::path;

using namespace fmt;
using namespace std;


// without this it wont work. I'm not sure what it does.
// it has something to do with locking the blockchain and tx pool
// during certain operations to avoid deadlocks.

namespace epee {
    unsigned int g_test_dbg_lock_sleep = 0;
}


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
    auto address_opt      = opts.get_option<string>("address");
    auto viewkey_opt      = opts.get_option<string>("viewkey");
    auto start_height_opt = opts.get_option<size_t>("start-height");
    auto start_date_opt   = opts.get_option<string>("start-date");
    auto out_csv_file_opt = opts.get_option<string>("out-csv-file");
    auto bc_path_opt      = opts.get_option<string>("bc-path");
    auto testnet_opt      = opts.get_option<bool>("testnet");


    // get the program command line options, or
    // some default values for quick check
    string address_str   = address_opt ? *address_opt : "48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU";
    string viewkey_str   = viewkey_opt ? *viewkey_opt : "1ddabaa51cea5f6d9068728dc08c7ffaefe39a7a4b5f39fa8a976ecbe2cb520a";
    size_t start_height  = start_height_opt ? *start_height_opt : 0;
    string start_date    = start_date_opt ? *start_date_opt : "1970-01-01";
    string out_csv_file  = out_csv_file_opt ? *out_csv_file_opt : "/tmp/xmr_incomming.csv";
    bool testnet         = *testnet_opt ;

    cout << testnet << endl;

    path blockchain_path;

    if (!xmreg::get_blockchain_path(bc_path_opt, blockchain_path))
    {
        // if problem obtaining blockchain path, finish.
        return 1;
    }


    print("Blockchain path: {:s}\n", blockchain_path);


    // enable basic monero log output
    xmreg::enable_monero_log();

    // create instance of our MicroCore
    xmreg::MicroCore mcore;

    // initialize the core using the blockchain path
    if (!mcore.init(blockchain_path.string()))
    {
        cerr << "Error accessing blockchain." << endl;
        return 1;
    }

    // get the high level cryptonote::Blockchain object to interact
    // with the blockchain lmdb database
    cryptonote::Blockchain& core_storage = mcore.get_core();

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage.get_current_blockchain_height();

    if (start_height > height)
    {
        cerr << "Given height is greater than blockchain height" << endl;
        return 1;
    }


    if (start_date_opt)
    {
        // if we gave starting date use the date to
        // determine the starting block height

        print("Requested starting date: {:s}\n", start_date);

        try
        {
            // estimate blockchain height from the start date provided
            start_height = xmreg::estimate_bc_height(start_date);

            // get block of based on the start_height
            // and display its timestamp so that we can
            // see how far in the past are we, or
            // how roughly the height was estimated from
            // the given date.
            cryptonote::block blk;

            if (!mcore.get_block_by_date(start_date, blk, start_height))
            {
                cerr << "Cant get block by date: " << start_date << endl;
                return 1;
            }


            // set new start_height block based on the date given
            start_height = cryptonote::get_block_height(blk);
        }
        catch (const exception& e)
        {
            cerr << e.what() << endl;
            return 1;
        }
    }


    cryptonote::block blk;

    if (!mcore.get_block_by_height(start_height, blk))
    {
        cerr << "Cant get block by date: " << start_date << endl;
        return 1;
    }

    print("Start block date       : {:s}\n", xmreg::timestamp_to_str(blk.timestamp));
    print("Start block height     : {:d}\n", start_height);

    // parse string representing given monero address
    cryptonote::account_public_address address;

    if (!xmreg::parse_str_address(address_str,  address, testnet))
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


    // lets check our keys
    cout << "\n"
         << "address          : <" << xmreg::print_address(address, testnet) << ">\n"
         << "private view key : "  << prv_view_key << "\n"
         << endl;


    csv::ofstream csv_os {out_csv_file.c_str()};

    if (!csv_os.is_open())
    {
        cerr << "Cant open file: " << out_csv_file << endl;
        return 1;
    }

    // write the header
    csv_os << "Data" << "Time" << " Block_no"
           << "Tx_hash" << "Out_num"<< "Amount" << NEWLINE;

    // show command line output for every i-th block
    uint64_t EVERY_ith_BLOCK {2000};

    if (EVERY_ith_BLOCK > height)
    {
        EVERY_ith_BLOCK = height / 10;
    }

    for (uint64_t i = start_height; i < height; ++i) {


        cryptonote::block blk;

        try
        {
            blk = core_storage.get_db().get_block_from_height(i);
        }
        catch (std::exception& e)
        {
            cerr << e.what() << endl;
            continue;
        }

        string blk_time = xmreg::timestamp_to_str(blk.timestamp);

        // show every nth output, just to give
        // a console some break
        if (i % EVERY_ith_BLOCK == 0)
        {
            print("Analysing block {:08d}/{:08d} - date {:s}\n",
                  i, height, blk_time);
        }

        // get all transactions in the block found
        // initialize the first list with transaction for solving
        // the block i.e. coinbase.
        list<cryptonote::transaction> txs {blk.miner_tx};
        list<crypto::hash> missed_txs;

        if (!mcore.get_core().get_transactions(blk.tx_hashes, txs, missed_txs))
        {
            cerr << "Cant find transcations in block: " << height << endl;
            csv_os.flush();
            csv_os.close();
            return 1;
        }

        for (const cryptonote::transaction& tx : txs)
        {
            crypto::hash tx_hash = cryptonote::get_transaction_hash(tx);

            vector<xmreg::transfer_details> found_outputs
                    = xmreg::get_belonging_outputs(blk, tx, prv_view_key,
                                                   address.m_spend_public_key, i);

            if (!found_outputs.empty())
            {
                print(" - found {:02d} outputs in block {:08d} ({:s}) - writing to the csv\n",
                      found_outputs.size(), i, blk_time);

                // save found transfers to the csv file
                for (const auto& tr_details: found_outputs)
                {
                    csv_os << tr_details << NEWLINE;
                }

            }

        }

    } // for (uint64_t i = 0; i < height; ++i)


    csv_os.flush();
    csv_os.close();

    cout << "\nCsv saved as: " << out_csv_file << endl;

    cout << "\nEnd of program." << endl;

    return 0;
}
