#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <io/file_system_io.h>

using namespace pandora::io;

class FileSystemIOTest : public testing::Test {
public:
protected:
  static void SetUpTestCase() {
    isCaseSetupSuccess = false;
    uint32_t retries = 0;
    while (createDirectory(testCaseDir) != 0 && ++retries <= 5u) { removeDirectory(testCaseDir); std::this_thread::sleep_for(std::chrono::milliseconds(1LL)); }

    retries = 0;
    while (!isCaseSetupSuccess && ++retries <= 5u) {
      FileHandle fp = openFileEntry(testCaseFile.c_str(), fileCreateMode.c_str());
      if (fp.isOpen()) {
        fputs("directory_test", fp.handle());
        isCaseSetupSuccess = true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1LL));
    }
  }
  static void TearDownTestCase() {
    removeFileEntry(testCaseFile);
    removeDirectory(testCaseDir);
  }

  void SetUp() override {
    ASSERT_TRUE(isCaseSetupSuccess);
  }
  void TearDown() override {}

protected:
  static std::string testCaseFile;
  static std::string testCaseFileName;
  static std::string testCaseDir;
  static std::string fileCreateMode;
# ifdef _WINDOWS
    static std::wstring testCaseWFile;
    static std::wstring testCaseWFileName;
    static std::wstring testCaseWDir;
    static std::wstring wfileCreateMode;
# endif
private:
  static bool isCaseSetupSuccess;
};

std::string FileSystemIOTest::testCaseFile = "./testDir/testFile.txt";
std::string FileSystemIOTest::testCaseFileName = "testFile.txt";
std::string FileSystemIOTest::testCaseDir = "testDir";
std::string FileSystemIOTest::fileCreateMode = "w";
#ifdef _WINDOWS
  std::wstring FileSystemIOTest::testCaseWFile = L"./testDir/testFile.txt";
  std::wstring FileSystemIOTest::testCaseWFileName = L"testFile.txt";
  std::wstring FileSystemIOTest::testCaseWDir = L"testDir";
  std::wstring FileSystemIOTest::wfileCreateMode = L"w";
#endif
bool FileSystemIOTest::isCaseSetupSuccess = false;

#ifdef _WINDOWS
# define _FileSystemEntry FileSystemEntry<false>
# define _FileSystemEntryMetadata FileSystemEntryMetadata<false>
#else
# define _FileSystemEntry FileSystemEntry
# define _FileSystemEntryMetadata FileSystemEntryMetadata
#endif


// -- constructors/accessors --

TEST_F(FileSystemIOTest, ctorsAccessors) {
  _FileSystemEntry testDirEmpty;
  EXPECT_TRUE(testDirEmpty.path().empty());
  EXPECT_EQ(FileSystemPathType::relative, testDirEmpty.pathType());
  EXPECT_EQ(FileSystemPathType::relative, getFileSystemPathType(testDirEmpty.path().c_str()));
  EXPECT_EQ(FileSystemEntryType::unknown, testDirEmpty.entryType());
  EXPECT_EQ(FileSystemEntryType::unknown, getFileSystemEntryType(testDirEmpty.path().c_str()));
  EXPECT_TRUE(testDirEmpty.pathSegments().empty());
  EXPECT_TRUE(getFileSystemPathSegments(testDirEmpty.path()).empty());
# ifdef _WINDOWS
    FileSystemEntry<true> wtestDirEmpty;
    EXPECT_TRUE(wtestDirEmpty.path().empty());
    EXPECT_EQ(FileSystemPathType::relative, wtestDirEmpty.pathType());
    EXPECT_EQ(FileSystemPathType::relative, getFileSystemPathType(wtestDirEmpty.path().c_str()));
    EXPECT_EQ(FileSystemEntryType::unknown, wtestDirEmpty.entryType());
    EXPECT_EQ(FileSystemEntryType::unknown, getFileSystemEntryType(wtestDirEmpty.path().c_str()));
    EXPECT_TRUE(wtestDirEmpty.pathSegments().empty());
    EXPECT_TRUE(getFileSystemPathSegments(wtestDirEmpty.path()).empty());
# endif

  char testDirInit[2]{ '.', '\0' };
  _FileSystemEntry testDir(testDirInit);
  EXPECT_EQ(std::string("."), testDir.path());
  EXPECT_EQ(FileSystemPathType::relative, testDir.pathType());
  EXPECT_EQ(FileSystemPathType::relative, getFileSystemPathType(testDir.path().c_str()));
  EXPECT_EQ(FileSystemEntryType::directory, testDir.entryType());
  EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testDir.path().c_str()));
  EXPECT_EQ(size_t{ 1u }, getFileSystemPathSegments(testDir.path()).size());
  auto segments = testDir.pathSegments();
  ASSERT_EQ(size_t{ 1u }, segments.size());
  EXPECT_EQ(std::string("."), segments[0]);
