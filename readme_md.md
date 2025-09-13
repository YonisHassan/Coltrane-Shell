# Coltrane Shell

A Unix-style shell written in C++, named after the legendary jazz saxophonist John Coltrane. Like Coltrane's music, this shell aims to be both powerful and elegant.

## Features

- Command execution with pipes and redirection
- Built-in commands (cd, pwd, history, alias, jobs, help, exit)
- Background process execution
- Command history with readline support
- Tab completion
- I/O redirection (`>`, `>>`, `<`)
- Colorized prompt

## Building

```bash
git clone https://github.com/YonisHassan/Coltrane-Shell.git
cd Coltrane-Shell
make
```

### Dependencies

- C++17 compiler
- GNU Readline library

On Ubuntu/Debian:
```bash
sudo apt install build-essential libreadline-dev
```

## Usage

```bash
./coltrane
```

### Examples

```bash
# Basic commands
ls -la
cat file.txt

# Pipes
ps aux | grep coltrane | wc -l

# Redirection  
echo "hello world" > test.txt
sort < names.txt > sorted.txt

# Background jobs
sleep 30 &

# Aliases
alias ll='ls -la'
alias la='ls -A'
```

## Built-in Commands

- `cd [dir]` - Change directory (supports ~ expansion)
- `pwd` - Print working directory  
- `history` - Show command history
- `alias [name=value]` - Create or display aliases
- `jobs` - Show background processes
- `help` - Display help
- `exit` / `quit` - Exit shell

## Project Structure

```
Coltrane-Shell/
├── src/
│   └── shell.cpp
├── Makefile
├── README.md
└── .gitignore
```

## Implementation Notes

The shell handles:
- Process creation with `fork()` and `execvp()`
- I/O redirection using file descriptors
- Pipe communication between processes
- Signal handling for background jobs
- Command parsing and tokenization

## License

MIT License