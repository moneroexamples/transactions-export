#include "src/MicroCore.h"
#include "src/CmdLineOptions.h"
#include "src/tools.h"

#include "ext/minicsv.h"
#include "ext/format.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <type_traits>

using boost::filesystem::path;

using namespace fmt;
using namespace std;

// declare  address_parse_info, for sfinae.
// if we work with moenro v0.11.0 there is no address_parse_info.
// but if with work with newer version, there is address_parse_info and we
// use this to get address;
namespace cryptonote {

    // just have it declared. Dont need to provide any definitoin
    // now. we let sfinae to choose the declaration which has definition.

    struct address_parse_info;

    std::string get_account_address_as_str(
            bool testnet, const account_public_address& adr);

}

template <typename T = cryptonote::address_parse_info>
struct address_helper
{

    template <typename Q = T>
    bool get_account_address_from_str_helper(
            Q& address, bool testnet, std::string const& address_str)
    {
        return get_account_address_from_str(address, testnet, address_str);
    }

    std::string get_account_address_as_str_helper(
            bool testnet, bool subaddress,
            const cryptonote::account_public_address& address)
    {
        (void) subaddress;
        return xmreg::my_get_account_address_as_str(testnet, address);
    }

    // should be used when cryptonote::address_parse_info is avaliable
    template <typename Q = T>
    typename std::enable_if<std::is_constructible<Q>::value, string>::type
    print_address(Q const& address, bool testnet)
    {
        return get_account_address_as_str_helper(testnet,
                                          false /*assume we only have base address*/,
                                          address);
    }

    // should be used when cryptonote::address_parse_info is NOT avaliable
    template <typename Q = T>
    typename std::enable_if<!std::is_constructible<Q>::value, string>::type
    print_address(Q const& address, bool testnet)
    {
        return get_account_address_as_str(testnet, address);
    }

    // should be used when cryptonote::address_parse_info is avaliable
    template<typename Q = T>
    typename std::enable_if<std::is_constructible<Q>::value, bool>::type
    operator()(string const& address_str, bool testnet, cryptonote::account_public_address& addr)
    {
        Q address_info;

        if (get_account_address_from_str_helper(address_info, testnet, address_str))
        {
            addr = address_info.address;
        }

        return true;
    }

