//
// Created by mwo on 6/11/15.
//

#include "CmdLineOptions.h"


namespace xmreg
{
    /**
     * Take the acc and *avv[] from the main() and check and parse
     * all the options given
     */
    CmdLineOptions::CmdLineOptions(int acc, const char *avv[]) {

        positional_options_description p;

        options_description desc(
                "xmr2csv, export all your transactions into csv file");

        desc.add_options()
                ("help,h", value<bool>()->default_value(false)->implicit_value(true),
                 "produce help message")
                ("address,a", value<string>(),
                 "monero address string")
                ("viewkey,v", value<string>(),
                 "private view key string")
                ("spendkey,s", value<string>(),
                 "private spend key string")
                ("start-height,t", value<size_t>(),
                 "start from given height")
                ("stop-height,g", value<size_t>()->default_value(0),
                 "stop scan at the given height. 0 is scan till end")
                ("no-of-blocks,n", value<size_t>()->default_value(0),
                 "number of blocks to search starting from start-height (0 = all blocks)")
                ("start-date,d", value<string>(),
                 "start roughly from given date: yyyy-mm-dd")
                ("out-csv-file,c", value<string>()->default_value("xmr_report.csv"),
                 "name of outputs csv file")
                ("out-csv-file2,r", value<string>()->default_value("xmr_report_ring_members.csv"),
                 "name of outputs csv file for file containing  out outputs as ring members")
                ("out-csv-file3,r", value<string>()->default_value("xmr_report_ring_members_freq.csv"),
                 "name of outputs csv file for file containing frequencies of outputs as ring members")
                ("out-csv-file4,r", value<string>()->default_value("xmr_report_key_images_outputs.csv"),
                 "name of outputs csv file for file containing all key "
                         "images scanned with the referenced output public keys")
                ("bc-path,b", value<string>(),
                 "path to lmdb blockchain")
                ("testnet",  value<bool>()->default_value(false)->implicit_value(true),
                 "is the address from testnet network")
                ("stagenet",  value<bool>()->default_value(false)->implicit_value(true),
                 "is the address from stagenet network")
                ("ring-members,m",  value<bool>()->default_value(false)->implicit_value(true),
                 "search where our outputs are as ring members")
                ("all-outputs",  value<bool>()->default_value(false)->implicit_value(true),
                 "save all outputs, whether they are ours or not")
                ("all-key-images",  value<bool>()->default_value(false)->implicit_value(true),
                "save all key_images, whether they are ours or not, with referenced output public keys");


        store(command_line_parser(acc, avv)
                          .options(desc)
                          .run(), vm);

        notify(vm);

        if (vm.count("help"))
        {
            if (vm["help"].as<bool>())
                cout << desc << "\n";
        }


    }

    /**
     * Return the value of the argument passed to the program
     * in wrapped around boost::optional
     */
    template<typename T>
    boost::optional<T>
    CmdLineOptions::get_option(const string & opt_name) const
    {

        if (!vm.count(opt_name))
        {
            return boost::none;
        }

        return vm[opt_name].as<T>();
    }


    // explicit instantiations of get_option template function
    template  boost::optional<string>
    CmdLineOptions::get_option<string>(const string & opt_name) const;

    template  boost::optional<bool>
            CmdLineOptions::get_option<bool>(const string & opt_name) const;

    template  boost::optional<size_t>
            CmdLineOptions::get_option<size_t>(const string & opt_name) const;

}
