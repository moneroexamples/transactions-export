//
// Created by mwo on 14/11/15.
//

#include "tx_details.h"


namespace xmreg
{



    crypto::hash
    transfer_details::tx_hash() const
    {
        return get_transaction_hash(m_tx);
    };


    public_key
    transfer_details::tx_pub_key() const
    {
        return get_tx_pub_key_from_extra(m_tx);
    };


    uint64_t
    transfer_details::amount() const
    {
        uint64_t xmr_amount = m_tx.vout[m_internal_output_index].amount;

        return xmr_amount;
    }


    uint64_t
    transfer_details::amount(secret_key prv_view_key) const
    {
        uint64_t xmr_amount = m_tx.vout[m_internal_output_index].amount;

        // cointbase txs have amounts in plain sight.
        // so use amount from ringct, only for non-coinbase txs
        if (m_tx.version > 1 && !is_coinbase(m_tx))
        {
            uint64_t rct_amount = 0;

            bool r;

            public_key addr_pub_view_key = m_addr.m_view_public_key;

            uint64_t out_idx = m_internal_output_index;

            rct::key mask = m_tx.rct_signatures.ecdhInfo[out_idx].mask;

            r = decode_ringct(m_tx.rct_signatures,
                              get_tx_pub_key_from_extra(m_tx),
                              prv_view_key,
                              out_idx,
                              mask,
                              rct_amount);

            if (!r)
            {
                cerr << "Cant decode ringCT!" << endl;
            }

            xmr_amount = rct_amount;
        }

        return xmr_amount;
    }



    ostream&
    operator<<(ostream& os, const transfer_details& td)
    {
        os << "Block: "        << td.m_block_height
           << " time: "        << timestamp_to_str(td.m_block_timestamp)
           << " tx hash: "     << td.tx_hash()
           << " tx pub key: "  << td.tx_pub_key()
           << " out idx: "     << td.m_internal_output_index
           << " out pk:  "     << td.out_pub_key
           << " key img:  "    << td.key_img
           << " amount: "      << td.m_amount; //print_money(td.amount());

        return os;
    }




    /**
     * Get tx outputs associated with the given private view and public spend keys
     *
     *
     */
    vector<xmreg::transfer_details>
    get_belonging_outputs(const block& blk,
                          const transaction& tx,
                          const account_public_address& addr,
                          const secret_key& private_view_key,
                          uint64_t block_height)
    {
        // vector to be returned
        vector<xmreg::transfer_details> our_outputs;


        // get transaction's public key
        public_key pub_tx_key = get_tx_pub_key_from_received_outs(tx);

        // check if transaction has valid public key
        // if no, then skip
        if (pub_tx_key == null_pkey)
        {
            return our_outputs;
        }



        // get tx payment id
        crypto::hash payment_id;

        if (!xmreg::get_payment_id(tx, payment_id))
        {
            payment_id = null_hash;
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
            cerr << "Cant get dervied key for: "  << "\n"
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
                              addr.m_spend_public_key,
                              pubkey);

            // get tx output public key
            const txout_to_key tx_out_to_key
                    = boost::get<txout_to_key>(tx.vout[i].target);

            // check if generated public key matches the current output's key
            bool mine_output = (tx_out_to_key.key == pubkey);

            // check if the output's public key is ours
            if (mine_output)
            {
                uint64_t xmr_amount = tx.vout[i].amount;

                // if mine output has RingCT, i.e., tx version is 2
                // need to decode its amount. otherwise its zero.
                if (mine_output && tx.version > 1)
                {
                    // initialize with regular amount value
                    // for ringct, except coinbase, it will be 0
                    uint64_t rct_amount_val = xmr_amount;

                    if (!is_coinbase(tx))
                    {
                        bool r;

                        rct::key mask = tx.rct_signatures.ecdhInfo[i].mask;

                        r = decode_ringct(tx.rct_signatures,
                                          pub_tx_key,
                                          private_view_key,
                                          i,
                                          mask,
                                          rct_amount_val);

                        if (!r) {
                            cerr << "Cant decode ringCT!" << endl;
                            throw std::runtime_error("Cant decode ringCT!");
                        }

                        xmr_amount = rct_amount_val;
                    }

                }  //  if (mine_output && tx.version > 1)


                // if so, then add this output to the
                // returned vector
                //our_outputs.push_back(tx.vout[i]);
                our_outputs.push_back(
                        xmreg::transfer_details {addr,
                                                 block_height,
                                                 blk.timestamp,
                                                 xmr_amount,
                                                 tx,
                                                 payment_id,
                                                 i,
                                                 tx_out_to_key.key,
                                                 key_image{},
                                                 false}
                );

                xmreg::transfer_details& last =  our_outputs.back();

                last.m_amount = last.amount(private_view_key);

            } // if (mine_output)

        } //  for (size_t i = 0; i < output_no; ++i)

