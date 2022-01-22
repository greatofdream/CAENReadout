# https://root.cern.ch/root/htmldoc/guides/users-guide/WritingGUI.html#:~:text=The%20ROOT%20GUI%20classes%20support%20an%20extensive%20and,Win32%2C%20thereby%20making%20the%20ROOT%20GUI%20fully%20cross-platform.
#execute_process(COMMAND root-config --cflags --glibs OUTPUT_VARIABLE rootconfig OUTPUT_STRIP_TRAILING_WHITESPACE)
#MESSAGE(${rootconfig})
# rootcling -f dict.cxx -c main.cxx LinkDef.h
SET(rootconfig -pthread -std=c++11 -m64 -I/home/wangzhe/ROOT/root/include -L/home/wangzhe/ROOT/root/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTDataFrame -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -pthread -lm -ldl -rdynamic)
#add_link_options(${rootconfig})
add_executable(ReadoutGui gui/root/main.cxx gui/root/dict.cxx src/readout.cxx)
target_include_directories(ReadoutGui PUBLIC include gui/root)
target_include_directories(ReadoutGui PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(ReadoutGui PUBLIC ${CAENDigitizer_INCLUE_DIR} ${CAENVME_INCLUDE_DIRS})
target_include_directories(ReadoutGui PUBLIC ${BOOST_INCLUDE_DIRS})

target_link_libraries(ReadoutGui ${rootconfig})

target_link_libraries(ReadoutGui ${ROOT_LIBRARIES})
target_link_libraries(ReadoutGui ${CAENDigitizer_LIBRARY} ${CAENVME_LIBRARY})
install(TARGETS ReadoutGui RUNTIME DESTINATION bin)