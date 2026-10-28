/**
   \file unzp.cpp
*/

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
# include <io.h>
# include <direct.h>
#else
# include <unistd.h>
# include <utime.h>
#endif

#include "ok.h"
#include "unzip.h"
#include "unzp.h"
#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#if defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif

#include "utils.h"

UNZP::UNZP()
{
}

UNZP::~UNZP()
{
#ifdef D_B_G
    printf("%s\n", __func__);
#endif
}

int UNZP::error()
{
    return m_error;
}

const std::string &UNZP::strerror()
{
    return m_str_error;
}

const char *UNZP::unz_strerror(int err) const
{
    static char buf[80];
    int e = errno;
    if (err == UNZ_ERRNO) {
        if (e > 0) {
            snprintf(buf, sizeof(buf), "(%d) %s", e, ::strerror(e));
            return buf;
        }
    }
    if (err == UNZ_OK) return "OK";
    if (err == UNZ_END_OF_LIST_OF_FILE) return "UNZ_END_OF_LIST_OF_FILE";
    if (err == UNZ_EOF) return "UNZ_EOF";
    if (err == UNZ_PARAMERROR) return "UNZ_PARAMERROR";
    if (err == UNZ_BADZIPFILE) return "UNZ_BADZIPFILE";
    if (err == UNZ_INTERNALERROR) return "UNZ_INTERNALERROR";
    if (err == UNZ_CRCERROR) return "UNZ_CRCERROR";
    snprintf(buf, sizeof(buf), "(%d)", err);
    return buf;
}

const std::vector<std::string> &UNZP::files()
{
    return m_files;
}


/**
 * @breif change the date/time of a file
 * @param filename - the filename of the file where date/time must be modified
 * @param dosdate - the new date at the MSDOS format (4 bytes)
 * @param tmu_date - the SAME new date at the tm_unz format
*/
static void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
#ifdef _WIN32
    HANDLE hFile;
    FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;

    hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING, 0, NULL);
    GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
    DosDateTimeToFileTime((WORD)(dosdate>>16), (WORD)dosdate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftm);
    SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
    CloseHandle(hFile);
    (void)tmu_date;
#else
#if defined(unix) || defined(__APPLE__)
    (void)dosdate;
    struct utimbuf ut;
    struct tm newdate;
    newdate.tm_sec = tmu_date.tm_sec;
    newdate.tm_min = tmu_date.tm_min;
    newdate.tm_hour = tmu_date.tm_hour;
    newdate.tm_mday = tmu_date.tm_mday;
    newdate.tm_mon = tmu_date.tm_mon;
    if (tmu_date.tm_year > 1900)
        newdate.tm_year = tmu_date.tm_year - 1900;
    else
        newdate.tm_year = tmu_date.tm_year ;
    newdate.tm_isdst = -1;

    ut.actime = ut.modtime = mktime(&newdate);
    utime(filename, &ut);
#else
    (void)filename;
    (void)dosdate;
    (void)tmu_date;
#endif
#endif
}

int UNZP::list_files(const std::string filename)
{
    char buf[256];
    if (filename.empty()){
        m_error = -1;
        m_str_error = std::string(UTILS::transl("No filename specified"));
        return m_error;
    }

#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile uf = unzOpen2_64(filename.c_str(), &ffunc);
#else
    unzFile uf = unzOpen64(filename.c_str());
#endif
    if (!uf) {
        m_error = -1;
        m_str_error = std::string(UTILS::transl("It seems this is not a zip file"));
        return m_error;
    }

    m_files.clear();
    unz_global_info64 gi;
    unzGetGlobalInfo64(uf, &gi);

    while(1)
    {
        char filename_inzip[1024];
        unz_file_info64 file_info;
        int err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK)
        {
            m_error = err;
            snprintf(buf, sizeof(buf), "%s\n\n%s",
                     UTILS::transl("unzip error in unzGetCurrentFileInfo").c_str(),
                     unz_strerror(err));
            m_str_error = std::string(buf);
            m_files.clear();
            return m_error;
        }
        m_files.emplace_back(filename_inzip);

        err = unzGoToNextFile(uf);
        if (err == UNZ_END_OF_LIST_OF_FILE) {
            break;
        }
        if (err != UNZ_OK)
        {
            m_error = err;
            snprintf(buf, sizeof(buf), "%s\n\n%s",
                     UTILS::transl("unzip error in unzGoToNextFile").c_str(),
                     unz_strerror(err));
            m_str_error = std::string(buf);
            m_files.clear();
            return m_error;
        }
    }

    unzClose(uf);
    return 0;
}

