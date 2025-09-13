#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <map>

using namespace std;

class ColtraneShell {
private:
    map<string, string> aliases;
    vector<string> cmd_history;
    
public:
    void run() {
        string input;
        cout << "Coltrane Shell - smooth as jazz" << endl;
        
        while (true) {
            cout << getPrompt();
            getline(cin, input);
            
            if (input.empty()) continue;
            
            cmd_history.push_back(input);
            
            if (input == "exit" || input == "quit") {
                break;
            }
            
            processCommand(input);
        }
    }
    
private:
    string getPrompt() {
        char* user = getenv("USER");
        string cwd_path = getcwd(nullptr, 0);
        string dir_name = cwd_path.substr(cwd_path.find_last_of("/") + 1);
        
        return string(user ? user : "user") + ":" + dir_name + "$ ";
    }
    
    void processCommand(const string& input) {
        if (input.find('|') != string::npos) {
            handlePipes(input);
            return;
        }
        
        if (input.find('>') != string::npos || input.find('<') != string::npos) {
            handleRedirection(input);
            return;
        }
        
        vector<string> tokens = tokenize(input);
        if (tokens.empty()) return;
        
        bool background = false;
        if (!tokens.empty() && tokens.back() == "&") {
            background = true;
            tokens.pop_back();
        }
        
        if (handleBuiltins(tokens)) {
            return;
        }
        
        executeCommand(tokens, background);
    }
    
    vector<string> tokenize(const string& input) {
        vector<string> tokens;
        istringstream iss(input);
        string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    bool handleBuiltins(const vector<string>& tokens) {
        if (tokens.empty()) return false;
        
        const string& cmd = tokens[0];
        
        if (cmd == "cd") {
            change_dir(tokens);
            return true;
        }
        
        if (cmd == "pwd") {
            char* cwd = getcwd(nullptr, 0);
            cout << cwd << endl;
            free(cwd);
            return true;
        }
        
        if (cmd == "history") {
            show_history();
            return true;
        }
        
        if (cmd == "help") {
            show_help();
            return true;
        }
        
        return false;
    }
    
    void change_dir(const vector<string>& tokens) {
        string path;
        
        if (tokens.size() == 1) {
            path = getenv("HOME") ? getenv("HOME") : "/";
        } else {
            path = tokens[1];
            if (path[0] == '~') {
                string home = getenv("HOME") ? getenv("HOME") : "/";
                path = home + path.substr(1);
            }
        }
        
        if (chdir(path.c_str()) != 0) {
            perror("cd");
        }
    }
    
    void show_history() {
        for (size_t i = 0; i < cmd_history.size(); ++i) {
            cout << " " << i + 1 << "  " << cmd_history[i] << endl;
        }
    }
    
    void show_help() {
        cout << "Coltrane Shell v1.0" << endl;
        cout << "Built-ins: cd, pwd, history, help, exit" << endl;
        cout << "Features: pipes (|), redirection (>, <), background (&)" << endl;
    }
    
    void executeCommand(const vector<string>& tokens, bool background = false) {
        vector<char*> args;
        for (const auto& token : tokens) {
            args.push_back(const_cast<char*>(token.c_str()));
        }
        args.push_back(nullptr);
        
        pid_t pid = fork();
        
        if (pid == 0) {
            if (execvp(args[0], args.data()) == -1) {
                perror(tokens[0].c_str());
                exit(1);
            }
        } else if (pid > 0) {
            if (!background) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                cout << "[" << pid << "] running in background" << endl;
            }
        } else {
            perror("fork failed");
        }
    }
    
    void handlePipes(const string& input) {
        vector<string> commands;
        stringstream ss(input);
        string cmd;
        
        while (getline(ss, cmd, '|')) {
            // trim spaces
            cmd.erase(0, cmd.find_first_not_of(" \t"));
            cmd.erase(cmd.find_last_not_of(" \t") + 1);
            commands.push_back(cmd);
        }
        
        if (commands.size() < 2) return;
        
        int pipefd[2];
        int input_fd = 0;
        
        for (size_t i = 0; i < commands.size(); ++i) {
            if (i < commands.size() - 1) {
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    return;
                }
            }
            
            pid_t pid = fork();
            
            if (pid == 0) {
                if (input_fd != 0) {
                    dup2(input_fd, 0);
                    close(input_fd);
                }
                
                if (i < commands.size() - 1) {
                    dup2(pipefd[1], 1);
                    close(pipefd[0]);
                    close(pipefd[1]);
                }
                
                vector<string> tokens = tokenize(commands[i]);
                vector<char*> args;
                for (const auto& token : tokens) {
                    args.push_back(const_cast<char*>(token.c_str()));
                }
                args.push_back(nullptr);
                
                if (execvp(args[0], args.data()) == -1) {
                    perror(tokens[0].c_str());
                    exit(1);
                }
            } else if (pid > 0) {
                if (input_fd != 0) {
                    close(input_fd);
                }
                
                if (i < commands.size() - 1) {
                    close(pipefd[1]);
                    input_fd = pipefd[0];
                }
                
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("fork");
                return;
            }
        }
    }
    
    void handleRedirection(const string& input) {
        string cmd = input;
        string output_file;
        string input_file;
        bool append = false;
        
        size_t out_pos = cmd.find(">>");
        if (out_pos != string::npos) {
            append = true;
            output_file = cmd.substr(out_pos + 2);
            cmd = cmd.substr(0, out_pos);
        } else {
            out_pos = cmd.find('>');
            if (out_pos != string::npos) {
                output_file = cmd.substr(out_pos + 1);
                cmd = cmd.substr(0, out_pos);
            }
        }
        
        size_t in_pos = cmd.find('<');
        if (in_pos != string::npos) {
            input_file = cmd.substr(in_pos + 1);
            cmd = cmd.substr(0, in_pos);
        }
        
        // trim whitespace
        cmd.erase(0, cmd.find_first_not_of(" \t"));
        cmd.erase(cmd.find_last_not_of(" \t") + 1);
        
        if (!output_file.empty()) {
            output_file.erase(0, output_file.find_first_not_of(" \t"));
            output_file.erase(output_file.find_last_not_of(" \t") + 1);
        }
        
        if (!input_file.empty()) {
            input_file.erase(0, input_file.find_first_not_of(" \t"));
            input_file.erase(input_file.find_last_not_of(" \t") + 1);
        }
        
        vector<string> tokens = tokenize(cmd);
        if (tokens.empty()) return;
        
        pid_t pid = fork();
        
        if (pid == 0) {
            if (!input_file.empty()) {
                int fd = open(input_file.c_str(), O_RDONLY);
                if (fd == -1) {
                    perror("input file");
                    exit(1);
                }
                dup2(fd, 0);
                close(fd);
            }
            
            if (!output_file.empty()) {
                int flags = O_WRONLY | O_CREAT;
                flags |= append ? O_APPEND : O_TRUNC;
                int fd = open(output_file.c_str(), flags, 0644);
                if (fd == -1) {
                    perror("output file");
                    exit(1);
                }
                dup2(fd, 1);
                close(fd);
            }
            
            vector<char*> args;
            for (const auto& token : tokens) {
                args.push_back(const_cast<char*>(token.c_str()));
            }
            args.push_back(nullptr);
            
            if (execvp(args[0], args.data()) == -1) {
                perror(tokens[0].c_str());
                exit(1);
            }
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
        }
    }
};

int main() {
    ColtraneShell shell;
    shell.run();
    cout << "Take five!" << endl;
    return 0;
}
