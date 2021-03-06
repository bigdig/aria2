#include "FileEntry.h"

#include <cppunit/extensions/HelperMacros.h>

#include "InorderURISelector.h"
#include "util.h"

namespace aria2 {

class FileEntryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FileEntryTest);
  CPPUNIT_TEST(testRemoveURIWhoseHostnameIs);
  CPPUNIT_TEST(testExtractURIResult);
  CPPUNIT_TEST(testGetRequest);
  CPPUNIT_TEST(testGetRequest_withoutUriReuse);
  CPPUNIT_TEST(testGetRequest_withUniqueProtocol);
  CPPUNIT_TEST(testReuseUri);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testAddUris);
  CPPUNIT_TEST(testInsertUri);
  CPPUNIT_TEST(testRemoveUri);
  CPPUNIT_TEST(testPutBackRequest);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void testRemoveURIWhoseHostnameIs();
  void testExtractURIResult();
  void testGetRequest();
  void testGetRequest_withoutUriReuse();
  void testGetRequest_withUniqueProtocol();
  void testReuseUri();
  void testAddUri();
  void testAddUris();
  void testInsertUri();
  void testRemoveUri();
  void testPutBackRequest();
};


CPPUNIT_TEST_SUITE_REGISTRATION( FileEntryTest );

namespace {
SharedHandle<FileEntry> createFileEntry()
{
  const char* uris[] = { "http://localhost/aria2.zip",
                         "ftp://localhost/aria2.zip",
                         "http://mirror/aria2.zip" };
  SharedHandle<FileEntry> fileEntry(new FileEntry());
  fileEntry->setUris(std::vector<std::string>(&uris[0], &uris[3]));
  return fileEntry;
}
} // namespace

void FileEntryTest::testRemoveURIWhoseHostnameIs()
{
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->removeURIWhoseHostnameIs("localhost");
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
                       fileEntry->getRemainingUris()[0]);
}


void FileEntryTest::testExtractURIResult()
{
  FileEntry fileEntry;
  fileEntry.addURIResult("http://timeout/file", error_code::TIME_OUT);
  fileEntry.addURIResult("http://finished/file", error_code::FINISHED);
  fileEntry.addURIResult("http://timeout/file2", error_code::TIME_OUT);
  fileEntry.addURIResult("http://unknownerror/file", error_code::UNKNOWN_ERROR);

  std::deque<URIResult> res;
  fileEntry.extractURIResult(res, error_code::TIME_OUT);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://timeout/file"), res[0].getURI());
  CPPUNIT_ASSERT_EQUAL(std::string("http://timeout/file2"), res[1].getURI());

  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry.getURIResults().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://finished/file"),
                       fileEntry.getURIResults()[0].getURI());
  CPPUNIT_ASSERT_EQUAL(std::string("http://unknownerror/file"),
                       fileEntry.getURIResults()[1].getURI());

  res.clear();

  fileEntry.extractURIResult(res, error_code::TIME_OUT);
  CPPUNIT_ASSERT(res.empty());
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry.getURIResults().size());
}

void FileEntryTest::testGetRequest()
{
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<Request> req =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());
  fileEntry->poolRequest(req);

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req3rd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req3rd->getProtocol());

  SharedHandle<Request> req4th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(!req4th);

  fileEntry->setMaxConnectionPerServer(2);

  SharedHandle<Request> req5th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req5th->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req5th->getProtocol());

  SharedHandle<Request> req6th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req6th->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req6th->getProtocol());

  SharedHandle<Request> req7th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(!req7th);
}

void FileEntryTest::testGetRequest_withoutUriReuse()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setMaxConnectionPerServer(2);
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  SharedHandle<Request> req = fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req3rd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req3rd->getProtocol());

  SharedHandle<Request> req4th =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT(!req4th);
}

void FileEntryTest::testGetRequest_withUniqueProtocol()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setUniqueProtocol(true);
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  SharedHandle<Request> req =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(!req3rd);

  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"),
                       fileEntry->getRemainingUris()[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
                       fileEntry->getRemainingUris()[1]);
}