# ifdef _WINDOWS
    wchar_t testWDirInit[2]{ L'.', L'\0' };
    FileSystemEntry<true> testWDir(testWDirInit);
    EXPECT_EQ(std::wstring(L"."), testWDir.path());
    EXPECT_EQ(FileSystemPathType::relative, testWDir.pathType());
    EXPECT_EQ(FileSystemPathType::relative, getFileSystemPathType(testWDir.path().c_str()));
    EXPECT_EQ(FileSystemEntryType::directory, testWDir.entryType());
    EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testWDir.path().c_str()));
    EXPECT_EQ(size_t{ 1u }, getFileSystemPathSegments(testWDir.path()).size());
    auto wsegments = testWDir.pathSegments();
    ASSERT_EQ(size_t{ 1u }, wsegments.size());
    EXPECT_EQ(std::wstring(L"."), wsegments[0]);
# endif

# ifdef _WINDOWS
    std::string testDirAbsInit = "C:\\Windows\\";
    _FileSystemEntry testDirAbs(testDirAbsInit);
    EXPECT_EQ(std::string("C:\\Windows\\"), testDirAbs.path());
# else
    std::string testDirAbsInit = "/usr/bin/";
    _FileSystemEntry testDirAbs(testDirAbsInit);
    EXPECT_EQ(std::string("/usr/bin/"), testDirAbs.path());
# endif
  EXPECT_EQ(FileSystemPathType::absolute, testDirAbs.pathType());
  EXPECT_EQ(FileSystemPathType::absolute, getFileSystemPathType(testDirAbs.path().c_str()));
# if !defined(__APPLE__) && !defined(__ANDROID__)
    EXPECT_EQ(FileSystemEntryType::directory, testDirAbs.entryType());
    EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testDirAbs.path().c_str()));
    EXPECT_FALSE(testDirAbs.isHidden());
    EXPECT_FALSE(isFileSystemEntryHidden(testDirAbs.path().c_str()));
# endif
  EXPECT_EQ(size_t{ 2u }, getFileSystemPathSegments(testDirAbs.path()).size());
  segments = testDirAbs.pathSegments();
  ASSERT_EQ(size_t{ 2u }, segments.size());
# ifdef _WINDOWS
    EXPECT_EQ(std::string("C:"), segments[0]);
    EXPECT_EQ(std::string("Windows"), segments[1]);
# else
    EXPECT_EQ(std::string("usr"), segments[0]);
    EXPECT_EQ(std::string("bin"), segments[1]);
# endif

