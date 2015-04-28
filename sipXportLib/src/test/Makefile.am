
SUBDIRS = @SIPX_UNIT_TARGET@ testplugin

## All tests under this GNU variable should run relatively quickly
## and of course require no setup
# for performance numbers, add to TESTS: UtlListPerformance UtlHashMapPerformance
TESTS = testsuite

check_PROGRAMS = testsuite sandbox UtlListPerformance UtlHashMapPerformance

## To load source in gdb for libsipXport.la, type the 'share' at the
## gdb console just before stepping into function in sipXportLib
testsuite_LDADD = \
    @SIPX_UNIT_TARGET@/libsipXunit.la \
    ../libsipXport.la

testsuite_CXXFLAGS = \
	-I$(top_srcdir)/include \
	@PCRE_CFLAGS@ \
	@SSL_CXXFLAGS@ \
	@CPPUNIT_CFLAGS@ \
    -I$(SIPXUNITINC) \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DTEST_DIR=\"@abs_builddir@\" \
	-fno-gcse -O0

testsuite_CPPFLAGS = @CPPUNIT_CFLAGS@

# ORDER NOTE: the utl/UtlLinkTest and utl/UtlChainTest must be before
#             any test that uses any UtlContainer.
testsuite_SOURCES = \
    os/UnitTestLogHooks.cpp \
    os/OsAdapterInfoTest.cpp \
    os/OsCallbackTest.cpp \
    os/OsConfigDbTest.cpp \
    os/OsContactListTest.cpp  \
    os/OsContactTest.cpp \
    os/OsDirTest.cpp \
    os/OsEventTest.cpp \
    os/OsFileIteratorTest.cpp \
    os/OsFileSystemTest.cpp \
    os/OsFileTest.cpp \
    os/OsLockTest.cpp \
    os/OsMsgQTest.cpp \
    os/OsMsgDispatcherTest.cpp \
    os/OsMutexTest.cpp \
    os/OsNameDbTest.cpp \
    os/OsPathTest.cpp \
    os/OsProcessIteratorTest.cpp \
    os/OsProcessMgrTest.cpp \
    os/OsProcessTest.cpp \
    os/OsSemTest.cpp \
    os/OsServerTaskTest.cpp \
    os/OsSharedLibMgrTest.cpp \
    os/OsSocketTest.cpp \
    os/OsTestUtilities.cpp \
    os/OsTestUtilities.h \
    os/OsTimerTaskTest.cpp \
    os/OsTimerTest.cpp \
    os/OsTimeTest.cpp \
    utl/CircularBufferTest.cpp \
    utl/PluginHooksTest.cpp \
    utl/TestPlugin.h \
    utl/UtlChainTest.cpp \
    utl/UtlContainableTestStub.cpp \
    utl/UtlContainableTestStub.h \
    utl/UtlCrc32Test.cpp \
    utl/UtlCryptoTest.cpp \
    utl/UtlDList.cpp \
    utl/UtlDListIterator.cpp \
    utl/UtlHashBag.cpp \
    utl/UtlHashBagIterator.cpp \
    utl/UtlHashMap.cpp \
    utl/UtlHashMapIterator.cpp \
    utl/UtlInt.cpp \
    utl/UtlLinkTest.cpp \
    utl/UtlLongLongInt.cpp \
    utl/UtlRegex.cpp \
    utl/UtlSList.cpp \
    utl/UtlSListIteratorTest.cpp \
    utl/UtlSortedList.cpp \
    utl/UtlSortedListIteratorTest.cpp \
    utl/UtlStringTest.cpp \
    utl/UtlStringTest.h \
    utl/UtlStringTest_ConstructiveManipulators.cpp \
    utl/UtlStringTest_DestructiveManipulators.cpp \
    utl/UtlStringTest_NonMutating.cpp \
    utl/UtlTokenizerTest.cpp \
    utl/UtlVoidPtr.cpp \
    utl/XmlContentTest.cpp

# [DEBUG ASSISTANT] List a single unittest to isoloate a unittest for
# debugging or when developing a new unittest. As courtesy remove all
# unittests before checking in, although forgetting to has no ill
# effects. To build and run:
#
#   cd src/test
#   make sandbox
#   ./sandbox
#
#sandbox_LDFLAGS = $(testsuite_LDFLAGS)
sandbox_LDADD = $(testsuite_LDADD)

sandbox_CXXFLAGS = $(testsuite_CXXFLAGS)
sandbox_SOURCES = \
    os/OsConfigDbTest.cpp \
    utl/PluginHooksTest.cpp

# Performance test of UtlList

UtlListPerformance_SOURCES = \
	utl/UtlListPerformance.cpp


UtlListPerformance_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

UtlListPerformance_LDADD = \
    ../libsipXport.la

# Performance test of UtlHashMap

UtlHashMapPerformance_SOURCES = \
	utl/UtlHashMapPerformance.cpp


UtlHashMapPerformance_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include

UtlHashMapPerformance_LDADD = \
    ../libsipXport.la


EXTRA_DIST=

