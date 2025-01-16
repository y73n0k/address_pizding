# Address pizding
Universal (tested only on x64 and only Arch, Ubuntu distros) tool for dumping base address of loaded to executable library.

## Tools
* `disable_aslr.c` - tool for executing process with disabled aslr :scream_cat:
* `address_pizding.c` - tool for dumping address

## Build
`address_pizding.c` has customizable parameters:
* `SUBSTR` - substring to match in path of library
* `WORDLEN` - length of machine word in `char`s
* `SKIPS` - number of execv syscalls to skip before dumping address
* `IS32` - this should be defined if target executable is compiled in 32-bti mode

You can manyally pass them to `gcc` by `-D`:

```
gcc address_pizding.c -o address_pizding -DWORDLEN=4 -DSUBSTR=\"libc.so\" -DSKIPS=1 -DIS32
```

Or to `make`:

```
make IS32=1 WORDLEN=6 SUBSTR=\\\"ld\\\" SKIPS=2
```

## Usage
Very simple

```
./disable_aslr <exe> [params of exe]
```

```
./address_pizding <exe> [params of exe]
```