    // should be used when cryptonote::address_parse_info is NOT avaliable
    template<typename Q = T>
    typename std::enable_if<!std::is_constructible<Q>::value, bool>::type
    operator()(string const& address_str, bool testnet, cryptonote::account_public_address& addr)
    {
        cryptonote::account_public_address address;

        if (get_account_address_from_str_helper(address, testnet, address_str))
        {
            addr = address;
            return true;
        }

        return false;
    }

};

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
    auto address_opt       = opts.get_option<string>("address");
    auto viewkey_opt       = opts.get_option<string>("viewkey");
    auto spendkey_opt      = opts.get_option<string>("spendkey");
    auto start_height_opt  = opts.get_option<size_t>("start-height");
    auto start_date_opt    = opts.get_option<string>("start-date");
    auto out_csv_file_opt  = opts.get_option<string>("out-csv-file");  // for our outputs only
    auto out_csv_file2_opt = opts.get_option<string>("out-csv-file2"); // for our outputs as ring members in other txs
    auto out_csv_file3_opt = opts.get_option<string>("out-csv-file3"); // for frequency of outputs as ring members in other txs
    auto bc_path_opt       = opts.get_option<string>("bc-path");
    auto testnet_opt       = opts.get_option<bool>("testnet");
    auto ring_members_opt  = opts.get_option<bool>("ring-members");
    auto all_outputs_opt   = opts.get_option<bool>("all-outputs");


    // get the program command line options, or
    // some default values for quick check
    string address_str   = address_opt ? *address_opt
                          : "43A7NUmo5HbhJoSKbw9bRWW4u2b8dNfhKheTR5zxoRwQ7bULK5TgUQeAvPS5EVNLAJYZRQYqXCmhdf26zG2Has35SpiF1FP";
    string viewkey_str   = viewkey_opt ? *viewkey_opt
                          : "9c2edec7636da3fbb343931d6c3d6e11bcd8042ff7e11de98a8d364f31976c04";
    string spendkey_str  = spendkey_opt ? *spendkey_opt
                          : "";
    size_t start_height  = start_height_opt ? *start_height_opt : 0;
    string start_date    = start_date_opt ? *start_date_opt : "1970-01-01";
    string out_csv_file  = *out_csv_file_opt ;
    string out_csv_file2 = *out_csv_file2_opt ;
    string out_csv_file3 = *out_csv_file3_opt ;
    bool testnet         = *testnet_opt ;
    bool ring_members    = *ring_members_opt ;
    bool all_outputs     = *all_outputs_opt;


    bool SPEND_KEY_GIVEN = (spendkey_str.empty() ? false : true);

    path blockchain_path;

    if (!xmreg::get_blockchain_path(bc_path_opt, blockchain_path, testnet))
    {
        cerr << "Error getting blockchain path." << '\n';
        return EXIT_FAILURE;
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


    // set monero log output level
    uint32_t log_level = 0;
    mlog_configure("", true);

    // create instance of our MicroCore
    // and make pointer to the Blockchain
    xmreg::MicroCore mcore;
    cryptonote::Blockchain* core_storage;

    // initialize mcore and core_storage
    if (!xmreg::init_blockchain(blockchain_path.string(),
                                mcore, core_storage))
    {
        cerr << "Error accessing blockchain." << '\n';
        return EXIT_FAILURE;
    }

    // get the current blockchain height. Just to check
    // if it reads ok.
    uint64_t height = core_storage->get_current_blockchain_height();

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
    struct address_helper<cryptonote::address_parse_info> get_address;

    cryptonote::account_public_address address;

    if (!get_address(address_str,  testnet, address))
    {
        cerr << "Cant parse string address: " << address_str << '\n';
        return EXIT_FAILURE;
    }


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
         << "address          : " << get_address.print_address(address, testnet) << '\n'
         << "private view key : "  << prv_view_key << '\n';

    if (SPEND_KEY_GIVEN)
        cout << "private spend key: " << prv_spend_key << '\n';
    else
        cout << "private spend key: " << "not given" << '\n';

    cout << '\n';


    unique_ptr<csv::ofstream> csv_os(new csv::ofstream {out_csv_file.c_str()});

    if (!csv_os->is_open())
    {
        cerr << "Cant open file: " << out_csv_file << endl;
        return 1;
    }


    // write the header of the csv file to be created
    *csv_os << "Data" << "Time" << " Block_no"
           << "Tx_hash" << "Tx_public_key"
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
        *csv_os2 << "Timestamp" << "Output_pub_key" << "Tx_hash"
                 << "Key_image" << "ring_no"
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

    unordered_map<crypto::public_key, uint64_t> ring_member_frequency;


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
            blk = core_storage->get_db().get_block_from_height(i);
        }
        catch (std::exception& e)
        {
            cerr << e.what() << '\n';
            continue;
        }

        string blk_time = xmreg::timestamp_to_str(blk.timestamp);

        // show every nth output, just to give
        // a console some break
        if (i % EVERY_ith_BLOCK == 0)
            print("Analysing block {:08d}/{:08d} - date {:s}\n", i, height, blk_time);

        // get all transactions in the block found
        // initialize the first list with transaction for solving
        // the block i.e. coinbase.
        list<cryptonote::transaction> txs {blk.miner_tx};
        list<crypto::hash> missed_txs;

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
                // output only our outputs
                found_outputs = xmreg::get_belonging_outputs(
                        blk, tx, address, prv_view_key, i);
            }
            else
            {
                found_outputs = xmreg::get_outputs(blk, tx, i);
            }


            // get tx public key from extras field
            crypto::public_key pub_tx_key = xmreg::get_tx_pub_key_from_received_outs(tx);


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

                        if (!generate_key_derivation(pub_tx_key, prv_view_key, derivation))
                        {
                            cerr << "Cant get derived key for output with: " << "\n"
                                 << "pub_tx_key: " << prv_view_key << '\n';
                            return EXIT_FAILURE;
                        }

                        // generate key_image of this output
                        crypto::key_image key_img;

                        if (!xmreg::generate_key_image(derivation,
                                                       tr_details.m_internal_output_index, /* position in the tx */
                                                       prv_spend_key,
                                                       account_keys.m_account_address.m_spend_public_key,
                                                       key_img))
                        {
                            cerr << "Cant generate key image for output: "
                                 << tr_details.out_pub_key << '\n';
                            return EXIT_FAILURE;
                        }

                        cout << "   - output pub key: " << tr_details.out_pub_key
                             << ",  key image: " << key_img << '\n';

                        key_images_gen.push_back(key_img);

                        // copy key_image to tr_details to be saved
                        tr_details.key_img = key_img;

                        // check if output was spent
                        tr_details.m_spent = core_storage->have_tx_keyimg_as_spent(key_img);

                      } // if (SPEND_KEY_GIVEN)

                      // store public key and amount of output that belongs to us
                      known_outputs_keys.push_back({tr_details.out_pub_key, tr_details.amount()});

                      *csv_os << tr_details << NEWLINE;

                      csv_os->flush();

                } // for (const auto& tr_details: found_outputs)

            } // if (!found_outputs.empty())

            // we finished checking outputs in a tx
            // thus check for inputs, if spend key was given or
            // we want to check only if our outputs were used
            // as ring members somewhere

            // get the total number of inputs in a transaction.
            // some of these inputs might be our spendings
            size_t input_no = tx.vin.size();

            //cout << tx_hash << ", input_no " << input_no << endl;

            for (size_t ii = 0; ii < input_no; ++ii)
            {

                if(tx.vin[ii].type() != typeid(cryptonote::txin_to_key))
                    continue;

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

                uint64_t xmr_amount = tx_in_to_key.amount;

                bool our_key_image = (it != key_images_gen.end());

                if (our_key_image)
                {
                    cout << " - found our input: " << ", " << tx_in_to_key.k_image
                         << ", amount: " << cryptonote::print_money(tx_in_to_key.amount)
                         << '\n';
                }

                if (ring_members && !our_key_image)
                {
                    // search if any of the outputs
                    // have been used as a ring member.
                    // this will include our own key images

                    // get absolute offsets of mixins
                    std::vector<uint64_t> absolute_offsets
                            = cryptonote::relative_output_offsets_to_absolute(
                                    tx_in_to_key.key_offsets);

                    // get public keys of outputs used in the mixins that match to the offests
                    std::vector<cryptonote::output_data_t> mixin_outputs;

                    try
                    {
                        core_storage->get_db().get_output_key(xmr_amount,
                                                              absolute_offsets,
                                                              mixin_outputs);
                    }
                    catch (const cryptonote::OUTPUT_DNE& e)
                    {
                        cerr << "Mixins key images not found" << '\n';
                        continue;
                    }

                    // mixin counter
                    size_t count = 0;

                    // for each found output public key check if its ours or not
                    for (const uint64_t& abs_offset: absolute_offsets)
                    {
                        // get basic information about mixn's output
                        cryptonote::output_data_t output_data = mixin_outputs.at(count);

                        // before going to the mysql, check our known outputs cash
                        // if the key exists. Its much faster than going to mysql
                        // for this.

                        auto it =  std::find_if(
                                known_outputs_keys.begin(),
                                known_outputs_keys.end(),
                                [&](const pair<crypto::public_key, uint64_t>& known_output)
                                {
                                    return output_data.pubkey == known_output.first;
                                });

                        if (it == known_outputs_keys.end())
                        {
                            // this mixins's output is unknown.
                            ++count;
                            continue;
                        }

                        // this seems to be our mixin.

                        ring_member_frequency[it->first] += 1;

                        cout << " - found output as ring member: " << count << ", " << it->first
                             << ", tx hash: " << tx_hash << '\n';

                        *csv_os2 << blk_time
                                 << epee::string_tools::pod_to_hex(it->first)
                                 << epee::string_tools::pod_to_hex(tx_hash)
                                 << epee::string_tools::pod_to_hex(tx_in_to_key.k_image)
                                 << count << NEWLINE;

                        csv_os2->flush();

                        ++count;

                    } // for (const cryptonote::output_data_t& output_data: outputs)

                } //else if (it != key_images_gen.end())

            } // for (size_t ii = 0; ii < input_no; ++ii)

        } // for (const cryptonote::transaction& tx : txs)

    } // for (uint64_t i = 0; i < height; ++i)

    csv_os->close();

    if (ring_members)
        csv_os2->close();

    // set timezone to orginal value
    if (tz_org != 0)
    {
        setenv("TZ", old_tz, 1);
        tzset();
    }

    cout << "\nCsv saved as: " << out_csv_file << '\n';


    if (ring_members)
    {

        // we need to sort the frequencies from most frequent to lease
        // frequent. For this, <public_key, freq> values from map
        // are copied to vector, which is then sorted by freq.
        // since I dont want to kep passing the pairs by value 2 times
        // (map->vector->sorting), I just keep pointers to map pairs
        // in vector and operate on the pointers only.

        using map_ptr_t = decltype(ring_member_frequency)::value_type const*;

        vector<map_ptr_t> sorted_frequencies;
        sorted_frequencies.reserve(ring_member_frequency.size());

        for (auto const& kvp: ring_member_frequency)
            sorted_frequencies.push_back(&kvp);


        std::sort(sorted_frequencies.begin(), sorted_frequencies.end(),
                  [](map_ptr_t const& left, map_ptr_t const&  right)
                  {
                      // types look complicated. so 'left' is
                      // referenece to 'const pointer' which points to 'const map_t'
                      return left->second > right->second;
                  });


        unique_ptr<csv::ofstream> csv_os3(new csv::ofstream {out_csv_file3.c_str()});

        if (!csv_os3->is_open())
            cerr << "Cant open file: " << out_csv_file3 << endl;

        // write the header of the csv file to be created
        *csv_os3 << "Output_pub_key" << "Frequency" << NEWLINE;

        cout << "\nMost frequent outputs used as ring members are:\n";

        size_t i {0};

        for (map_ptr_t& kvp: sorted_frequencies)
        {

            if (++i < 10) // dont show more than ten. all of them are in output csv.
                cout << " - " << kvp->first << ": " << kvp->second << '\n';

            if  (csv_os3->is_open())
                *csv_os3 << epee::string_tools::pod_to_hex(kvp->first) << kvp->second << NEWLINE;
        }

        if (csv_os3->is_open())
            csv_os3->close();

    } // if (ring_members)


    cout << "\nEnd of program." << '\n';

    return 0;
}
