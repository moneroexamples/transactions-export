//
// Created by mwo on 5/11/15.
//

#ifndef XMREG01_MICROCORE_H
#define XMREG01_MICROCORE_H

#include <iostream>

#include "monero_headers.h"
#include "tx_details.h"



namespace xmreg
{
    using namespace cryptonote;
    using namespace crypto;
    using namespace std;

    /**
     * Micro version of cryptonode::core class
     * Micro version of constructor,
     * init and destructor are implemted.
     *
     * Just enough to read the blockchain
     * database for use in the example.
     */
    class MicroCore {

        tx_memory_pool m_mempool;
        Blockchain m_blockchain_storage;

    public:
        MicroCore();

        bool
        init(const string& blockchain_path);

        Blockchain&
        get_core();

        bool
        get_block_by_height(const uint64_t& height, block& blk);

        bool
        get_tx(const crypto::hash& tx_hash, transaction& tx);


        virtual ~MicroCore();
    };

}



#endif //XMREG01_MICROCORE_H
