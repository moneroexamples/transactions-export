# Export our transactions from blockchain into csv file

In this example, it is shown how to export our transactions from the blockchain
into a csv file. This can be very useful
for making summaries or reports of our incoming and outgoing (when spend key is given)
 transactions.



## Compilation on Ubuntu 16.04

##### Compile latest Monero release v0.11

Download and compile recent Monero into your home folder:

```bash
# first install monero dependecines
sudo apt update

sudo apt install git build-essential cmake libboost-all-dev miniupnpc libunbound-dev graphviz doxygen libunwind8-dev pkg-config libssl-dev libcurl4-openssl-dev libgtest-dev libreadline-dev libzmq3-dev

# go to home folder
cd ~

git clone https://github.com/monero-project/monero

cd monero/

# checkout last monero version
git checkout -b last_release v0.11.0.0

make
```


##### Compile the transactions-export

Once the Monero is compiles, the transactions-export can be downloaded and compiled
as follows:

```bash
# go to home folder if still in ~/monero
cd ~

# download the source code
https://github.com/moneroexamples/transactions-export.git

# enter the downloaded sourced code folder
cd transactions-export

# make a build folder and enter it
mkdir build && cd build

# create the makefile
cmake ..

# altearnatively can use: cmake -DMONERO_DIR=/path/to/monero_folder ..
# if monero is not in ~/monero

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

## Testing address and viewkey

For testing of the viewer, one can use [official address and viewkey](https://github.com/monero-project/bitmonero#supporting-the-project)
of the Monero, i.e.,

- Address: 44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A
- Viewkey: f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501

```bash
./xmr2csv -a 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp -v c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c -c ./current.csv 
```

Monero [forum donation](https://www.reddit.com/r/Monero/comments/5j2rm7/in_last_four_weekes_there_were_about_850_xmr/dbdmzt7/?context=3),

- Address: 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp
- Viewkey: c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c

```bash
./xmr2csv -a ./xmr2csv -a 44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A -v f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501 -c ./forum.csv 
```

[Old Monero donation](https://github.com/monero-project/monero/pull/714/files) address:

- Address: 46BeWrHpwXmHDpDEUmZBWZfoQpdc6HaERCNmx1pEYL2rAcuwufPN9rXHHtyUA4QVy66qeFQkn6sfK8aHYjA3jk3o1Bv16em 
- Viewkey: e422831985c9205238ef84daf6805526c14d96fd7b059fe68c7ab98e495e5703

```bash
./xmr2csv -a 46BeWrHpwXmHDpDEUmZBWZfoQpdc6HaERCNmx1pEYL2rAcuwufPN9rXHHtyUA4QVy66qeFQkn6sfK8aHYjA3jk3o1Bv16em -v e422831985c9205238ef84daf6805526c14d96fd7b059fe68c7ab98e495e5703 -c ./old.csv 
```


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.










