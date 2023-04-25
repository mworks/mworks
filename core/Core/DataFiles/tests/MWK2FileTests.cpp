//
//  MWK2FileTests.cpp
//  MWorksCoreTest
//
//  Created by Christopher Stawarz on 9/7/18.
//

#include "MWK2FileTests.hpp"

#include <random>

#include <compression.h>

#include "MWorksCore/MWK2File.hpp"
#include "MWorksCore/MessagePackAdaptors.hpp"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


class NamedTempFile : boost::noncopyable {
    
public:
    explicit NamedTempFile(bool create = false) :
        filename(filenameTemplate.begin(), filenameTemplate.end() + 1),  // Include NUL terminator
        fd(-1)
    {
        if (!create) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            mktemp(filename.data());
#pragma clang diagnostic pop
        } else if (-1 == (fd = mkstemp(filename.data()))) {
            throw std::runtime_error("Cannot create named temporary file");
        }
    }
    
    ~NamedTempFile() {
        if (-1 != fd) {
            (void)close(fd);
        }
        (void)unlink(filename.data());
    }
    
    const char * getFilename() const {
        return filename.data();
    }
    
private:
    static const std::string filenameTemplate;
    
    std::vector<char> filename;
    int fd;
    
};


const std::string NamedTempFile::filenameTemplate("/tmp/mwk2_test_file_XXXXXX");


inline void assertEqualStrings(const std::string &expected, const std::string &actual) {
    CPPUNIT_ASSERT_EQUAL( expected, actual );
}


END_NAMESPACE()


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MWK2FileTests, "Unit Test" );


void MWK2FileTests::testFileAlreadyExists() {
    NamedTempFile tempFile(true);
    try {
        MWK2Writer writer(tempFile.getFilename());
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Cannot create data file: File exists", e.what() );
    }
}


void MWK2FileTests::testFileDoesNotExist() {
    try {
        MWK2Reader reader("not_a_file.mwk2");
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Cannot open data file: No such file or directory", e.what() );
    }
}


void MWK2FileTests::testInvalidFile() {
    NamedTempFile tempFile;
    
    {
        auto fp = fopen(tempFile.getFilename(), "wxb");
        CPPUNIT_ASSERT( fp );
        // Write some garbage
        for (unsigned char i = 0; i < 255; i++) {
            CPPUNIT_ASSERT_EQUAL( std::size_t(1), fwrite(&i, 1, 1, fp) );
        }
        CPPUNIT_ASSERT_EQUAL( 0, fclose(fp) );
    }
    
    try {
        MWK2Reader reader(tempFile.getFilename());
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Cannot open data file: file is not a database (26)", e.what() );
    }
}


void MWK2FileTests::testWriterLock() {
    NamedTempFile tempFile;
    MWK2Writer writer(tempFile.getFilename());
    try {
        MWK2Reader reader(tempFile.getFilename());
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Cannot open data file: database is locked (5)", e.what() );
    }
}