void FileEntryTest::testReuseUri()
{
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setMaxConnectionPerServer(3);
  size_t numUris = fileEntry->getRemainingUris().size();
  std::vector<std::pair<size_t, std::string> > usedHosts;
  for(size_t i = 0; i < numUris; ++i) {
    fileEntry->getRequest(selector, false, usedHosts);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)0, fileEntry->getRemainingUris().size());
  fileEntry->addURIResult("http://localhost/aria2.zip",
                          error_code::UNKNOWN_ERROR);
  std::vector<std::string> ignore;
  fileEntry->reuseUri(ignore);
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry->getRemainingUris().size());
  std::deque<std::string> uris = fileEntry->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"), uris[1]);
  for(size_t i = 0; i < 2; ++i) {
    fileEntry->getRequest(selector, false, usedHosts);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)0, fileEntry->getRemainingUris().size());
  ignore.clear();
  ignore.push_back("mirror");
  fileEntry->reuseUri(ignore);
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry->getRemainingUris().size());
  uris = fileEntry->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"), uris[0]);
}

void FileEntryTest::testAddUri()
{
  FileEntry file;
  CPPUNIT_ASSERT(file.addUri("http://good"));
  CPPUNIT_ASSERT(!file.addUri("bad"));
  // Test for percent-encode
  CPPUNIT_ASSERT(file.addUri("http://host:80/file<with%2 %20space/"
                             "file with space;param%?a=/?"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80"
                                   "/file%3Cwith%2%20%20space/"
                                   "file%20with%20space;param%"
                                   "?a=/?"),
                       file.getRemainingUris()[1]);
}

void FileEntryTest::testAddUris()
{
  FileEntry file;
  std::string uris[] = {"bad", "http://good"};
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.addUris(&uris[0], &uris[2]));
}

void FileEntryTest::testInsertUri()
{
  FileEntry file;
  CPPUNIT_ASSERT(file.insertUri("http://example.org/1", 0));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/2", 0));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/3", 1));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/4", 5));
  std::deque<std::string> uris = file.getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/2"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/3"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/1"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/4"), uris[3]);
  // Test for percent-encode
  CPPUNIT_ASSERT(file.insertUri("http://host:80/file<with%2 %20space/"
                                "file with space;param%?a=/?", 0));

  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80"
                                   "/file%3Cwith%2%20%20space/"
                                   "file%20with%20space;param%"
                                   "?a=/?"),
                       file.getRemainingUris()[0]);
}

void FileEntryTest::testRemoveUri()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  FileEntry file;
  file.addUri("http://example.org/");
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT(file.getRemainingUris().empty());
  CPPUNIT_ASSERT(!file.removeUri("http://example.org/"));

  file.addUri("http://example.org/");
  SharedHandle<Request> exampleOrgReq =
    file.getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(!exampleOrgReq->removalRequested());
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.getSpentUris().size());
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT(file.getSpentUris().empty());
  CPPUNIT_ASSERT(exampleOrgReq->removalRequested());
  file.poolRequest(exampleOrgReq);
  CPPUNIT_ASSERT_EQUAL((size_t)0, file.countPooledRequest());

  file.addUri("http://example.org/");
  exampleOrgReq = file.getRequest(selector, true, usedHosts);
  file.poolRequest(exampleOrgReq);
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.countPooledRequest());
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT_EQUAL((size_t)0, file.countPooledRequest());
  CPPUNIT_ASSERT(file.getSpentUris().empty());

  file.addUri("http://example.org/");
  CPPUNIT_ASSERT(!file.removeUri("http://example.net"));
}

void FileEntryTest::testPutBackRequest()
{
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  SharedHandle<InorderURISelector> selector(new InorderURISelector());
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<Request> req1 =
    fileEntry->getRequest(selector, false, usedHosts);
  SharedHandle<Request> req2 =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry->getRemainingUris().size());
  fileEntry->poolRequest(req2);
  fileEntry->putBackRequest();
  const std::deque<std::string>& uris = fileEntry->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/aria2.zip"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"), uris[2]);
}

} // namespace aria2
