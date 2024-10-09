//
//  FileHash.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/9/24.
//

#include "FileHash.hpp"

#include <CommonCrypto/CommonDigest.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <boost/scope_exit.hpp>

#include "Utilities.h"


BEGIN_NAMESPACE_MW


std::string computeFileHash(const std::string &filePath) {
    //
    // Open file
    //
    
    auto fd = ::open(filePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot open file", strerror(errno));
    }
    
    BOOST_SCOPE_EXIT( &fd ) {
        if (-1 == ::close(fd)) {
            merror(M_FILE_MESSAGE_DOMAIN, "Cannot close file: %s", strerror(errno));
        }
    } BOOST_SCOPE_EXIT_END
    
    //
    // Get file status (for st_blksize)
    //
    
    struct stat fileStat;
    if (-1 == fstat(fd, &fileStat)) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot obtain file status", strerror(errno));
    }
    
    //
    // Compute SHA-1 message digest of the file's contents
    //
    
    CC_SHA1_CTX ctx;
    if (!CC_SHA1_Init(&ctx)) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot initialize SHA-1 hash computation");
    }
    
    std::vector<std::uint8_t> readBuf(fileStat.st_blksize);
    while (true) {
        auto readSize = ::read(fd, readBuf.data(), readBuf.size());
        if (-1 == readSize) {
            throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot read file", strerror(errno));
        }
        if (0 == readSize) {
            // End of file
            break;
        }
        if (!CC_SHA1_Update(&ctx, readBuf.data(), readSize)) {
            throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot update SHA-1 hash computation");
        }
    }
    
    std::array<unsigned char, CC_SHA1_DIGEST_LENGTH> digest;
    if (!CC_SHA1_Final(digest.data(), &ctx)) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot finalize SHA-1 hash computation");
    }
    
    //
    // Convert message digest to a hexadecimal string and return the result
    //
    
    std::ostringstream os;
    os.fill('0');
    os << std::hex;
    for (auto byte : digest) {
        os << std::setw(2) << static_cast<unsigned int>(byte);
    }
    
    return os.str();
}


END_NAMESPACE_MW
