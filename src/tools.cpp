//
// Created by marcin on 5/11/15.
//

#include "tools.h"

namespace xmreg
{


    /**
     * Parse key string, e.g., a viewkey in a string
     * into crypto::secret_key or crypto::public_key
     * depending on the template argument.
     */
    template <typename T>
    bool
    parse_str_secret_key(const string& key_str, T& secret_key)
    {

        // hash and keys have same structure, so to parse string of
        // a key, e.g., a view key, we can first parse it into the hash
        // object using parse_hash256 function, and then copy the reslting
        // hash data into secret key.
        crypto::hash hash_;

        if(!parse_hash256(key_str, hash_))
        {
            cerr << "Cant parse a key (e.g. viewkey): " << key_str << endl;
            return false;
        }

        // crypto::hash and crypto::secret_key have basicly same
        // structure. They both keep they key/hash as c-style char array
        // of fixed size. Thus we can just copy data from hash
        // to key
        copy(begin(hash_.data), end(hash_.data), secret_key.data);

        return true;
    }

    // explicit instantiations of get template function
    template bool parse_str_secret_key<crypto::secret_key>(const string& key_str, crypto::secret_key& secret_key);
    template bool parse_str_secret_key<crypto::public_key>(const string& key_str, crypto::public_key& secret_key);


    /**
     * Get transaction tx using given tx hash. Hash is represent as string here,
     * so before we can tap into the blockchain, we need to pare it into
     * crypto::hash object.
     */
    bool
    get_tx_pub_key_from_str_hash(Blockchain& core_storage, const string& hash_str, transaction& tx)
    {
        crypto::hash tx_hash;
        parse_hash256(hash_str, tx_hash);

        try
        {
            // get transaction with given hash
            tx = core_storage.get_db().get_tx(tx_hash);
        }
        catch (const TX_DNE& e)
        {
            cerr << e.what() << endl;
            return false;
        }

        return true;
    }

    /**
     * Parse monero address in a string form into
     * cryptonote::account_public_address object
     */
    bool
    parse_str_address(const string& address_str, account_public_address& address)
    {

        if (!get_account_address_from_str(address, false, address_str))
        {
            cerr << "Error getting address: " << address_str << endl;
            return false;
        }

        return true;
    }


    /**
     * Return string representation of monero address
     */
    string
    print_address(const account_public_address& address)
    {
        return get_account_address_as_str(false, address);
    }



    /**
     * Check if a character is a path seprator
     */
    inline bool
    is_separator(char c)
    {
        // default linux path separator
        const char separator = PATH_SEPARARTOR;

        return c == separator;
    }



    /**
     * Remove trailinig path separator.
     */
    string
    remove_trailing_path_separator(const string& in_path)
    {
        string new_string = in_path;
        if (!new_string.empty() && is_separator(new_string[new_string.size() - 1]))
            new_string.erase(new_string.size() - 1);
        return new_string;
    }

    bf::path
    remove_trailing_path_separator(const bf::path& in_path)
    {
        string path_str = in_path.native();
        return bf::path(remove_trailing_path_separator(path_str));
    }



    /**
     * Get tx outputs associated with the given private view and public spend keys
     *
     *
     */
    vector<tx_out>
    get_belonging_outputs(const transaction& tx,
                          const secret_key& private_view_key,
                          const public_key& public_spend_key)
    {
        // vector to be returned
        vector<tx_out> our_outputs;


        // get transaction's public key
        public_key pub_tx_key = get_tx_pub_key_from_extra(tx);

        // check if transaction has valid public key
        // if no, then skip
        if (pub_tx_key == null_pkey)
        {
            return our_outputs;
        }


        // get the total number of outputs in a transaction.
        size_t output_no = tx.vout.size();

        // check if the given transaction has any outputs
        // if no, then finish
        if (output_no == 0)
        {
            return our_outputs;
        }


        // public transaction key is combined with our viewkey
        // to create, so called, derived key.
        key_derivation derivation;

        if (!generate_key_derivation(pub_tx_key, private_view_key, derivation))
        {
            cerr << "Cant get dervied key for: " << "\n"
            << "pub_tx_key: " << private_view_key << " and "
            << "prv_view_key" << private_view_key << endl;
            return our_outputs;
        }


        // each tx that we (or the address we are checking) received
        // contains a number of outputs.
        // some of them are ours, some not. so we need to go through
        // all of them in a given tx block, to check which outputs are ours.



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
           public_key pubkey;

           derive_public_key(derivation,
                                      i,
                                      public_spend_key,
                                      pubkey);

            // get tx output public key
            const txout_to_key tx_out_to_key
                    = boost::get<txout_to_key>(tx.vout[i].target);


            //cout << "Output no: " << i << ", " << tx_out_to_key.key;

            // check if the output's public key is ours
            if (tx_out_to_key.key == pubkey)
            {
                // if so, then add this output to the
                // returned vector
                our_outputs.push_back(tx.vout[i]);
            }
        }

        return our_outputs;
    }

//    inline
//    vector<tx_out>
//    get_belonging_outputs(const transaction& tx,
//                          const account_public_address& address)
//    {
//        return  get_belonging_outputs(tx, address.)
//    }
//
//



}
