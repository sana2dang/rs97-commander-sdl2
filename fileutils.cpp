#include <unistd.h>

#ifdef _POSIX_SPAWN
#include <spawn.h>
#else
#include <signal.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include "fileutils.h"
#include "def.h"
#include "dialog.h"
#include "sdlutils.h"

namespace {

int WaitPid(pid_t id)
{
    int status;
    while (waitpid(id, &status, WNOHANG) == 0)
        usleep(50 * 1000);
    if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status))
    {
        perror("하위 프로세스 오류");
        return -1;
    }
    return 0;
}

// If child_pid is NULL, waits for the child process to finish.
// Otherwise, doesn't wait and stores child process pid into child_pid.
int SpawnAndWait(const char *argv[])
{
    pid_t child_pid;
#ifdef _POSIX_SPAWN
    // This const cast is OK, see https://stackoverflow.com/a/190208.
    if (::posix_spawnp(&child_pid, argv[0], nullptr, nullptr, (char **)argv, nullptr) != 0)
    {
        perror(argv[0]);
        return -1;
    }
    return WaitPid(child_pid);
#else
    struct sigaction sa, save_quit, save_int;
    sigset_t save_mask;
    int wait_val;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    /* __sigemptyset(&sa.sa_mask); - done by memset() */
    /* sa.sa_flags = 0; - done by memset() */

    sigaction(SIGQUIT, &sa, &save_quit);
    sigaction(SIGINT, &sa, &save_int);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, &save_mask);
    if ((child_pid = vfork()) < 0)
    {
        wait_val = -1;
        goto out;
    }
    if (child_pid == 0)
    {
        sigaction(SIGQUIT, &save_quit, NULL);
        sigaction(SIGINT, &save_int, NULL);
        sigprocmask(SIG_SETMASK, &save_mask, NULL);

        // This const cast is OK, see https://stackoverflow.com/a/190208.
        if (execvp(argv[0], (char **)argv) == -1)
            perror(argv[0]);
        exit(127);
    }
    wait_val = WaitPid(child_pid);
out:
    sigaction(SIGQUIT, &save_quit, NULL);
    sigaction(SIGINT, &save_int, NULL);
    sigprocmask(SIG_SETMASK, &save_mask, NULL);
    return wait_val;
#endif
}

const char *AsConstCStr(const char *s) { return s; }
const char *AsConstCStr(const std::string &s) { return s.c_str(); }

template <typename... Args>
bool Run(Args... args)
{
    const char *execve_args[] = {AsConstCStr(args)..., nullptr};
    return SpawnAndWait(execve_args) == 0;
}

} // namespace

void File_utils::copyFile(const std::vector<std::string> &p_src, const std::string &p_dest)
{
    std::string l_destFile;
    std::string l_fileName;
    bool l_loop(true);
    bool l_confirm(true);
    bool l_execute(true);
    for (std::vector<std::string>::const_iterator l_it = p_src.begin(); l_loop && (l_it != p_src.end()); ++l_it)
    {
        l_execute = true;
        l_fileName = getFileName(*l_it);
        l_destFile = p_dest + (p_dest.at(p_dest.size() - 1) == '/' ? "" : "/") + l_fileName;

        // Check if destination files already exists
        if (l_confirm)
        {
            if (fileExists(l_destFile))
            {
                INHIBIT(std::cout << "File " << l_destFile << " already exists => ask for confirmation" << std::endl;)
                CDialog l_dialog("질문:", 0, 0);
                l_dialog.addLabel(l_fileName + "을 덮어쓸까요?");
                l_dialog.addOption("예");
                l_dialog.addOption("모두 예");
                l_dialog.addOption("아니오");
                l_dialog.addOption("취소");
                l_dialog.init();
                switch (l_dialog.execute())
                {
                    case 1:
                        // Yes
                        break;
                    case 2:
                        // Yes to all
                        l_confirm = false;
                        break;
                    case 3:
                        // No
                        l_execute = false;
                        break;
                    default:
                        // Cancel
                        l_execute = false;
                        l_loop = false;
                        break;
                }
            }
        }
        if (l_execute)
        {
            Run("cp", "-r", *l_it, p_dest);
            Run("sync", l_destFile);
        }
    }
}