# ifdef _WINDOWS
    std::wstring testWDirAbsInit = L"C:\\Windows\\";
    FileSystemEntry<true> testWDirAbs(testWDirAbsInit);
    EXPECT_EQ(std::wstring(L"C:\\Windows\\"), testWDirAbs.path());
    EXPECT_EQ(FileSystemPathType::absolute, testWDirAbs.pathType());
    EXPECT_EQ(FileSystemPathType::absolute, getFileSystemPathType(testWDirAbs.path().c_str()));
    EXPECT_EQ(FileSystemEntryType::directory, testWDirAbs.entryType());
    EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testWDirAbs.path().c_str()));
    EXPECT_FALSE(testWDirAbs.isHidden());
    EXPECT_FALSE(isFileSystemEntryHidden(testWDirAbs.path().c_str()));
    EXPECT_EQ(size_t{ 2u }, getFileSystemPathSegments(testWDirAbs.path()).size());
    wsegments = testWDirAbs.pathSegments();
    ASSERT_EQ(size_t{ 2u }, wsegments.size());
    EXPECT_EQ(std::wstring(L"C:"), wsegments[0]);
    EXPECT_EQ(std::wstring(L"Windows"), wsegments[1]);

    std::string testDirAbsMediaInit = "AB0:\\Windows\\";
    FileSystemEntry<false> testDirAbsMedia(testDirAbsMediaInit);
    EXPECT_EQ(FileSystemPathType::absolute, testDirAbsMedia.pathType());
    std::wstring testWDirAbsMediaInit = L"AB0:\\Windows\\";
    FileSystemEntry<true> testWDirAbsMedia(testWDirAbsMediaInit);
    EXPECT_EQ(FileSystemPathType::absolute, testWDirAbsMedia.pathType());
    std::string testDirWebMediaInit = "ftp://Windows\\";
    FileSystemEntry<false> testDirWebMedia(testDirWebMediaInit);
    EXPECT_EQ(FileSystemPathType::absolute, testDirWebMedia.pathType());
    std::wstring testWDirWebMediaInit = L"ftp://Windows\\";
    FileSystemEntry<true> testWDirWebMedia(testWDirWebMediaInit);
    EXPECT_EQ(FileSystemPathType::absolute, testWDirWebMedia.pathType());
    std::string testDirNoMediaInit = "C:A\\Windows\\";
    FileSystemEntry<false> testDirNoMedia(testDirNoMediaInit);
    EXPECT_EQ(FileSystemPathType::relative, testDirNoMedia.pathType());
    std::wstring testWDirNoMediaInit = L"C:A\\Windows\\";
    FileSystemEntry<true> testWDirNoMedia(testWDirNoMediaInit);
    EXPECT_EQ(FileSystemPathType::relative, testWDirNoMedia.pathType());
# endif
}

TEST_F(FileSystemIOTest, copyMoveCtorAccessors) {
  _FileSystemEntry testDir("./abc/d&f");
  EXPECT_EQ(std::string("./abc/d&f"), testDir.path());
  auto type = testDir.entryType();

  _FileSystemEntry testDirCopy(testDir);
  EXPECT_EQ(std::string("./abc/d&f"), testDirCopy.path());
  EXPECT_EQ(type, testDirCopy.entryType());
  _FileSystemEntry testDirMove(std::move(testDir));
  EXPECT_EQ(std::string("./abc/d&f"), testDirMove.path());
  EXPECT_EQ(type, testDirMove.entryType());

  testDir = _FileSystemEntry(".");
  EXPECT_EQ(std::string("."), testDir.path());
  testDir = testDirCopy;
  EXPECT_EQ(std::string("./abc/d&f"), testDir.path());
  EXPECT_EQ(type, testDir.entryType());

  testDir = _FileSystemEntry(".");
  testDir = std::move(testDirMove);
  EXPECT_EQ(std::string("./abc/d&f"), testDir.path());
  EXPECT_EQ(type, testDir.entryType());
  
# ifdef _WINDOWS
    FileSystemEntry<true> testWDir(L"./abc/d&f");
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDir.path());
    auto wtype = testWDir.entryType();

    FileSystemEntry<true> testWDirCopy(testWDir);
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDirCopy.path());
    EXPECT_EQ(wtype, testWDirCopy.entryType());
    FileSystemEntry<true> testWDirMove(std::move(testWDir));
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDirMove.path());
    EXPECT_EQ(wtype, testWDirMove.entryType());

    testWDir = FileSystemEntry<true>(L".");
    EXPECT_EQ(std::wstring(L"."), testWDir.path());
    testWDir = testWDirCopy;
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDir.path());
    EXPECT_EQ(wtype, testWDir.entryType());

    testWDir = FileSystemEntry<true>(L".");
    testWDir = std::move(testWDirMove);
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDir.path());
    EXPECT_EQ(wtype, testWDir.entryType());
# endif
}

