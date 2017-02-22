Yuchen Cai
CSE303
Assignment #5 security

Testing Details:

	- running environment: sunlab

	- Instructions on each task for running and testing


Task #1: 

	server:
		./obj64/server -p 1234

	client:
		./obj64/client -P test2.txt -s 128.180.120.71 -p 1234
		./obj64/client -G test2.txt -s 128.180.120.71  -p 1234 -S get1.txt


Task #2:

	server:
		./obj64/server -p 1234

	client:
		./obj64/client -P test2.txt -C -s 128.180.120.71 -p 1234
		./obj64/client -G test2.txt -C -s 128.180.120.71  -p 1234 -S get2.txt


Task #3:

	server:
		./obj64/server -p 1234 -l 3

	client:
		./obj64/client -P test2.txt -s 128.180.120.71 -p 1234 
		./obj64/client -P test3.txt -C -s 128.180.120.71 -p 1234
		./obj64/client -P test1.txt -C -s 128.180.120.71 -p 1234
		./obj64/client -P test4.txt -s 128.180.120.71 -p 1234
		./obj64/client -G test3.txt -C -s 128.180.120.71  -p 1234 -S get3.txt
		./obj64/client -G test4.txt -s 128.180.120.71  -p 1234 -S get4.txt


Task #4:
	
	server:
		./obj64/server -p 1234 -l 3
		./obj64/server -p 1234

	client:
		./obj64/client -P test1.txt -C -e -s 128.180.120.71 -p 1234
		./obj64/client -G test1.txt -e -C -s 128.180.120.71 -p 1234 -S getFILE1.txt


		./obj64/client -P test2.txt -C -e -s 128.180.120.71 -p 1234
		./obj64/client -G test2.txt -e -C -s 128.180.120.71 -p 1234 -S getFILE2.txt

		./obj64/client -P test3.txt -e -s 128.180.120.71 -p 1234
		./obj64/client -G test3.txt -e -C -s 128.180.120.71 -p 1234 -S getFILE3.txt

 