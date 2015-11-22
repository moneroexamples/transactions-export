//
// Created by marcin on 5/11/15.
//

#ifndef XMREG01_TOOLS_H
#define XMREG01_TOOLS_H

#define PATH_SEPARARTOR '/'

#include "monero_headers.h"
#include "tx_details.h"

#include "../ext/dateparser.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <string>
#include <vector>

/**
 * Some helper functions used in the example.
 * Names are rather self-explanatory, so I think
 * there is no reason for any detailed explanations here
 */
namespace xmreg
{
    using namespace cryptonote;
    using namespace crypto;
    using namespace std;

    namespace bf = boost::filesystem;
    namespace pt = boost::posix_time;
    namespace gt = boost::gregorian;

    template <typename T>
    bool
    parse_str_secret_key(const string& key_str, T& secret_key);


    bool
    get_tx_pub_key_from_str_hash(Blockchain& core_storage,
                             const string& hash_str,
                             transaction& tx);

    bool
    parse_str_address(const string& address_str,
                      account_public_address& address);

    inline bool
    is_separator(char c);

    string
    print_address(const account_public_address& address);

    string
    remove_trailing_path_separator(const string& in_path);

    bf::path
    remove_trailing_path_separator(const bf::path& in_path);

    string
    timestamp_to_str(time_t timestamp, const char* format = "%F %T");


    string
    get_default_lmdb_folder();

    bool
    generate_key_image(const crypto::key_derivation& derivation,
                       const std::size_t output_index,
                       const crypto::secret_key& sec_key,
                       const crypto::public_key& pub_key,
                       crypto::key_image& key_img);

    bool
    get_blockchain_path(const boost::optional<string>& bc_path,
                        bf::path& blockchain_path);


    inline void
    enable_monero_log() {
        uint32_t log_level = 0;
        epee::log_space::get_set_log_detalisation_level(true, log_level);
        epee::log_space::log_singletone::add_logger(LOGGER_CONSOLE, NULL, NULL);
    }


    uint64_t
    estimate_bc_height(const string& date, const char* format = "%Y-%m-%d");



}

#endif //XMREG01_TOOLS_H
