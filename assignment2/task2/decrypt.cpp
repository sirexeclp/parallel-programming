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

struct Result
{
    bool found = false;
    UserTask task;
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
        exit(1);
    }

    const auto passwdFilepath = argv[1];
    const auto dictFilepath = argv[2];

    std::vector<UserTask> tasks;
    std::vector<std::string> dictPasswords;

    readPasswordFile(passwdFilepath, tasks);
    readDictFile(dictFilepath, dictPasswords);

    Result result1 = Result();
    Result result2 = Result();

    #pragma omp parallel for shared(result1, result2)
    for (int i = 0; i < tasks.size(); i++)
    {
        auto task = tasks[i];
        auto salt = task.salt.c_str();
        auto hash = task.hash.c_str();

        if (result1.found && result2.found) continue;

        for (int j = 0; j < dictPasswords.size(); j++)
        {
            auto password = dictPasswords[j];
            struct crypt_data data;
            data.initialized = 0;

            if (result1.found && result2.found) continue;

            if (strcmp(crypt_r(password.c_str(), salt, &data), hash) == 0)
            {
                task.password = password;
                result1.found = true;
                result1.task = task;
                break;
            }

            data.initialized = 0;

            {
                if (strcmp(crypt_r((password + "0").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "0";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "1").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "1";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "2").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "2";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "3").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "3";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "4").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "4";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "5").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "5";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "6").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "6";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "7").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "7";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "8").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "8";
                    result2.found = true;
                    result2.task = task;
                    break;
                }

                data.initialized = 0;

                if (strcmp(crypt_r((password + "9").c_str(), salt, &data), hash) == 0)
                {
                    task.password = password + "9";
                    result2.found = true;
                    result2.task = task;
                    break;
                }
            }
        }
    }

    std::cout << result1.task.username << ";" << result1.task.password << std::endl;
    std::cout << result2.task.username << ";" << result2.task.password << std::endl;

    return 0;
}