void MWK2FileTests::testEmptyFile() {
    NamedTempFile tempFile;
    
    {
        MWK2Writer writer(tempFile.getFilename());
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( std::size_t(0), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(0), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(0), reader.getTimeMax() );
    
    // Default selection
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Explicit selection
    reader.selectEvents();
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // OK to call nextEvent again after running out of events
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testDataTypes() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(false),
        Datum(true),
        Datum(0),
        Datum(123),
        Datum(-45),
        Datum(0.0),
        Datum(1.5),
        Datum(-2.5),
        Datum(""),
        Datum("abcdef"),
        Datum("abc\0def"),
        Datum(Datum::list_value_type {}),
        Datum(Datum::list_value_type { Datum(true), Datum(2), Datum(3.0) }),
        Datum(Datum::dict_value_type {}),
        Datum(Datum::dict_value_type { { Datum("a"), Datum(false) }, { Datum(2.0), Datum("b") } }),
        Datum()
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            writer.writeEvent(i + 1, i + 2, testData.at(i));
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(testData.size() + 1), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( int(i + 1), code );
        CPPUNIT_ASSERT_EQUAL( MWTime(i + 2), time );
        
        auto &expectedData = testData.at(i);
        auto expectedDataType = expectedData.getDataType();
        if (expectedDataType == M_BOOLEAN) {
            // Bare booleans are stored as integers
            expectedDataType = M_INTEGER;
        }
        
        CPPUNIT_ASSERT_EQUAL( expectedDataType, data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testEventConcatenation() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(123),
        Datum(1.5),
        Datum("abcdef"),
        Datum(Datum::list_value_type { Datum(true), Datum(2), Datum(3.0) }),
        Datum()
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            writer.writeEvent(1, 2, testData.at(i));
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        code = -1;
        time = -1;
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( 1, code );
        CPPUNIT_ASSERT_EQUAL( MWTime(2), time );
        
        auto &expectedData = testData.at(i);
        CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testSorting() {
    NamedTempFile tempFile;
    
    const std::vector<std::pair<int, MWTime>> testData = {
        { 1, 1 },
        { 1, 2 },
        { 2, 1 },
        { 2, 2 },
        { 2, 3 },
        { 3, 1 },
        { 4, 1 },
        { 4, 2 }
    };
    
    {
        std::random_device randDev;
        std::mt19937 randGen(randDev());
        auto shuffledData = testData;
        std::shuffle(shuffledData.begin(), shuffledData.end(), randGen);
        CPPUNIT_ASSERT( !(shuffledData == testData) );
        
        MWK2Writer writer(tempFile.getFilename());
        for (auto &item : shuffledData) {
            writer.writeEvent(item.first, item.second, Datum(0));
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(1), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(3), reader.getTimeMax() );
    
    for (auto &item : testData) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( item.first, code );
        CPPUNIT_ASSERT_EQUAL( item.second, time );
        CPPUNIT_ASSERT_EQUAL( M_INTEGER, data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( Datum(0), data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testSelection() {
    NamedTempFile tempFile;
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (int i = 1; i <= 5; i++) {
            for (MWTime j = 1; j <= 4; j++) {
                writer.writeEvent(i, j, Datum(1.5));
            }
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( std::size_t(20), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(1), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(4), reader.getTimeMax() );
    
    // Everything
    reader.selectEvents();
    for (int i = 1; i <= 5; i++) {
        for (MWTime j = 1; j <= 4; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // One code
    reader.selectEvents({ 3 });
    for (MWTime j = 1; j <= 4; j++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        CPPUNIT_ASSERT_EQUAL( 3, code );
        CPPUNIT_ASSERT_EQUAL( j, time );
        CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Multiple codes
    reader.selectEvents({ 2, 4 });
    for (int i = 2; i <= 4; i += 2) {
        for (MWTime j = 1; j <= 4; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // No matching codes
    reader.selectEvents({ 6, 7 });
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Min time
    reader.selectEvents({}, 2);
    for (int i = 1; i <= 5; i++) {
        for (MWTime j = 2; j <= 4; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // No times greater than min time
    reader.selectEvents({}, 5);
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Max time
    reader.selectEvents({}, MIN_MWORKS_TIME(), 3);
    for (int i = 1; i <= 5; i++) {
        for (MWTime j = 1; j <= 3; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // No times less than max time
    reader.selectEvents({}, MIN_MWORKS_TIME(), 0);
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Min time and max time
    reader.selectEvents({}, 2, 3);
    for (int i = 1; i <= 5; i++) {
        for (MWTime j = 2; j <= 3; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Min time equal to max time
    reader.selectEvents({}, 2, 2);
    for (int i = 1; i <= 5; i++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        CPPUNIT_ASSERT_EQUAL( i, code );
        CPPUNIT_ASSERT_EQUAL( MWTime(2), time );
        CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    // Min time greater than max time
    try {
        reader.selectEvents({}, 3, 2);
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Minimum event time must be less than or equal to maximum event time", e.what() );
    }
    
    // Codes, min time, and max time
    reader.selectEvents({ 2, 4 }, 2, 3);
    for (int i = 2; i <= 4; i += 2) {
        for (MWTime j = 2; j <= 3; j++) {
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            CPPUNIT_ASSERT_EQUAL( i, code );
            CPPUNIT_ASSERT_EQUAL( j, time );
            CPPUNIT_ASSERT_EQUAL( Datum(1.5), data );
        }
    }
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testUnpackingBufferNotExhausted() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(123),
        Datum(1.5),
        Datum("abcdef"),
        Datum(Datum::list_value_type { Datum(true), Datum(2), Datum(3.0) })
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size() / 2; i++) {
            writer.writeEvent(1, 2, testData.at(i));
        }
        for (std::size_t i = testData.size() / 2; i < testData.size(); i++) {
            writer.writeEvent(1, 3, testData.at(i));
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(3), reader.getTimeMax() );
    
    // Make a selection that returns a pair of concatenated events, but read only the first event
    reader.selectEvents({ 1 }, 2, 2);
    {
        code = -1;
        time = -1;
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( 1, code );
        CPPUNIT_ASSERT_EQUAL( MWTime(2), time );
        
        auto &expectedData = testData.at(0);
        CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    // Make a new selection that returns a different pair of concatenated events, and read both
    reader.selectEvents({ 1 }, 3, 3);
    for (std::size_t i = testData.size() / 2; i < testData.size(); i++) {
        code = -1;
        time = -1;
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( 1, code );
        CPPUNIT_ASSERT_EQUAL( MWTime(3), time );
        
        auto &expectedData = testData.at(i);
        CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testWriteEvent() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(123),
        Datum(-2.5),
        Datum("abcdef"),
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            auto event = boost::make_shared<Event>(i + 1, i + 2, testData.at(i));
            writer.writeEvent(event);
        }
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(4), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( int(i + 1), code );
        CPPUNIT_ASSERT_EQUAL( MWTime(i + 2), time );
        
        auto &expectedData = testData.at(i);
        CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testWriteEvents() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(123),
        Datum(-2.5),
        Datum("abcdef"),
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        std::vector<boost::shared_ptr<Event>> events;
        for (std::size_t i = 0; i < testData.size(); i++) {
            events.push_back(boost::make_shared<Event>(i + 1, i + 2, testData.at(i)));
        }
        writer.writeEvents(events);
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(4), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( int(i + 1), code );
        CPPUNIT_ASSERT_EQUAL( MWTime(i + 2), time );
        
        auto &expectedData = testData.at(i);
        CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
        CPPUNIT_ASSERT_EQUAL( expectedData, data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testInvalidEventData() {
    NamedTempFile tempFile;
    
    const std::vector<Datum> testData = {
        Datum(123),
        Datum(Datum::list_value_type { Datum(1), Datum(2), Datum(3) }),
        Datum(456)
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            writer.writeEvent(i + 1, i + 2, testData.at(i));
        }
    }
    
    {
        sqlite3 *conn = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_open_v2(tempFile.getFilename(),
                                                         &conn,
                                                         SQLITE_OPEN_READWRITE,
                                                         nullptr) );
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_exec(conn,
                                                      R"(
                                                      UPDATE events
                                                      /* This is the msgpack representation of the list, but
                                                         truncated to omit the third item */
                                                      SET data = x'930102'
                                                      WHERE code = 2
                                                      )",
                                                      nullptr,
                                                      nullptr,
                                                      nullptr) );
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_close(conn) );
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    try {
        reader.getNumEvents();
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Cannot determine number of events in data file: SQL logic error (1)", e.what() );
    }
    
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(4), reader.getTimeMax() );
    
    CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
    CPPUNIT_ASSERT_EQUAL( 1, code );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), time );
    CPPUNIT_ASSERT_EQUAL( testData.at(0), data );
    
    try {
        reader.nextEvent(code, time, data);
        CPPUNIT_FAIL( "Exception not thrown" );
    } catch (const SimpleException &e) {
        assertEqualStrings( "Data file contains invalid or corrupt event data", e.what() );
    }
}


static std::string compressEventData(const std::string &data, int extTypeCode) {
    std::vector<char> compressionBuffer(data.size());
    auto compressedSize = compression_encode_buffer(reinterpret_cast<std::uint8_t *>(compressionBuffer.data()),
                                                    compressionBuffer.size(),
                                                    reinterpret_cast<const std::uint8_t *>(data.data()),
                                                    data.size(),
                                                    nullptr,
                                                    COMPRESSION_ZLIB);
    CPPUNIT_ASSERT( compressedSize > 0 && compressedSize < data.size() );
    
    std::stringstream packingBuffer;
    msgpack::packer<std::stringstream> packer(packingBuffer);
    packer.pack_ext(compressedSize, extTypeCode);
    packer.pack_ext_body(compressionBuffer.data(), compressedSize);
    return packingBuffer.str();
}


void MWK2FileTests::testEventDataCompression() {
    std::vector<std::string> testData(4);
    for (std::size_t i = 0; i < 100; i++) {
        // Uncompressible text
        testData.at(0).push_back(i + 1);
        // Compressible text
        testData.at(1).push_back((i % 5) + 1);
        // Uncompressible blob
        testData.at(2).push_back(i);
        // Compressible blob
        testData.at(3).push_back(i % 5);
    }
    testData.push_back(testData.at(1));  // Compressible text marked not compressible
    testData.push_back(testData.at(3));  // Compressible blob marked not compressible
    
    constexpr std::array<bool, 6> testDataCompressibleFlag { true, true, true, true, false, false };
    CPPUNIT_ASSERT_EQUAL( testDataCompressibleFlag.size(), testData.size() );  // Sanity check
    
    NamedTempFile tempFile;
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            Datum value(testData.at(i));
            if (!(testDataCompressibleFlag.at(i))) {
                value.setCompressible(false);
            }
            writer.writeEvent(i + 1, i + 2, std::move(value));
        }
    }
    
    {
        sqlite3 *conn = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_open_v2(tempFile.getFilename(),
                                                         &conn,
                                                         SQLITE_OPEN_READONLY,
                                                         nullptr) );
        sqlite3_stmt *stmt = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_prepare(conn,
                                                         "SELECT data FROM events",
                                                         -1,
                                                         &stmt,
                                                         nullptr) );
        
        // Uncompressible text
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_TEXT, sqlite3_column_type(stmt, 0) );
            auto text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            CPPUNIT_ASSERT_EQUAL( testData.at(0), std::string(text, size) );
        }
        
        // Compressible text
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_BLOB, sqlite3_column_type(stmt, 0) );
            auto blob = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            CPPUNIT_ASSERT_EQUAL( compressEventData(testData.at(1), 1), std::string(blob, size) );
        }
        
        // Uncompressible blob
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_BLOB, sqlite3_column_type(stmt, 0) );
            
            auto blob = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            
            std::stringstream buffer;
            msgpack::packer<std::stringstream> packer(buffer);
            packer.pack_bin(testData.at(2).size());
            packer.pack_bin_body(testData.at(2).data(), testData.at(2).size());
            
            CPPUNIT_ASSERT_EQUAL( buffer.str(), std::string(blob, size) );
        }
        
        // Compressible blob
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_BLOB, sqlite3_column_type(stmt, 0) );
            
            auto blob = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            
            std::stringstream buffer;
            msgpack::packer<std::stringstream> packer(buffer);
            packer.pack_bin(testData.at(3).size());
            packer.pack_bin_body(testData.at(3).data(), testData.at(3).size());

            CPPUNIT_ASSERT_EQUAL( compressEventData(buffer.str(), 2), std::string(blob, size) );
        }
        
        // Compressible text marked not compressible
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_TEXT, sqlite3_column_type(stmt, 0) );
            auto text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            CPPUNIT_ASSERT_EQUAL( testData.at(4), std::string(text, size) );
        }
        
        // Compressible blob marked not compressible
        {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_BLOB, sqlite3_column_type(stmt, 0) );
            
            auto blob = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            
            std::stringstream buffer;
            msgpack::packer<std::stringstream> packer(buffer);
            packer.pack_bin(testData.at(5).size());
            packer.pack_bin_body(testData.at(5).data(), testData.at(5).size());
            
            CPPUNIT_ASSERT_EQUAL( buffer.str(), std::string(blob, size) );
        }
        
        CPPUNIT_ASSERT_EQUAL( SQLITE_DONE, sqlite3_step(stmt) );
        (void)sqlite3_finalize(stmt);
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_close(conn) );
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( testData.size(), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(7), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
        
        CPPUNIT_ASSERT_EQUAL( int(i + 1), code );
        CPPUNIT_ASSERT_EQUAL( MWTime(i + 2), time );
        
        auto &expectedData = testData.at(i);
        CPPUNIT_ASSERT_EQUAL( Datum(expectedData), data );
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testEventConcatenationWithCompression() {
    NamedTempFile tempFile;
    
    std::string compressibleText, compressibleBlob;
    for (int i = 0; i < 100; i++) {
        compressibleText += 'a' + (i % 5);
        compressibleBlob += i % 5;
    }
    
    const std::vector<std::vector<Datum>> testData = {
        { Datum(1), Datum(2) },  // New value is not compressed
        { Datum(1), Datum(compressibleText) },  // New value is compressed text
        { Datum(1), Datum(compressibleBlob) },  // New value is compressed blob
        { Datum(1), Datum(2), Datum(compressibleText) },  // Old value is uncompressed blob
        { Datum(1), Datum(compressibleText), Datum(compressibleBlob) }  // Old value is compressed blob
    };
    
    {
        MWK2Writer writer(tempFile.getFilename());
        for (std::size_t i = 0; i < testData.size(); i++) {
            for (auto &data : testData.at(i)) {
                writer.writeEvent(i+1, i+2, data);
            }
        }
    }
    
    {
        sqlite3 *conn = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_open_v2(tempFile.getFilename(),
                                                         &conn,
                                                         SQLITE_OPEN_READONLY,
                                                         nullptr) );
        sqlite3_stmt *stmt = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_prepare(conn,
                                                         "SELECT data FROM events",
                                                         -1,
                                                         &stmt,
                                                         nullptr) );
        
        for (std::size_t i = 0; i < testData.size(); i++) {
            CPPUNIT_ASSERT_EQUAL( SQLITE_ROW, sqlite3_step(stmt) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_BLOB, sqlite3_column_type(stmt, 0) );
            
            auto blob = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
            auto size = sqlite3_column_bytes(stmt, 0);
            
            std::stringstream buffer;
            msgpack::packer<std::stringstream> packer(buffer);
            for (auto &data : testData.at(i)) {
                packer.pack(data);
            }
            
            if (i == 0) {
                CPPUNIT_ASSERT_EQUAL( buffer.str(), std::string(blob, size) );
            } else {
                CPPUNIT_ASSERT_EQUAL( compressEventData(buffer.str(), 2), std::string(blob, size) );
            }
        }
        
        CPPUNIT_ASSERT_EQUAL( SQLITE_DONE, sqlite3_step(stmt) );
        (void)sqlite3_finalize(stmt);
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_close(conn) );
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    CPPUNIT_ASSERT_EQUAL( std::size_t(12), reader.getNumEvents() );
    CPPUNIT_ASSERT_EQUAL( MWTime(2), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(6), reader.getTimeMax() );
    
    for (std::size_t i = 0; i < testData.size(); i++) {
        for (auto &expectedData : testData.at(i)) {
            code = -1;
            time = -1;
            CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
            
            CPPUNIT_ASSERT_EQUAL( int(i+1), code );
            CPPUNIT_ASSERT_EQUAL( MWTime(i+2), time );
            
            CPPUNIT_ASSERT_EQUAL( expectedData.getDataType(), data.getDataType() );
            CPPUNIT_ASSERT_EQUAL( expectedData, data );
        }
    }
    
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
}


