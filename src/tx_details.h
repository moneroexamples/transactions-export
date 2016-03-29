//
// Created by mwo on 14/11/15.
//

#ifndef XMR2CSV_TXDATA_H
#define XMR2CSV_TXDATA_H



#include "../ext/minicsv.h"

#include "monero_headers.h"
#include "tools.h"

#include <iomanip>
#include <sstream>

namespace xmreg
{

    using namespace cryptonote;
    using namespace crypto;
    using namespace std;


    struct transfer_details
    {
        uint64_t m_block_height;
        uint64_t m_block_timestamp;
        transaction m_tx;
        crypto::hash payment_id;
        size_t m_internal_output_index;
        public_key out_pub_key;
        key_image key_img;
        bool m_spent;

        crypto::hash tx_hash() const;

        uint64_t amount() const;
    };


    ostream&
    operator<<(ostream& os, const transfer_details& dt);


    vector<xmreg::transfer_details>
    get_belonging_outputs(const block& blk,
                          const transaction& tx,
                          const secret_key& private_view_key,
                          const public_key& public_spend_key,
                          uint64_t block_height = 0);


    bool
    get_payment_id(const transaction& tx,
                   const account_public_address& addr,
                   crypto::hash& payment_id);

    bool
    get_payment_id(const transaction& tx,
                   crypto::hash& payment_id);

}

template<>
csv::ostringstream&
operator<<(csv::ostringstream& ostm, const xmreg::transfer_details& td);


#endif //XMR2CSV_TXDATA_H
