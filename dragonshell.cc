#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <set>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <signal.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>


using namespace std;
#define MAXLINE 128
string curPath = "/bin/:/usr/bin/";
pid_t nPid = (pid_t) - 1;
pid_t bPid = (pid_t) - 1;

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */
std::vector<std::string> tokenize(const std::string &str, const char *delim) {
  char* cstr = new char[str.size() + 1];
  std::strcpy(cstr, str.c_str());

  char* tokenized_string = strtok(cstr, delim);

  std::vector<std::string> tokens;
  while (tokenized_string != NULL)
  {
    tokens.push_back(std::string(tokenized_string));
    tokenized_string = strtok(NULL, delim);
  }
  delete[] cstr;

  return tokens;
}

// change directory
void cd (vector<string> dir){
  if (dir.size() == 2){
    if (chdir(dir[1].c_str()) != 0){
      cout << "dragonshell: No such file or directory" << endl;
    }
  }
  else if (dir.size() > 2){
    cout << "dragonshell: \"cd\" expects only one argument" << endl;
  }
  else{
    cout << "dragonshell: expect arguments to \"cd\"" << endl;
  }
}

// show current directory
void pwd(){
  char curDir[1024];
  cout << getcwd(curDir, sizeof(curDir)) << endl;
}

// show path
void path(){
  cout << "Current Path: " << curPath << endl;
}

// add to path
void a2path(vector<string> addPath){
  if (addPath.size() == 2){
    if (addPath[1].substr(0,5).compare("$PATH") == 0){
      if (curPath.compare("") != 0){
        curPath += addPath[1].substr(5);
      }
      else{
        curPath += addPath[1].substr(6);
      }
    }
    else{
      curPath = addPath[1];
    }
  }
  else if (addPath.size() < 2){
    curPath = "";
  }
  else{
    cout << "dragonshell: too many arguments to \"a2path\"" << endl;
  }
}

// exit program
void exit(){
  if (bPid != -1){
    kill(bPid, SIGINT);
  }
  while(wait(NULL) > 0);
  _exit(0);
}

// run external program, check through all paths if program exists
void runExtProgram(vector<string> extProgram, bool back){
  int pid;
  bool exist = false;
  string program = extProgram[0];
  vector<string> pathArray = tokenize(curPath, ":");
  char *programName[extProgram.size() + 1];
  for (size_t i = 0; i < extProgram.size(); i++)
  {
    programName[i] = const_cast<char*>(extProgram[i].c_str());
  }
  programName[extProgram.size()] = NULL;
  for (size_t i = 0; i < pathArray.size(); i++)
  {
    string path = pathArray[i];
    string tmp = path + program;
    if (!access(tmp.c_str(),F_OK)){
      program = tmp;
      exist = true;
      break;
    }
  }

  if (!exist){
    cout << "dragonshell: Program is not found" << endl;
    return;
  }

  if ((pid = fork()) == 0){
    if (execv(program.c_str(), programName) == -1){
      perror("execv");
    }
  }
  else if (pid == -1){
    perror("fork error");
  }
  else if (pid > 0){
    if (back){
      bPid = pid;
      cout << "PID: "<< bPid <<" is running in the background" << endl;
      return;
    }
    else{
      nPid = -1;
      wait(NULL);
    }
  }
}

// set output redirectory
void outRedir(string reDir){
  vector<string> newDir = tokenize(reDir, " ");
  if (newDir[0] != ""){
    string outDir = newDir[newDir.size() - 1];
    // idea of the redirection is from stackoverflow
    // Author: Sehe
    // Title: redirecting output to a file in C
    // Date: Dec 15 '11
    // Link: https://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c
    int out = open("cout.log", O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) {
      perror("opening cout.log"); 
      return;
    }
    int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) {
      perror("cannot redirect stdout"); 
      return; 
    }
    fflush(stdout); 
    close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);
    int fd1 = open(outDir.c_str(), O_CREAT | O_WRONLY,  S_IRUSR | S_IWUSR);
    if (fd1 == -1){
      perror("open");
      _exit(EXIT_FAILURE);
    }
  }
}

// run commands
void runCmd(vector<string> tokens){
    if (!tokens[0].compare("")){
      cout << "dragonshell: Command expected" << endl;
    }
    else if (!tokens[0].compare("cd")){
      cd(tokens);
    }
    else if (!tokens[0].compare("pwd")){
      pwd();
    }
    else if (!tokens[0].compare("$PATH")){
      path();
    }
    else if (!tokens[0].compare("a2path")){
      a2path(tokens);
    }
    else if (!tokens[0].compare("exit")){
      exit();
    }
    else {
      // run at background
      if(!tokens[tokens.size() - 1].compare("&")){
        tokens.erase(tokens.end());
        if (bPid == -1){
          runExtProgram(tokens, true);
        }
        else{
          cout << "dragonshell: another program is running background" << endl;
        }
      }
      else{
        runExtProgram(tokens, false);
      }
    }
}

// pipe
void pipe(string cmd){
  pid_t pid;
  int fd[2];
  vector<string> twoCmd = tokenize(cmd, "|");


  if (pipe(fd) < 0){
    perror("pipe error");
  }
  if ((pid = fork()) < 0){
    perror("fork error");
  }
  if (pid == 0){
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    runCmd(tokenize(twoCmd[1], " "));
    _exit(0);
  }
  else{
    close(fd[1]);       
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    runCmd(tokenize(twoCmd[0], " "));
    wait(NULL);
  }
}

// handle signal ctrl z and ctrl c to only terminate process running background
void signal_callback_handler(int signum) {
  if ((signum == SIGTSTP) | (signum == SIGINT)){
    if ((getpid() != nPid) && (nPid != -1)){
      kill(nPid, signum);
      nPid = -1;
    }
  }

  if (signum == SIGCHLD){
    if ((getpid() != bPid) & (bPid != -1)){
      kill(bPid, SIGINT);
      bPid = -1;
    }
  }
}

// check if needs pipe or redirectory
void checkPipeRedir(string cmd){
  if (tokenize(cmd, "|").size() > 1){
    pipe(cmd);
  }
  else if (tokenize(cmd, ">").size() > 1){
    outRedir(cmd);
    vector<string> tokens = tokenize(cmd, " ");
    runCmd(tokens);
  }
  else{
    vector<string> tokens = tokenize(cmd, " ");
    runCmd(tokens);
  }
}

int main(int argc, char **argv) {
  // print the string prompt without a newline, before beginning to read
  // tokenize the input, run the command(s), and print the result
  // do this in a loop
  // code for signal handling is from lab notes
  struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = signal_callback_handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
  sigaction(SIGCHLD, &sa, NULL);

  cout << "Welcome to Dragon Shell!" << endl;
  
  while (!cin.eof())
  { 
    cout << "dragonshell > ";
    string input;
    getline(cin, input);
    vector<string> cmds = tokenize(input, ";");
    
    for (size_t i = 0; i < cmds.size(); ++i)
    { 
      checkPipeRedir(cmds[i]);
    }
  }
  exit();
  return 0;
}