void File_utils::moveFile(const std::vector<std::string> &p_src, const std::string &p_dest)
{
    std::string l_destFile;
    std::string l_fileName;
    bool l_loop(true);
    bool l_confirm(true);
    bool l_execute(true);
    for (std::vector<std::string>::const_iterator l_it = p_src.begin(); l_loop && (l_it != p_src.end()); ++l_it)
    {
        l_execute = true;
        // Check if destination files already exists
        if (l_confirm)
        {
            l_fileName = getFileName(*l_it);
            l_destFile = p_dest + (p_dest.at(p_dest.size() - 1) == '/' ? "" : "/") + l_fileName;
            if (fileExists(l_destFile))
            {
                INHIBIT(std::cout << "File " << l_destFile << " already exists => ask for confirmation" << std::endl;)
                CDialog l_dialog("Question:", 0, 0);
                l_dialog.addLabel(l_fileName + "을 덮어쓸까요?");
                l_dialog.addOption("예");
                l_dialog.addOption("모두 예");
                l_dialog.addOption("아니오");
                l_dialog.addOption("취소");

                l_dialog.init();
                switch (l_dialog.execute())
                {
                    case 1:
                        // Yes
                        break;
                    case 2:
                        // Yes to all
                        l_confirm = false;
                        break;
                    case 3:
                        // No
                        l_execute = false;
                        break;
                    default:
                        // Cancel
                        l_execute = false;
                        l_loop = false;
                        break;
                }
            }
        }
        if (l_execute)
        {
            Run("mv", *l_it, p_dest);
            Run("sync", p_dest);
        }
    }
}

void File_utils::renameFile(const std::string &p_file1, const std::string &p_file2)
{
    bool l_execute(true);
    // Check if destination files already exists
    if (fileExists(p_file2))
    {
        INHIBIT(std::cout << "File " << p_file2 << " already exists => ask for confirmation" << std::endl;)
        CDialog l_dialog("질문:", 0, 0);
        l_dialog.addLabel(getFileName(p_file2) + "을 덮어쓸까요?");
        l_dialog.addOption("예");
        l_dialog.addOption("아니오");
        l_dialog.init();
        if (l_dialog.execute() != 1)
            l_execute = false;
    }
    if (l_execute)
    {
        Run("mv", p_file1, p_file2);
        Run("sync", p_file2);
    }
}

void File_utils::removeFile(const std::vector<std::string> &p_files)
{
    for (std::vector<std::string>::const_iterator l_it = p_files.begin(); l_it != p_files.end(); ++l_it)
    {
        Run("rm", "-rf", *l_it);
    }
}

void File_utils::makeDirectory(const std::string &p_file)
{
    Run("mkdir", "-p", p_file);
    Run("sync", p_file);
}

const bool File_utils::fileExists(const std::string &p_path)
{
    struct stat l_stat;
    return stat(p_path.c_str(), &l_stat) == 0;
}

static void AsciiToLower(std::string *s)
{
    for (char &c : *s)
        if (c >= 'A' && c <= 'Z')
            c -= ('Z' - 'z');
}

std::string File_utils::getLowercaseFileExtension(const std::string &name) {
    const auto dot_pos = name.rfind('.');
    if (dot_pos == std::string::npos)
        return "";
    std::string ext = name.substr(dot_pos + 1);
    AsciiToLower(&ext);
    return ext;
}

const std::string File_utils::getFileName(const std::string &p_path)
{
    size_t l_pos = p_path.rfind('/');
    return p_path.substr(l_pos + 1);
}

const std::string File_utils::getPath(const std::string &p_path)
{
    size_t l_pos = p_path.rfind('/');
    return p_path.substr(0, l_pos);
}

void File_utils::executeFile(const std::string &p_file)
{
    // Command
    INHIBIT(std::cout << "File_utils::executeFile: " << p_file << " in " << getPath(p_file) << std::endl;)
    // CD to the file's location
    chdir(getPath(p_file).c_str());
    // Quit
    SDL_utils::hastalavista();
    // Execute file
    if (getLowercaseFileExtension(p_file) == "opk") {
        execlp("opkrun", "opkrun", p_file.c_str(), nullptr);
    } else {
        execl(p_file.c_str(), p_file.c_str(), nullptr);
    }
    // If we're here, there was an error with the execution
    perror("Child process error");
    std::cerr << "Error executing file " << p_file << std::endl;
    // Relaunch DinguxCommander
    const std::string self_name = getSelfExecutionName();
    INHIBIT(std::cout << "File_utils::executeFile: " << self_name << " in " << getSelfExecutionPath() << std::endl;)
    chdir(getSelfExecutionPath().c_str());
    execl(self_name.c_str(), self_name.c_str(), NULL);
}

const std::string File_utils::getSelfExecutionPath(void)
{
    // Get execution path
    std::string l_exePath("");
    char l_buff[255];
    int l_i = readlink("/proc/self/exe", l_buff, 255);
    l_exePath = l_buff;
    l_exePath = l_exePath.substr(0, l_i);
    l_i = l_exePath.rfind("/");
    l_exePath = l_exePath.substr(0, l_i);
    return l_exePath;
}

