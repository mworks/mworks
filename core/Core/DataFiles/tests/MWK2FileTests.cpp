//
//  MWK2FileTests.cpp
//  MWorksCoreTest
//
//  Created by Christopher Stawarz on 9/7/18.
//

#include "MWK2FileTests.hpp"

#include <random>

#include "MWorksCore/MWK2File.hpp"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


class NamedTempFile : boost::noncopyable {
    
public:
    explicit NamedTempFile(bool create = false) :
        filename(filenameTemplate.begin(), filenameTemplate.end() + 1),  // Include NUL terminator
        fd(-1)
    {
        if (!create) {
            mktemp(filename.data());
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
    
    CPPUNIT_ASSERT_EQUAL( std::size_t(1), reader.getNumEvents() );
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
    
    // Min time greater than max time
    reader.selectEvents({}, 3, 2);
    CPPUNIT_ASSERT( !reader.nextEvent(code, time, data) );
    
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


END_NAMESPACE_MW
