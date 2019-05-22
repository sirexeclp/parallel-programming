#include <string>
#include <fstream>
#include <iostream>
#include <vector>

struct UserTask
{
    std::string username;
    std::string salt;
    std::string hash;
};


int main(int argc, char const *argv[])
{
    const auto delimiter = ":";
    const auto lengthSalt = 2;

    if (argc != 3)
    {
        std::cerr << "[Error] Usage: " << argv[0] << " password-filepath dictionary-filepath" << std::endl;
    }

    const auto passwdFilepath = argv[1];
    const auto dictFilepath = argv[2];

    std::vector<UserTask> tasks;

    std::ifstream passwdFile(passwdFilepath);
    std::string line = "";
    while (passwdFile >> line)
    {
        UserTask task = UserTask();
        auto posDeliminator = line.find(delimiter);
        task.username = line.substr(0, posDeliminator);
        task.salt = line.substr(posDeliminator + 1, lengthSalt);
        task.hash = line.substr(posDeliminator + 1 + lengthSalt, line.length() - task.username.length() - lengthSalt);

        tasks.push_back(task);
    }

    for (UserTask task: tasks) {
        std::cout << task.username << " " << task.salt << " " << task.hash << std::endl;
    }

    return 0;
}
