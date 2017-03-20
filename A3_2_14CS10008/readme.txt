"data.txt" is considered a file containing a maximum of 100 student records. The record for each
student is kept in one line in the file and contains the following, separated by one or
more spaces: First_name (ascii string, one word with no spaces in between, max. 20
characters), Last_Name (ascii string, one word with no spaces in between, max 20
charaters), Roll_No (integer), CGPA (float). Roll no. is unique for a student, but
others may be the same for two students. 

A process X, when started, first loads all the records in shared memory from a file
whose name is passed to it as a command line argument.

So try:
<Terminal 1>
gcc x.c -o x
gcc y.c -o y
./x data.txt

<Terminals 2 to k>
./y
*Make updates to CGPA*
