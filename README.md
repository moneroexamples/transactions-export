# Exporting our transactions from blockchain and searching for ring members into csv files

In this example, it is shown how to export our transactions from the blockchain
into a csv file. This can be very useful
for making summaries or reports of our incoming and outgoing (when spend key is given)
 transactions.

Also it shows how we can search in which transactions used our outputs as
its ring members.



## Compilation on Ubuntu 20.04

##### Compile latest Monero 

Download and compile recent Monero into your home folder as shown in the following link:

https://github.com/moneroexamples/monero-compilation#example-compilation-of-master-branch-ie-development-version-of-monero-on-ubuntu-2004

##### Compile the transactions-export

Once the Monero is compiles, the transactions-export can be downloaded and compiled
as follows:

```bash
# go to home folder if still in ~/monero
cd ~

# download the source code
git clone https://github.com/moneroexamples/transactions-export.git

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
  -n [ --no-of-blocks ] arg (=0)        number of blocks to search starting 
                                        from start-height
  -d [ --start-date ] arg               start roughly from given date: 
                                        yyyy-mm-dd
  -c [ --out-csv-file ] arg (=xmr_report.csv)
                                        name of outputs csv file
  -r [ --out-csv-file2 ] arg (=xmr_report_ring_members.csv)
                                        name of outputs csv file for file 
                                        containing  out outputs as ring members
  -r [ --out-csv-file3 ] arg (=xmr_report_ring_members_freq.csv)
                                        name of outputs csv file for file 
                                        containing frequencies of outputs as 
                                        ring members
  -r [ --out-csv-file4 ] arg (=xmr_report_key_images_outputs.csv)
                                        name of outputs csv file for file 
                                        containing all key images scanned with 
                                        the referenced output public keys
  -b [ --bc-path ] arg                  path to lmdb blockchain
  --testnet [=arg(=1)] (=0)             is the address from testnet network
  -m [ --ring-members ] [=arg(=1)] (=0) search where our outputs are as ring 
                                        members
  --all-outputs [=arg(=1)] (=0)         save all outputs, whether they are ours
                                        or not
  --all-key-images [=arg(=1)] (=0)      save all key_images, whether they are 
                                        ours or not, with referenced output 
                                        public keys
```

## Getting our outputs using address and viewkey

For testing of the viewer, one can use [official address and viewkey](https://github.com/monero-project/bitmonero#supporting-the-project)
of the Monero, i.e.,

- Address: 44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A
- Viewkey: f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501

```bash
./xmr2csv -a 44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A -v f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501 -c ./current.csv 
```

Monero [forum donation](https://www.reddit.com/r/Monero/comments/5j2rm7/in_last_four_weekes_there_were_about_850_xmr/dbdmzt7/?context=3),

- Address: 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp
- Viewkey: c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c

```bash
./xmr2csv -a 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp -v c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c -c ./forum.csv
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
./xmr2csv -t 00580000 -m -a 45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp -v c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c
```

Example frequency list output:

```
Most frequent outputs used as ring members are:
 - <6c04b7393d223b4c3b35493219c6a4d9bb37b6739c9a22193394d5e81964b5a3>: 12 times with ring sizes of 3_3_4_23_3_3_3_3_5_11_5_21
 - <da3cbc0bb751e938d058c556f3c1a3ce215231c04d834f99e6a603c67d17648a>: 11 times with ring sizes of 3_3_3_5_4_3_4_3_5_5_5
 - <d442396157c8bbf8175964c4de41f9a5f1b0afda9feca952e9837d82e8a0b8fd>: 11 times with ring sizes of 3_3_3_5_3_3_5_3_41_5_5
 - <51897c55c3b25daae30bf04cc8cebacddf559fb0cb6f4194f1e292931b57c29a>: 10 times with ring sizes of 4_3_3_3_3_5_10_3_3_5
 - <3ae472a405ffb000fa14cb380a5408cdebe25610ab16ced17cc9cf601dcf3860>: 10 times with ring sizes of 26_11_4_9_13_11_4_3_5_11
 - <04782e24ca5c944f0d4bb764fd3b95c6b6f59144ee2a3c7530a2269e4a19a650>: 10 times with ring sizes of 1_27_21_26_11_3_5_3_3_3
 - <435d2667c93dd7f5960d02cb920ee8db8fda693dd699e1c659703bf1fc359a86>: 10 times with ring sizes of 3_3_5_5_3_5_5_21_5_5
 - <0c6eec50b66c9a453ba6ad7b5228d3d224d187cd53ef215076d55d2347018969>: 10 times with ring sizes of 3_5_5_3_5_3_3_5_5_5
 - <253641711e995282ecd9182a862d474a782b2d642369dd4759e4c81dd8c117e7>: 10 times with ring sizes of 26_5_5_5_3_3_3_3_3_5
```

#### Example 2 (Monerujo project donation address)


```bash
./xmr2csv -t 01381000 -m -a 4AdkPJoxn7JCvAby9szgnt93MSEwdnxdhaASxbTBm6x5dCwmsDep2UYN4FhStDn5i11nsJbpU7oj59ahg8gXb1Mg3viqCuk -v b1aff2a12191723da0afbe75516f94dd8b068215f6e847d8da57aca5f1f98e0c
```

Example frequency list output:

```
Most frequent outputs used as ring members are:
 - <024a777f20a02a3c145c27e8dd53d05c708b78e1c9832f6b843acfd29c6ac469>: 10 times with ring sizes of 5_5_6_5_5_5_5_5_5_41
 - <64d6394f2f11142ef04dc67bb7bd2ae5e0ff0f11d0a211e4fe28d58dfdd6d6e3>: 7 times with ring sizes of 5_5_6_5_5_5_5
 - <a05963e393f65f225beccea21c0aab29db261ead252444da9063445c60fdc2c3>: 7 times with ring sizes of 5_5_26_5_5_5_5
 - <b50f169d04c4e74c35437efc9da832091737c61a8e990182951758a2702d6080>: 7 times with ring sizes of 5_3_3_5_5_5_21
 - <cc75ba4c950d62e6fe27235021e1c492d9a92bfe66fb90cc28225e7fd169a5dc>: 7 times with ring sizes of 6_5_5_5_5_5_5
 - <77328c5ea44e92c487205b24e10860bf73167907924b178923261dcb2d4b41d3>: 7 times with ring sizes of 5_5_5_5_5_5_5
 - <ff00dc60cbc599e520d33d5bbc999daf1f4e30479e558b425ef8d092809af5d5>: 6 times with ring sizes of 5_5_6_3_5_5
 - <d836c0eddcee6e2f0b0b6f4d541194a70502dbcbd9c2b6858b6d7f035f163627>: 6 times with ring sizes of 5_5_5_9_5_5
 - <1b92b3e6c650c248a4865a95a15e1d264037b4068c7053119d5288aa70e60c8f>: 6 times with ring sizes of 5_5_5_5_5_21
```

## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.










