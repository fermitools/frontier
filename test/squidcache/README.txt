Updated Aug 2, 2004

Included in this area are a set of scripts and files used to run a test which tests filling the squid cache with CDF database information, from tables with CID's. There are threee parts:

1. A program which uses DCOracle to access the database to create a list of tables and valid CIDs for each.
2. An archived version of the list from step 1, in python pickle format (*.pkl file). the one Used in the test was cdf_cid_table_list_1.pkl
3. The script which executes the test called squidcache.py. To use the generic file as input, the following command should work:
./squidcache.py -o output.dat cdfdbfrontier.fnal.gov 8000 cdf_cid_table_list_1.pkl -1

Note: the -1 at the end means run until finished with all CIDs, otherwise you can put a number which will be the maximum number of objects to retrieve, useful for testing to see if things are working.

There is also an "R" file which is an example of an R analysis script which reads in the output from 3, and generates a plot. You must have the R program installed to use this. 