TEST_F(FileSystemIOTest, swapObjects) {
  _FileSystemEntry testDir("./abc/d&f");
  _FileSystemEntry testDir2("/usr/bin");
  EXPECT_EQ(std::string("./abc/d&f"), testDir.path());
  auto type = testDir.entryType();
  EXPECT_EQ(std::string("/usr/bin"), testDir2.path());
  auto type2 = testDir2.entryType();

  testDir.swap(testDir2);
  EXPECT_EQ(std::string("/usr/bin"), testDir.path());
  EXPECT_EQ(type2, testDir.entryType());
  EXPECT_EQ(std::string("./abc/d&f"), testDir2.path());
  EXPECT_EQ(type, testDir2.entryType());
  
# ifdef _WINDOWS
    FileSystemEntry<true> testWDir(L"./abc/d&f");
    FileSystemEntry<true> testWDir2(L"C:\\Windows");
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDir.path());
    auto wtype = testWDir.entryType();
    EXPECT_EQ(std::wstring(L"C:\\Windows"), testWDir2.path());
    auto wtype2 = testWDir2.entryType();

    testWDir.swap(testWDir2);
    EXPECT_EQ(std::wstring(L"C:\\Windows"), testWDir.path());
    EXPECT_EQ(wtype2, testWDir.entryType());
    EXPECT_EQ(std::wstring(L"./abc/d&f"), testWDir2.path());
    EXPECT_EQ(wtype, testWDir2.entryType());
# endif
}


// -- access mode --

TEST_F(FileSystemIOTest, accessMode) {
  std::string invalidPath = "dummy?\\//Non!Existing$%:@";
  _FileSystemEntry testInvalidDir(invalidPath);
  EXPECT_FALSE(testInvalidDir.hasAccessMode(FileSystemAccessMode::existence));
  EXPECT_FALSE(testInvalidDir.hasAccessMode(FileSystemAccessMode::read));
  EXPECT_FALSE(testInvalidDir.hasAccessMode(FileSystemAccessMode::write));
  EXPECT_FALSE(testInvalidDir.hasAccessMode(FileSystemAccessMode::readWrite));
  EXPECT_FALSE(verifyFileSystemAccessMode(invalidPath.c_str(), FileSystemAccessMode::existence));
  EXPECT_FALSE(verifyFileSystemAccessMode(invalidPath.c_str(), FileSystemAccessMode::read));
  EXPECT_FALSE(verifyFileSystemAccessMode(invalidPath.c_str(), FileSystemAccessMode::write));
  EXPECT_FALSE(verifyFileSystemAccessMode(invalidPath.c_str(), FileSystemAccessMode::readWrite));
  
  _FileSystemEntry testValidDir(testCaseFile);
  EXPECT_TRUE(testValidDir.hasAccessMode(FileSystemAccessMode::existence));
  EXPECT_TRUE(testValidDir.hasAccessMode(FileSystemAccessMode::read));
  EXPECT_TRUE(testValidDir.hasAccessMode(FileSystemAccessMode::write));
  EXPECT_TRUE(testValidDir.hasAccessMode(FileSystemAccessMode::readWrite));
  EXPECT_TRUE(verifyFileSystemAccessMode(testCaseFile.c_str(), FileSystemAccessMode::existence));
  EXPECT_TRUE(verifyFileSystemAccessMode(testCaseFile.c_str(), FileSystemAccessMode::read));
  EXPECT_TRUE(verifyFileSystemAccessMode(testCaseFile.c_str(), FileSystemAccessMode::write));
  EXPECT_TRUE(verifyFileSystemAccessMode(testCaseFile.c_str(), FileSystemAccessMode::readWrite));
  
# ifdef _WINDOWS
    std::wstring invalidWPath = L"dummy?\\//Non!Existing$%:@";
    FileSystemEntry<true> testInvalidWDir(invalidWPath);
    EXPECT_FALSE(testInvalidWDir.hasAccessMode(FileSystemAccessMode::existence));
    EXPECT_FALSE(testInvalidWDir.hasAccessMode(FileSystemAccessMode::read));
    EXPECT_FALSE(testInvalidWDir.hasAccessMode(FileSystemAccessMode::write));
    EXPECT_FALSE(testInvalidWDir.hasAccessMode(FileSystemAccessMode::readWrite));
    EXPECT_FALSE(verifyFileSystemAccessMode(invalidWPath.c_str(), FileSystemAccessMode::existence));
    EXPECT_FALSE(verifyFileSystemAccessMode(invalidWPath.c_str(), FileSystemAccessMode::read));
    EXPECT_FALSE(verifyFileSystemAccessMode(invalidWPath.c_str(), FileSystemAccessMode::write));
    EXPECT_FALSE(verifyFileSystemAccessMode(invalidWPath.c_str(), FileSystemAccessMode::readWrite));
    
    FileSystemEntry<true> testValidWDir(testCaseWFile);
    EXPECT_TRUE(testValidWDir.hasAccessMode(FileSystemAccessMode::existence));
    EXPECT_TRUE(testValidWDir.hasAccessMode(FileSystemAccessMode::read));
    EXPECT_TRUE(testValidWDir.hasAccessMode(FileSystemAccessMode::write));
    EXPECT_TRUE(testValidWDir.hasAccessMode(FileSystemAccessMode::readWrite));
    EXPECT_TRUE(verifyFileSystemAccessMode(testCaseWFile.c_str(), FileSystemAccessMode::existence));
    EXPECT_TRUE(verifyFileSystemAccessMode(testCaseWFile.c_str(), FileSystemAccessMode::read));
    EXPECT_TRUE(verifyFileSystemAccessMode(testCaseWFile.c_str(), FileSystemAccessMode::write));
    EXPECT_TRUE(verifyFileSystemAccessMode(testCaseWFile.c_str(), FileSystemAccessMode::readWrite));
# endif
}


