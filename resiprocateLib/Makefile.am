## Wrapper for building resiprocate as a set of static libraries

include $(top_builddir)/config/sipXcommon.mak

SUBDIRS = . include src

# To do the 'build' operation, we perform the Resiprocate build and install
# operations for all the appropriate components in order.
# We have to do the Resiprocate install because the build of each
# component depends on the previous components being installed.
# That will install the files in what the Resiprocate build system thinks
# is the install directory, which is ./build.
# The automake 'install' operation will copy everything from ./build
# to the real install directory.
all-local: rutil install-rutil resip install-resip dum install-dum

# Commands to copy the Resiprocate files from the Resiprocate install
# directory to the sipX install directory.
install-exec-local: \
	$(RESIP_INCLUDE_FILES_TO_INSTALL) \
	$(RESIP_LIB_FILES_TO_INSTALL)

$(RESIP_INCLUDE_FILES_TO_INSTALL) : $(includedir)/% : include/%
	@INSTALL@ -D $< $@

$(RESIP_LIB_FILES_TO_INSTALL) : $(libdir)/% : lib/%
	@INSTALL@ -D $< $@

RESIP_INCLUDE_FILES_TO_INSTALL = \
	$(includedir)/resip/stack/TransactionTerminated.hxx \
	$(includedir)/resip/stack/ParserContainerBase.hxx \
	$(includedir)/resip/stack/ApiCheckList.hxx \
	$(includedir)/resip/stack/ParseException.hxx \
	$(includedir)/resip/stack/CSeqCategory.hxx \
	$(includedir)/resip/stack/DtlsMessage.hxx \
	$(includedir)/resip/stack/RequestLine.hxx \
	$(includedir)/resip/stack/StatisticsManager.hxx \
	$(includedir)/resip/stack/HeaderHash.hxx \
	$(includedir)/resip/stack/RAckCategory.hxx \
	$(includedir)/resip/stack/Contents.hxx \
	$(includedir)/resip/stack/TransactionController.hxx \
	$(includedir)/resip/stack/ApplicationMessage.hxx \
	$(includedir)/resip/stack/TransportFailure.hxx \
	$(includedir)/resip/stack/StatelessHandler.hxx \
	$(includedir)/resip/stack/UnknownHeaderType.hxx \
	$(includedir)/resip/stack/ExistsParameter.hxx \
	$(includedir)/resip/stack/SipStack.hxx \
	$(includedir)/resip/stack/ParameterTypes.hxx \
	$(includedir)/resip/stack/TcpBaseTransport.hxx \
	$(includedir)/resip/stack/Rlmi.hxx \
	$(includedir)/resip/stack/DeprecatedDialog.hxx \
	$(includedir)/resip/stack/TransactionUser.hxx \
	$(includedir)/resip/stack/ShutdownMessage.hxx \
	$(includedir)/resip/stack/RportParameter.hxx \
	$(includedir)/resip/stack/StackThread.hxx \
	$(includedir)/resip/stack/InternalTransport.hxx \
	$(includedir)/resip/stack/SipMessage.hxx \
	$(includedir)/resip/stack/StringCategory.hxx \
	$(includedir)/resip/stack/ExtensionHeader.hxx \
	$(includedir)/resip/stack/CpimContents.hxx \
	$(includedir)/resip/stack/SipFrag.hxx \
	$(includedir)/resip/stack/TimerQueue.hxx \
	$(includedir)/resip/stack/Helper.hxx \
	$(includedir)/resip/stack/ContentsFactoryBase.hxx \
	$(includedir)/resip/stack/Pidf.hxx \
	$(includedir)/resip/stack/CancelableTimerQueue.hxx \
	$(includedir)/resip/stack/DataParameter.hxx \
	$(includedir)/resip/stack/TimerMessage.hxx \
	$(includedir)/resip/stack/MsgHeaderScanner.hxx \
	$(includedir)/resip/stack/TransactionUserMessage.hxx \
	$(includedir)/resip/stack/ParserContainer.hxx \
	$(includedir)/resip/stack/SecurityTypes.hxx \
	$(includedir)/resip/stack/UdpTransport.hxx \
	$(includedir)/resip/stack/HeaderTypes.hxx \
	$(includedir)/resip/stack/DtlsTransport.hxx \
	$(includedir)/resip/stack/MultipartRelatedContents.hxx \
	$(includedir)/resip/stack/MessageWaitingContents.hxx \
	$(includedir)/resip/stack/ExpiresCategory.hxx \
	$(includedir)/resip/stack/TransactionMap.hxx \
	$(includedir)/resip/stack/TcpConnection.hxx \
	$(includedir)/resip/stack/DnsResult.hxx \
	$(includedir)/resip/stack/ApplicationSip.hxx \
	$(includedir)/resip/stack/ExternalBodyContents.hxx \
	$(includedir)/resip/stack/QopParameter.hxx \
	$(includedir)/resip/stack/TuSelector.hxx \
	$(includedir)/resip/stack/FloatParameter.hxx \
	$(includedir)/resip/stack/SendData.hxx \
	$(includedir)/resip/stack/SdpContents.hxx \
	$(includedir)/resip/stack/Pkcs7Contents.hxx \
	$(includedir)/resip/stack/TuIM.hxx \
	$(includedir)/resip/stack/UnknownParameterType.hxx \
	$(includedir)/resip/stack/stun/stun.h \
	$(includedir)/resip/stack/stun/udp.h \
	$(includedir)/resip/stack/XMLCursor.hxx \
	$(includedir)/resip/stack/WarningCategory.hxx \
	$(includedir)/resip/stack/TlsConnection.hxx \
	$(includedir)/resip/stack/Headers.hxx \
	$(includedir)/resip/stack/MethodTypes.hxx \
	$(includedir)/resip/stack/AsyncProcessHandler.hxx \
	$(includedir)/resip/stack/Via.hxx \
	$(includedir)/resip/stack/KeepAliveMessage.hxx \
	$(includedir)/resip/stack/TransactionState.hxx \
	$(includedir)/resip/stack/MacSecurity.hxx \
	$(includedir)/resip/stack/MultipartMixedContents.hxx \
	$(includedir)/resip/stack/Mime.hxx \
	$(includedir)/resip/stack/Tuple.hxx \
	$(includedir)/resip/stack/Embedded.hxx \
	$(includedir)/resip/stack/GenericContents.hxx \
	$(includedir)/resip/stack/ParameterTypeEnums.hxx \
	$(includedir)/resip/stack/Message.hxx \
	$(includedir)/resip/stack/GenericUri.hxx \
	$(includedir)/resip/stack/config.hxx.in \
	$(includedir)/resip/stack/IntegerParameter.hxx \
	$(includedir)/resip/stack/ParserCategory.hxx \
	$(includedir)/resip/stack/Uri.hxx \
	$(includedir)/resip/stack/Symbols.hxx \
	$(includedir)/resip/stack/ExtensionParameter.hxx \
	$(includedir)/resip/stack/MessageFilterRule.hxx \
	$(includedir)/resip/stack/UnknownParameter.hxx \
	$(includedir)/resip/stack/StatisticsMessage.hxx \
	$(includedir)/resip/stack/Connection.hxx \
	$(includedir)/resip/stack/HeaderFieldValueList.hxx \
	$(includedir)/resip/stack/ApiCheck.hxx \
	$(includedir)/resip/stack/InterruptableStackThread.hxx \
	$(includedir)/resip/stack/Pkcs8Contents.hxx \
	$(includedir)/resip/stack/DateCategory.hxx \
	$(includedir)/resip/stack/X509Contents.hxx \
	$(includedir)/resip/stack/FlowId.hxx \
	$(includedir)/resip/stack/WinSecurity.hxx \
	$(includedir)/resip/stack/SecurityAttributes.hxx \
	$(includedir)/resip/stack/ValueFifo.hxx \
	$(includedir)/resip/stack/LazyParser.hxx \
	$(includedir)/resip/stack/NameAddr.hxx \
	$(includedir)/resip/stack/TlsTransport.hxx \
	$(includedir)/resip/stack/ContentsFactory.hxx \
	$(includedir)/resip/stack/TransportSelector.hxx \
	$(includedir)/resip/stack/MultipartAlternativeContents.hxx \
	$(includedir)/resip/stack/BranchParameter.hxx \
	$(includedir)/resip/stack/TransactionMessage.hxx \
	$(includedir)/resip/stack/TcpTransport.hxx \
	$(includedir)/resip/stack/StatusLine.hxx \
	$(includedir)/resip/stack/Transport.hxx \
	$(includedir)/resip/stack/Aor.hxx \
	$(includedir)/resip/stack/ConnectionBase.hxx \
	$(includedir)/resip/stack/PlainContents.hxx \
	$(includedir)/resip/stack/ParserCategories.hxx \
	$(includedir)/resip/stack/OctetContents.hxx \
	$(includedir)/resip/stack/MethodHash.hxx \
	$(includedir)/resip/stack/QuotedDataParameter.hxx \
	$(includedir)/resip/stack/HeaderFieldValue.hxx \
	$(includedir)/resip/stack/MultipartSignedContents.hxx \
	$(includedir)/resip/stack/Token.hxx \
	$(includedir)/resip/stack/Parameter.hxx \
	$(includedir)/resip/stack/SelectInterruptor.hxx \
	$(includedir)/resip/stack/Auth.hxx \
	$(includedir)/resip/stack/CallId.hxx \
	$(includedir)/resip/stack/ParameterHash.hxx \
	$(includedir)/resip/stack/DnsInterface.hxx \
	$(includedir)/resip/stack/TimeAccumulate.hxx \
	$(includedir)/resip/stack/IntegerCategory.hxx \
	$(includedir)/resip/stack/Security.hxx \
	$(includedir)/resip/stack/ConnectionManager.hxx \
	$(includedir)/resip/dum/HandleManager.hxx \
	$(includedir)/resip/dum/DumThread.hxx \
	$(includedir)/resip/dum/Handle.hxx \
	$(includedir)/resip/dum/Dialog.hxx \
	$(includedir)/resip/dum/DumFeature.hxx \
	$(includedir)/resip/dum/BaseSubscription.hxx \
	$(includedir)/resip/dum/DumDecrypted.hxx \
	$(includedir)/resip/dum/PagerMessageCreator.hxx \
	$(includedir)/resip/dum/UserProfile.hxx \
	$(includedir)/resip/dum/DumException.hxx \
	$(includedir)/resip/dum/RegistrationCreator.hxx \
	$(includedir)/resip/dum/ServerPagerMessage.hxx \
	$(includedir)/resip/dum/AppDialog.hxx \
	$(includedir)/resip/dum/KeepAliveTimeout.hxx \
	$(includedir)/resip/dum/InviteSession.hxx \
	$(includedir)/resip/dum/Handled.hxx \
	$(includedir)/resip/dum/HandleException.hxx \
	$(includedir)/resip/dum/DialogSet.hxx \
	$(includedir)/resip/dum/ContactInstanceRecord.hxx \
	$(includedir)/resip/dum/DumProcessHandler.hxx \
	$(includedir)/resip/dum/RegistrationPersistenceManager.hxx \
	$(includedir)/resip/dum/MergedRequestKey.hxx \
	$(includedir)/resip/dum/SubscriptionCreator.hxx \
	$(includedir)/resip/dum/ClientInviteSession.hxx \
	$(includedir)/resip/dum/DialogSetHandler.hxx \
	$(includedir)/resip/dum/UserAuthInfo.hxx \
	$(includedir)/resip/dum/AppDialogSetFactory.hxx \
	$(includedir)/resip/dum/PublicationHandler.hxx \
	$(includedir)/resip/dum/ServerRegistration.hxx \
	$(includedir)/resip/dum/OutOfDialogReqCreator.hxx \
	$(includedir)/resip/dum/ServerOutOfDialogReq.hxx \
	$(includedir)/resip/dum/ClientPublication.hxx \
	$(includedir)/resip/dum/InMemoryRegistrationDatabase.hxx \
	$(includedir)/resip/dum/ClientRegistration.hxx \
	$(includedir)/resip/dum/RegistrationHandler.hxx \
	$(includedir)/resip/dum/DumTimeout.hxx \
	$(includedir)/resip/dum/KeepAliveManager.hxx \
	$(includedir)/resip/dum/RedirectHandler.hxx \
	$(includedir)/resip/dum/UsageUseException.hxx \
	$(includedir)/resip/dum/SubscriptionPersistenceManager.hxx \
	$(includedir)/resip/dum/DialogId.hxx \
	$(includedir)/resip/dum/DumCommand.hxx \
	$(includedir)/resip/dum/SubscriptionState.hxx \
	$(includedir)/resip/dum/EncryptionRequest.hxx \
	$(includedir)/resip/dum/ClientSubscription.hxx \
	$(includedir)/resip/dum/RemoteCertStore.hxx \
	$(includedir)/resip/dum/ClientAuthManager.hxx \
	$(includedir)/resip/dum/ServerPublication.hxx \
	$(includedir)/resip/dum/CertMessage.hxx \
	$(includedir)/resip/dum/HttpProvider.hxx \
	$(includedir)/resip/dum/NonDialogUsage.hxx \
	$(includedir)/resip/dum/RefCountedDestroyer.hxx \
	$(includedir)/resip/dum/MasterProfile.hxx \
	$(includedir)/resip/dum/DumFeatureChain.hxx \
	$(includedir)/resip/dum/OutgoingEvent.hxx \
	$(includedir)/resip/dum/IdentityHandler.hxx \
	$(includedir)/resip/dum/AppDialogSet.hxx \
	$(includedir)/resip/dum/DefaultServerReferHandler.hxx \
	$(includedir)/resip/dum/ClientPagerMessage.hxx \
	$(includedir)/resip/dum/BaseCreator.hxx \
	$(includedir)/resip/dum/ServerSubscription.hxx \
	$(includedir)/resip/dum/DumShutdownHandler.hxx \
	$(includedir)/resip/dum/OutOfDialogHandler.hxx \
	$(includedir)/resip/dum/DumHelper.hxx \
	$(includedir)/resip/dum/DumFeatureMessage.hxx \
	$(includedir)/resip/dum/ExternalTimer.hxx \
	$(includedir)/resip/dum/ServerAuthManager.hxx \
	$(includedir)/resip/dum/InviteSessionHandler.hxx \
	$(includedir)/resip/dum/Profile.hxx \
	$(includedir)/resip/dum/ClientOutOfDialogReq.hxx \
	$(includedir)/resip/dum/Handles.hxx \
	$(includedir)/resip/dum/ServerInviteSession.hxx \
	$(includedir)/resip/dum/RedirectManager.hxx \
	$(includedir)/resip/dum/NetworkAssociation.hxx \
	$(includedir)/resip/dum/DialogUsage.hxx \
	$(includedir)/resip/dum/PublicationCreator.hxx \
	$(includedir)/resip/dum/PagerMessageHandler.hxx \
	$(includedir)/resip/dum/DialogSetId.hxx \
	$(includedir)/resip/dum/DestroyUsage.hxx \
	$(includedir)/resip/dum/HttpGetMessage.hxx \
	$(includedir)/resip/dum/BaseUsage.hxx \
	$(includedir)/resip/dum/TargetCommand.hxx \
	$(includedir)/resip/dum/SubscriptionHandler.hxx \
	$(includedir)/resip/dum/InviteSessionCreator.hxx \
	$(includedir)/resip/dum/DialogUsageManager.hxx \
	$(includedir)/resip/dum/EncryptionManager.hxx \
	$(includedir)/rutil/compat.hxx \
	$(includedir)/rutil/RWMutex.hxx \
	$(includedir)/rutil/ParseBuffer.hxx \
	$(includedir)/rutil/Condition.hxx \
	$(includedir)/rutil/SharedPtr.hxx \
	$(includedir)/rutil/RecursiveMutex.hxx \
	$(includedir)/rutil/DataStream.hxx \
	$(includedir)/rutil/Logger.hxx \
	$(includedir)/rutil/Data.hxx \
	$(includedir)/rutil/HashMap.hxx \
	$(includedir)/rutil/SysLogStream.hxx \
	$(includedir)/rutil/AbstractFifo.hxx \
	$(includedir)/rutil/Lock.hxx \
	$(includedir)/rutil/MD5Stream.hxx \
	$(includedir)/rutil/TimeLimitFifo.hxx \
	$(includedir)/rutil/Mutex.hxx \
	$(includedir)/rutil/GenericIPAddress.hxx \
	$(includedir)/rutil/Random.hxx \
	$(includedir)/rutil/FiniteFifo.hxx \
	$(includedir)/rutil/IntrusiveListElement.hxx \
	$(includedir)/rutil/SharedCount.hxx \
	$(includedir)/rutil/CircularBuffer.hxx \
	$(includedir)/rutil/Fifo.hxx \
	$(includedir)/rutil/vthread.hxx \
	$(includedir)/rutil/WinLeakCheck.hxx \
	$(includedir)/rutil/HeapInstanceCounter.hxx \
	$(includedir)/rutil/WinCompat.hxx \
	$(includedir)/rutil/Subsystem.hxx \
	$(includedir)/rutil/Log.hxx \
	$(includedir)/rutil/SysLogBuf.hxx \
	$(includedir)/rutil/Coders.hxx \
	$(includedir)/rutil/BaseException.hxx \
	$(includedir)/rutil/Lockable.hxx \
	$(includedir)/rutil/Poll.hxx \
	$(includedir)/rutil/Timer.hxx \
	$(includedir)/rutil/Socket.hxx \
	$(includedir)/rutil/AsyncID.hxx \
	$(includedir)/rutil/FileSystem.hxx \
	$(includedir)/rutil/vmd5.hxx \
	$(includedir)/rutil/Inserter.hxx \
	$(includedir)/rutil/dns/AresDns.hxx \
	$(includedir)/rutil/dns/RROverlay.hxx \
	$(includedir)/rutil/dns/RRFactory.hxx \
	$(includedir)/rutil/dns/ExternalDnsFactory.hxx \
	$(includedir)/rutil/dns/DnsCnameRecord.hxx \
	$(includedir)/rutil/dns/DnsNaptrRecord.hxx \
	$(includedir)/rutil/dns/RRCache.hxx \
	$(includedir)/rutil/dns/DnsHostRecord.hxx \
	$(includedir)/rutil/dns/RRVip.hxx \
	$(includedir)/rutil/dns/ExternalDns.hxx \
	$(includedir)/rutil/dns/DnsStub.hxx \
	$(includedir)/rutil/dns/RRList.hxx \
	$(includedir)/rutil/dns/DnsHandler.hxx \
	$(includedir)/rutil/dns/DnsAAAARecord.hxx \
	$(includedir)/rutil/dns/LocalDns.hxx \
	$(includedir)/rutil/dns/DnsSrvRecord.hxx \
	$(includedir)/rutil/dns/QueryTypes.hxx \
	$(includedir)/rutil/dns/DnsResourceRecord.hxx \
	$(includedir)/rutil/ThreadIf.hxx \
	$(includedir)/rutil/TransportType.hxx \
	$(includedir)/rutil/DnsUtil.hxx \
	$(includedir)/rutil/SHA1Stream.hxx \
	$(includedir)/rutil/CountStream.hxx

