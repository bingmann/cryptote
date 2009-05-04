# $Id$

noinst_LIBRARIES = libstc.a

libstc_a_SOURCES = \
	scintilla/include/Accessor.h \
	scintilla/include/KeyWords.h \
	scintilla/include/Platform.h \
	scintilla/include/PropSet.h \
	scintilla/include/SString.h \
	scintilla/include/SciLexer.h \
	scintilla/include/Scintilla.h \
	scintilla/include/ScintillaWidget.h \
	scintilla/include/WindowAccessor.h \
	scintilla/include/Face.py \
	scintilla/include/HFacer.py \
	scintilla/include/Scintilla.iface \
	\
	scintilla/src/AutoComplete.cxx \
	scintilla/src/AutoComplete.h \
	scintilla/src/CallTip.cxx \
	scintilla/src/CallTip.h \
	scintilla/src/CellBuffer.cxx \
	scintilla/src/CellBuffer.h \
	scintilla/src/CharClassify.cxx \
	scintilla/src/CharClassify.h \
	scintilla/src/CharacterSet.h \
	scintilla/src/ContractionState.cxx \
	scintilla/src/ContractionState.h \
	scintilla/src/Decoration.cxx \
	scintilla/src/Decoration.h \
	scintilla/src/Document.cxx \
	scintilla/src/Document.h \
	scintilla/src/DocumentAccessor.cxx \
	scintilla/src/DocumentAccessor.h \
	scintilla/src/Editor.cxx \
	scintilla/src/Editor.h \
	scintilla/src/ExternalLexer.cxx \
	scintilla/src/ExternalLexer.h \
	scintilla/src/Indicator.cxx \
	scintilla/src/Indicator.h \
	scintilla/src/KeyMap.cxx \
	scintilla/src/KeyMap.h \
	scintilla/src/KeyWords.cxx \
	scintilla/src/LineMarker.cxx \
	scintilla/src/LineMarker.h \
	scintilla/src/Partitioning.h \
	scintilla/src/PerLine.cxx \
	scintilla/src/PerLine.h \
	scintilla/src/PositionCache.cxx \
	scintilla/src/PositionCache.h \
	scintilla/src/PropSet.cxx \
	scintilla/src/RESearch.cxx \
	scintilla/src/RESearch.h \
	scintilla/src/RunStyles.cxx \
	scintilla/src/RunStyles.h \
	scintilla/src/SVector.h \
	scintilla/src/ScintillaBase.cxx \
	scintilla/src/ScintillaBase.h \
	scintilla/src/SplitVector.h \
	scintilla/src/Style.cxx \
	scintilla/src/Style.h \
	scintilla/src/StyleContext.cxx \
	scintilla/src/StyleContext.h \
	scintilla/src/UniConversion.cxx \
	scintilla/src/UniConversion.h \
	scintilla/src/ViewStyle.cxx \
	scintilla/src/ViewStyle.h \
	scintilla/src/WindowAccessor.cxx \
	scintilla/src/XPM.cxx \
	scintilla/src/XPM.h \
	\
	PlatWX.cpp \
	PlatWX.h \
	ScintillaWX.cpp \
	ScintillaWX.h \
	stc.cpp \
	stc.h

EXTRA_DIST = README.txt \
	scintilla/README \
	scintilla/License.txt \
	scintilla/version.txt \
	scintilla/doc/Design.html \
	scintilla/doc/Icons.html \
	scintilla/doc/Lexer.txt \
	scintilla/doc/SciBreak.jpg \
	scintilla/doc/SciCoding.html \
	scintilla/doc/SciRest.jpg \
	scintilla/doc/SciTEIco.png \
	scintilla/doc/SciWord.jpg \
	scintilla/doc/ScintillaDoc.html \
	scintilla/doc/ScintillaDownload.html \
	scintilla/doc/ScintillaHistory.html \
	scintilla/doc/ScintillaRelated.html \
	scintilla/doc/ScintillaToDo.html \
	scintilla/doc/ScintillaUsage.html \
	scintilla/doc/Steps.html \
	scintilla/doc/index.html

AM_CFLAGS = -W -Wall -D__WX__ -DSCI_LEXER -DSCI_NAMESPACE -I$(srcdir)/scintilla/include -I$(srcdir)/scintilla/src $(WX_CFLAGS)
AM_CXXFLAGS = -W -Wall -D__WX__ -DSCI_LEXER -DSCI_NAMESPACE -I$(srcdir)/scintilla/include -I$(srcdir)/scintilla/src $(WX_CXXFLAGS)
