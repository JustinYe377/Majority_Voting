# Assignment 06: Majority Voting

## Build and run

```sh
cd assignment6
make
./lab6
```

Enter the voter count (at least 3), then the maximum number of rounds (at
least 1), each on its own line. For example:

To clean ./lab6, use 
```sh
make clean
```


### Output

```text

~/Majority_Voting$ make
cc -std=c11 -Wall -Wextra -Wpedantic -O2    lab6.c   -o lab6

~/Majority_Voting$ ./lab6
Enter the number of voters: 5
Enter the number of rounds: 3
Round 1: Voter 0 votes: NO
Round 2: Voter 0 votes: NO
Round 3: Voter 0 votes: NO
Round 1: Voter 1 votes: NO
Round 2: Voter 1 votes: YES
Round 3: Voter 1 votes: NO
Round 1: Voter 2 votes: NO
Round 2: Voter 2 votes: YES
Round 3: Voter 2 votes: NO
Round 1: Voter 3 votes: NO

------ Round 1 -------
Round 2: Voter 3 votes: YES
Round 3: Voter 3 votes: YES
Round 1: Voter 4 votes: NO
Round 2: Voter 4 votes: NO
--- Round 1: The voting result is a NO.

------ Round 2 -------
Round 3: Voter 4 votes: NO
--- Round 2: The voting result is a YES.

------ Round 3 -------
--- Round 3: The voting result is a NO.



```