        return our_outputs;
    }



    /**
     * Get tx outputs associated with the given private view and public spend keys
     *
     *
     */
    vector<xmreg::transfer_details>
    get_outputs(const block& blk,
                          const transaction& tx,
                          uint64_t block_height)
    {
        // vector to be returned
        vector<xmreg::transfer_details> our_outputs;


        // get transaction's public key
        public_key pub_tx_key = get_tx_pub_key_from_extra(tx);

        // check if transaction has valid public key
        // if no, then skip
        if (pub_tx_key == null_pkey)
        {
            return our_outputs;
        }



        // get tx payment id
        crypto::hash payment_id;

        if (!xmreg::get_payment_id(tx, payment_id))
        {
            payment_id = null_hash;
        }

        // get the total number of outputs in a transaction.
        size_t output_no = tx.vout.size();

        // check if the given transaction has any outputs
        // if no, then finish
        if (output_no == 0)
        {
            return our_outputs;
        }



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

            // get tx output public key
            const txout_to_key tx_out_to_key
                    = boost::get<txout_to_key>(tx.vout[i].target);

            // if so, then add this output to the
            // returned vector
            //our_outputs.push_back(tx.vout[i]);
            our_outputs.push_back(
                    xmreg::transfer_details {account_public_address(),
                                             block_height,
                                             blk.timestamp,
                                             tx.vout[i].amount,
                                             tx,
                                             payment_id,
                                             i,
                                             tx_out_to_key.key,
                                             key_image{},
                                             false}
            );
        }

        return our_outputs;
    }


    bool
    get_payment_id(const transaction& tx,
                   const account_public_address& addr,
                   crypto::hash& payment_id)
    {

        payment_id = null_hash;

        //crypto::secret_key secret_key = get_tx_pub_key_from_extra(tx);

        std::vector<tx_extra_field> tx_extra_fields;

        if(!parse_tx_extra(tx.extra, tx_extra_fields))
        {
            return false;
        }

        tx_extra_nonce extra_nonce;

        if (find_tx_extra_field_by_type(tx_extra_fields, extra_nonce))
        {
//            crypto::hash8 payment_id8 = null_hash8;
//
//            if(get_encrypted_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id8))
//            {
//                if (decrypt_payment_id(payment_id8, addr.m_view_public_key, ptx.tx_key))
//                {
//                    memcpy(payment_id.data, payment_id8.data, 8);
//                }
//            }
//            else if (!get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id))
//            {
//                payment_id = cryptonote::null_hash;
//            }

            if (!get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id))
            {
                return false;
            }
        }
        return true;
    }

    bool
    get_payment_id(const transaction& tx,
                   crypto::hash& payment_id)
    {

        payment_id = null_hash;

        //crypto::secret_key secret_key = get_tx_pub_key_from_extra(tx);

        std::vector<tx_extra_field> tx_extra_fields;

        if(!parse_tx_extra(tx.extra, tx_extra_fields))
        {
            return false;
        }

        tx_extra_nonce extra_nonce;

        if (find_tx_extra_field_by_type(tx_extra_fields, extra_nonce))
        {
            if (!get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id))
            {
                return false;
            }
        }
        return true;
    }


}

template<>
csv::ofstream&
operator<<(csv::ofstream& ostm, const xmreg::transfer_details& td)
{

    std::stringstream ss;

    // get strings to remove "<" and ">" from begining and end of hashes
    ss << td.tx_hash();
    std::string tx_hash_str = ss.str();

    ss.str(std::string());

    // get tx pub key str
    ss << td.tx_pub_key();
    std::string tx_pub_key_str = ss.str();

    ss.str(std::string());

    // get strings to remove "<" and ">" from begining and end of hashes
    ss << td.payment_id;
    std::string payment_id_str = ss.str();

    ss.str(std::string());

    // get strings to remove "<" and ">" from begining and end of keys
    ss << td.out_pub_key;
    std::string out_pk_str = ss.str();

    ss.str(std::string());

    // get strings for key_image
    ss << td.key_img;
    std::string key_img = ss.str();


    ostm << xmreg::timestamp_to_str(td.m_block_timestamp);
    ostm << td.m_block_height;
    ostm << tx_hash_str.substr(1, tx_hash_str.length()-2);
    ostm << tx_pub_key_str.substr(1, tx_hash_str.length()-2);
    ostm << td.m_tx.version;
    ostm << payment_id_str.substr(1, tx_hash_str.length()-2);
    ostm << td.m_internal_output_index;
    //ostm << cryptonote::print_money(td.m_amount);
    ostm << td.m_amount;
    ostm << out_pk_str.substr(1, out_pk_str.length()-2);
    ostm << key_img.substr(1, out_pk_str.length()-2);
    ostm << td.m_spent;

    return ostm;
}
