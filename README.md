# Export our transactions from blockchain into csv file

In this example, it is shown how to export our transactions from the blockchain
into a csv file. This can be very useful
for making summaries or reports of our incoming and outgoing (when spend key is given)
 transactions.


## Prerequisites

The code was written and tested on Ubuntu 16.04/10 x86_64.

Instruction for Monero compilation:
 - [Ubuntu 16.04 x86_64](https://github.com/moneroexamples/compile-monero-09-on-ubuntu-16-04/)

The Monero C++ development environment was set as shown in the above link.

## C++ code

Only main loop from `main.cpp` shown.

```C++

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

        vector<xmreg::transfer_details> found_outputs;

        if (all_outputs == false)
        {
            // outout only our outputs
            found_outputs = xmreg::get_belonging_outputs(
                    blk, tx, address, prv_view_key, i);
        }
        else
        {
            found_outputs = xmreg::get_outputs(blk, tx, i);
        }


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
                    tr_details.m_spent = core_storage->have_tx_keyimg_as_spent(key_img);

                  } // if (SPEND_KEY_GIVEN)


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

            uint64_t xmr_amount = tx_in_to_key.amount;

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
'''


## How to compile

Assuming Monero headers and libraries set up as shown here: 

- https://github.com/moneroexamples/compile-monero-09-on-ubuntu-16-04

```bash
git clone https://github.com/moneroexamples/transactions-export.git
cd transactions-export
mkdir build && cd build
cmake ..
make
```

## Compile this example

If so then to download and compile this example, the following
steps can be executed:

```bash
# download the source code
git clone https://github.com/moneroexamples/transactions-export.git

# enter the downloaded sourced code folder
cd transactions-export

# create the makefile
cmake .

# compile
make
```

## Program options
```bash
[mwo@arch cmake-build-debug]$ ./xmr2csv -h
xmr2csv, export all your transactions into csv file:
  -h [ --help ] [=arg(=1)] (=0) produce help message
  -a [ --address ] arg          monero address string
  -v [ --viewkey ] arg          private view key string
  -s [ --spendkey ] arg         private spend key string
  -t [ --start-height ] arg     start from given height
  -d [ --start-date ] arg       start roughly from given date: yyyy-mm-dd
  -c [ --out-csv-file ] arg     name of outputs csv file
  -b [ --bc-path ] arg          path to lmdb blockchain
  --testnet [=arg(=1)] (=0)     is the address from testnet network
  --all-outputs [=arg(=1)] (=0) output all outputs, whether they are ours or 
                                not
```

The Monero C++ development environment was set as shown in the following link:
- [Ubuntu 16.04 x86_64](https://github.com/moneroexamples/compile-monero-09-on-ubuntu-16-04/)

## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```










