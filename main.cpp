#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "ext/minicsv.h"
#include "fmt/ostream.h"
#include "fmt/format.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <tuple>
#include <type_traits>

using boost::filesystem::path;

using namespace fmt;
using namespace std;

int
main(int ac, const char* av[])
{

// get command line options
xmreg::CmdLineOptions opts {ac, av};

auto help_opt = opts.get_option<bool>("help");

// if help was chosen, display help text and finish
if (*help_opt)
{
    return 0;
}

// get other options
auto address_opt         = opts.get_option<string>("address");
auto viewkey_opt         = opts.get_option<string>("viewkey");
auto spendkey_opt        = opts.get_option<string>("spendkey");
auto start_height_opt    = opts.get_option<size_t>("start-height");
auto stop_height_opt     = opts.get_option<size_t>("stop-height");
auto no_of_blocks_opt    = opts.get_option<size_t>("no-of-blocks");
auto start_date_opt      = opts.get_option<string>("start-date");
auto out_csv_file_opt    = opts.get_option<string>("out-csv-file");  // for our outputs only
auto out_csv_file2_opt   = opts.get_option<string>("out-csv-file2"); // for our outputs as ring members in other txs
auto out_csv_file3_opt   = opts.get_option<string>("out-csv-file3"); // for frequency of outputs as ring members in other txs
auto out_csv_file4_opt   = opts.get_option<string>("out-csv-file4"); // for all key_images with referenced output public keys
auto bc_path_opt         = opts.get_option<string>("bc-path");
auto testnet_opt         = opts.get_option<bool>("testnet");
auto stagenet_opt        = opts.get_option<bool>("stagenet");
auto ring_members_opt    = opts.get_option<bool>("ring-members");
auto all_outputs_opt     = opts.get_option<bool>("all-outputs");
auto all_key_images_opt  = opts.get_option<bool>("all-key-images");


// get the program command line options, or
// some default values for quick check
string address_str   = address_opt ? *address_opt
                      : "43A7NUmo5HbhJoSKbw9bRWW4u2b8dNfhKheTR5zxoRwQ7bULK5TgUQeAvPS5EVNLAJYZRQYqXCmhdf26zG2Has35SpiF1FP";
string viewkey_str   = viewkey_opt ? *viewkey_opt
                      : "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
string spendkey_str  = spendkey_opt ? *spendkey_opt
                      : "";
size_t start_height  = start_height_opt ? *start_height_opt : 0;
size_t stop_height   = *stop_height_opt;
size_t no_of_blocks  = *no_of_blocks_opt;
string start_date    = start_date_opt ? *start_date_opt : "1970-01-01";

string out_csv_file  = *out_csv_file_opt;
string out_csv_file2 = *out_csv_file2_opt;
string out_csv_file3 = *out_csv_file3_opt;
string out_csv_file4 = *out_csv_file4_opt;

bool testnet         = *testnet_opt;
bool stagenet        = *stagenet_opt;
bool ring_members    = *ring_members_opt ;
bool all_outputs     = *all_outputs_opt;
bool all_key_images  = *all_key_images_opt;
bool SPEND_KEY_GIVEN = (spendkey_str.empty() ? false : true);


if (testnet && stagenet)
{
    cerr << "testnet and stagenet cannot be specified at the same time!" << endl;
    return EXIT_FAILURE;
}

const cryptonote::network_type nettype = testnet ?
      cryptonote::network_type::TESTNET : stagenet ?
      cryptonote::network_type::STAGENET : cryptonote::network_type::MAINNET;


path blockchain_path;

if (!xmreg::get_blockchain_path(bc_path_opt, blockchain_path, nettype))
{
    cerr << "Error getting blockchain path." << '\n';
    return EXIT_FAILURE;
}

print("Blockchain path: {:s}\n", blockchain_path);

// change timezone to Universtal time zone
char old_tz[128];
const char *tz_org = getenv("TZ");

if (tz_org)
    strcpy(old_tz, tz_org);

// set new timezone
std::string tz = "TZ=Coordinated Universal Time";
putenv(const_cast<char *>(tz.c_str()));
tzset(); // Initialize timezone data


// set monero log output level
uint32_t log_level = 0;
mlog_configure("", true);

// create instance of our MicroCore
// and make pointer to the Blockchain
xmreg::MicroCore mcore;
cryptonote::Blockchain* core_storage;

// initialize mcore and core_storage
if (!xmreg::init_blockchain(blockchain_path.string(),
                            mcore, core_storage, nettype))
{
    cerr << "Error accessing blockchain." << '\n';
    return EXIT_FAILURE;
}

// get the current blockchain height. Just to check
// if it reads ok.
uint64_t height = stop_height == 0 ? core_storage->get_current_blockchain_height() : stop_height + 1;

if (start_height > height)
{
    cerr << "Given height is greater than blockchain height" << '\n';
    return EXIT_FAILURE;
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
            cerr << "Cant get block by date: " << start_date << '\n';
            return EXIT_FAILURE;
        }


        // set new start_height block based on the date given
        start_height = cryptonote::get_block_height(blk);
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
}