const std::string File_utils::getSelfExecutionName(void)
{
    // Get execution path
    std::string l_exePath("");
    char l_buff[255];
    int l_i = readlink("/proc/self/exe", l_buff, 255);
    l_exePath = l_buff;
    l_exePath = l_exePath.substr(0, l_i);
    l_i = l_exePath.rfind("/");
    l_exePath = l_exePath.substr(l_i + 1);
    return l_exePath;
}

void File_utils::stringReplace(std::string &p_string, const std::string &p_search, const std::string &p_replace)
{
    // Replace all occurrences of p_search by p_replace in p_string
    size_t l_pos = p_string.find(p_search, 0);
    while (l_pos != std::string::npos)
    {
        p_string.replace(l_pos, p_search.length(), p_replace);
        l_pos = p_string.find(p_search, l_pos + p_replace.length());
    }
}

const unsigned long int File_utils::getFileSize(const std::string &p_file)
{
    unsigned long int l_ret(0);
    struct stat l_stat;
    if (stat(p_file.c_str(), &l_stat) == -1)
        std::cerr << "File_utils::getFileSize: Error stat " << p_file << std::endl;
    else
        l_ret = l_stat.st_size;
    return l_ret;
}

void File_utils::diskInfo(void)
{
    std::string l_line("");
    SDL_utils::pleaseWait();
    // Execute command df -h
    {
        char l_buffer[256];
        FILE *l_pipe = popen("df -h " FILE_SYSTEM, "r");
        if (l_pipe == NULL)
        {
            std::cerr << "File_utils::diskInfo: Error popen" << std::endl;
            return;
        }
        while (l_line.empty() && fgets(l_buffer, sizeof(l_buffer), l_pipe) != NULL)
            if (strstr(l_buffer, FILE_SYSTEM) != NULL)
                l_line = l_buffer;
        pclose(l_pipe);
    }
    if (!l_line.empty())
    {
        // Separate line by spaces
        std::istringstream l_iss(l_line);
        std::vector<std::string> l_tokens;
        copy(std::istream_iterator<std::string>(l_iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string> >(l_tokens));
        // Display dialog
        CDialog l_dialog("디스크 정보:", 0, 0);
        l_dialog.addLabel("총 용량: " + l_tokens[1]);
        l_dialog.addLabel("사용된 용량: " + l_tokens[2] + " (" + l_tokens[4] + ")");
        l_dialog.addLabel("여유 용량: " + l_tokens[3]);
        l_dialog.addOption("확인");
        l_dialog.init();
        l_dialog.execute();
    }
    else
        std::cerr << "File_utils::diskInfo: Unable to find " << FILE_SYSTEM << std::endl;
}

void File_utils::diskUsed(const std::vector<std::string> &p_files)
{
    std::string l_line("");
    // Waiting message
    SDL_utils::pleaseWait();
    // Build and execute command
    {
        std::string l_command("du -csh");
        for (std::vector<std::string>::const_iterator l_it = p_files.begin(); l_it != p_files.end(); ++l_it)
            l_command = l_command + " \"" + *l_it + "\"";
        char l_buffer[256];
        FILE *l_pipe = popen(l_command.c_str(), "r");
        if (l_pipe == NULL)
        {
            std::cerr << "File_utils::diskUsed: Error popen" << std::endl;
            return;
        }
        while (fgets(l_buffer, sizeof(l_buffer), l_pipe) != NULL);
        l_line = l_buffer;
        pclose(l_pipe);
    }
    // Separate line by spaces
    {
        std::istringstream l_iss(l_line);
        std::vector<std::string> l_tokens;
        copy(std::istream_iterator<std::string>(l_iss), std::istream_iterator<std::string>(), std::back_inserter<std::vector<std::string> >(l_tokens));
        l_line = l_tokens[0];
    }
    // Dialog
    std::ostringstream l_stream;
    CDialog l_dialog("속성:", 0, 0);
    l_stream << p_files.size() << "개 선택됨";
    l_dialog.addLabel(l_stream.str());
    l_dialog.addLabel("할당된 용량: " + l_line);
    l_dialog.addOption("확인");
    l_dialog.init();
    l_dialog.execute();
}

void File_utils::formatSize(std::string &p_size)
{
    // Format 123456789 to 123,456,789
    int l_i = p_size.size() - 3;
    while (l_i > 0)
    {
        p_size.insert(l_i, ",");
        l_i -= 3;
    }
}


////////////////////////////////////////////////
void File_utils::decompressionFile(const std::string &p_file)
{
        chdir(getPath(p_file).c_str());
        Run("tar", "xvfp", p_file);
}
///////////