void MWK2FileTests::testInvalidCompressedData() {
    NamedTempFile tempFile;
    
    std::string compressibleText;
    for (int i = 0; i < 100; i++) {
        compressibleText += 'a' + (i % 5);
    }
    
    {
        MWK2Writer writer(tempFile.getFilename());
        writer.writeEvent(1, 1, Datum(compressibleText));
    }
    
    std::vector<std::string> invalidData = {
        compressEventData(compressibleText, 1) + compressibleText,  // Extra data after valid compressed data
        compressEventData(compressibleText, 3)  // Unrecognized msgpack extension type
    };
    
    // Invalid compressed data (decode error)
    {
        auto data = compressEventData(compressibleText, 1);
        for (std::size_t i = data.size() / 2; i < data.size(); i++) {
            data.at(i) = i;
        }
        invalidData.emplace_back(data);
    }
    
    {
        sqlite3 *conn = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_open_v2(tempFile.getFilename(),
                                                         &conn,
                                                         SQLITE_OPEN_READWRITE,
                                                         nullptr) );
        sqlite3_stmt *stmt = nullptr;
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_prepare(conn,
                                                         "INSERT INTO events VALUES (?, ?, ?)",
                                                         -1,
                                                         &stmt,
                                                         nullptr) );
        
        for (std::size_t i = 0; i < invalidData.size(); i++) {
            CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_bind_int(stmt, 1, i+2) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_bind_int64(stmt, 2, i+2) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_bind_blob64(stmt,
                                                                 3,
                                                                 invalidData.at(i).data(),
                                                                 invalidData.at(i).size(),
                                                                 SQLITE_TRANSIENT) );
            CPPUNIT_ASSERT_EQUAL( SQLITE_DONE, sqlite3_step(stmt) );
            (void)sqlite3_reset(stmt);
        }
        
        (void)sqlite3_finalize(stmt);
        CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_close(conn) );
    }
    
    MWK2Reader reader(tempFile.getFilename());
    
    // The extra data in invalidData[0] is misinterpreted as additional events
    CPPUNIT_ASSERT( reader.getNumEvents() > 4 );
    
    CPPUNIT_ASSERT_EQUAL( MWTime(1), reader.getTimeMin() );
    CPPUNIT_ASSERT_EQUAL( MWTime(4), reader.getTimeMax() );
    
    reader.selectEvents({ 1 });
    CPPUNIT_ASSERT( reader.nextEvent(code, time, data) );
    CPPUNIT_ASSERT_EQUAL( 1, code );
    CPPUNIT_ASSERT_EQUAL( MWTime(1), time );
    CPPUNIT_ASSERT_EQUAL( Datum(compressibleText), data );
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
    for (std::size_t i = 0; i < invalidData.size(); i++) {
        reader.selectEvents({ int(i + 2) });
        try {
            reader.nextEvent(code, time, data);
            CPPUNIT_FAIL( "Exception not thrown" );
        } catch (const SimpleException &e) {
            assertEqualStrings( "Data file contains invalid or corrupt event data", e.what() );
        }
    }
}