// -- file operations --

TEST_F(FileSystemIOTest, knownFileAttributes) {
  EXPECT_EQ(FileSystemEntryType::file, getFileSystemEntryType(testCaseFile.c_str()));
  EXPECT_FALSE(isFileSystemEntryHidden(testCaseFile.c_str()));
  EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testCaseDir.c_str()));
  EXPECT_FALSE(isFileSystemEntryHidden(testCaseDir.c_str()));
# ifdef _WINDOWS
    EXPECT_EQ(FileSystemEntryType::file, getFileSystemEntryType(testCaseWFile.c_str()));
    EXPECT_FALSE(isFileSystemEntryHidden(testCaseWFile.c_str()));
    EXPECT_EQ(FileSystemEntryType::directory, getFileSystemEntryType(testCaseWDir.c_str()));
    EXPECT_FALSE(isFileSystemEntryHidden(testCaseWDir.c_str()));
# else
    EXPECT_FALSE(isFileSystemEntryHidden(""));
    EXPECT_FALSE(isFileSystemEntryHidden("."));
    EXPECT_FALSE(isFileSystemEntryHidden(".."));
    EXPECT_FALSE(isFileSystemEntryHidden("./test"));
    EXPECT_FALSE(isFileSystemEntryHidden("../test"));
    EXPECT_TRUE(isFileSystemEntryHidden(".a"));
    EXPECT_TRUE(isFileSystemEntryHidden(".test"));
# endif
}

TEST_F(FileSystemIOTest, knownFileMetadata) {
  std::string invalidPath = "dummyNonExisting";
  _FileSystemEntryMetadata metadata;
  _FileSystemEntry notFound(invalidPath);
  _FileSystemEntry testedFile(testCaseFile);
  
  EXPECT_FALSE(notFound.readMetadata(metadata));
  EXPECT_FALSE(readFileSystemEntryMetadata(invalidPath, metadata));

  EXPECT_TRUE(testedFile.readMetadata(metadata));
  EXPECT_TRUE(readFileSystemEntryMetadata(testCaseFile, metadata));
  EXPECT_EQ(testCaseFileName, metadata.name);
  EXPECT_EQ(FileSystemEntryType::file, metadata.type);
  EXPECT_EQ(FileSystemAccessMode::readWrite, metadata.accessMode);
  EXPECT_TRUE(metadata.size > 0);
  EXPECT_FALSE(metadata.isHidden);

  EXPECT_TRUE(readFileSystemEntryMetadata(testCaseDir, metadata));
  EXPECT_EQ(testCaseDir, metadata.name);
  EXPECT_EQ(FileSystemEntryType::directory, metadata.type);
  EXPECT_EQ(FileSystemAccessMode::readWrite, metadata.accessMode);
  EXPECT_FALSE(metadata.isHidden);
  
  EXPECT_FALSE(readFileSystemEntryMetadata(std::string{}, metadata));
  EXPECT_TRUE(readFileSystemEntryMetadata(testCaseDir, metadata));
  EXPECT_EQ(testCaseDir, metadata.name);
  EXPECT_EQ(FileSystemEntryType::directory, metadata.type);
  EXPECT_TRUE(readFileSystemEntryMetadata(testCaseDir + "/", metadata));
  EXPECT_EQ(testCaseDir, metadata.name);
  EXPECT_EQ(FileSystemEntryType::directory, metadata.type);

# ifdef _WINDOWS
    std::wstring invalidWPath = L"dummyNonExisting";
    FileSystemEntryMetadata<true> wmetadata;
    FileSystemEntry<true> wnotFound(invalidWPath);
    FileSystemEntry<true> testedWFile(testCaseWFile);
    
    EXPECT_FALSE(wnotFound.readMetadata(wmetadata));
    EXPECT_FALSE(readFileSystemEntryMetadata(invalidWPath, wmetadata));

    EXPECT_TRUE(testedWFile.readMetadata(wmetadata));
    EXPECT_TRUE(readFileSystemEntryMetadata(testCaseWFile, wmetadata));
    EXPECT_EQ(testCaseWFileName, wmetadata.name);
    EXPECT_EQ(FileSystemEntryType::file, wmetadata.type);
    EXPECT_EQ(FileSystemAccessMode::readWrite, wmetadata.accessMode);
    EXPECT_TRUE(wmetadata.size > 0);
    EXPECT_FALSE(wmetadata.isHidden);

    EXPECT_TRUE(readFileSystemEntryMetadata(testCaseWDir, wmetadata));
    EXPECT_EQ(testCaseWDir, wmetadata.name);
    EXPECT_EQ(FileSystemEntryType::directory, wmetadata.type);
    EXPECT_EQ(FileSystemAccessMode::readWrite, wmetadata.accessMode);
    EXPECT_FALSE(wmetadata.isHidden);
    
    EXPECT_FALSE(readFileSystemEntryMetadata(std::wstring{}, wmetadata));
    EXPECT_TRUE(readFileSystemEntryMetadata(testCaseWDir, wmetadata));
    EXPECT_EQ(testCaseWDir, wmetadata.name);
    EXPECT_EQ(FileSystemEntryType::directory, wmetadata.type);
    EXPECT_TRUE(readFileSystemEntryMetadata(testCaseWDir + L"/", wmetadata));
    EXPECT_EQ(testCaseWDir, wmetadata.name);
    EXPECT_EQ(FileSystemEntryType::directory, wmetadata.type);
# endif
}

