#include "file_manager.hpp"
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <algorithm>


void getFileCreationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
    printf("Last modified time: %s", ctime(&attr.st_mtime));
}

FileManager::FileManager(){}

int FileManager::create_directory(std::string &path)
{
    //mkdir(this->base_path.c_str(), 0777);

    std::string full_path = this->base_path + path;
    int err = mkdir(full_path.c_str(), 0777);
    printf("MKDIR: %d | %d | %s\n", err, errno, full_path.c_str());
    return errno == EEXIST ? 1 : err;
}

void FileManager::set_base_path(std::string &path)
{
    this->base_path = path.c_str();
}

int FileManager::list_directory(std::string &path, std::string &out)
{
    DIR *dir;
    struct dirent *ent;
    struct stat attr;
    out += "\n";
    bool empty = true;
    int count = 0;
    //std::vector<std::string> unordered_files;
    if ((dir = opendir (path.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                stat(ent->d_name, &attr);
                if(S_ISREG(attr.st_mode))
                {
                    empty = false;
                }
                std::string creation = ctime((const time_t *)&attr.st_ctim);
                std::string modification = ctime((const time_t *)&attr.st_mtim);
                std::string access = ctime((const time_t *)&attr.st_atim);
                //unordered_files.push_back(std::string(ent->d_name) + "\n\tCreated  at " + creation + "\tModified at " + modification + "\tAccessed at " + access + "\n");
                count += 1;
                out += std::string(ent->d_name) + "\n"; 
                out += "\tCreated  at " + creation + "\tModified at " + modification + "\tAccessed at " + access + "\n";
                
            }
        }
        if(empty)
        {
            out = "Empty folder";
        }
        else
        {
            /*std::sort(unordered_files.begin(), unordered_files.end());
            for (std::string file : unordered_files)
            {
                out += file;
            }*/
        }
        closedir (dir);
    } else {
        return -1;
    }

    return count;
} 