void MWK2FileTests::testReaderLock() {
    NamedTempFile tempFile;
    {
        MWK2Writer writer(tempFile.getFilename());
        writer.writeEvent(1, 2, 3);
        writer.writeEvent(4, 5, 6);
    }
    
    {
        MWK2Reader reader1(tempFile.getFilename());
        CPPUNIT_ASSERT_EQUAL( std::size_t(2), reader1.getNumEvents() );
        
        {
            // Multiple concurrent readers is OK
            MWK2Reader reader2(tempFile.getFilename());
            CPPUNIT_ASSERT_EQUAL( std::size_t(2), reader2.getNumEvents() );
            
            // Attempting to write while there are readers causes the write to
            // be delayed until all readers are closed
            {
                sqlite3 *conn = nullptr;
                CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_open_v2(tempFile.getFilename(),
                                                                 &conn,
                                                                 SQLITE_OPEN_READWRITE,
                                                                 nullptr) );
                CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_exec(conn,
                                                              "INSERT INTO events VALUES (7, 8, 9)",
                                                              nullptr,
                                                              nullptr,
                                                              nullptr) );
                CPPUNIT_ASSERT_EQUAL( SQLITE_OK, sqlite3_close(conn) );
            }
            
            // Event count unchanged
            CPPUNIT_ASSERT_EQUAL( std::size_t(2), reader2.getNumEvents() );
        }
        
        // Event count unchanged
        CPPUNIT_ASSERT_EQUAL( std::size_t(2), reader1.getNumEvents() );
    }
    
    MWK2Reader reader(tempFile.getFilename());
    // Event count changed
    CPPUNIT_ASSERT_EQUAL( std::size_t(3), reader.getNumEvents() );
}


END_NAMESPACE_MW
