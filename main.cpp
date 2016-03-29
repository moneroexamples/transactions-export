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
    auto spendkey_opt     = opts.get_option<string>("spendkey");
    auto start_height_opt = opts.get_option<size_t>("start-height");
    auto start_date_opt   = opts.get_option<string>("start-date");
    auto out_csv_file_opt = opts.get_option<string>("out-csv-file");
    auto bc_path_opt      = opts.get_option<string>("bc-path");
    auto testnet_opt      = opts.get_option<bool>("testnet");


    // get the program command line options, or
    // some default values for quick check
    string address_str  = address_opt ? *address_opt
                          : "43A7NUmo5HbhJoSKbw9bRWW4u2b8dNfhKheTR5zxoRwQ7bULK5TgUQeAvPS5EVNLAJYZRQYqXCmhdf26zG2Has35SpiF1FP";
    string viewkey_str  = viewkey_opt ? *viewkey_opt
                          : "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
    string spendkey_str = spendkey_opt ? *spendkey_opt
                          : "";
    size_t start_height = start_height_opt ? *start_height_opt : 0;
    string start_date   = start_date_opt ? *start_date_opt : "1970-01-01";
    string out_csv_file = out_csv_file_opt ? *out_csv_file_opt : "./xmr_report.csv";
    bool testnet        = *testnet_opt ;


    bool SPEND_KEY_GIVEN {false};

    if (!spendkey_str.empty())
    {
        SPEND_KEY_GIVEN = true;
    }


    path blockchain_path;

    if (!xmreg::get_blockchain_path(bc_path_opt, blockchain_path))
    {
        // if problem obtaining blockchain path, finish.
        return 1;
    }


    print("Blockchain path: {:s}\n", blockchain_path);


    // change timezone to Universtal time zone
    char old_tz[128];
    const char *tz_org = getenv("TZ");

    if (tz_org)
    {
        strcpy(old_tz, tz_org);
    }

    // set new timezone
    std::string tz = "TZ=Coordinated Universal Time";
    putenv(const_cast<char *>(tz.c_str()));
    tzset(); // Initialize timezone data


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

    crypto::secret_key prv_spend_key;

    // parse string representing given private spend
    if (SPEND_KEY_GIVEN && !xmreg::parse_str_secret_key(spendkey_str, prv_spend_key))
    {
        cerr << "Cant parse spend key: " << spendkey_str << endl;
        return 1;
    }

    cryptonote::account_keys account_keys;

    if (SPEND_KEY_GIVEN)
    {
        // set account keys values
        account_keys.m_account_address  = address;
        account_keys.m_spend_secret_key = prv_spend_key;
        account_keys.m_view_secret_key  = prv_view_key;
    }


    // lets check our keys
    cout << "\n"
         << "address          : <" << xmreg::print_address(address, testnet) << ">\n"
         << "private view key : "  << prv_view_key << "\n";

    if (SPEND_KEY_GIVEN)
    {
        cout << "private spend key: " << prv_spend_key << "\n";
    }
    else
    {
        cout << "private spend key: " << "not given" << "\n";
    }

    cout << endl;


    csv::ofstream csv_os {out_csv_file.c_str()};

    if (!csv_os.is_open())
    {
        cerr << "Cant open file: " << out_csv_file << endl;
        return 1;
    }

    // write the header of the csv file to be created
    csv_os << "Data" << "Time" << " Block_no"
           << "Tx_hash" << "Payment_id" << "Out_idx" << "Amount"
           << "Output_pub_key" << "Output_key_img"
           << "Output_spend"
           << NEWLINE;

    // show command line output for every i-th block
    uint64_t EVERY_ith_BLOCK {2000};

    if (EVERY_ith_BLOCK > height)
    {
        EVERY_ith_BLOCK = height / 10;
    }

    // to check which inputs our ours, we need
    // to compare inputs's key image with our key_images.
    // our key_images are generated from our outputs and private spend_key.
    // thus while we search for our inputs, we are going to generate key_images
    // for each found input as we go and key them in a vector.
    // this way, checking which input is ours, is as
    // simple as veryfing if a given key_image exist in our vector.
    vector<crypto::key_image> key_images_gen;

    for (uint64_t i = start_height; i < height; ++i)
    {
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
            cerr << "Cant find transactions in block: " << height << endl;
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

            // get tx public key from extras field
            crypto::public_key pub_tx_key = cryptonote::get_tx_pub_key_from_extra(tx);


            if (!found_outputs.empty())
            {
                print(" - found {:02d} outputs in block {:08d} ({:s}) - writing to the csv\n",
                      found_outputs.size(), i, blk_time);

                // save found our outputs to the csv file
                for (auto& tr_details: found_outputs)
                {

                    if (SPEND_KEY_GIVEN)
                    {
                        // before output details are saved in csv,  lets
                        // generate its key image using our private spend key (if given)
                        // and save it in global vector key_images_gen
                        // the key_image generated is saved in the csv file

                        // public tx key is combined with our private view key
                        // to create, so called, derived key.
                        // the derived key is used to produce the key_image
                        // that we want.
                        crypto::key_derivation derivation;

                        if (!generate_key_derivation(pub_tx_key, prv_view_key, derivation)) {
                            cerr << "Cant get derived key for output with: " << "\n"
                                 << "pub_tx_key: " << prv_view_key << endl;
                            return 1;
                        }

                        // generate key_image of this output
                        crypto::key_image key_img;

                        if (!xmreg::generate_key_image(derivation,
                                                       tr_details.m_internal_output_index, /* position in the tx */
                                                       prv_spend_key,
                                                       account_keys.m_account_address.m_spend_public_key,
                                                       key_img)) {
                            cerr << "Cant generate key image for output: "
                                 << tr_details.out_pub_key << endl;
                            return 1;
                        }

                        cout << " - output pub key: " << tr_details.out_pub_key << endl;
                        cout << " - key image: " << key_img << endl;

                        key_images_gen.push_back(key_img);

                        // copy key_image to tr_details to be saved
                        tr_details.key_img = key_img;

                        // check if output was spent
                        tr_details.m_spent = core_storage.have_tx_keyimg_as_spent(key_img);

                    }

                    csv_os << tr_details << NEWLINE;

                } // for (const auto& tr_details: found_outputs)

            } // if (!found_outputs.empty())

            // we finished checking outputs in a tx
            // thus check for inputs, if spend key was given

            if (!SPEND_KEY_GIVEN || key_images_gen.empty())
            {
                // if spend key not given, or key_images_gen vector is empty
                // skip rest of this for loop
                continue;
            }


            // get the total number of inputs in a transaction.
            // some of these inputs might be our spendings
            size_t input_no = tx.vin.size();

            //cout << tx_hash << ", input_no " << input_no << endl;

            for (size_t ii = 0; ii < input_no; ++ii)
            {

                if(tx.vin[ii].type() != typeid(cryptonote::txin_to_key))
                {
                    continue;
                }

                //cout << tx_hash << endl;
                //cout << "checkint inputs:" << ii << "/" << input_no << endl;

                // get tx input key
                const cryptonote::txin_to_key& tx_in_to_key
                        = boost::get<cryptonote::txin_to_key>(tx.vin[ii]);

                // check if the public key image of this input
                // matches any of your key images that were
                // generated for every output that we received
                std::vector<crypto::key_image>::iterator it;

                it = find(key_images_gen.begin(), key_images_gen.end(),
                          tx_in_to_key.k_image);

                //cout << "Input no: " << ii << ", " << tx_in_to_key.k_image;

                if (it != key_images_gen.end())
                {
                    cout << "Input no: " << ii << ", " << tx_in_to_key.k_image;
                    cout << ", mine key image: "
                         << cryptonote::print_money(tx_in_to_key.amount) << endl;
                }
                else
                {
                   // cout << ", not mine key image " << endl;
                }
            } // for (size_t ii = 0; ii < input_no; ++ii)

        } // for (const cryptonote::transaction& tx : txs)

    } // for (uint64_t i = 0; i < height; ++i)


    csv_os.flush();
    csv_os.close();


    // set timezone to orginal value
    if (tz_org != 0)
    {
        setenv("TZ", old_tz, 1);
        tzset();
    }

    cout << "\nCsv saved as: " << out_csv_file << endl;

    cout << "\nEnd of program." << endl;

    return 0;
}