cryptonote::block blk;

if (!mcore.get_block_by_height(start_height, blk))
{
    cerr << "Cant get block by date: " << start_date << '\n';
    return EXIT_FAILURE;
}

print("Start block date       : {:s}\n", xmreg::timestamp_to_str(blk.timestamp));
print("Start block height     : {:d}\n", start_height);
print("Search for ring members: {:s}\n", (ring_members ? "True" : "False"));

// parse string representing given monero address
cryptonote::address_parse_info address_info;

if (!get_account_address_from_str(address_info,  nettype, address_str))
{
    cerr << "Cant parse string address: " << address_str << '\n';
    return EXIT_FAILURE;
}

cryptonote::account_public_address address = address_info.address;


// parse string representing given private viewkey
crypto::secret_key prv_view_key;

if (!xmreg::parse_str_secret_key(viewkey_str, prv_view_key))
{
    cerr << "Cant parse view key: " << viewkey_str << '\n';
    return EXIT_FAILURE;
}

crypto::secret_key prv_spend_key;

// parse string representing given private spend
if (SPEND_KEY_GIVEN && !xmreg::parse_str_secret_key(spendkey_str, prv_spend_key))
{
    cerr << "Cant parse spend key: " << spendkey_str << '\n';
    return EXIT_FAILURE;
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
cout << '\n'
     << "address          : " << xmreg::print_address(address_info, nettype) << '\n'
     << "private view key : "  << prv_view_key << '\n';

if (SPEND_KEY_GIVEN)
    cout << "private spend key: " << prv_spend_key << '\n';
else
    cout << "private spend key: " << "not given\n";

cout << '\n';


unique_ptr<csv::ofstream> csv_os(new csv::ofstream {out_csv_file.c_str()});

if (!csv_os->is_open())
{
    cerr << "Cant open file: " << out_csv_file << endl;
    return 1;
}


// write the header of the csv file to be created
*csv_os <<"Timestamp" << "Block_no"
       << "Tx_hash" << "Tx_public_key" << "Tx_version"
       << "Payment_id" << "Out_idx" << "Amount"
       << "Output_pub_key" << "Output_key_img"
       << "Output_spend"
       << NEWLINE;


unique_ptr<csv::ofstream> csv_os2;

if (ring_members)
{
    csv_os2.reset(new csv::ofstream {out_csv_file2.c_str()});

    if (!csv_os2->is_open())
    {
        cerr << "Cant open file: " << out_csv_file2 << '\n';
        return EXIT_FAILURE;
    }

    // write the header of the csv file to be created
    *csv_os2 << "Timestamp" << "Block_no"  << "Tx_hash" << "Output_pub_key"
             << "Key_image" << "ring_no/ring_size"
             << NEWLINE;
}

unique_ptr<csv::ofstream> csv_os4;

if (all_key_images)
{
    csv_os4.reset(new csv::ofstream {out_csv_file4.c_str()});

    if (!csv_os4->is_open())
    {
        cerr << "Cant open file: " << out_csv_file4 << '\n';
        return EXIT_FAILURE;
    }

    // write the header of the csv file to be created
    *csv_os4 << "Timestamp" << "Block_no" << "Tx_hash"
             << "Key_image" << "Ring_size" << "Absolute_key_offset"
             << "Referenced_output_pub_key"
             << "Referenced_tx_hash"
             << "Reference_out_index_in_the_ref_tx"
             << NEWLINE;
}



// show command line output for every i-th block
uint64_t EVERY_ith_BLOCK {1000};

if (EVERY_ith_BLOCK > height)
    EVERY_ith_BLOCK = height / 10;

// here we will store our outputs that we find as we scan the blockchain
// this will be especially useful to check if our outputs are used in some
// txs as ring members of other people txs.
vector<pair<crypto::public_key, uint64_t>> known_outputs_keys;

// here we store number of times our output was used as ring member
// so that at the end of the progrem, we can show most frequently used
// outputs.
//               output pub key ,       freq.   , ring sizes
unordered_map<crypto::public_key, tuple<uint64_t, vector<uint64_t>>> ring_member_frequency;


// to check which inputs our ours, we need
// to compare inputs's key image with our key_images.
// our key_images are generated from our outputs and private spend_key.
// thus while we search for our inputs, we are going to generate key_images
// for each found input as we go and key them in a vector.
// this way, checking which input is ours, is as
// simple as veryfing if a given key_image exist in our vector.
vector<crypto::key_image> key_images_gen;

size_t blk_counter {0};
size_t same_pubkey_tx_counter {0}; // no of tx with same duplicate pubkeys

auto same_pubkey_txs = csv::ofstream {"same_pubkey_txs.csv"};

same_pubkey_txs << "index" << "timestamp" << "block" << "tx_hash"
                << "no_of_same_pubkeys" << NEWLINE;

for (uint64_t i = start_height; i < height; ++i)
{
    cryptonote::block blk;

    try
    {
        blk = core_storage->get_db().get_block_from_height(i);
    }
    catch (std::exception& e)
    {
        cerr << e.what() << '\n';
        continue;
    }

    // break the loob if we processed more than
    // requested no_of_blocks
    if (no_of_blocks > 0 && ++blk_counter > no_of_blocks)
        break;

    string blk_time = xmreg::timestamp_to_str(blk.timestamp);

    // show every nth output, just to give
    // a console some break
    if (i % EVERY_ith_BLOCK == 0)
        print("Analysing block {:08d}/{:08d} - date {:s}\n", i, height, blk_time);

    // get all transactions in the block found
    // initialize the first list with transaction for solving
    // the block i.e. coinbase.
    vector<cryptonote::transaction> txs {blk.miner_tx};
    vector<crypto::hash> missed_txs;

    if (!mcore.get_core().get_transactions(blk.tx_hashes, txs, missed_txs))
    {
        cerr << "Cant find transactions in block: " << height << '\n';
        return EXIT_FAILURE;
    }

    for (const cryptonote::transaction& tx : txs)
    {

        crypto::hash tx_hash = cryptonote::get_transaction_hash(tx);

        vector<xmreg::transfer_details> found_outputs;

        if (all_outputs == false)
        {
            try
            {
                // output only our outputs
                found_outputs = xmreg::get_belonging_outputs(
                        blk, tx, address, prv_view_key, i);
            }
            catch (std::exception const& e)
            {
                cerr << e.what() << " for tx: " << epee::string_tools::pod_to_hex(tx_hash)
                     << " Skipping this tx!" << endl;
                continue;
            }
        }
        else
        {
            found_outputs = xmreg::get_outputs(blk, tx, i);
        }


        // get tx public key from extras field
        crypto::public_key pub_tx_key = xmreg::get_tx_pub_key_from_received_outs(tx);                

        std::vector<cryptonote::tx_extra_field> tx_extra_fields;

        string tx_hash_str = epee::string_tools::pod_to_hex(tx_hash);

        if(!parse_tx_extra(tx.extra, tx_extra_fields))
        {
          // Extra may only be partially parsed, it's OK if tx_extra_fields contains public key
          cerr << "Transaction extra has unsupported format: "
               << tx_hash_str << '\n';
          continue;
        }

        std::unordered_set<crypto::public_key> public_keys_seen;

        size_t pk_index = 0;

        size_t no_of_same_pubkeys {1};

        while (!tx.vout.empty())
        {
            cryptonote::tx_extra_pub_key pub_key_field;

            if(!find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, pk_index++))
            {
              if (pk_index > 1)
                break;

              cerr << "Public key wasn't found in the transaction extra. Skipping transaction "
                   << tx_hash_str << '\n';
              break;
            }

            if (public_keys_seen.find(pub_key_field.pub_key) != public_keys_seen.end())
            {

              ++no_of_same_pubkeys;
            }

            public_keys_seen.insert(pub_key_field.pub_key);
        }

        if (no_of_same_pubkeys > 1)
        {
            cout << "  - " << (++same_pubkey_tx_counter)
                 << " the same tx pubkey is present more than once ("
                 << no_of_same_pubkeys << " times) in block / timestamp / tx: "
                 << i << " / " << xmreg::timestamp_to_str(blk.timestamp)
                 << " / " << tx_hash_str << '\n';

            same_pubkey_txs << same_pubkey_tx_counter
                            << xmreg::timestamp_to_str(blk.timestamp)
                            << i<< tx_hash_str
                            << no_of_same_pubkeys
                            << NEWLINE;

            same_pubkey_txs.flush();
        }

    } // for (const cryptonote::transaction& tx : txs)

} // for (uint64_t i = 0; i < height; ++i)

same_pubkey_txs.close();
cout << "\nCsv saved as: ./same_pubkey_txs.csv\n";



// set timezone to orginal value
if (tz_org != 0)
{
    setenv("TZ", old_tz, 1);
    tzset();
}



cout << "\nEnd of program." << '\n';

return 0;
}
