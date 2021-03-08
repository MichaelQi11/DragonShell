1. Built-in Commands
    a. cd: 
        Used chdir() to implement cd by checking if the input is valid and then change directory using chdir(newDirectory)
        Test method: cd correct directory, cd .., cd wrong directory, cd, cd multiple directory
    b. pwd:
        Used getcwd() instead of system call to get current directory
        Test method: check if the output is correct
    c. a2path:
        No system call used, simply created a default local path variable and add or overwrite the path variable depending on the input command
        Test method: add one path and check path, overwrite path and check path, check path, clear path and check path
    d. exit:
        Used kill() and _exit() to kill any program running at background and exit dragonshell.
        Test method: have a while true program running at background and exit dragonshell, directly exit dragonshell using both exit and ctrl d.
2. Run external program
    Used fork() and wait() to create child process and wait for child process. First check if program exists by going through all path + program. If exists, create child process and execute program, wait for the child process to finish and not wait when running at background.
    Test case: running linux program including ls, find, mkdir, etc; running user defined program both in front and background
3. Output redirection
    Idea of redirection is from stackoverflow.
    Author: Sehe
    Title: redirecting output to a file in C
    Date: Dec 15 '11
    Link: https://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c
    Used open(), dup(), dup2(), fflush(), close(), _exit() to open the file by path name, create a duplicated file descriptor, save to the output directory and delete the original one, then exit the program.
    Test case: ls > ls.txt
4. Pipe
    Used pipe(), dup2(), close() to create pipe connecting two processes, close std out for the children process (first command) and send to pipe instead and then close std in for the parent process, wait until children process to finish and then get input and pipe then output to terminal. For some reason, the program stucks at waiting for children process to finish and I could not figure it out.
    Test case: find ./ | sort (failed)
5. Running multiple commands
    Simply using tokenize() provided to split the command and use for loop to run individually
    Test case: try running multiple different commands
6. Handling signals
    Used sigaction and kill to check for the specific signals and kill the process running in background. Almost the same code and idea as the lab introduced.
    Test case: try ctrl c and ctrl z to terminate process running background but not dragonshell
7. Putting jobs background
    Created a background pid to record the id of process running at background to kill or wait. Detailed effect on each feature is explained above.
    Test case: running one and multiple command at background.
