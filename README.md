## Domino Webserver


This project is for sending and retrieving files on a webserver. The motivation for this was that we needed a system to store all of our files and easily retrieve it without paying for a service like E3. So, I decided to make a cool webserver here!

## Running Webserver
Inorder to run this we use make and g++
```
make run
```
The server will listen on port 8080, you can connect to localhost:8080 and see if any requests were sent.

## Testing Webserver
Currently we only have testing for the Database Handler and you have to manually check if outputs are correct. Plans are for a python script to check if tests are correct.
Run
```
make run_test
```
Then in testing directory check if commands.out is the same as commands.expected!
