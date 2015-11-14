//
// Created by marcin on 5/11/15.
//

#ifndef XMREG01_TOOLS_H
#define XMREG01_TOOLS_H

#define PATH_SEPARARTOR '/'

#include "monero_headers.h"
#include "tx_details.h"

#include <boost/filesystem.hpp>

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

}

#endif //XMREG01_TOOLS_H
