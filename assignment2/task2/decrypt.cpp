#include <crypt.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

#define delimiter ":"
#define lengthSalt 2

struct UserTask
{
    std::string username;
    std::string salt;
    std::string hash;
    std::string password;
};

void readPasswordFile(const std::string passwdFilepath, std::vector<UserTask> &tasks)
{
    std::ifstream passwdFile(passwdFilepath);
    std::string userLine = "";

    while (passwdFile >> userLine)
    {
        auto posDeliminator = userLine.find(delimiter);

        UserTask task = UserTask();
        task.username = userLine.substr(0, posDeliminator);
        task.salt = userLine.substr(posDeliminator + 1, lengthSalt);
        task.hash = userLine.substr(posDeliminator + 1, userLine.length() - task.username.length() - lengthSalt + 1);

        tasks.push_back(task);
    }
}

void readDictFile(const std::string dictFilepath, std::vector<std::string> &dictPasswords)
{
    std::ifstream dictFile(dictFilepath);
    std::string entry = "";

    while (dictFile >> entry)
    {
        dictPasswords.push_back(entry);
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

    readPasswordFile(passwdFilepath, tasks);
    readDictFile(dictFilepath, dictPasswords);

    bool firstFound = false;
    bool secondFound = false;
    UserTask t1;
    UserTask t2;

    for (auto task : tasks)
    {
        auto salt = task.salt.c_str();
        auto hash = task.hash.c_str();

        for (auto password : dictPasswords)
        {
            if (strcmp(crypt(password.c_str(), salt), hash) == 0)
            {
                task.password = password;
                t1 = task;
                firstFound = true;
                break;
            }

            {
                if (strcmp(crypt((password + "0").c_str(), salt), hash) == 0)
                {
                    task.password = password + "0";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "1").c_str(), salt), hash) == 0)
                {
                    task.password = password + "1";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "2").c_str(), salt), hash) == 0)
                {
                    task.password = password + "2";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "3").c_str(), salt), hash) == 0)
                {
                    task.password = password + "3";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "4").c_str(), salt), hash) == 0)
                {
                    task.password = password + "4";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "5").c_str(), salt), hash) == 0)
                {
                    task.password = password + "5";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "6").c_str(), salt), hash) == 0)
                {
                    task.password = password + "6";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "7").c_str(), salt), hash) == 0)
                {
                    task.password = password + "7";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "8").c_str(), salt), hash) == 0)
                {
                    task.password = password + "8";
                    t2 = task;
                    secondFound = true;
                    break;
                }

                if (strcmp(crypt((password + "9").c_str(), salt), hash) == 0)
                {
                    task.password = password + "9";
                    t2 = task;
                    secondFound = true;
                    break;
                }
            }

            if (firstFound && secondFound)
            {
                break;
            }
        }

        if (firstFound && secondFound)
        {
            break;
        }
    }

    std::cout << t1.username << ";" << t1.password << std::endl;
    std::cout << t2.username << ";" << t2.password << std::endl;

    return 0;
}
