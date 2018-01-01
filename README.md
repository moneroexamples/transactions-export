# Exporting our transactions from blockchain and searching for ring members into csv files

In this example, it is shown how to export our transactions from the blockchain
into a csv file. This can be very useful
for making summaries or reports of our incoming and outgoing (when spend key is given)
 transactions.

Also it shows how we can search in which transactions used our outputs as
its ring members.



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
git checkout -b last_release v0.11.1.0

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
  -h [ --help ] [=arg(=1)] (=0)         produce help message
  -a [ --address ] arg                  monero address string
  -v [ --viewkey ] arg                  private view key string
  -s [ --spendkey ] arg                 private spend key string
  -t [ --start-height ] arg             start from given height
  -d [ --start-date ] arg               start roughly from given date:
                                        yyyy-mm-dd
  -c [ --out-csv-file ] arg             name of outputs csv file
  -r [ --out-csv-file2 ] arg            name of outputs csv file for file
                                        containing  out outputs as ring members
  -b [ --bc-path ] arg                  path to lmdb blockchain
  --testnet [=arg(=1)] (=0)             is the address from testnet network
  -m [ --ring-members ] [=arg(=1)] (=0) search where our outputs are as ring
                                        members
  --all-outputs [=arg(=1)] (=0)         output all outputs, whether they are
                                        ours or not
```

## Getting our outputs using address and viewkey

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
./xmr2csv -a 44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A -v f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501 -c ./forum.csv
```

[Old Monero donation](https://github.com/monero-project/monero/pull/714/files) address:

- Address: 46BeWrHpwXmHDpDEUmZBWZfoQpdc6HaERCNmx1pEYL2rAcuwufPN9rXHHtyUA4QVy66qeFQkn6sfK8aHYjA3jk3o1Bv16em 
- Viewkey: e422831985c9205238ef84daf6805526c14d96fd7b059fe68c7ab98e495e5703

```bash
./xmr2csv -a 46BeWrHpwXmHDpDEUmZBWZfoQpdc6HaERCNmx1pEYL2rAcuwufPN9rXHHtyUA4QVy66qeFQkn6sfK8aHYjA3jk3o1Bv16em -v e422831985c9205238ef84daf6805526c14d96fd7b059fe68c7ab98e495e5703 -c ./old.csv
```



## Searching txs which use our outputs as ring members and their frequency

Just add `-m` flag. This will produce `xmr_report_ring_members.csv` and `xmr_report_ring_members_frew.csv` files
 (default names) which
contain the list of txs which use our outputs, and frequency of outputs use, respectively. To speed up the search
we can use `-t` flag to specify starting block.

#### Example 1 (Monero project donation address)

```bash
./xmr2csv -t 550000 -m -a 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp -v c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c
```

Example frequency list output:

```
Output_pub_key	Frequency
3ae472a405ffb000fa14cb380a5408cdebe25610ab16ced17cc9cf601dcf3860	10
1a163937a399ff6712806eec5c5ed0701ae2a8718d8fcc583d23798202f952f0	10
04782e24ca5c944f0d4bb764fd3b95c6b6f59144ee2a3c7530a2269e4a19a650	10
6c04b7393d223b4c3b35493219c6a4d9bb37b6739c9a22193394d5e81964b5a3	9
74f56217c41d98026f7c063c574eb346cc1ff6948f29f5f11f625385d6adccfb	9
d5a68831eafb592b8f9b71b916c1583fd4ac99ac8d1767d30170875a42abf3e7	9
ca3937ba591d68f4dc87bdd180a141dcebd1584dcdde4736af691a98b5d9b97e	9
960efddca6a706ed70b6b17ac0799f15efe88d9f26ae5aa235a6f16995651288	9
da3cbc0bb751e938d058c556f3c1a3ce215231c04d834f99e6a603c67d17648a	9
f38102f85dfd72326c533c7d8c47559f49648ab6a3f7e7413819262be312df08	8
ab3b1e10466706d6a4b1dde91a3243e534d95d54bee391edc313c43c842b0eff	8
99e55c3c4e75b9880db36489eb70299c8d7dff637613eed91ebca7338e713052	8
6e8c3e47fcfb91e4d6bd9e19f2d42a0a0ec3836116034423d3b9aa4b5802efc6	8
d6914592b30902f77e563ff38ccfc30c70c95395981246d8d26bf17a1575fadb	8
958a0535ef3a2ce61a683dd35996ecdf2f3139c5dbc53e37033f13849bab9221	8
fdbd02a04b27393391c49f9fbf17eb0aa2fac05997f214339b482edfa1746a37	8
bf989786802686c6be141831f9973e2108fd84c24552e38e3ef29b139513d9d9	8
a19b6874392162abdb26a3ac64618450a1a850abc3ebe1ac72ef8a3092b26c53	8
39ea511eaa1b90ada153ae62cbb986411f601e71fed6582528b84723f10dd5c8	8
```

#### Example 2 (Monerujo project donation address)


```bash
./xmr2csv -t 01381000 -m -a 4AdkPJoxn7JCvAby9szgnt93MSEwdnxdhaASxbTBm6x5dCwmsDep2UYN4FhStDn5i11nsJbpU7oj59ahg8gXb1Mg3viqCuk -v b1aff2a12191723da0afbe75516f94dd8b068215f6e847d8da57aca5f1f98e0c
```

Example frequency list output:

```
Most frequent outputs used as ring members are:
 - <024a777f20a02a3c145c27e8dd53d05c708b78e1c9832f6b843acfd29c6ac469>: 8
 - <b50f169d04c4e74c35437efc9da832091737c61a8e990182951758a2702d6080>: 7
 - <ff00dc60cbc599e520d33d5bbc999daf1f4e30479e558b425ef8d092809af5d5>: 5
 - <2dd2a007071b0a612d601d2fa1989ee0cdc351ef2db21519697f9b3b03839e68>: 5
 - <64d6394f2f11142ef04dc67bb7bd2ae5e0ff0f11d0a211e4fe28d58dfdd6d6e3>: 5
 - <673f5784b7ed703c206fea8a9076b356b93e76e7a26f55b558626434f5268c0f>: 5
 - <95569a43937da8ceca8598535f9029c82249bec0ef5ef0890a52086e566fcc85>: 4
 - <1b92b3e6c650c248a4865a95a15e1d264037b4068c7053119d5288aa70e60c8f>: 4
 - <643687fa00850056492dd2da90c720a5daff590745c6ab331210163f663e0bf8>: 4

## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.