int UNZP::unzip_one_file(const std::string &zipfilename, const std::string &filename, const std::string &out_filename)
{
    char errmsg[256];
    int ret, err;

    const char *unp_filename = out_filename.c_str();
    if (zipfilename.empty() || filename.empty() || out_filename.empty()) {
        m_error = -1;
        m_str_error = std::string(UTILS::transl("Invalid parameters in uzip_one_file"));
        return -1;
    }

    // 1. open zip file
#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile uf = unzOpen2_64(zipfilename.c_str(), &ffunc);
#else
    unzFile uf = unzOpen64(zipfilename.c_str());
#endif
    if (!uf) {
        m_error = -1;
        m_str_error = std::string(UTILS::transl("Can't open zip file"));
        return m_error;
    }

    // 2. check file exists in zip
    err = unzLocateFile(uf, filename.c_str(), CASESENSITIVITY);
    if (err)
    {
        m_error = err;
        snprintf(errmsg, sizeof(errmsg), "%s\n%s\n\n%s",
                 UTILS::transl("file not found in the zip").c_str(),
                 filename.c_str(), unz_strerror(m_error));
        m_str_error = std::string(errmsg);
        return m_error;
    }

    // 3. get extracting file info
    char filename_inzip[256];
    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (err != UNZ_OK)
    {
        m_error = err;
        snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                 UTILS::transl("unzip error in unzGetGlobalInfo").c_str(), unz_strerror(m_error));
        m_str_error = std::string(errmsg);
        return m_error;
    }

    err = unzOpenCurrentFilePassword(uf, NULL);
    if (err != UNZ_OK)
    {
        m_error = err;
        snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                 UTILS::transl("unzip error in unzOpenCurrentFilePassword").c_str(),
                 unz_strerror(m_error));
        m_str_error = std::string(errmsg);
        return m_error;
    }

    // 4. mem for unzipping
    uInt mem_size = WRITEBUFFERSIZE;
    void *mem = malloc(mem_size);
    if (mem == NULL)
    {
        m_error = errno;
        snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                 UTILS::transl("Error allocating memory").c_str(),
                 unz_strerror(UNZ_ERRNO));
        m_str_error = std::string(errmsg);
        return m_error;
    }

    // 5. file for writing
    FILE *fout = FOPEN_FUNC(unp_filename, "wb");
    if (fout == NULL)
    {
        m_error = err;
        snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                 UTILS::transl("Can't open file").c_str(),
                 unz_strerror(UNZ_ERRNO));
        m_str_error = std::string(errmsg);
        return m_error;
    }

    // 6. extracting
    do
    {
        ret = unzReadCurrentFile(uf, mem, mem_size);
        if (ret < 0)
        {
            m_error = ret;
            snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                     UTILS::transl("unzip error in unzReadCurrentFile").c_str(),
                     unz_strerror(ret));
            m_str_error = std::string(errmsg);
            break;
        }
        if (ret > 0) {
            if (fwrite(mem, (size_t)ret, 1, fout) != 1)
            {
                m_error = errno;
                snprintf(errmsg, sizeof(errmsg), "%s\n\n%s",
                         UTILS::transl("error in writing extracted file").c_str(),
                         unz_strerror(UNZ_ERRNO));
                m_str_error = std::string(errmsg);
                break;
            }
        }
    } while (ret > 0);
    fclose(fout);

    if (ret == 0) {
        change_file_date(unp_filename, file_info.dosDate, file_info.tmu_date);
    }

    unzCloseCurrentFile(uf);
    free(mem);
    unzClose(uf);
    return ret;
}