TEST_F(FileSystemIOTest, readFile) {
  std::string invalidPath = "dummyNonExisting";
  std::string readMode = "r";
  EXPECT_FALSE(FileHandle{}.isOpen());
  FileHandle fpInvalid = openFileEntry(invalidPath.c_str(), readMode.c_str());
  EXPECT_FALSE(fpInvalid.isOpen());
  EXPECT_FALSE(fpInvalid);
  fpInvalid = _FileSystemEntry(invalidPath.c_str(), FileSystemEntryType::file).open(readMode.c_str());
  EXPECT_FALSE(fpInvalid.isOpen());
  fpInvalid = _FileSystemEntry(std::string("no\\//path!:%"), FileSystemEntryType::file).open(readMode.c_str());
  EXPECT_FALSE(fpInvalid.isOpen());

  FileHandle fp = openFileEntry(testCaseFile.c_str(), readMode.c_str());
  ASSERT_TRUE(fp.isOpen());
  char buffer[64];
  EXPECT_TRUE(fgets(buffer, 64, fp.handle()) != nullptr);
  EXPECT_EQ(std::string("directory_test"), std::string(buffer));
  fp = _FileSystemEntry(testCaseFile, FileSystemEntryType::file).open(readMode.c_str());
  ASSERT_TRUE(fp.isOpen());
  EXPECT_TRUE(fp);
  EXPECT_TRUE(fgets(buffer, 64, fp.handle()) != nullptr);
  EXPECT_EQ(std::string("directory_test"), std::string(buffer));
  FileHandle movedFp(std::move(fp));
  EXPECT_FALSE(fp.isOpen());
  EXPECT_TRUE(movedFp.isOpen());
  fp.close();
  
# ifdef _WINDOWS
    std::wstring invalidWPath = L"dummyNonExisting";
    std::wstring wreadMode = L"r";
    fpInvalid = openFileEntry(invalidWPath.c_str(), wreadMode.c_str());
    EXPECT_FALSE(fpInvalid.isOpen());
    fpInvalid = FileSystemEntry<true>(invalidWPath.c_str(), FileSystemEntryType::file).open(wreadMode.c_str());
    EXPECT_FALSE(fpInvalid.isOpen());

    fp = openFileEntry(testCaseWFile.c_str(), wreadMode.c_str());
    ASSERT_TRUE(fp.isOpen());
    EXPECT_TRUE(fgets(buffer, 64, fp.handle()) != nullptr);
    EXPECT_EQ(std::string("directory_test"), std::string(buffer));
    fp = FileSystemEntry<true>(testCaseWFile.c_str(), FileSystemEntryType::file).open(wreadMode.c_str());
    ASSERT_TRUE(fp.isOpen());
    EXPECT_TRUE(fgets(buffer, 64, fp.handle()) != nullptr);
    EXPECT_EQ(std::string("directory_test"), std::string(buffer));
# endif
}

