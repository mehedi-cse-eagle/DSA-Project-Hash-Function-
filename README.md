# Hash Verse Visualizer (C++ console edition)

A terminal program that shows how hashing works, in five demos:

1. **Memory Grid**: hash table with linear probing, quadratic probing and separate chaining. Shows collisions and automatic rehashing at 70% load.
2. **Password Avalanche**: change one character and see how many of the 64 hash bits change.
3. **Plagiarism Scanner**: finds copied parts of text using a rolling hash (Rabin-Karp).
4. **LRU Cache**: a 3-slot cache that evicts the least recently used item.
5. **Break the Hash**: pick a target box and find a word that lands in it.

## Requirements

- A C++17 compiler, for example `g++` on Ubuntu:
  ```
  sudo apt install g++
  ```

## Build and run

```
g++ -std=c++17 -Wall -Wextra -O2 -o hash_verse main.cpp
./hash_verse
```

The program uses ANSI colours, so it works best in a standard terminal (Ubuntu terminal, GNOME Terminal, VS Code terminal).

## Note

The hash functions here are for learning only. Real password storage needs a slow, salted algorithm such as bcrypt or argon2.
