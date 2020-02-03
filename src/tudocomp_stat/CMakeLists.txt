# Disable stat tracking altogether
if(STATS_DISABLED)
    message("[CAUTION] Statistic tracking is disabled!")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSTATS_DISABLED")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTATS_DISABLED")
endif(STATS_DISABLED)

# Disable only malloc counter
if(MALLOC_DISABLED)
    message("[CAUTION] malloc override is disabled, no memory tracking possible!")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMALLOC_DISABLED")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMALLOC_DISABLED")
endif(MALLOC_DISABLED)

add_library(tudocomp_stat STATIC
    malloc.cpp
    StatPhase.cpp
)