RESIP_LIB_FILES_TO_INSTALL = \
	$(libdir)/librutil.a \
	$(libdir)/libares.a \
	$(libdir)/libdum.a \
	$(libdir)/libresip.a

include $(srcdir)/config/sipXprojtop.mak

check: resip-test

# Configure the Resiprocate build system to install into ./build.
@abs_top_srcdir@/resiprocate/build/Makefile.conf:
	@abs_top_srcdir@/resiprocate/configure -y --disable-shared-libs \
		--prefix=@abs_top_builddir@

# rutil also builds contrib/ares
# Use the ARES_PREFIX variable to pass in the Resiprocate install directory,
# as ares has a separate ./configure that needs to know it.
.PHONY: rutil
rutil: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate ARES_PREFIX=@abs_top_builddir@ rutil 

# install-rutil also installs contriib/ares
.PHONY: install-rutil
install-rutil:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-ares
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-rutil

.PHONY: resip
resip: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate resiprocate

.PHONY: resip-test
resip-test:
	$(MAKE) -C @abs_top_srcdir@/resiprocate tests

.PHONY: install-resip
install-resip:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-resip

.PHONY: dum
dum: @abs_top_srcdir@/resiprocate/build/Makefile.conf
	$(MAKE) -C @abs_top_srcdir@/resiprocate dum 

.PHONY: install-dum
install-dum:
	$(MAKE) -C @abs_top_srcdir@/resiprocate install-dum

clean: clean-resiprocate

.PHONY: clean-resiprocate
clean-resiprocate:
	$(MAKE) -C @abs_top_srcdir@/resiprocate clean
	rm -f @abs_top_srcdir@/resiprocate/build/Makefile.conf