TEST_F(FileSystemIOTest, createRemoveFile) {
  std::string invalidPath = "$.\\//./$:|\\.";
  std::string validPath = "abc.txt";
  std::string validPath2 = "abc2.txt";

  FileHandle fpInvalid = openFileEntry(invalidPath.c_str(), fileCreateMode.c_str());
  EXPECT_FALSE(fpInvalid.isOpen());
  EXPECT_FALSE(createFileEntry(invalidPath.c_str()) == 0);
  int res = _FileSystemEntry(invalidPath.c_str(), FileSystemEntryType::file).create();
  EXPECT_FALSE(res == 0);

  EXPECT_FALSE(removeFileEntry(invalidPath.c_str()) == 0);
  EXPECT_FALSE(removeFileEntry(validPath.c_str()) == 0); // non existing yet
  ASSERT_TRUE(createFileEntry(validPath.c_str()) == 0);
  EXPECT_TRUE(removeFileEntry(validPath.c_str()) == 0);
  
  _FileSystemEntry newFile(validPath2.c_str(), FileSystemEntryType::file);
  ASSERT_TRUE(newFile.create() == 0);
  EXPECT_TRUE(newFile.remove() == 0);
  
# ifdef _WINDOWS
    std::wstring invalidWPath = L"$.\\//./$:|\\.";
    std::wstring validWPath = L"abcd.txt";
    std::wstring validWPath2 = L"abcd2.txt";
    
    fpInvalid = openFileEntry(invalidWPath.c_str(), wfileCreateMode.c_str());
    EXPECT_FALSE(fpInvalid.isOpen());
    EXPECT_FALSE(createFileEntry(invalidWPath.c_str()) == 0);
    res = FileSystemEntry<true>(invalidWPath.c_str(), FileSystemEntryType::file).create();
    EXPECT_FALSE(res == 0);
    
    EXPECT_FALSE(removeFileEntry(invalidWPath.c_str()) == 0);
    EXPECT_FALSE(removeFileEntry(validWPath.c_str()) == 0); // non existing yet
    ASSERT_TRUE(createFileEntry(validWPath.c_str()) == 0);
    EXPECT_TRUE(removeFileEntry(validWPath.c_str()) == 0);
    
    FileSystemEntry<true> newWFile(validWPath2.c_str(), FileSystemEntryType::file);
    ASSERT_TRUE(newWFile.create() == 0);
    EXPECT_TRUE(newWFile.remove() == 0);
# endif
}


// -- directory content --

