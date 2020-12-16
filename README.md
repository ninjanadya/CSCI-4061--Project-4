# Information

* The purpose of this program is to take requests from clients via the server and fetch the request from the filesystem if that files is there.

* Compile the program using the make command as follows:
$ make

# Server Side
Then run this command to start the server:
$ ./web_server port path num_dispatch num_workers dynamic_flag qlen cache_entries

* @port is the port number
* @path is the directory from which the requests will be found (@path)/(@request_path)
* @num_dispatch is the number of dispatcher threads you want upon starting the server
* @num_workers is the number of worker threads you want upon starting the server
* @dynamic_flag is 0 if you don't want number of dispatcher/worker threads to change and 1 if you do
* @qlen is max queue length size for reqeusts from clients
* @cache_entries is 0 for no cache entries and 1 for keeping cache entries to retrieve files faster

# Client Side
Run this command from client side to make a request to the server:
$ wget --http-no-alive http://127.0.0.1:(@port)/(@request_path)

* --http-no-alive use this option if you don't want to extend the connection for a limited time since we don't have the extra credit implemented you have to use this option
* @port is the port number you passed in for the server
* @request_path is the path to the file the client is requesting

# How it works
* The program accepts the input as mentioned above at compilation and runs as a multithreaded webserver, waiting for requests to continue running. 
* Once a request is received and accepted, it is sent to a dispatcher which will queue up the request that will later be handled by the worker threads. 
* All of this occuring will also update a log file of the actions taken.

* Assumptions were made solely based on the ones in the Project 4 document.

# Contributions
* Team met via Zoom and worked on the code via a shared github repository
* Coding, testing, and debugging were performed by Sean and Nadya
* CSE Lab Machine testing and README.md writen by Nadya and Maxim

* test machine : athena.cla.umn.edu
* date : 12-16-2020
* name : Maxim Zabilo, Sean Berg, Nadya Postolaki
* x500 : zabil003, berg2007, posto018
