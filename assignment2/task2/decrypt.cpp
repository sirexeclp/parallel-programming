#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#define delimiter ":"
#define lengthSalt 2

struct UserTask
{
    std::string username;
    std::string salt;
    std::string hash;
};

void readPasswordFile(const std::string passwdFilepath, std::vector<UserTask> * tasks) {
    std::ifstream passwdFile(passwdFilepath);
    std::string userLine = "";

    while (passwdFile >> userLine)
    {
        auto posDeliminator = userLine.find(delimiter);
        
        UserTask task = UserTask();
        task.username = userLine.substr(0, posDeliminator);
        task.salt = userLine.substr(posDeliminator + 1, lengthSalt);
        task.hash = userLine.substr(posDeliminator + 1 + lengthSalt, userLine.length() - task.username.length() - lengthSalt);

        tasks->push_back(task);
    }
}

void readDictFile(const std::string dictFilepath, std::vector<std::string> * dictPasswords) {
    std::ifstream dictFile(dictFilepath);
    std::string entry = "";

    while (dictFile >> entry)
    {
        dictPasswords->push_back(entry);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        std::cerr << "[Error] Usage: " << argv[0] << " password-filepath dictionary-filepath" << std::endl;
    }

    const auto passwdFilepath = argv[1];
    const auto dictFilepath = argv[2];

    std::vector<UserTask> tasks;
    std::vector<std::string> dictPasswords;

    readPasswordFile(passwdFilepath, &tasks);
    readDictFile(dictFilepath, &dictPasswords);

    for (UserTask task: tasks) {
        std::cout << task.username << std::endl;
    }

    return 0;
}
