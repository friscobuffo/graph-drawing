#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdexcept>

int main() {
    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("fork failed!");
    if (pid == 0) {
        // child process
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull == -1)
            throw std::runtime_error("Failed to open /dev/null");
        if (dup2(devNull, STDOUT_FILENO) == -1 || dup2(devNull, STDERR_FILENO) == -1) {
            close(devNull);
            throw std::runtime_error("dup2 failed!");
        }
        close(devNull);
        execl("./glucose", "glucose", ".conjunctive_normal_form.cnf", 
              ".output.txt", "-certified", "-certified-output=.proof.txt", (char *)NULL);
        throw std::runtime_error("Failed to run executable");
    } else {
        // Parent process: Wait for the child to finish
        int status;
        if (waitpid(pid, &status, 0) == -1)
            throw std::runtime_error("waitpid failed!");
        return WEXITSTATUS(status);
    }
}
