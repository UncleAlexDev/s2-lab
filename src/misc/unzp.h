#ifndef __UNZ_H__
#define __UNZ_H__

#include <string>
#include <vector>

class UNZP
{
public:
    UNZP();
    virtual ~UNZP();

    int unzip_one_file(const std::string &zipfilename, const std::string &filename, const std::string &out_filename);
    int list_files(const std::string filename);
    const std::vector<std::string> &files();

    int  error();
    const std::string &strerror();
    const char *unz_strerror(int err) const;

private:
    std::vector<std::string> m_files;
    std::string m_str_error;
    int m_error;
};


#endif // __UNZ_H__