TEST_F(FileSystemIOTest, listSubFiles) {
  std::string invalidPath = "dummyNonExisting";
  std::vector<_FileSystemEntry> subFiles;
  std::vector<_FileSystemEntryMetadata> subFilesMetadata;
  
  EXPECT_FALSE(listFilesInDirectory(invalidPath, subFiles));
  ASSERT_TRUE(listFilesInDirectory(testCaseDir, subFiles));
  EXPECT_TRUE(subFiles.size() >= 1);
  bool testFileFound = false;
  for (auto& file : subFiles) {
    auto segments = file.pathSegments();
    if (segments.back() == testCaseFileName) {
      testFileFound = true;
      break;
    }
  }
  EXPECT_TRUE(testFileFound);

  ASSERT_TRUE(listFilesInDirectory(testCaseDir + "/", subFiles));
  EXPECT_TRUE(subFiles.size() >= 1);
  testFileFound = false;
  for (auto& file : subFiles) {
    auto segments = file.pathSegments();
    if (segments.back() == testCaseFileName) {
      testFileFound = true;
      break;
    }
  }
  EXPECT_TRUE(testFileFound);

  EXPECT_FALSE(listFilesInDirectory(invalidPath, subFilesMetadata));
  ASSERT_TRUE(listFilesInDirectory(testCaseDir, subFilesMetadata));
  EXPECT_TRUE(subFilesMetadata.size() >= 1);
  testFileFound = false;
  for (auto& file : subFilesMetadata) {
    if (file.name == testCaseFileName) {
      EXPECT_EQ(testCaseFileName, file.name);
      EXPECT_EQ(FileSystemEntryType::file, file.type);
      EXPECT_EQ(FileSystemAccessMode::readWrite, file.accessMode);
      EXPECT_TRUE(file.size > 0);
      EXPECT_FALSE(file.isHidden);
      testFileFound = true;
      break;
    }
  }
  EXPECT_TRUE(testFileFound);
  
# ifdef _WINDOWS
    std::wstring invalidWPath = L"dummyNonExisting";
    std::vector<FileSystemEntry<true> > wsubFiles;
    std::vector<FileSystemEntryMetadata<true> > wsubFilesMetadata;
    
    EXPECT_FALSE(listFilesInDirectory(invalidWPath, wsubFiles));
    ASSERT_TRUE(listFilesInDirectory(testCaseWDir, wsubFiles));
    EXPECT_TRUE(wsubFiles.size() >= 1);
    testFileFound = false;
    for (auto& file : wsubFiles) {
      auto segments = file.pathSegments();
      if (segments.back() == testCaseWFileName) {
        testFileFound = true;
        break;
      }
    }
    EXPECT_TRUE(testFileFound);

    EXPECT_FALSE(listFilesInDirectory(invalidWPath, wsubFilesMetadata));
    ASSERT_TRUE(listFilesInDirectory(testCaseWDir, wsubFilesMetadata));
    EXPECT_TRUE(wsubFilesMetadata.size() >= 1);
    testFileFound = false;
    for (auto& file : wsubFilesMetadata) {
      if (file.name == testCaseWFileName) {
        EXPECT_EQ(testCaseWFileName, file.name);
        EXPECT_EQ(FileSystemEntryType::file, file.type);
        EXPECT_EQ(FileSystemAccessMode::readWrite, file.accessMode);
        EXPECT_TRUE(file.size > 0);
        EXPECT_FALSE(file.isHidden);
        testFileFound = true;
        break;
      }
    }
    EXPECT_TRUE(testFileFound);
# endif
}

TEST_F(FileSystemIOTest, createRemoveDirectory) {
  std::string invalidPath = "$.\\//./$:|\\.";
  std::string validPath = "abc";
  std::string validPath2 = "abc2";

  EXPECT_FALSE(createDirectory(invalidPath) == 0);
  EXPECT_FALSE(removeDirectory(invalidPath) == 0);
  EXPECT_FALSE(removeDirectory(validPath) == 0); // non existing yet
  _FileSystemEntry noDir(invalidPath.c_str(), FileSystemEntryType::directory);
  EXPECT_FALSE(noDir.create() == 0);
  EXPECT_FALSE(noDir.remove() == 0);

  ASSERT_TRUE(createDirectory(validPath) == 0);
  EXPECT_TRUE(removeDirectory(validPath) == 0);
  _FileSystemEntry newDir(validPath2.c_str(), FileSystemEntryType::directory);
  ASSERT_TRUE(newDir.create() == 0);
  EXPECT_TRUE(newDir.remove() == 0);
  
# ifdef _WINDOWS
    std::wstring invalidWPath = L"$.\\//./$:|\\.";
    std::wstring validWPath = L"abcd";
    std::wstring validWPath2 = L"abcd2";
    
    EXPECT_FALSE(createDirectory(invalidWPath) == 0);
    EXPECT_FALSE(removeDirectory(invalidWPath) == 0);
    EXPECT_FALSE(removeDirectory(validWPath) == 0); // non existing yet
    FileSystemEntry<true> noWDir(invalidWPath.c_str(), FileSystemEntryType::directory);
    EXPECT_FALSE(noWDir.create() == 0);
    EXPECT_FALSE(noWDir.remove() == 0);

    ASSERT_TRUE(createDirectory(validWPath) == 0);
    EXPECT_TRUE(removeDirectory(validWPath) == 0);
    FileSystemEntry<true> newWDir(validWPath2.c_str(), FileSystemEntryType::directory);
    ASSERT_TRUE(newWDir.create() == 0);
    EXPECT_TRUE(newWDir.remove() == 0);
# endif
}
